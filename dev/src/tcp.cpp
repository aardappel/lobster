// Copyright 2026 Wouter van Oortmerssen. All rights reserved.
// Licensed under the Apache License, Version 2.0 (see LICENSE.txt).

#include "lobster/stdafx.h"
#include "lobster/natreg.h"

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "Ws2_32.lib")
    using SocketHandle = SOCKET;
    using SocketLength = int;
    static constexpr auto invalid_socket = INVALID_SOCKET;
    static int SocketError() { return WSAGetLastError(); }
    static bool WouldBlock(int e) { return e == WSAEWOULDBLOCK || e == WSAEINPROGRESS || e == WSAEINTR; }
    static void CloseSocket(SocketHandle s) { closesocket(s); }
    struct SocketSystem {
        int error;
        SocketSystem() { WSADATA data; error = WSAStartup(MAKEWORD(2, 2), &data); }
        ~SocketSystem() { if (!error) WSACleanup(); }
    };
#else
    #include <sys/socket.h>
    #include <sys/select.h>
    #include <netinet/in.h>
    #include <netinet/tcp.h>
    #include <arpa/inet.h>
    #include <fcntl.h>
    #include <unistd.h>
    #include <errno.h>
    using SocketHandle = int;
    using SocketLength = socklen_t;
    static constexpr auto invalid_socket = -1;
    static int SocketError() { return errno; }
    static bool WouldBlock(int e) { return e == EAGAIN || e == EWOULDBLOCK || e == EINPROGRESS || e == EINTR; }
    static void CloseSocket(SocketHandle s) { close(s); }
    struct SocketSystem { int error = 0; };
#endif

namespace lobster {

static ResourceType tcplistener_type = { "tcplistener" };
static ResourceType tcpsocket_type = { "tcpsocket" };

struct TCPSocket : Resource {
    SocketHandle handle = invalid_socket;
    bool connecting = false;
    int connect_error = 0;
    ~TCPSocket() override { if (handle != invalid_socket) CloseSocket(handle); }
};

static LString *TCPError(VM &vm, const char *operation, int error) {
    return vm.NewString(cat("tcp.", operation, ": socket error ", error));
}

static int ConfigureSocket(SocketHandle s) {
    #ifdef _WIN32
        u_long mode = 1;
        if (ioctlsocket(s, FIONBIO, &mode)) return SocketError();
    #else
        auto flags = fcntl(s, F_GETFL, 0);
        if (flags < 0 || fcntl(s, F_SETFL, flags | O_NONBLOCK) < 0 ||
            fcntl(s, F_SETFD, FD_CLOEXEC) < 0) return SocketError();
    #endif
    int on = 1;
    if (setsockopt(s, IPPROTO_TCP, TCP_NODELAY, (const char *)&on, sizeof(on))) return SocketError();
    #ifdef SO_NOSIGPIPE
        if (setsockopt(s, SOL_SOCKET, SO_NOSIGPIPE, (const char *)&on, sizeof(on))) return SocketError();
    #endif
    return 0;
}

static LString *OpenSocket(VM &vm, LResource **out, LString *ip, iint port, bool listener) {
    *out = nullptr;
    static SocketSystem system;
    if (system.error) return TCPError(vm, "init", system.error);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    if (port < (listener ? 0 : 1) || port > 65535 ||
        ip->strv().find('\0') != string_view::npos ||
        inet_pton(AF_INET, ip->data(), &addr.sin_addr) != 1)
        return vm.NewString("tcp: expected a numeric IPv4 address and a valid port");
    addr.sin_port = htons((uint16_t)port);
    auto socket = make_unique<TCPSocket>();
    socket->handle = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket->handle == invalid_socket) return TCPError(vm, "open", SocketError());
    if (auto err = ConfigureSocket(socket->handle)) return TCPError(vm, "configure", err);
    if (listener) {
        int on = 1;
        #ifdef _WIN32
            const auto reuse_option = SO_EXCLUSIVEADDRUSE;
        #else
            const auto reuse_option = SO_REUSEADDR;
        #endif
        if (setsockopt(socket->handle, SOL_SOCKET, reuse_option, (const char *)&on, sizeof(on)))
            return TCPError(vm, "listen", SocketError());
        if (::bind(socket->handle, (sockaddr *)&addr, sizeof(addr)) || ::listen(socket->handle, 8))
            return TCPError(vm, "listen", SocketError());
    } else if (::connect(socket->handle, (sockaddr *)&addr, sizeof(addr))) {
        auto err = SocketError();
        if (!WouldBlock(err)) return TCPError(vm, "connect", err);
        socket->connecting = true;
    }
    *out = vm.NewResource(listener ? &tcplistener_type : &tcpsocket_type, socket.release());
    return nullptr;
}

// SO_ERROR must only be tested after readiness, and remembered: reading it clears it.
static int ConnectionStatus(TCPSocket &s) {
    if (s.connect_error) return -1;
    if (!s.connecting) return 1;
    fd_set writable, failed;
    FD_ZERO(&writable);
    FD_ZERO(&failed);
    #ifndef _WIN32
        if (s.handle >= FD_SETSIZE) { s.connect_error = EMFILE; return -1; }
    #endif
    FD_SET(s.handle, &writable);
    FD_SET(s.handle, &failed);
    timeval timeout{};
    auto ready = select((int)s.handle + 1, nullptr, &writable, &failed, &timeout);
    if (ready < 0) {
        auto err = SocketError();
        if (WouldBlock(err)) return 0;
        s.connect_error = err;
        return -1;
    }
    if (!ready) return 0;
    SocketLength len = sizeof(s.connect_error);
    if (getsockopt(s.handle, SOL_SOCKET, SO_ERROR, (char *)&s.connect_error, &len))
        s.connect_error = SocketError();
    if (s.connect_error) return -1;
    s.connecting = false;
    return 1;
}

BuiltinGroup tcp_builtins;
#define BUILTIN_GROUP tcp_builtins
#define BUILTIN_SYM(name) builtin_tcp_##name

BUILTIN(listen, "ip,port", "SI", "R:tcplistener?S?",
    "open a nonblocking IPv4 TCP listener; use 127.0.0.1 for local access. Port 0 selects a free port."
    " Returns listener, error (nil on success). Resources close their sockets when destroyed.")
(VM &vm, LResource **out, LString *ip, iint port) {
    return OpenSocket(vm, out, ip, port, true);
}

BUILTIN(connect, "ip,port", "SI", "R:tcpsocket?S?",
    "begin a nonblocking connection to a numeric IPv4 address. Returns socket, error."
    " Poll connected for completion; no DNS lookup or blocking wait is performed.")
(VM &vm, LResource **out, LString *ip, iint port) {
    return OpenSocket(vm, out, ip, port, false);
}

BUILTIN(accept, "listener", "R:tcplistener", "R:tcpsocket?S?",
    "accept one connection without blocking. Returns socket, error; nil, nil means none waiting.")
(VM &vm, LResource **out, LResource *listener) {
    *out = nullptr;
    auto &s = GetResourceDec<TCPSocket>(listener, &tcplistener_type);
    auto socket = make_unique<TCPSocket>();
    socket->handle = ::accept(s.handle, nullptr, nullptr);
    if (socket->handle == invalid_socket) {
        auto err = SocketError();
        return WouldBlock(err) ? nullptr : TCPError(vm, "accept", err);
    }
    if (auto err = ConfigureSocket(socket->handle)) return TCPError(vm, "configure", err);
    *out = vm.NewResource(&tcpsocket_type, socket.release());
    return nullptr;
}

BUILTIN(port, "listener", "R:tcplistener", "IS?",
    "returns the bound local port and an optional error (useful after listen with port 0).")
(VM &vm, iint *port, LResource *listener) {
    *port = 0;
    auto &s = GetResourceDec<TCPSocket>(listener, &tcplistener_type);
    sockaddr_in addr{};
    SocketLength len = sizeof(addr);
    if (getsockname(s.handle, (sockaddr *)&addr, &len)) return TCPError(vm, "port", SocketError());
    *port = ntohs(addr.sin_port);
    return nullptr;
}

BUILTIN(connected, "socket", "R:tcpsocket", "BS?",
    "poll connection establishment. Returns false, nil while pending; true, nil once established;"
    " false, error on failure. Established sockets report remote closure through receive, not this call.")
(VM &vm, iint *connected, LResource *socket) {
    auto &s = GetResourceDec<TCPSocket>(socket, &tcpsocket_type);
    auto status = ConnectionStatus(s);
    *connected = status == 1;
    return status < 0 ? TCPError(vm, "connect", s.connect_error) : nullptr;
}

BUILTIN(send, "socket,data,offset", "R:tcpsocketSI?", "IS?",
    "send bytes without blocking. Returns byte count, error; 0, nil means pending/would block (or empty data)."
    " Partial writes are normal: retain unsent bytes and retry with the advanced offset. No internal queue.")
(VM &vm, iint *sent, LResource *socket, LString *data, iint offset) {
    *sent = 0;
    if (offset < 0 || offset > data->len) return vm.NewString("tcp.send: offset out of bounds");
    auto &s = GetResourceDec<TCPSocket>(socket, &tcpsocket_type);
    auto status = ConnectionStatus(s);
    if (status < 0) return TCPError(vm, "connect", s.connect_error);
    if (!status || offset == data->len) return (LString *)nullptr;
    int flags = 0;
    #ifdef MSG_NOSIGNAL
        flags = MSG_NOSIGNAL;
    #endif
    auto n = ::send(s.handle, data->data() + offset, (int)min<iint>(data->len - offset, INT_MAX), flags);
    if (n < 0) {
        auto err = SocketError();
        return WouldBlock(err) ? nullptr : TCPError(vm, "send", err);
    }
    *sent = n;
    return (LString *)nullptr;
}

BUILTIN(receive, "socket,max_bytes", "R:tcpsocketI", "S?S?",
    "receive up to max_bytes (1..1048576), without blocking. Returns data, error."
    " nil, nil means pending/would block; empty string, nil means EOF. TCP has no message boundaries.")
(VM &vm, LString **data, LResource *socket, iint max_bytes) {
    *data = nullptr;
    if (max_bytes < 1 || max_bytes > 1048576) return vm.NewString("tcp.receive: max_bytes must be 1..1048576");
    auto &s = GetResourceDec<TCPSocket>(socket, &tcpsocket_type);
    auto status = ConnectionStatus(s);
    if (status < 0) return TCPError(vm, "connect", s.connect_error);
    if (!status) return (LString *)nullptr;
    // Poll before allocating a receive buffer: idle callers normally have nothing to read.
    char first;
    auto peek = ::recv(s.handle, &first, 1, MSG_PEEK);
    if (peek < 0) {
        auto err = SocketError();
        return WouldBlock(err) ? nullptr : TCPError(vm, "receive", err);
    }
    if (!peek) { *data = vm.NewString(0); return (LString *)nullptr; }
    string buf((size_t)max_bytes, '\0');
    auto n = ::recv(s.handle, buf.data(), (int)max_bytes, 0);
    if (n < 0) {
        auto err = SocketError();
        return WouldBlock(err) ? nullptr : TCPError(vm, "receive", err);
    }
    *data = vm.NewString(string_view(buf.data(), (size_t)n));
    return (LString *)nullptr;
}

BUILTIN(wait, "listener,socket,writable,seconds", "R:tcplistener?R:tcpsocket?BF", "B",
    "the one call here that blocks, for a loop with nothing to do until its peer acts: sleeps for at"
    " most seconds (0..1), or until the listener has a connection to accept, or the socket has data"
    " to receive, was closed, or (with writable) can be sent to. Either may be nil. Returns whether"
    " to try those calls now, rather than whether they will succeed.")
(VM &vm, LResource *listener, LResource *socket, iint writable, double seconds) {
    if (!(seconds >= 0.0 && seconds <= 1.0)) vm.BuiltinError("tcp.wait: seconds must be 0..1");
    fd_set reading, writing;
    FD_ZERO(&reading);
    FD_ZERO(&writing);
    int highest = -1;
    auto watch = [&](SocketHandle handle, fd_set &set) {
        #ifndef _WIN32
            if (handle >= FD_SETSIZE) return false;
        #endif
        FD_SET(handle, &set);
        highest = max(highest, (int)handle);
        return true;
    };
    if (listener && !watch(GetResourceDec<TCPSocket>(listener, &tcplistener_type).handle, reading)) return true;
    if (socket) {
        auto &s = GetResourceDec<TCPSocket>(socket, &tcpsocket_type);
        // A connection that is still being made reports its outcome by becoming writable.
        if (!watch(s.handle, reading) || ((writable || s.connecting) && !watch(s.handle, writing))) return true;
    }
    if (highest < 0) {
        // Windows rejects a select without sockets instead of sleeping.
        std::this_thread::sleep_for(std::chrono::duration<double>(seconds));
        return false;
    }
    timeval timeout;
    timeout.tv_sec = (long)seconds;
    timeout.tv_usec = (long)((seconds - (double)timeout.tv_sec) * 1000000.0);
    // An error counts as ready too: the call that is tried next reports it.
    return select(highest + 1, &reading, &writing, nullptr, &timeout) != 0;
}

}  // namespace lobster
