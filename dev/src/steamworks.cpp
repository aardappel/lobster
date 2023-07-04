// Copyright 2017 Wouter van Oortmerssen. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "lobster/stdafx.h"

#include "lobster/natreg.h"

#include "lobster/glinterface.h"
#include "lobster/sdlinterface.h"

#ifdef PLATFORM_STEAMWORKS

#include "steam/steam_api.h"

struct SteamPeer {
    SteamNetworkingIdentity identity{};
    HSteamNetConnection connection = k_HSteamNetConnection_Invalid;
    bool is_listen_connection = false;
    bool is_connected = false;
};

struct SteamState {
    HSteamListenSocket listen_socket = k_HSteamListenSocket_Invalid;
    HSteamNetPollGroup poll_group = k_HSteamNetPollGroup_Invalid;
    vector<SteamPeer> peers;
    bool steamoverlayactive = false;

    ~SteamState() {
        for (auto &peer: peers) {
            // Don't close connections that were connected via the listen
            // socket, that seems to crash Steam.
            if (peer.connection != k_HSteamNetConnection_Invalid && !peer.is_listen_connection) {
                char ident[SteamNetworkingIdentity::k_cchMaxString]{};
                peer.identity.ToString(ident, sizeof(ident));
                LOG_INFO("closing connection to \"", ident, "\" on SteamState destroy");
                auto ok = SteamNetworkingSockets()->CloseConnection(
                    peer.connection, k_ESteamNetConnectionEnd_App_Generic, nullptr, false);
                if (!ok) {
                    LOG_INFO("  closing the connection failed");
                }
            }
        }
        // Make sure to close connections before closing the listen socket, that
        // seems to crash Steam.
        if (listen_socket != k_HSteamListenSocket_Invalid) {
            LOG_INFO("closing listen socket on SteamState destroy");
            auto ok = SteamNetworkingSockets()->CloseListenSocket(listen_socket);
            if (!ok) LOG_INFO("  closing listen socket failed");
        }
        if (poll_group != k_HSteamNetPollGroup_Invalid) {
            LOG_INFO("destroying poll group on SteamState destroy");
            auto ok = SteamNetworkingSockets()->DestroyPollGroup(poll_group);
            if (!ok) LOG_INFO("  destroying poll group failed");
        }
    }

    STEAM_CALLBACK(SteamState, OnGameOverlayActivated, GameOverlayActivated_t);
    STEAM_CALLBACK(SteamState, OnScreenshotRequested, ScreenshotRequested_t);
	STEAM_CALLBACK(SteamState, OnNetConnectionStatusChanged, SteamNetConnectionStatusChangedCallback_t);

    auto FindPeer(const CSteamID &id) {
        return find_if(peers.begin(), peers.end(), [&](const auto& peer) {
            return peer.identity.GetSteamID() == id;
        });
    }

    auto FindPeer(HSteamNetConnection conn) {
        return find_if(peers.begin(), peers.end(), [&](const auto& peer) {
            return peer.connection == conn;
        });
    }

    bool P2PListen() {
        if (listen_socket != k_HSteamListenSocket_Invalid) {
            LOG_INFO("listen socket is already open");
            return false;
        }
        auto socket = SteamNetworkingSockets()->CreateListenSocketP2P(1, 0, nullptr);
        listen_socket = socket;
        LOG_INFO("created ListenSocket");
        return socket != k_HSteamListenSocket_Invalid;
    }

    bool P2PConnect(string_view_nt str_identity) {
        SteamNetworkingIdentity identity{};
        identity.ParseString(str_identity.c_str());
        auto connection = SteamNetworkingSockets()->ConnectP2P(identity, 1, 0, nullptr);
        LOG_INFO("opened connection");
        return connection != k_HSteamNetConnection_Invalid;
    }

    bool CloseListen() {
        if (listen_socket == k_HSteamListenSocket_Invalid) {
            LOG_INFO("listen socket is already closed");
            return true;
        }
        auto result = SteamNetworkingSockets()->CloseListenSocket(listen_socket);
        LOG_INFO("closed ListenSocket");
        listen_socket = k_HSteamListenSocket_Invalid;
        return result;
    }

    bool SendMessage(string_view_nt str_identity, string_view buf) {
        SteamNetworkingIdentity identity{};
        identity.ParseString(str_identity.c_str());
        auto peer = FindPeer(identity.GetSteamID());
        if (peer == peers.end()) return false;
        if (!peer->is_connected) return false;

        const char *data = buf.data();
        uint32_t size = (uint32_t)buf.size();
        int flags = k_nSteamNetworkingSend_Reliable;
        auto result = SteamNetworkingSockets()->SendMessageToConnection(peer->connection, data, size, flags, nullptr);
        return result != k_EResultOK;
    }

    bool BroadcastMessage(string_view buf) {
        const char *data = buf.data();
        uint32_t size = (uint32_t)buf.size();
        int flags = k_nSteamNetworkingSend_Reliable;
        bool ok = true;
        // TODO: We should be able to use SendMessages here so we don't need to
        // make multiple copies of the buffer. The message struct itself is
        // ref-counted, but also contains the destination connection, so
        // it can't be shared. We'd need to have the data itself be ref-counted
        // too.
        for (auto &peer: peers) {
            if (!peer.is_connected) continue;
            auto result = SteamNetworkingSockets()->SendMessageToConnection(peer.connection, data, size, flags, nullptr);
            ok &= result == k_EResultOK;
        }
        return ok;
    }

    vector<SteamNetworkingMessage_t*> ReceiveMessages() {
        if (poll_group == k_HSteamNetPollGroup_Invalid) return {};

        vector<SteamNetworkingMessage_t*> messages;
        while (true) {
            constexpr int max_messages = 32;
            SteamNetworkingMessage_t *message_buf[max_messages];
            int res = SteamNetworkingSockets()->ReceiveMessagesOnPollGroup(poll_group, message_buf, max_messages);
            // TODO: better handling of invalid connection?
            if (res <= 0) break;
            for (int i = 0; i < res; ++i) {
                messages.push_back(message_buf[i]);
            }
        }
        return messages;
    }
};

void SteamState::OnGameOverlayActivated(GameOverlayActivated_t *callback) {
    steamoverlayactive = callback->m_bActive;
    LOG_INFO("steam overlay toggle: ", steamoverlayactive);
}

void SteamState::OnScreenshotRequested(ScreenshotRequested_t *) {
    LOG_INFO("steam screenshot requested");
    auto size = GetScreenSize();
    auto pixels = ReadPixels(int2(0), size);
    SteamScreenshots()->WriteScreenshot(pixels, size.x * size.y * 3, size.x, size.y);
    delete[] pixels;
}

void SteamState::OnNetConnectionStatusChanged(SteamNetConnectionStatusChangedCallback_t *callback) {
	HSteamNetConnection conn = callback->m_hConn;
	SteamNetConnectionInfo_t info = callback->m_info;
	ESteamNetworkingConnectionState old_state = callback->m_eOldState;
	ESteamNetworkingConnectionState new_state = info.m_eState;

    char ident[SteamNetworkingIdentity::k_cchMaxString]{};
    info.m_identityRemote.ToString(ident, sizeof(ident));
    auto get_state_name = [](auto state) -> string {
        if (state >= 0 && state <= 5) {
            static const char* state_names[] = {
                "none",      "connecting",     "finding route",
                "connected", "closed by peer", "problem detected locally",
            };
            return state_names[state];
        }
        return to_string(state);
    };
    LOG_INFO("steam net connection status changed: ident=", ident, " old state='",
             get_state_name(old_state), "' new state='", get_state_name(new_state), "'");

	if (info.m_hListenSocket != k_HSteamListenSocket_Invalid &&
		old_state == k_ESteamNetworkingConnectionState_None &&
		new_state == k_ESteamNetworkingConnectionState_Connecting) {
		// Connection from a peer on the listen socket, make sure they're not in there already
        auto peer = FindPeer(info.m_identityRemote.GetSteamID());
        if (peer != peers.end()) {
            // For now at least, only allow one connection to a peer with a given steam ID.
            LOG_INFO("Peer \"", ident, "\" connecting, but already in list?");
            SteamNetworkingSockets()->CloseConnection(conn, k_ESteamNetConnectionEnd_AppException_Generic, "Already connected", false );
        } else {
            peers.emplace_back();
            peer = peers.end() - 1;
            EResult res = SteamNetworkingSockets()->AcceptConnection(conn);
            if (res != k_EResultOK) {
                LOG_INFO("AcceptConnection failed with error=", res);
                peers.pop_back();
                SteamNetworkingSockets()->CloseConnection(conn, k_ESteamNetConnectionEnd_AppException_Generic, "Failed to accept connection", false );
                return;
            }

            peer->connection = conn;
            peer->identity = info.m_identityRemote;
            peer->is_connected = false;
            peer->is_listen_connection = true;
            LOG_INFO("Connecting peer \"", ident, "\"");
            // TODO: start authentication here, if needed
        }
    } else if (new_state == k_ESteamNetworkingConnectionState_Connected) {
        auto peer = FindPeer(conn);
        if (peer == peers.end() || !peer->is_connected) {
            if (peer == peers.end()) {
                peers.emplace_back();
                peer = peers.end() - 1;
            }
            peer->connection = conn;
            peer->identity = info.m_identityRemote;
            peer->is_connected = true;
            peer->is_listen_connection = info.m_hListenSocket != k_HSteamListenSocket_Invalid;
            if (poll_group == k_HSteamNetPollGroup_Invalid) {
                LOG_INFO("Creating poll group");
                poll_group = SteamNetworkingSockets()->CreatePollGroup();
            }
            SteamNetworkingSockets()->SetConnectionPollGroup(conn, poll_group);
            LOG_INFO("Connected peer \"", ident, "\"");
        }
	} else if (new_state == k_ESteamNetworkingConnectionState_ClosedByPeer ||
               new_state == k_ESteamNetworkingConnectionState_ProblemDetectedLocally) {
		// Handle disconnecting a peer
        auto peer = FindPeer(conn);
        if (peer != peers.end()) {
            auto ok = SteamNetworkingSockets()->CloseConnection(
                peer->connection, k_ESteamNetConnectionEnd_App_Generic, nullptr, false);
            peers.erase(peer);
            LOG_INFO("Disconnected from peer \"", ident, "\" msg=\"", info.m_szEndDebug, "\" close connection ok?=", ok);
        } else {
            LOG_INFO("Peer \"", ident, "\" disconnecting, but not in list?  msg=\"", info.m_szEndDebug, "\"");
        }
	}
}

extern "C" void __cdecl SteamAPIDebugTextHook(int severity, const char *debugtext) {
    if (severity) LOG_WARN(debugtext) else LOG_INFO(debugtext);
}

SteamState *steam = nullptr;

#endif  // PLATFORM_STEAMWORKS

void SteamShutDown() {
    #ifdef PLATFORM_STEAMWORKS
        if (steam) {
            delete steam;
            SteamAPI_Shutdown();
        }
        steam = nullptr;
    #endif  // PLATFORM_STEAMWORKS
}

int SteamInit(iint appid, bool screenshots, bool initrelay) {
    SteamShutDown();
    #ifdef PLATFORM_STEAMWORKS
        #ifndef NDEBUG
            (void)appid;
        #else
            if (appid && SteamAPI_RestartAppIfNecessary((uint32_t)appid)) {
                LOG_INFO("Not started from Steam");
                return -1;
            }
        #endif
        bool steaminit = SteamAPI_Init();
        LOG_INFO("Steam init: ", steaminit);
        if (!steaminit) return 0;
        SteamUtils()->SetWarningMessageHook(&SteamAPIDebugTextHook);
        steam = new SteamState();
        SteamUserStats()->RequestCurrentStats();
        if (screenshots) SteamScreenshots()->HookScreenshots(true);
        if (initrelay) SteamNetworkingUtils()->InitRelayNetworkAccess();
        return 1;
    #else
        return 0;
    #endif  // PLATFORM_STEAMWORKS
}

void SteamUpdate() {
    #ifdef PLATFORM_STEAMWORKS
        if (steam) SteamAPI_RunCallbacks();
    #endif  // PLATFORM_STEAMWORKS
}

const char *UserName() {
    #ifdef PLATFORM_STEAMWORKS
        if (steam) return SteamFriends()->GetPersonaName();
    #endif  // PLATFORM_STEAMWORKS
    return "";
}

bool UnlockAchievement(string_view_nt name) {
    #ifdef PLATFORM_STEAMWORKS
        if (steam) {
            auto ok = SteamUserStats()->SetAchievement(name.c_str());
            return SteamUserStats()->StoreStats() && ok;  // Force this to run.
        }
    #else
        (void)name;
    #endif  // PLATFORM_STEAMWORKS
    return false;
}

int SteamReadFile(string_view_nt fn, string &buf) {
    #ifdef PLATFORM_STEAMWORKS
        if (steam) {
            auto len = SteamRemoteStorage()->GetFileSize(fn.c_str());
            if (len) {
                buf.resize(len);
                return SteamRemoteStorage()->FileRead(fn.c_str(), (void *)buf.data(), len);
            }
        }
    #endif  // PLATFORM_STEAMWORKS
    return 0;
}

bool SteamWriteFile(string_view_nt fn, string_view buf) {
    #ifdef PLATFORM_STEAMWORKS
        if (steam) {
        return SteamRemoteStorage()->FileWrite(fn.c_str(), buf.data(), (int)buf.size());
        }
    #endif  // PLATFORM_STEAMWORKS
    return false;
}

bool OverlayActive() {
    #ifdef PLATFORM_STEAMWORKS
        return steam && steam->steamoverlayactive;
    #endif  // PLATFORM_STEAMWORKS
    return false;
}

bool SteamP2PListen() {
    #ifdef PLATFORM_STEAMWORKS
        if (steam) {
            return steam->P2PListen();
        }
    #endif  // PLATFORM_STEAMWORKS
    return false;
}

bool SteamCloseListen() {
    #ifdef PLATFORM_STEAMWORKS
        if (steam) {
            return steam->CloseListen();
        }
    #endif  // PLATFORM_STEAMWORKS
    return false;
}

bool SteamP2PConnect(string_view_nt identity) {
    #ifdef PLATFORM_STEAMWORKS
        if (steam) {
            return steam->P2PConnect(identity);
        }
    #endif  // PLATFORM_STEAMWORKS
    return false;
}

bool SteamSendMessage(string_view_nt dest_identity, string_view buf) {
    #ifdef PLATFORM_STEAMWORKS
        if (steam) {
            return steam->SendMessage(dest_identity, buf);
        }
    #endif  // PLATFORM_STEAMWORKS
    return false;
}

bool SteamBroadcastMessage(string_view buf) {
    #ifdef PLATFORM_STEAMWORKS
        if (steam) {
            return steam->BroadcastMessage(buf);
        }
    #endif  // PLATFORM_STEAMWORKS
    return false;
}


using namespace lobster;

LString* GetIdentityString(VM &vm, const SteamNetworkingIdentity &identity) {
    char ident[SteamNetworkingIdentity::k_cchMaxString];
    identity.ToString(ident, sizeof(ident));
    return vm.NewString(ident);
}

void AddSteam(NativeRegistry &nfr) {

nfr("steam_init", "appid,allowscreenshots,initrelay", "IBB", "I",
    "initializes SteamWorks. returns 1 if succesful, 0 on failure. Specify a non-0 appid if you"
    " want to restart from steam if this wasn't started from steam (the return value in this"
    " case will be -1 to indicate you should terminate this instance). If you don't specify an"
    " appid here or in steam_appid.txt, init will likely fail. The other functions can still be"
    " called even if steam isn't active."
    " allowscreenshots automatically uploads screenshots to steam (triggered by steam)."
    " initrelay initializes the relay network for p2p early, to save time when it is first used.",
    [](StackPtr &, VM &, Value &appid, Value &ss, Value &relay) {
        return Value(SteamInit(appid.ival(), ss.True(), relay.True()));
    });

nfr("steam_shutdown", "", "", "",
    "", [](StackPtr &, VM &) { SteamShutDown(); });

nfr("steam_overlay", "", "", "B",
    "returns true if the steam overlay is currently on (you may want to auto-pause if so)",
    [](StackPtr &, VM &) {
        return Value(OverlayActive());
    });

nfr("steam_username", "", "", "S",
    "returns the name of the steam user, or empty string if not available.",
    [](StackPtr &, VM &vm) {
        return Value(vm.NewString(UserName()));
    });

nfr("steam_unlock_achievement", "achievementname", "S", "B",
    "Unlocks an achievement and shows the achievement overlay if not already achieved before."
    " Will also Q-up saving achievement to Steam."
    " Returns true if succesful.",
    [](StackPtr &, VM &, Value &name) {
        auto ok = UnlockAchievement(name.sval()->strvnt());
        return Value(ok);
    });

nfr("steam_write_file", "file,contents", "SS", "B",
    "writes a file with the contents of a string to the steam cloud, or local storage if that"
    " fails, returns false if writing wasn't possible at all",
    [](StackPtr &, VM &, Value &file, Value &contents) {
        auto fn = file.sval()->strvnt();
        auto s = contents.sval();
        auto ok = SteamWriteFile(fn, s->strv());
        if (!ok) {
            ok = WriteFile(fn.sv, true, s->strv(), false);
        }
        return Value(ok);
    });

nfr("steam_read_file", "file", "S", "S?",
    "returns the contents of a file as a string from the steam cloud if available, or otherwise"
    " from local storage, or nil if the file can't be found at all.",
    [](StackPtr &, VM &vm, Value &file) {
        auto fn = file.sval()->strvnt();
        string buf;
        auto len = SteamReadFile(fn, buf);
        if (!len) len = (int)LoadFile(fn.sv, &buf);
        if (len < 0) return NilVal();
        auto s = vm.NewString(buf);
        return Value(s);
    });

nfr("steam_update", "", "", "",
    "you must call this function in your game loop when using most steam APIs",
    [](StackPtr &, VM &) {
        SteamUpdate();
    });

nfr("steam_net_identity", "", "", "S", "returns the steam identity for this"
    " user. This same ID will be used for connecting to peers, sending messages,"
    " etc.",
    [](StackPtr &sp, VM &vm) {
        SteamNetworkingIdentity identity{};
        SteamNetworkingSockets()->GetIdentity(&identity);
        Push(sp, GetIdentityString(vm, identity));
    });

nfr("steam_p2p_listen", "", "", "B", "open a listen socket to receive new connections",
    [](StackPtr &, VM &) {
        auto ok = SteamP2PListen();
        return Value(ok);
    });

nfr("steam_p2p_close_listen", "", "", "B", "close the listen socket and stop accepting new connections",
    [](StackPtr &, VM &) {
        auto ok = SteamCloseListen();
        return Value(ok);
    });

nfr("steam_p2p_connect", "ident", "S", "B", "connect to a user with a given steam identity that has opened a listen socket",
    [](StackPtr &, VM &, Value &ident) {
        auto ok = SteamP2PConnect(ident.sval()->strvnt());
        return Value(ok);
    });

nfr("steam_p2p_get_connections", "", "", "S]", "get a list of the steam identites that are currently connected",
    [](StackPtr &, VM &vm) {
        auto *peers_vec = vm.NewVec(0, 0, TYPE_ELEM_VECTOR_OF_STRING);

    #ifdef PLATFORM_STEAMWORKS
        if (steam) {
            for (auto &peer: steam->peers) {
                if (!peer.is_connected) continue;
                peers_vec->Push(vm, GetIdentityString(vm, peer.identity));
            }
        }
    #endif  // PLATFORM_STEAMWORKS

        return Value(peers_vec);
    });

nfr("steam_p2p_send_message", "ident,data", "SS", "B", "send a reliable message to a given steam identity",
    [](StackPtr &, VM &, Value &ident, Value &data) {
        auto ok = SteamSendMessage(ident.sval()->strvnt(), data.sval()->strv());
        return Value(ok);
    });

nfr("steam_p2p_broadcast_message", "data", "S", "B", "send a reliable message to all connected peers",
    [](StackPtr &, VM &, Value &data) {
        auto ok = SteamBroadcastMessage(data.sval()->strv());
        return Value(ok);
    });

nfr("steam_p2p_receive_messages", "", "", "S]S]", "receive messages from all"
    " connected peers. The first return value is an array of messages, the second"
    " return value is an array of the steam identities that sent each message",
    [](StackPtr &sp, VM &vm) {
        auto *client_vec = vm.NewVec(0, 0, TYPE_ELEM_VECTOR_OF_STRING);
        auto *data_vec = vm.NewVec(0, 0, TYPE_ELEM_VECTOR_OF_STRING);

        #ifdef PLATFORM_STEAMWORKS
            if (steam) {
                auto messages = steam->ReceiveMessages();
                for (auto *message: messages) {
                    auto *data = vm.NewString(string_view((char*)message->m_pData, message->m_cbSize));
                    client_vec->Push(vm, GetIdentityString(vm, message->m_identityPeer));
                    data_vec->Push(vm, data);
                    message->Release();
                }
            }
        #endif  // PLATFORM_STEAMWORKS

        Push(sp, data_vec);
        Push(sp, client_vec);
    });

}  // AddSteam