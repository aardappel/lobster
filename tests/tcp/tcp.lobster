// Run without a window: lobster tests/tcp/tcp.lobster
// Exercises both endpoints, binary data, partial I/O, EOF/resource destruction and errors.
import std

let listener_opt, listen_error = tcp.listen("127.0.0.1", 0)
assert not listen_error
let listener = assert listener_opt
let port, port_error = tcp.port(listener)
assert not port_error and port > 0
let duplicate, duplicate_error = tcp.listen("127.0.0.1", port)
assert not duplicate and duplicate_error
let invalid, invalid_error = tcp.connect("not an IP", port)
assert not invalid and invalid_error
let none, accept_error = tcp.accept(listener)
assert not none and not accept_error

def exchange(server_listener, server_port):
    var socket, connect_error = tcp.connect("127.0.0.1", server_port)
    assert not connect_error
    assert socket
    var peer:resource<tcpsocket>? = nil
    let deadline = seconds_elapsed() + 10.0
    var connected = false
    while not peer or not connected:
        assert seconds_elapsed() < deadline
        let ready, err = tcp.connected(socket)
        assert not err
        connected = ready
        if not peer:
            let accepted, accept_err = tcp.accept(server_listener)
            assert not accept_err
            peer = accepted
    let server = assert peer
    let empty, recv_error = tcp.receive(server, 1024)
    assert not recv_error and empty == nil
    let bad_count, bad_send = tcp.send(socket, "test", 5)
    assert not bad_count and bad_send
    let bad_data, bad_receive = tcp.receive(server, 0)
    assert not bad_data and bad_receive
    let payload = repeat_string("binary\x00\xFFdata", 200000)
    var sent = 0
    var received = ""
    while sent < payload.length or received.length < payload.length:
        assert seconds_elapsed() < deadline
        let n, err = tcp.send(socket, payload, sent)
        assert not err
        sent += n
        let data, rerr = tcp.receive(server, 65536)
        assert not rerr
        if data: received += data
    assert received == payload
    let n, err = tcp.send(server, "reply")
    assert not err and n == 5
    var reply = ""
    while reply.length < 5:
        assert seconds_elapsed() < deadline
        let data, reply_err = tcp.receive(socket, 1024)
        assert not reply_err
        if data: reply += data
    assert reply == "reply"
    // Inlining can extend local lifetimes, so explicitly release the last reference.
    socket = nil
    return server

let peer = exchange(listener, port)
let deadline = seconds_elapsed() + 10.0
var eof = false
while not eof:
    assert seconds_elapsed() < deadline
    let data, err = tcp.receive(peer, 1024)
    assert not err
    eof = data != nil and data.length == 0

def closed_port():
    var socket, err = tcp.listen("127.0.0.1", 0)
    assert not err
    let freed_port, err2 = tcp.port(assert socket)
    assert not err2
    socket = nil
    return freed_port

let refused, connect_error = tcp.connect("127.0.0.1", closed_port())
if refused:
    var failed = false
    while not failed:
        assert seconds_elapsed() < deadline
        let connected, err = tcp.connected(refused)
        assert not connected
        failed = err != nil
    let connected, err = tcp.connected(refused)
    assert not connected and err  // Polling must not clear the failure.
else:
    assert connect_error
print "TCP test passed"
