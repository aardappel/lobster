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

struct QueuedMessage {
    SteamNetworkingIdentity identity{};
    string message;
};

struct SteamPeer {
    string ident;
    SteamNetworkingIdentity net_identity;
    HSteamNetConnection connection = k_HSteamNetConnection_Invalid;
    bool is_listen_connection = false;
    bool is_connected = false;
    vector<QueuedMessage> queued_messages;
};

struct SteamState {
    HSteamListenSocket listen_socket = k_HSteamListenSocket_Invalid;
    HSteamNetPollGroup poll_group = k_HSteamNetPollGroup_Invalid;
    vector<SteamPeer> peers;
    bool steamoverlayactive = false;
    // Lobby Data
    CSteamID created_lobby;           // The most recently created lobby.
    CSteamID joined_lobby;            // The most recently joined lobby.
    vector<CSteamID> joined_lobbies;  // A list of all joined lobbies.
    int matched_lobbies = 0;          // -1: in progress. 0: not requested, or no matches. > 0: number of matches
    // Needs to be static because the callback function doesn't have user-data.
    static mutex debug_output_mutex;
    static vector<string> debug_output;

    ~SteamState() {
        // Leave all lobbies.
        for (const auto &lobby_id : joined_lobbies) {
            SteamMatchmaking()->LeaveLobby(lobby_id);
        }
        SteamAPI_RunCallbacks();
        for (auto &peer: peers) {
            // Don't close connections that were connected via the listen
            // socket, that seems to crash Steam.
            if (peer.connection != k_HSteamNetConnection_Invalid && !peer.is_listen_connection) {
                LOG_DEBUG("~SteamState(): closing connection to \"", peer.ident, "\"");
                auto ok = SteamNetworkingSockets()->CloseConnection(
                    peer.connection, k_ESteamNetConnectionEnd_App_Generic, nullptr, false);
                if (!ok) {
                    LOG_ERROR("~SteamState(): closing the connection failed");
                }
            }
        }
        SteamAPI_RunCallbacks();
        // Make sure to close connections before closing the listen socket, that
        // seems to crash Steam.
        if (listen_socket != k_HSteamListenSocket_Invalid) {
            LOG_DEBUG("~SteamState(): closing listen socket");
            auto ok = SteamNetworkingSockets()->CloseListenSocket(listen_socket);
            if (!ok) LOG_ERROR("~SteamState(): closing listen socket failed");
        }
        SteamAPI_RunCallbacks();
        if (poll_group != k_HSteamNetPollGroup_Invalid) {
            LOG_DEBUG("~SteamState(): destroying poll group");
            auto ok = SteamNetworkingSockets()->DestroyPollGroup(poll_group);
            if (!ok) LOG_ERROR("~SteamState(): destroying poll group failed");
        }
        SteamAPI_RunCallbacks();
    }

    STEAM_CALLBACK(SteamState, OnGameOverlayActivated, GameOverlayActivated_t);
    STEAM_CALLBACK(SteamState, OnScreenshotRequested, ScreenshotRequested_t);
	STEAM_CALLBACK(SteamState, OnNetConnectionStatusChanged, SteamNetConnectionStatusChangedCallback_t);

    // P2P Functions
    string GetIpAddressIdentity() {
        if (!listen_socket) {
            LOG_ERROR("GetIpAddressIdentity(): listen socket is not open");
            return "";
        }
        SteamNetworkingIPAddr ip_address;
        if (!SteamNetworkingSockets()->GetListenSocketAddress(listen_socket, &ip_address)) {
            LOG_ERROR("GetIpAddressIdentity(): GetListenSocketAddress failed");
            return "";
        }
        SteamNetworkingIdentity id;
        id.m_eType = k_ESteamNetworkingIdentityType_IPAddress;
        id.SetIPAddr(ip_address);
        char ident[SteamNetworkingIdentity::k_cchMaxString];
        id.ToString(ident, sizeof(ident));
        return ident;
    }

    auto FindPeer(const SteamNetworkingIdentity &net_identity) {
        return find_if(peers.begin(), peers.end(), [&](const auto &peer) {
            return peer.net_identity == net_identity;
        });
    }

    auto FindPeer(const string_view ident) {
        return find_if(peers.begin(), peers.end(), [&](const auto& peer) {
            return peer.ident == ident;
        });
    }

    auto FindPeer(HSteamNetConnection conn) {
        return find_if(peers.begin(), peers.end(), [&](const auto& peer) {
            return peer.connection == conn;
        });
    }

    auto FindOrCreatePeer(HSteamNetConnection conn) {
        bool found = true;
        auto peer = FindPeer(conn);
        if (peer == peers.end()) {
            peers.emplace_back();
            peer = peers.end() - 1;
            found = false;
        }
        return make_pair(peer, found);
    }

    auto RenamePeer(string_view_nt str_identity, string_view_nt str_new_identity) {
        auto peer = FindPeer(str_identity.sv);
        if (peer == peers.end()) {
            LOG_ERROR("RenamePeer(): no peer named ", str_identity);
            return false;
        }
        auto new_peer = FindPeer(str_new_identity.sv);
        // Don't allow renaming to a name that already exists.
        if (new_peer != peers.end()) {
            LOG_ERROR("RenamePeer(): peer named ", str_new_identity, " already exists.");
            return false;
        }
        peer->ident = str_new_identity.sv;
        return true;
    }

    bool SetGlobalConfigValue(ESteamNetworkingConfigValue eValue, int val) {
        return SteamNetworkingUtils()->SetGlobalConfigValueInt32(eValue, val);
    }

    bool P2PListen() {
        if (listen_socket != k_HSteamListenSocket_Invalid) {
            LOG_ERROR("P2PListen(): listen socket is already open");
            return false;
        }
        auto socket = SteamNetworkingSockets()->CreateListenSocketP2P(1, 0, nullptr);
        listen_socket = socket;
        LOG_DEBUG("P2PListen(): created listen socket");
        return socket != k_HSteamListenSocket_Invalid;
    }

    bool P2PListenIP(string_view_nt ip_addr) {
        if (listen_socket != k_HSteamListenSocket_Invalid) {
            LOG_ERROR("P2PListenIP(): listen socket is already open");
            return false;
        }
        SteamNetworkingIPAddr ip;
        ip.ParseString(ip_addr.c_str());
        char real_ip[SteamNetworkingIdentity::k_cchMaxString]{};
        ip.ToString(real_ip, sizeof(real_ip), true);
        auto socket = SteamNetworkingSockets()->CreateListenSocketIP(ip, 0, nullptr);
        listen_socket = socket;
        LOG_DEBUG("P2PListenIP(): created listen socket for IP Address at ", real_ip);
        SteamNetworkingIPAddr address;
        if (!SteamNetworkingSockets()->GetListenSocketAddress(listen_socket, &address)) {
            LOG_ERROR("P2PListenIP(): GetListenSocketAddress failed");
            return false;
        }
        char ipaddr[SteamNetworkingIPAddr::k_cchMaxString]{};
        address.ToString(ipaddr, sizeof(ipaddr), true);
        LOG_DEBUG("P2PListenIP(): listen socket IP Address is ", ipaddr);
        return socket != k_HSteamListenSocket_Invalid;
    }

    bool P2PConnect(string_view_nt str_identity) {
        SteamNetworkingIdentity identity{};
        identity.ParseString(str_identity.c_str());
        HSteamNetConnection connection = k_HSteamNetConnection_Invalid;
        if (identity.m_eType == k_ESteamNetworkingIdentityType_IPAddress) {
            const SteamNetworkingIPAddr *ip = identity.GetIPAddr();
            if (ip == NULL) {
                LOG_ERROR("P2PConnect(): identity ", str_identity, " has type IP but GetIPAddr() failed?");
                return false;
            }
            char ip_addr[SteamNetworkingIdentity::k_cchMaxString]{};
            ip->ToString(ip_addr, sizeof(ip_addr), true);
            LOG_DEBUG("P2PConnect(): opened connection for IP address to ", ip_addr);
            connection = SteamNetworkingSockets()->ConnectByIPAddress(*ip, 0, nullptr);
        } else {
            LOG_DEBUG("P2PConnect(): opening connection to ", str_identity);
            connection = SteamNetworkingSockets()->ConnectP2P(identity, 1, 0, nullptr);
        }
        if (connection == k_HSteamNetConnection_Invalid) {
            LOG_ERROR("P2PConnect(): failed to open connection to ", str_identity);
            return false;
        }
        peers.push_back(SteamPeer { string(str_identity.sv), {}, connection, false, false, {} });
        return true;
    }

    bool P2PConnectIP(string_view_nt ip_addr) {
        SteamNetworkingIPAddr ip;
        ip.ParseString(ip_addr.c_str());
        char real_ip[SteamNetworkingIdentity::k_cchMaxString]{};
        ip.ToString(real_ip, sizeof(real_ip), true);
        LOG_DEBUG("P2PConnectIP(): opened connection for IP address to ", ip_addr, " (parsed as ", real_ip, ")");
        auto connection = SteamNetworkingSockets()->ConnectByIPAddress(ip, 0, nullptr);
        if (connection == k_HSteamNetConnection_Invalid) {
            LOG_ERROR("P2PConnectIP(): failed to open connection to ", ip_addr);
            return false;
        }
        // Use the SteamNetworkingIdentity format for the peer's identity (e.g. "ip:<ip_addr>")
        SteamNetworkingIdentity id;
        id.m_eType = k_ESteamNetworkingIdentityType_IPAddress;
        id.SetIPAddr(ip);
        char str_id[SteamNetworkingIdentity::k_cchMaxString];
        id.ToString(str_id, sizeof(str_id));
        peers.push_back(SteamPeer { str_id, {}, connection, false, false, {} });
        return true;
    }

    bool P2PCloseConnection(string_view_nt str_identity, bool linger) {
        auto peer = FindPeer(str_identity.sv);
        if (peer == peers.end()) {
            LOG_ERROR("P2PCloseConnection(): no peer named ", str_identity);
            return false;
        }
        if (!peer->is_connected) {
            LOG_ERROR("P2PCloseConnection(): peer named ", str_identity, " is not connected");
            return false;
        }
        auto ok = SteamNetworkingSockets()->CloseConnection(peer->connection, k_ESteamNetConnectionEnd_App_Generic, "", linger);
        if (ok) {
            peers.erase(peer);
        }
        LOG_DEBUG("P2PCloseConnection(): connection closed");
        return ok;
    }

    bool CloseListen() {
        if (listen_socket == k_HSteamListenSocket_Invalid) {
            LOG_DEBUG("CloseListen(): listen socket is already closed");
            return true;
        }
        auto result = SteamNetworkingSockets()->CloseListenSocket(listen_socket);
        LOG_DEBUG("CloseListen(): closed listen socket");
        listen_socket = k_HSteamListenSocket_Invalid;
        return result;
    }

    bool SendMessage(string_view_nt str_identity, string_view buf, bool reliable, EResult *result) {
        auto peer = FindPeer(str_identity.sv);
        if (peer == peers.end()) {
            LOG_ERROR("SendMessage(): no peer named ", str_identity);
            return false;
        }
        if (!peer->is_connected) {
            if (reliable) {
                LOG_DEBUG("SendMessage(): queueing message to \"", str_identity, "\"");
                peer->queued_messages.push_back(QueuedMessage { peer->net_identity, string(buf) });
                return true;
            }
            LOG_ERROR("SendMessage(): peer named ", str_identity, " is not connected, dropping unreliable message");
            return false;
        }

        const char *data = buf.data();
        uint32_t size = (uint32_t)buf.size();
        int flags = reliable ? k_nSteamNetworkingSend_Reliable : k_nSteamNetworkingSend_Unreliable;
        *result = SteamNetworkingSockets()->SendMessageToConnection(peer->connection, data, size, flags, nullptr);
        if (*result != k_EResultOK) {
            LOG_ERROR("SendMessage(): trying to send message to \"", str_identity, "\" of size ", buf.size() ," got result ",  *result);
        }
        return *result == k_EResultOK;
    }

    bool BroadcastMessage(string_view buf, bool reliable) {
        const char *data = buf.data();
        uint32_t size = (uint32_t)buf.size();
        int flags = reliable ? k_nSteamNetworkingSend_Reliable : k_nSteamNetworkingSend_Unreliable;
        bool ok = true;
        // TODO: We should be able to use SendMessages here so we don't need to
        // make multiple copies of the buffer. The message struct itself is
        // ref-counted, but also contains the destination connection, so
        // it can't be shared. We'd need to have the data itself be ref-counted
        // too.
        for (auto &peer: peers) {
            if (!peer.is_connected) {
                char ident[SteamNetworkingIdentity::k_cchMaxString]{};
                peer.net_identity.ToString(ident, sizeof(ident));
                if (reliable) {
                    LOG_DEBUG("BroadcastMessage(): queueing message to \"", ident, "\"");
                    peer.queued_messages.push_back(QueuedMessage { peer.net_identity, string(buf) });
                    continue;
                }
                LOG_ERROR("BroadcastMessage(): peer named ", ident, " is not connected, dropping unreliable message");
                continue;
            }
            auto result = SteamNetworkingSockets()->SendMessageToConnection(peer.connection, data, size, flags, nullptr);
            ok &= result == k_EResultOK;
            if (result != k_EResultOK) {
                LOG_ERROR("BroadcastMessage(): trying to send message to \"", peer.ident, "\" of size ", buf.size() ," got result ",  result);
            }
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

    void UpdateP2P() {
        for (auto &peer : peers) {
            if (!peer.is_connected) continue;
            // Send the queued messages to the peer, now that they're connected.
            char ident[SteamNetworkingIdentity::k_cchMaxString]{};
            peer.net_identity.ToString(ident, sizeof(ident));
            for (auto &message : peer.queued_messages) {
                const char *data = message.message.data();
                uint32_t size = (uint32_t)message.message.size();
                int flags = k_nSteamNetworkingSend_Reliable;
                auto result = SteamNetworkingSockets()->SendMessageToConnection(peer.connection, data, size, flags, nullptr);
                if (result != k_EResultOK) {
                    LOG_ERROR("UpdateP2P(): trying to send message to \"", ident, "\" of size ", size ," got result ",  result);
                }
            }
            peer.queued_messages.clear();
        }
    }

    bool GetConnectionRealTimeStatus(string_view_nt str_identity, SteamNetConnectionRealTimeStatus_t* status) {
        auto peer = FindPeer(str_identity.sv);
        if (peer == peers.end()) {
            LOG_ERROR("GetConnectionRealTimeStatus(): no peer named ", str_identity);
            return false;
        }
        if (!peer->is_connected) {
            LOG_ERROR("GetConnectionRealTimeStatus(): peer named ", str_identity, " is not connected");
            return false;
        }
        auto result = SteamNetworkingSockets()->GetConnectionRealTimeStatus(peer->connection, status, 0, nullptr);
        if (result != k_EResultOK) {
            LOG_ERROR("GetConnectionRealTimeStatus(): failed to get realtime status of ", str_identity, " result=", result);
            return false;
        }
        return true;
    }

    bool SetGlobalConfigValue(int enum_, int value) {
        return SteamNetworkingUtils()->SetGlobalConfigValueInt32(ESteamNetworkingConfigValue(enum_),
                                                                 value);
    }

    bool SetGlobalConfigValue(int enum_, float value) {
        return SteamNetworkingUtils()->SetGlobalConfigValueFloat(ESteamNetworkingConfigValue(enum_),
                                                                 value);
    }

    // Note from steamworks API about this callback:
    //
    /// IMPORTANT: This may be called from a service thread, while we own a mutex, etc.
    /// Your output function must be threadsafe and fast!  Do not make any other
    /// Steamworks calls from within the handler.
    static void DebugOutputFn(ESteamNetworkingSocketsDebugOutputType /*nType*/,
                              const char *pszMsg) {
        lock_guard<mutex> guard(debug_output_mutex);
        debug_output.push_back(string(pszMsg));
    }

    vector<string> GetDebugOutput() {
        lock_guard<mutex> guard(debug_output_mutex);
        auto result = std::move(debug_output);
        debug_output.clear();
        return result;
    }

    void SetDebugOutputLevel(int level) {
        auto elevel = (ESteamNetworkingSocketsDebugOutputType)level;
        if (elevel == k_ESteamNetworkingSocketsDebugOutputType_None) {
            SteamNetworkingUtils()->SetDebugOutputFunction(elevel, nullptr);
        } else {
            // No userdata...
            SteamNetworkingUtils()->SetDebugOutputFunction(elevel, DebugOutputFn);
        }
    }

    // Lobby Functions
    STEAM_CALLBACK(SteamState, OnLobbyDataUpdate, LobbyDataUpdate_t);

    bool CreateLobby(int lobby_type, int max_members) {
        if (lobby_type < 0 || lobby_type > k_ELobbyTypeInvisible) {
            return false;
        }
        if (OnLobbyCreatedCallback.IsActive()) {
            return false;
        }
        created_lobby.Clear();
        joined_lobby.Clear();
        auto result = SteamMatchmaking()->CreateLobby((ELobbyType)lobby_type, max_members);
        OnLobbyCreatedCallback.Set(result, this, &SteamState::OnLobbyCreated);
        return true;
    }
	CCallResult<SteamState, LobbyCreated_t> OnLobbyCreatedCallback;
    void OnLobbyCreated(LobbyCreated_t *pCallback, bool bIOFailure);

    bool JoinLobby(CSteamID steam_id) {
        if (OnLobbyEnteredCallback.IsActive()) {
            return false;
        }
        joined_lobby.Clear();
        auto result = SteamMatchmaking()->JoinLobby(steam_id);
        OnLobbyEnteredCallback.Set(result, this, &SteamState::OnLobbyEntered);
        return true;
    }
	CCallResult<SteamState, LobbyEnter_t> OnLobbyEnteredCallback;
    void OnLobbyEntered(LobbyEnter_t *pCallback, bool bIOFailure);

    bool LeaveLobby(CSteamID steam_id) {
        SteamMatchmaking()->LeaveLobby(steam_id);
        if (steam_id == joined_lobby) {
            joined_lobby.Clear();
        }
        if (steam_id == created_lobby) {
            created_lobby.Clear();
        }
        auto iter = find(joined_lobbies.begin(), joined_lobbies.end(), steam_id);
        if (iter != joined_lobbies.end()) {
            joined_lobbies.erase(iter);
        }
        return true;
    }

    bool SetLobbyJoinable(CSteamID steam_id, bool joinable) {
        return SteamMatchmaking()->SetLobbyJoinable(steam_id, joinable);
    }

    const char* GetLobbyData(CSteamID steam_id, const char* key) {
        auto result = SteamMatchmaking()->GetLobbyData(steam_id, key);
        return result;
    }

    CSteamID GetLobbyOwner(CSteamID steam_id) {
        auto result = SteamMatchmaking()->GetLobbyOwner(steam_id);
        return result;
    }

    int GetLobbyDataCount(CSteamID steam_id) {
        auto count = SteamMatchmaking()->GetLobbyDataCount(steam_id);
        return count;
    }

    int GetLobbyDataByIndex(CSteamID steam_id, int index, char *key, int key_size, char *value, int value_size) {
        auto count = SteamMatchmaking()->GetLobbyDataByIndex(steam_id, index, key, key_size, value, value_size);
        return count;
    }

    bool SetLobbyData(CSteamID steam_id, const char* key, const char* value) {
        auto ok = SteamMatchmaking()->SetLobbyData(steam_id, key, value);
        return ok;
    }

    bool DeleteLobbyData(CSteamID steam_id, const char* key) {
        auto ok = SteamMatchmaking()->DeleteLobbyData(steam_id, key);
        return ok;
    }

    int GetNumLobbyMembers(CSteamID steam_id) {
        return SteamMatchmaking()->GetNumLobbyMembers(steam_id);
    }

    CSteamID GetLobbyMemberByIndex(CSteamID steam_id, int index) {
        return SteamMatchmaking()->GetLobbyMemberByIndex(steam_id, index);
    }

    bool AddRequestLobbyListNumericalFilter(const char *key, int value, ELobbyComparison cmp) {
        SteamMatchmaking()->AddRequestLobbyListNumericalFilter(key, value, cmp);
        return true;
    }

    bool AddRequestLobbyListStringFilter(const char *key, const char *value, ELobbyComparison cmp) {
        SteamMatchmaking()->AddRequestLobbyListStringFilter(key, value, cmp);
        return true;
    }

    bool AddRequestLobbyListResultCountFilter(int count) {
        SteamMatchmaking()->AddRequestLobbyListResultCountFilter(count);
        return true;
    }

    bool RequestLobbyData(CSteamID steam_id) {
        auto ok = SteamMatchmaking()->RequestLobbyData(steam_id);
        return ok;
    }

    bool LobbyListIsReady() {
        return matched_lobbies >= 0;
    }

    bool LobbyListReset() {
        OnLobbyMatchListCallback.Cancel();
        matched_lobbies = 0;
        return true;
    }

    bool RequestLobbyList() {
        if (OnLobbyMatchListCallback.IsActive()) {
            return false;
        }
        auto result = SteamMatchmaking()->RequestLobbyList();
        OnLobbyMatchListCallback.Set(result, this, &SteamState::OnLobbyMatchList);
        matched_lobbies = -1;
        return true;
    }
    CCallResult<SteamState, LobbyMatchList_t> OnLobbyMatchListCallback;
    void OnLobbyMatchList(LobbyMatchList_t *pLobbyMatchList, bool bIOFailure);

    CSteamID GetLobbyByIndex(int index) {
        return SteamMatchmaking()->GetLobbyByIndex(index);
    }

    CSteamID GetLobbyGameServer(CSteamID lobby_id) {
        CSteamID server_id;
        SteamMatchmaking()->GetLobbyGameServer(lobby_id, nullptr, nullptr, &server_id);
        return server_id;
    }

    bool SetLobbyGameServer(CSteamID lobby_id, CSteamID server_id) {
        SteamMatchmaking()->SetLobbyGameServer(lobby_id, 0, 0, server_id);
        return true;
    }

    const char *GetFriendPersonaName(CSteamID steam_id) {
        return SteamFriends()->GetFriendPersonaName(steam_id);
    }

    bool HasFriend(CSteamID steam_id, int friend_flags) {
        return SteamFriends()->HasFriend(steam_id, friend_flags);
    }

    // Workshop (UGC) Functions.

    // Sync state: for each installed item that has been copied into the game's dirs,
    // the install timestamp at the time of copying (so updates get recopied), plus
    // items that failed and should not be retried this session.
    map<PublishedFileId_t, uint32> workshop_copied;
    set<PublishedFileId_t> workshop_failed;

    // Upload state machine.
    enum class WorkshopUploadStage {
        Idle,
        CheckingOwner,   // Querying details of an existing item to verify ownership.
        Creating,        // Waiting for CreateItem() of a new item.
        Submitting,      // Waiting for SubmitItemUpdate().
        Done,
        Failed,
        FailedNotOwner,  // The existing item is owned by a different steam user.
    };
    WorkshopUploadStage workshop_upload_stage = WorkshopUploadStage::Idle;
    PublishedFileId_t workshop_upload_file_id = 0;
    UGCUpdateHandle_t workshop_upload_handle = k_UGCUpdateHandleInvalid;
    bool workshop_upload_is_new = false;
    bool workshop_needs_legal_agreement = false;
    // Upload params kept alive between the async stages.
    string workshop_upload_content_folder;
    string workshop_upload_title;
    string workshop_upload_description;
    string workshop_upload_metadata;
    string workshop_upload_changenote;
    string workshop_upload_preview;

    STEAM_CALLBACK(SteamState, OnDownloadItemResult, DownloadItemResult_t);

    // Steam APIs (SetItemContent etc.) and filesystem ops want absolute paths, but
    // Lobster games mostly deal in paths relative to their write dir.
    static string ResolveToAbsolutePath(string_view path) {
        auto p = SanitizePath(path);
        if (!IsAbsolute(p)) p = GetMainWriteDir() + p;
        error_code ec;
        auto abs = filesystem::absolute(filesystem::path(p), ec);
        return ec ? p : abs.string();
    }

    static bool CopyDirRecursive(const filesystem::path &src, const filesystem::path &dst,
                                 vector<string> &copied_files) {
        error_code ec;
        filesystem::create_directories(dst, ec);
        if (ec) return false;
        filesystem::recursive_directory_iterator iter(src, ec);
        if (ec) return false;
        for (auto &entry : iter) {
            auto rel = filesystem::relative(entry.path(), src, ec);
            if (ec) return false;
            auto target = dst / rel;
            if (entry.is_directory()) {
                filesystem::create_directories(target, ec);
                if (ec) return false;
            } else if (entry.is_regular_file()) {
                filesystem::copy_file(entry.path(), target,
                                      filesystem::copy_options::overwrite_existing, ec);
                if (ec) return false;
                copied_files.push_back(target.generic_string());
            }
        }
        return true;
    }

    // Makes subscribed workshop content progress towards being downloaded & copied
    // into the game's dirs, see the workshop_sync builtin below.
    int WorkshopSync(string_view dest_dir, bool own_subdirs, vector<string> &copied_files) {
        auto num = SteamUGC()->GetNumSubscribedItems();
        if (!num) return 0;
        vector<PublishedFileId_t> ids(num);
        num = SteamUGC()->GetSubscribedItems(ids.data(), num);
        ids.resize(num);
        int busy = 0;
        bool copy_error = false;
        for (auto id : ids) {
            if (workshop_failed.count(id)) continue;
            auto state = SteamUGC()->GetItemState(id);
            if (!(state & k_EItemStateInstalled) || (state & k_EItemStateNeedsUpdate)) {
                // Steam auto-downloads subscribed items in most cases, but items
                // subscribed to while the game is running (e.g. thru the overlay)
                // need an explicit kick.
                if (!(state & (k_EItemStateDownloading | k_EItemStateDownloadPending))) {
                    if (!SteamUGC()->DownloadItem(id, true)) {
                        LOG_ERROR("WorkshopSync(): DownloadItem() failed for item ", id);
                        workshop_failed.insert(id);
                        continue;
                    }
                }
                busy++;
                continue;
            }
            uint64 size_on_disk = 0;
            char folder[1024] = "";
            uint32 timestamp = 0;
            if (!SteamUGC()->GetItemInstallInfo(id, &size_on_disk, folder, sizeof(folder),
                                                &timestamp)) {
                LOG_ERROR("WorkshopSync(): GetItemInstallInfo() failed for item ", id);
                workshop_failed.insert(id);
                continue;
            }
            auto it = workshop_copied.find(id);
            if (it != workshop_copied.end() && it->second == timestamp) continue;
            // The item folder is named after the workshop file id, but the files
            // inside it keep the names they were uploaded with (see SetItemContent),
            // so copying preserves original filenames.
            auto dest = filesystem::path(ResolveToAbsolutePath(dest_dir));
            if (own_subdirs) dest /= to_string(id);
            if (!CopyDirRecursive(folder, dest, copied_files)) {
                LOG_ERROR("WorkshopSync(): failed to copy item ", id, " from ", folder);
                copy_error = true;
                continue;
            }
            LOG_DEBUG("WorkshopSync(): copied item ", id, " from ", folder);
            workshop_copied[id] = timestamp;
            // FIXME: if a game needs the uploader-supplied metadata (see
            // workshop_upload_start) at download time, add an async
            // CreateQueryUGCDetailsRequest stage here with SetReturnMetadata(true)
            // and return it alongside the copied files. Not needed for the basic
            // use case since filenames are already preserved by the copy.
        }
        if (copy_error) return -1;
        if (busy) return 1;
        return workshop_failed.empty() ? 0 : 2;
    }

    bool WorkshopUploadInProgress() {
        return workshop_upload_stage == WorkshopUploadStage::CheckingOwner ||
               workshop_upload_stage == WorkshopUploadStage::Creating ||
               workshop_upload_stage == WorkshopUploadStage::Submitting;
    }

    bool WorkshopUploadStart(string_view content_folder, string_view title,
                             string_view description, string_view metadata,
                             string_view changenote, string_view preview_image,
                             iint existing_file_id) {
        if (WorkshopUploadInProgress()) {
            LOG_ERROR("WorkshopUploadStart(): an upload is already in progress");
            return false;
        }
        workshop_upload_content_folder = ResolveToAbsolutePath(content_folder);
        workshop_upload_title = title;
        workshop_upload_description = description;
        workshop_upload_metadata = metadata;
        workshop_upload_changenote = changenote;
        workshop_upload_preview =
            preview_image.empty() ? string() : ResolveToAbsolutePath(preview_image);
        workshop_upload_file_id = (PublishedFileId_t)existing_file_id;
        workshop_upload_handle = k_UGCUpdateHandleInvalid;
        workshop_upload_is_new = !existing_file_id;
        workshop_needs_legal_agreement = false;
        if (workshop_upload_is_new) {
            auto call = SteamUGC()->CreateItem(SteamUtils()->GetAppID(),
                                               k_EWorkshopFileTypeCommunity);
            OnCreateItemCallback.Set(call, this, &SteamState::OnCreateItem);
            workshop_upload_stage = WorkshopUploadStage::Creating;
        } else {
            // Updating existing content: first verify the current user actually owns
            // it, only then submit the update.
            auto query = SteamUGC()->CreateQueryUGCDetailsRequest(&workshop_upload_file_id, 1);
            if (query == k_UGCQueryHandleInvalid) {
                LOG_ERROR("WorkshopUploadStart(): CreateQueryUGCDetailsRequest() failed");
                workshop_upload_stage = WorkshopUploadStage::Failed;
                return false;
            }
            auto call = SteamUGC()->SendQueryUGCRequest(query);
            OnOwnerQueryCallback.Set(call, this, &SteamState::OnOwnerQueryCompleted);
            workshop_upload_stage = WorkshopUploadStage::CheckingOwner;
        }
        return true;
    }
    CCallResult<SteamState, CreateItemResult_t> OnCreateItemCallback;
    void OnCreateItem(CreateItemResult_t *callback, bool io_failure);
    CCallResult<SteamState, SteamUGCQueryCompleted_t> OnOwnerQueryCallback;
    void OnOwnerQueryCompleted(SteamUGCQueryCompleted_t *callback, bool io_failure);
    CCallResult<SteamState, SubmitItemUpdateResult_t> OnSubmitItemUpdateCallback;
    void OnSubmitItemUpdate(SubmitItemUpdateResult_t *callback, bool io_failure);

    void WorkshopSubmitUpdate() {
        auto handle = SteamUGC()->StartItemUpdate(SteamUtils()->GetAppID(),
                                                  workshop_upload_file_id);
        if (handle == k_UGCUpdateHandleInvalid) {
            LOG_ERROR("WorkshopSubmitUpdate(): StartItemUpdate() failed");
            workshop_upload_stage = WorkshopUploadStage::Failed;
            return;
        }
        // Empty strings leave the existing values unchanged on an update.
        if (!workshop_upload_title.empty())
            SteamUGC()->SetItemTitle(handle, workshop_upload_title.c_str());
        if (!workshop_upload_description.empty())
            SteamUGC()->SetItemDescription(handle, workshop_upload_description.c_str());
        if (!workshop_upload_metadata.empty())
            SteamUGC()->SetItemMetadata(handle, workshop_upload_metadata.c_str());
        // Only force visibility for new items, the owner may have changed it on the
        // workshop page since.
        if (workshop_upload_is_new)
            SteamUGC()->SetItemVisibility(handle, k_ERemoteStoragePublishedFileVisibilityPublic);
        SteamUGC()->SetItemContent(handle, workshop_upload_content_folder.c_str());
        if (!workshop_upload_preview.empty())
            SteamUGC()->SetItemPreview(handle, workshop_upload_preview.c_str());
        auto call = SteamUGC()->SubmitItemUpdate(
            handle,
            workshop_upload_changenote.empty() ? nullptr : workshop_upload_changenote.c_str());
        OnSubmitItemUpdateCallback.Set(call, this, &SteamState::OnSubmitItemUpdate);
        workshop_upload_handle = handle;
        workshop_upload_stage = WorkshopUploadStage::Submitting;
    }

    int WorkshopUploadStatus(iint &file_id, bool &needs_legal, iint &processed, iint &total) {
        file_id = (iint)workshop_upload_file_id;
        needs_legal = workshop_needs_legal_agreement;
        processed = 0;
        total = 0;
        if (workshop_upload_stage == WorkshopUploadStage::Submitting &&
            workshop_upload_handle != k_UGCUpdateHandleInvalid) {
            uint64 p = 0, t = 0;
            SteamUGC()->GetItemUpdateProgress(workshop_upload_handle, &p, &t);
            processed = (iint)p;
            total = (iint)t;
        }
        switch (workshop_upload_stage) {
            case WorkshopUploadStage::Idle:          return 0;
            case WorkshopUploadStage::CheckingOwner:
            case WorkshopUploadStage::Creating:
            case WorkshopUploadStage::Submitting:    return 1;
            case WorkshopUploadStage::Done:          return 2;
            case WorkshopUploadStage::FailedNotOwner: return -2;
            default:                                 return -1;
        }
    }

    void WorkshopOpenPage() {
        auto url = cat("steam://url/SteamWorkshopPage/", SteamUtils()->GetAppID());
        SteamFriends()->ActivateGameOverlayToWebPage(url.c_str());
    }

    void WorkshopOpenItemPage(iint file_id) {
        auto url = cat("steam://url/CommunityFilePage/", (uint64)file_id);
        SteamFriends()->ActivateGameOverlayToWebPage(url.c_str());
    }

    void WorkshopOpenLegalAgreement() {
        SteamFriends()->ActivateGameOverlayToWebPage(
            "https://steamcommunity.com/sharedfiles/workshoplegalagreement");
    }
};

mutex SteamState::debug_output_mutex;
vector<string> SteamState::debug_output;

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

    SteamNetworkingIdentity remote_identity = info.m_identityRemote;
    char ident[SteamNetworkingIdentity::k_cchMaxString]{};
    remote_identity.ToString(ident, sizeof(ident));
    char ipaddr[SteamNetworkingIPAddr::k_cchMaxString]{};
    info.m_addrRemote.ToString(ipaddr, sizeof(ipaddr), true);

    // If the local identity and the remote identity are the same and they're
    // both using Steam ID, then we're trying to connect two clients on the same
    // machine. So just use the IP address as the identity instead.
    SteamNetworkingIdentity local_identity{};
    SteamNetworkingSockets()->GetIdentity(&local_identity);
    if (local_identity == remote_identity && local_identity.m_eType == k_ESteamNetworkingIdentityType_SteamID) {
        SteamNetworkingIdentity ip_identity;
        ip_identity.m_eType = k_ESteamNetworkingIdentityType_IPAddress;
        ip_identity.SetIPAddr(info.m_addrRemote);
        ip_identity.ToString(ident, sizeof(ident));
        remote_identity = ip_identity;
    }

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
    LOG_DEBUG("OnNetConnectionStatusChanged(): steam net connection status changed: ident=", ident, " ipaddr=", ipaddr, " old state='",
              get_state_name(old_state), "' new state='", get_state_name(new_state), "'");

	if (info.m_hListenSocket != k_HSteamListenSocket_Invalid &&
		old_state == k_ESteamNetworkingConnectionState_None &&
		new_state == k_ESteamNetworkingConnectionState_Connecting) {
        // For now at least, only allow one connection to a peer with a given steam ID.
        auto peer_by_ident = FindPeer(remote_identity);
        if (peer_by_ident != peers.end()) {
            LOG_ERROR("OnNetConnectionStatusChanged(): Peer \"", ident, "\" connecting, but already connected?");
            SteamNetworkingSockets()->CloseConnection(conn, k_ESteamNetConnectionEnd_AppException_Generic, "Already connected", false );
            peers.erase(peer_by_ident);
            return;
        }
        // Find or create the peer with this connection. It may have already
        // been created if we connected with P2PConnect*.
        auto [ peer, found_peer ] = FindOrCreatePeer(conn);
        EResult res = SteamNetworkingSockets()->AcceptConnection(conn);
        if (res != k_EResultOK) {
            LOG_ERROR("OnNetConnectionStatusChanged(): AcceptConnection failed with error=", res);
            SteamNetworkingSockets()->CloseConnection(conn, k_ESteamNetConnectionEnd_AppException_Generic, "Failed to accept connection", false );
            peers.erase(peer);
            return;
        }
        peer->connection = conn;
        peer->ident = ident;
        peer->net_identity = remote_identity;
        peer->is_connected = false;
        peer->is_listen_connection = peer->is_listen_connection || !found_peer;
        LOG_DEBUG("OnNetConnectionStatusChanged(): OnNetConnectionStatusChanged(): Connecting peer \"", ident, "\"");
        // TODO: start authentication here, if needed
    } else if (new_state == k_ESteamNetworkingConnectionState_Connected) {
        auto [ peer, found_peer ] = FindOrCreatePeer(conn);
        peer->connection = conn;
        peer->ident = ident;
        peer->net_identity = remote_identity;
        peer->is_connected = true;
        peer->is_listen_connection = peer->is_listen_connection || !found_peer;
        if (poll_group == k_HSteamNetPollGroup_Invalid) {
            LOG_DEBUG("OnNetConnectionStatusChanged(): Creating poll group");
            poll_group = SteamNetworkingSockets()->CreatePollGroup();
        }
        SteamNetworkingSockets()->SetConnectionPollGroup(conn, poll_group);
        LOG_DEBUG("OnNetConnectionStatusChanged(): Connected peer \"", ident, "\"");
	} else if (new_state == k_ESteamNetworkingConnectionState_ClosedByPeer ||
               new_state == k_ESteamNetworkingConnectionState_ProblemDetectedLocally) {
		// Handle disconnecting a peer
        auto peer = FindPeer(conn);
        if (peer != peers.end()) {
            auto ok = SteamNetworkingSockets()->CloseConnection(
                peer->connection, k_ESteamNetConnectionEnd_App_Generic, nullptr, false);
            peers.erase(peer);
            LOG_DEBUG("OnNetConnectionStatusChanged(): Disconnected from peer \"", ident, "\" msg=\"", info.m_szEndDebug, "\" close connection ok?=", ok);
        } else {
            LOG_DEBUG("OnNetConnectionStatusChanged(): Peer \"", ident, "\" disconnecting, but not in list?  msg=\"", info.m_szEndDebug, "\"");
        }
	}
}

// Lobby callbacks
void SteamState::OnLobbyCreated(LobbyCreated_t *callback, bool /*io_failure*/) {
    LOG_DEBUG("OnLobbyCreated(): result=", callback->m_eResult, " lobby=", callback->m_ulSteamIDLobby);
    if (callback->m_eResult == k_EResultOK) {
        CSteamID lobby_id(callback->m_ulSteamIDLobby);
        created_lobby = lobby_id;
        joined_lobby = lobby_id;
        auto iter = find(joined_lobbies.begin(), joined_lobbies.end(), lobby_id);
        if (iter == joined_lobbies.end()) {
            joined_lobbies.push_back(lobby_id);
        } else {
            LOG_DEBUG("OnLobbyCreated(): Already joined lobby=", callback->m_ulSteamIDLobby, "?");
        }
    }
}

void SteamState::OnLobbyEntered(LobbyEnter_t *callback, bool /*io_failure*/) {
    LOG_DEBUG("OnLobbyEntered(): lobby=", callback->m_ulSteamIDLobby,
              " permissions=", callback->m_rgfChatPermissions, " locked=", callback->m_bLocked, " response=", callback->m_EChatRoomEnterResponse);
    if (callback->m_EChatRoomEnterResponse == k_EChatRoomEnterResponseSuccess) {
        CSteamID lobby_id(callback->m_ulSteamIDLobby);
        joined_lobby = lobby_id;
        auto iter = find(joined_lobbies.begin(), joined_lobbies.end(), lobby_id);
        if (iter == joined_lobbies.end()) {
            joined_lobbies.push_back(lobby_id);
        } else {
            LOG_DEBUG("OnLobbyEntered(): Already joined lobby=", callback->m_ulSteamIDLobby, "?");
        }
    }
}

void SteamState::OnLobbyMatchList(LobbyMatchList_t *lobby_match_list, bool /*io_failure*/) {
    matched_lobbies = lobby_match_list->m_nLobbiesMatching;
    LOG_DEBUG("OnLobbyMatchList(): Matched ", matched_lobbies, " lobbies.");
}

void SteamState::OnLobbyDataUpdate(LobbyDataUpdate_t* callback) {
    LOG_DEBUG("OnLobbyDataUpdate(): lobby=", callback->m_ulSteamIDLobby,
              " member=", callback->m_ulSteamIDMember, " success=", callback->m_bSuccess);
}

// Workshop callbacks
void SteamState::OnDownloadItemResult(DownloadItemResult_t *callback) {
    if (callback->m_unAppID != SteamUtils()->GetAppID()) return;
    LOG_DEBUG("OnDownloadItemResult(): item=", callback->m_nPublishedFileId,
              " result=", callback->m_eResult);
    if (callback->m_eResult != k_EResultOK) {
        LOG_ERROR("OnDownloadItemResult(): download of workshop item ",
                  callback->m_nPublishedFileId, " failed, result=", callback->m_eResult);
        workshop_failed.insert(callback->m_nPublishedFileId);
    }
}

void SteamState::OnCreateItem(CreateItemResult_t *callback, bool io_failure) {
    LOG_DEBUG("OnCreateItem(): result=", callback->m_eResult,
              " item=", callback->m_nPublishedFileId,
              " needs_legal_agreement=", callback->m_bUserNeedsToAcceptWorkshopLegalAgreement);
    workshop_needs_legal_agreement |= callback->m_bUserNeedsToAcceptWorkshopLegalAgreement;
    if (io_failure || callback->m_eResult != k_EResultOK) {
        LOG_ERROR("OnCreateItem(): creating workshop item failed, result=", callback->m_eResult);
        workshop_upload_stage = WorkshopUploadStage::Failed;
        return;
    }
    workshop_upload_file_id = callback->m_nPublishedFileId;
    WorkshopSubmitUpdate();
}

void SteamState::OnOwnerQueryCompleted(SteamUGCQueryCompleted_t *callback, bool io_failure) {
    SteamUGCDetails_t details{};
    bool ok = !io_failure && callback->m_eResult == k_EResultOK &&
              callback->m_unNumResultsReturned > 0 &&
              SteamUGC()->GetQueryUGCResult(callback->m_handle, 0, &details) &&
              details.m_eResult == k_EResultOK;
    SteamUGC()->ReleaseQueryUGCRequest(callback->m_handle);
    if (!ok) {
        LOG_ERROR("OnOwnerQueryCompleted(): could not get details of existing workshop item ",
                  workshop_upload_file_id);
        workshop_upload_stage = WorkshopUploadStage::Failed;
        return;
    }
    if (details.m_ulSteamIDOwner != SteamUser()->GetSteamID().ConvertToUint64()) {
        LOG_ERROR("OnOwnerQueryCompleted(): workshop item ", workshop_upload_file_id,
                  " is owned by another user, not updating");
        workshop_upload_stage = WorkshopUploadStage::FailedNotOwner;
        return;
    }
    WorkshopSubmitUpdate();
}

void SteamState::OnSubmitItemUpdate(SubmitItemUpdateResult_t *callback, bool io_failure) {
    LOG_DEBUG("OnSubmitItemUpdate(): result=", callback->m_eResult,
              " item=", callback->m_nPublishedFileId,
              " needs_legal_agreement=", callback->m_bUserNeedsToAcceptWorkshopLegalAgreement);
    workshop_needs_legal_agreement |= callback->m_bUserNeedsToAcceptWorkshopLegalAgreement;
    if (io_failure || callback->m_eResult != k_EResultOK) {
        LOG_ERROR("OnSubmitItemUpdate(): submitting workshop item failed, result=",
                  callback->m_eResult);
        workshop_upload_stage = WorkshopUploadStage::Failed;
        return;
    }
    workshop_upload_stage = WorkshopUploadStage::Done;
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
        if (screenshots) SteamScreenshots()->HookScreenshots(true);
        if (initrelay) SteamNetworkingUtils()->InitRelayNetworkAccess();
        return 1;
    #else
        return 0;
    #endif  // PLATFORM_STEAMWORKS
}

void SteamUpdate() {
    #ifdef PLATFORM_STEAMWORKS
        if (steam) {
            SteamAPI_RunCallbacks();
            steam->UpdateP2P();
        }
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

using namespace lobster;

#ifdef PLATFORM_STEAMWORKS
LString* GetIdentityString(VM &vm, const SteamNetworkingIdentity &identity) {
    char ident[SteamNetworkingIdentity::k_cchMaxString];
    identity.ToString(ident, sizeof(ident));
    return vm.NewString(ident);
}

CSteamID SteamIDFromValue(Value steam_id) {
    return CSteamID((uint64)steam_id.ival());
}

iint IIntFromSteamID(const CSteamID &steam_id) {
    return (iint)steam_id.ConvertToUint64();
}
#endif

#ifdef PLATFORM_STEAMWORKS
#define STEAM_BOOL_VALUE(...)  Value(steam ? (__VA_ARGS__) : false)
#define STEAM_INT_VALUE(...)  Value(steam ? (__VA_ARGS__) : 0)
#define STEAM_IINT_VALUE(...)  Value(steam ? (__VA_ARGS__) : (iint)0)
#define STEAM_STRING_VALUE(vm, ...)  Value(vm.NewString(steam ? (__VA_ARGS__) : ""))
#else
#define STEAM_BOOL_VALUE(...) Value(false)
#define STEAM_INT_VALUE(...)  Value(0)
#define STEAM_IINT_VALUE(...)  Value((iint)0)
#define STEAM_STRING_VALUE(vm, ...)  Value(vm.NewString(""));
#endif

void AddSteam(NativeRegistry &nfr) {

nfr("init", "appid,allowscreenshots,initrelay", "IBB", "I",
    "initializes SteamWorks. returns 1 if succesful, 0 on failure. Specify a non-0 appid if you"
    " want to restart from steam if this wasn't started from steam (the return value in this"
    " case will be -1 to indicate you should terminate this instance). If you don't specify an"
    " appid here or in steam_appid.txt, init will likely fail. The other functions can still be"
    " called even if steam isn't active."
    " allowscreenshots automatically uploads screenshots to steam (triggered by steam)."
    " initrelay initializes the relay network for p2p early, to save time when it is first used.",
    [](StackPtr &, VM &, Value appid, Value ss, Value relay) {
        return Value(SteamInit(appid.ival(), ss.True(), relay.True()));
    });

nfr("shutdown", "", "", "",
    "", [](StackPtr &, VM &) { SteamShutDown(); });

nfr("overlay", "", "", "B",
    "returns true if the steam overlay is currently on (you may want to auto-pause if so)",
    [](StackPtr &, VM &) {
        return Value(OverlayActive());
    });

nfr("username", "", "", "S",
    "returns the name of the steam user, or empty string if not available.",
    [](StackPtr &, VM &vm) {
        return Value(vm.NewString(UserName()));
    });

nfr("unlock_achievement", "achievementname", "S", "B",
    "Unlocks an achievement and shows the achievement overlay if not already achieved before."
    " Will also Q-up saving achievement to Steam."
    " Returns true if succesful.",
    [](StackPtr &, VM &, Value name) {
        auto ok = UnlockAchievement(name.sval()->strvnt());
        return Value(ok);
    });

nfr("write_file", "file,contents", "SS", "B",
    "writes a file with the contents of a string to the steam cloud, or local storage if that"
    " fails, returns false if writing wasn't possible at all",
    [](StackPtr &, VM &, Value file, Value contents) {
        auto fn = file.sval()->strvnt();
        auto s = contents.sval();
        auto ok = SteamWriteFile(fn, s->strv());
        if (!ok) {
            ok = WriteFile(fn.sv, true, s->strv(), false);
        }
        return Value(ok);
    });

nfr("read_file", "file", "S", "S?",
    "returns the contents of a file as a string from the steam cloud if available, or otherwise"
    " from local storage, or nil if the file can't be found at all.",
    [](StackPtr &, VM &vm, Value file) {
        auto fn = file.sval()->strvnt();
        string buf;
        auto len = SteamReadFile(fn, buf);
        if (!len) len = (int)LoadFile(fn.sv, &buf);
        if (len < 0) return NilVal();
        auto s = vm.NewString(buf);
        return Value(s);
    });

nfr("update", "", "", "",
    "you must call this function in your game loop when using most steam APIs",
    [](StackPtr &, VM &) {
        SteamUpdate();
    });

nfr("get_steam_id", "", "", "I", "get the steam id of the current user",
    [](StackPtr &, VM &) {
        return STEAM_IINT_VALUE(IIntFromSteamID(SteamUser()->GetSteamID()));
    });

nfr("friend_get_username", "steam_id", "I", "S",
    "returns the name for the given steam id; this only works for friends, or users in the same "
    "lobby, chat room, game server, etc.",
    [](StackPtr &, VM &vm, Value steam_id) {
        return STEAM_STRING_VALUE(vm, steam->GetFriendPersonaName(SteamIDFromValue(steam_id)));
    });

nfr("has_friend", "steam_id,flags", "II", "B",
    "returns true if the given steam_id is a friend with the matching flags",
    [](StackPtr &, VM &vm, Value steam_id, Value friend_flags) {
        return STEAM_BOOL_VALUE(vm, steam->HasFriend(SteamIDFromValue(steam_id), friend_flags.intval()));
    });

nfr("net_identity", "", "", "S",
    "returns the steam identity for this"
    " user. This same ID will be used for connecting to peers, sending messages,"
    " etc.",
    [](StackPtr &, VM &vm) {
        #ifdef PLATFORM_STEAMWORKS
            if (steam) {
                SteamNetworkingIdentity identity{};
                SteamNetworkingSockets()->GetIdentity(&identity);
                return Value(GetIdentityString(vm, identity));
            }
        #endif
        return Value(vm.NewString("none"));
    });

nfr("net_identity_from_steam_id", "steam_id", "I", "S",
    "returns a network identity for the given steam id", [](StackPtr &, VM &vm, Value steam_id) {
        #ifdef PLATFORM_STEAMWORKS
            if (steam) {
                SteamNetworkingIdentity identity{};
                identity.SetSteamID((uint64)steam_id.ival());
                return Value(GetIdentityString(vm, identity));
            }
        #endif
        return Value(vm.NewString("none"));
    });

nfr("net_identity_ipaddr", "", "", "S",
    "returns the steam identity for this user, but as an IP address instead of a"
    " Steam ID. This can be useful when connecting two clients that are using the"
    " same Steam ID (e.g. on the same computer). NOTE: this will only work if "
    " p2p_listen_ipaddr is called first.",
    [](StackPtr &, VM &vm) {
        return STEAM_STRING_VALUE(vm, steam->GetIpAddressIdentity());
    });

nfr("p2p_set_send_buffer_size", "size", "I", "B", "set the upper limit of pending bytes to be sent",
    [](StackPtr &, VM &, Value size) {
        auto ok = STEAM_BOOL_VALUE(steam->SetGlobalConfigValue(k_ESteamNetworkingConfig_SendBufferSize, size.intval()));
        return Value(ok);
    });

nfr("p2p_set_recv_buffer_size", "size", "I", "",
    "upper limit on total size in bytes of received messages that will be buffered waiting "
    "to be processed by the application",
    [](StackPtr &, VM &, Value size) {
        auto ok = STEAM_BOOL_VALUE(steam->SetGlobalConfigValue(k_ESteamNetworkingConfig_RecvBufferSize, size.intval()));
        return Value(ok);
    });

nfr("p2p_get_connection_status", "ident", "S", "IFFFFFFIIIII",
    "receive realtime connection status info. Returned values are: ping, local quality, "
    "remote quality, out packets/sec, out bytes/sec, in packets/sec, in bytes/sec, send rate bytes/sec, "
    "pending unreliable packets, pending reliable packets, sent unACKed reliable packets, and queue time in usec. "
    "See ISteamNetworkingSockets::GetConnectionRealTimeStatus() for more info.",
    [](StackPtr &sp, VM &) {
        #ifdef PLATFORM_STEAMWORKS
            if (steam) {
                auto ident = Pop(sp).sval()->strvnt();
                SteamNetConnectionRealTimeStatus_t status;
                steam->GetConnectionRealTimeStatus(ident, &status);
                Push(sp, Value(status.m_nPing));
                Push(sp, Value(status.m_flConnectionQualityLocal));
                Push(sp, Value(status.m_flConnectionQualityRemote));
                Push(sp, Value(status.m_flOutPacketsPerSec));
                Push(sp, Value(status.m_flOutBytesPerSec));
                Push(sp, Value(status.m_flInPacketsPerSec));
                Push(sp, Value(status.m_flInBytesPerSec));
                Push(sp, Value(status.m_nSendRateBytesPerSecond));
                Push(sp, Value(status.m_cbPendingUnreliable));
                Push(sp, Value(status.m_cbPendingReliable));
                Push(sp, Value(status.m_cbSentUnackedReliable));
                Push(sp, Value(status.m_usecQueueTime));
                return;
            }
        #endif
        Push(sp, Value(0));
        Push(sp, Value(0.0f));
        Push(sp, Value(0.0f));
        Push(sp, Value(0.0f));
        Push(sp, Value(0.0f));
        Push(sp, Value(0.0f));
        Push(sp, Value(0.0f));
        Push(sp, Value(0));
        Push(sp, Value(0));
        Push(sp, Value(0));
        Push(sp, Value(0));
        Push(sp, Value(0));
    });

nfr("p2p_listen", "", "", "B", "open a listen socket to receive new connections",
    [](StackPtr &, VM &) {
        return STEAM_BOOL_VALUE(steam->P2PListen());
    });

nfr("p2p_listen_ipaddr", "ipaddr", "S", "B", "open a listen socket for receive new IP address connections",
    [](StackPtr &, VM &, Value ipaddr) {
        return STEAM_BOOL_VALUE(steam->P2PListenIP(ipaddr.sval()->strvnt()));
    });

nfr("p2p_close_listen", "", "", "B", "close the listen socket and stop accepting new connections",
    [](StackPtr &, VM &) {
        return STEAM_BOOL_VALUE(steam->CloseListen());
    });

nfr("p2p_connect", "ident", "S", "B", "connect to a user with a given steam "
    "identity (e.g. \"steam:XXXX\") or ip address (e.g. \"ip:127.0.0.1:5105\") that "
    "has opened a listen socket",
    [](StackPtr &, VM &, Value ident) {
        return STEAM_BOOL_VALUE(steam->P2PConnect(ident.sval()->strvnt()));
    });

nfr("p2p_connect_ipaddr", "ipaddr", "S", "B", "connect to a user with a given IP address that has opened a listen socket",
    [](StackPtr &, VM &, Value ipaddr) {
        return STEAM_BOOL_VALUE(steam->P2PConnectIP(ipaddr.sval()->strvnt()));
    });

nfr("p2p_close_connection", "ident,linger", "SB", "B",
    "close a connection opened with p2p_connect(); if linger is true then the connection will "
    "remain open for a short time to finish pending messages",
    [](StackPtr &, VM &, Value ident, Value linger) {
        return STEAM_BOOL_VALUE(steam->P2PCloseConnection(ident.sval()->strvnt(), linger.intval()));
    });

nfr("p2p_get_connections", "", "", "S]", "get a list of the steam identites that are currently connected",
    [](StackPtr &, VM &vm) {
        auto *peers_vec = vm.NewVec(0, 0, TYPE_ELEM_VECTOR_OF_STRING);

        #ifdef PLATFORM_STEAMWORKS
            if (steam) {
                SteamNetworkingIdentity local_identity{};
                SteamNetworkingSockets()->GetIdentity(&local_identity);
                for (auto &peer: steam->peers) {
                    if (!peer.is_connected) continue;
                    peers_vec->Push(vm, vm.NewString(peer.ident));
                }
            }
        #endif  // PLATFORM_STEAMWORKS

        return Value(peers_vec);
    });

nfr("p2p_rename_peer", "ident,new_ident", "SS", "B", "use a different identifier"
    " for this peer. This can be useful when connecting multiple users using"
    " peer-to-peer, so you can have all peers in the network use the listen socket IP"
    " address and port as the identifier.",
    [](StackPtr &, VM &, Value ident, Value new_ident) {
        return STEAM_BOOL_VALUE(steam->RenamePeer(ident.sval()->strvnt(), new_ident.sval()->strvnt()));
    });

nfr("p2p_send_message", "ident,data,reliable", "SSB", "BI", "send a reliable message to a given steam identity",
    [](StackPtr &sp, VM &) {
        auto reliable = Pop(sp);
        auto data = Pop(sp);
        auto ident = Pop(sp);
        #ifdef PLATFORM_STEAMWORKS
            EResult result = k_EResultNone;
            auto ok = steam && steam->SendMessage(ident.sval()->strvnt(), data.sval()->strv(), reliable.intval(), &result);
            Push(sp, Value(ok));
            Push(sp, Value(result));
        #else
            (void)reliable;
            (void)data;
            (void)ident;
            Push(sp, Value(false));
            Push(sp, Value(0));
        #endif
    });

nfr("p2p_broadcast_message", "data,reliable", "SB", "B", "send a reliable message to all connected peers",
    [](StackPtr &, VM &, Value data, Value reliable) {
        return STEAM_BOOL_VALUE(steam->BroadcastMessage(data.sval()->strv(), reliable.intval()));
    });

nfr("p2p_receive_messages", "", "", "S]S]", "receive messages from all"
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
                    auto peer = steam->FindPeer(message->m_conn);  // TODO: optimize?
                    if (peer != steam->peers.end()) {
                        client_vec->Push(vm, vm.NewString(peer->ident));
                        data_vec->Push(vm, data);
                    }
                    message->Release();
                }
            }
        #endif  // PLATFORM_STEAMWORKS

        Push(sp, data_vec);
        Push(sp, client_vec);
    });

nfr("p2p_set_global_config_value", "enum,value", "II", "", "",
    [](StackPtr &, VM &, Value enum_, Value value) {
        return STEAM_BOOL_VALUE(steam->SetGlobalConfigValue(enum_.intval(), value.intval()));
    });

nfr("p2p_set_global_config_value", "enum,value", "IF", "", "",
    [](StackPtr &, VM &, Value enum_, Value value) {
        return STEAM_BOOL_VALUE(steam->SetGlobalConfigValue(enum_.intval(), value.fltval()));
    });

nfr("p2p_set_debug_output_level", "level", "I", "",
    "set the debug level for networking output.",
    [](StackPtr &, VM &, Value level) {
        #ifdef PLATFORM_STEAMWORKS
            steam->SetDebugOutputLevel(level.intval());
        #endif
        return NilVal();
    });

nfr("p2p_get_debug_output", "", "", "S]",
    "get the debug networking output.",
    [](StackPtr &, VM &vm) {
        auto *output_vec = vm.NewVec(0, 0, TYPE_ELEM_VECTOR_OF_STRING);

        #ifdef PLATFORM_STEAMWORKS
            if (steam) {
                for (auto &output : steam->GetDebugOutput()) {
                    output_vec->Push(vm, vm.NewString(output));
                }
                steam->debug_output.clear();
            }
        #endif  // PLATFORM_STEAMWORKS

        return Value(output_vec);
    });

nfr("lobby_create", "max_members", "I", "B",
    "create a new lobby that allows at most a given number of members; this lobby will be "
    "automatically joined. use lobby_get_created() to get the newly created lobby's steam id",
    [](StackPtr &, VM &, Value max_members) {
        return STEAM_BOOL_VALUE(steam->CreateLobby(k_ELobbyTypePublic, max_members.intval()));
    });

nfr("lobby_create", "lobby_type,max_members", "II", "B",
    "create a new lobby of a given lobby type that allows at most a given number "
    "of members; this lobby will be automatically joined. use lobby_get_created() "
    "to get the newly created lobby's steam id",
    [](StackPtr &, VM &, Value lobby_type, Value max_members) {
        return STEAM_BOOL_VALUE(steam->CreateLobby(lobby_type.intval(), max_members.intval()));
    });

nfr("lobby_get_created", "", "", "I",
    "get the steam id of the most recently created lobby",
    [](StackPtr &, VM &) {
        return STEAM_IINT_VALUE(IIntFromSteamID(steam->created_lobby));
    });

nfr("lobby_join", "steam_id", "I", "B", "join a lobby with the given steam id",
    [](StackPtr &, VM &, Value steam_id) {
        return STEAM_BOOL_VALUE(steam->JoinLobby(SteamIDFromValue(steam_id)));
    });

nfr("lobby_leave", "steam_id", "I", "B", "leave a lobby with the given steam id",
    [](StackPtr &, VM &, Value steam_id) {
        return STEAM_BOOL_VALUE(steam->LeaveLobby(SteamIDFromValue(steam_id)));
    });

nfr("lobby_set_joinable", "steam_id,joinable", "IB", "B",
    "mark a lobby as joinable; only works if you are the owner",
    [](StackPtr &, VM &, Value steam_id, Value joinable) {
        return STEAM_BOOL_VALUE(
            steam->SetLobbyJoinable(SteamIDFromValue(steam_id), joinable.intval()));
    });

nfr("lobby_get_joined", "", "", "I",
    "get a list of the most recent lobby joined with lobby_create() or lobby_join()",
    [](StackPtr &, VM &) {
        return STEAM_IINT_VALUE(IIntFromSteamID(steam->joined_lobby));
    });

nfr("lobby_get_all_joined", "", "", "I]",
    "get a list of all of the lobbies that have been joined with lobby_create() or lobby_join()",
    [](StackPtr &, VM &vm) {
        auto *lobbies_vec = vm.NewVec(0, 0, TYPE_ELEM_VECTOR_OF_INT);

        #ifdef PLATFORM_STEAMWORKS
            if (steam) {
                for (const auto &lobby_id : steam->joined_lobbies) {
                    lobbies_vec->Push(vm, (iint)lobby_id.ConvertToUint64());
                }
            }
        #endif  // PLATFORM_STEAMWORKS

        return Value(lobbies_vec);
    });

nfr("lobby_get_owner", "steam_id", "I", "I",
    "get the steam id of the owner of the given lobby",
    [](StackPtr &, VM &, Value steam_id) {
        return STEAM_IINT_VALUE(IIntFromSteamID(steam->GetLobbyOwner(SteamIDFromValue(steam_id))));
    });

nfr("lobby_request_data", "steam_id", "I", "B",
    "refresh data for a given lobby; it is not necessary to call this for any lobby that you have "
    "joined",
    [](StackPtr &, VM &, Value steam_id) {
        return STEAM_BOOL_VALUE(steam->RequestLobbyData(SteamIDFromValue(steam_id)));
    });

nfr("lobby_get_data", "steam_id,key", "IS", "S",
    "get the matching value for a given key stored on this lobby; if the key has not been set then "
    "the result is an empty string",
    [](StackPtr &, VM &vm, Value steam_id, Value key) {
        return STEAM_STRING_VALUE(
            vm, steam->GetLobbyData(SteamIDFromValue(steam_id), key.sval()->strvnt().c_str()));
    });

nfr("lobby_get_all_data", "steam_id", "I", "S]S]", "get all key-value pairs stored on this lobby",
    [](StackPtr &sp, VM &vm) {
        auto vsteam_id = Pop(sp);
        (void)vsteam_id;
        auto *key_vec = vm.NewVec(0, 0, TYPE_ELEM_VECTOR_OF_STRING);
        auto *value_vec = vm.NewVec(0, 0, TYPE_ELEM_VECTOR_OF_STRING);

        #ifdef PLATFORM_STEAMWORKS
            if (steam) {
                CSteamID steam_id = SteamIDFromValue(vsteam_id);
                auto count = steam->GetLobbyDataCount(steam_id);
                for (int i = 0; i < count; ++i) {
                    char key[k_nMaxLobbyKeyLength];
                    char value[k_cubChatMetadataMax];
                    auto ok =
                        steam->GetLobbyDataByIndex(steam_id, i, key, sizeof(key), value, sizeof(value));
                    if (ok) {
                        key_vec->Push(vm, vm.NewString(key));
                        value_vec->Push(vm, vm.NewString(value));
                    }
                }
            }
        #endif  // PLATFORM_STEAMWORKS

        Push(sp, key_vec);
        Push(sp, value_vec);
    });

nfr("lobby_set_data", "steam_id,key,value", "ISS", "B",
    "set a key-value pair for this lobby; only works if you are the owner",
    [](StackPtr &, VM &, Value steam_id, Value key, Value value) {
        return STEAM_BOOL_VALUE(steam->SetLobbyData(SteamIDFromValue(steam_id),
                                                    key.sval()->strvnt().c_str(),
                                                    value.sval()->strvnt().c_str()));
    });

nfr("lobby_delete_data", "steam_id,key", "IS", "B",
    "delete a key-value pair for this lobby; only works if you are the owner",
    [](StackPtr &, VM &, Value steam_id, Value key) {
        return STEAM_BOOL_VALUE(
            steam->DeleteLobbyData(SteamIDFromValue(steam_id), key.sval()->strvnt().c_str()));
    });

nfr("lobby_get_num_members", "steam_id", "I", "I", "get the number of members in this lobby",
    [](StackPtr &, VM &, Value steam_id) {
        return STEAM_INT_VALUE(steam->GetNumLobbyMembers(SteamIDFromValue(steam_id)));
    });

nfr("lobby_get_members", "steam_id", "I", "I]",
    "get the steam ids of all members in this lobby; only works if you have joined the lobby",
    [](StackPtr &, VM &vm, Value vsteam_id) {
        auto *members_vec = vm.NewVec(0, 0, TYPE_ELEM_VECTOR_OF_INT);

        #ifdef PLATFORM_STEAMWORKS
            if (steam) {
                CSteamID steam_id = SteamIDFromValue(vsteam_id);
                int num_members = steam->GetNumLobbyMembers(steam_id);
                for (int i = 0; i < num_members; ++i) {
                    auto id = steam->GetLobbyMemberByIndex(steam_id, i);
                    if (id.IsValid()) {
                        // NOTE: Can't use id.Render() because it seems to be unimplemented?
                        members_vec->Push(vm, (iint)id.ConvertToUint64());
                    }
                }
            }
        #endif  // PLATFORM_STEAMWORKS

        return Value(members_vec);
    });

nfr("lobby_request_add_numerical_filter", "key,value,cmp", "SII", "B",
    "add a numerical filter for the next lobby request",
    [](StackPtr &, VM &, Value key, Value value, Value cmp) {
        return STEAM_BOOL_VALUE(steam->AddRequestLobbyListNumericalFilter(
            key.sval()->strvnt().c_str(), value.intval(), (ELobbyComparison)cmp.intval()));
    });

nfr("lobby_request_add_string_filter", "key,value,cmp", "SSI", "B",
    "add a string filter for the next lobby request",
    [](StackPtr &, VM &, Value key, Value value, Value cmp) {
        return STEAM_BOOL_VALUE(steam->AddRequestLobbyListStringFilter(
            key.sval()->strvnt().c_str(), value.sval()->strvnt().c_str(),
            (ELobbyComparison)cmp.intval()));
    });

nfr("lobby_request_add_result_count_filter", "count", "I", "B",
    "add a result count limit for the next lobby request", [](StackPtr &, VM &, Value count) {
        return STEAM_BOOL_VALUE(steam->AddRequestLobbyListResultCountFilter(count.intval()));
    });

nfr("lobby_request_list", "", "", "B",
    "request a list of lobbies that match the current set of filters; this function completes "
    "asynchronously, call lobby_request_is_ready() to determine when it is ready and "
    "lobby_request_get_lobbies() to get the results",
    [](StackPtr &, VM &) {
        return STEAM_BOOL_VALUE(steam->RequestLobbyList());
    });

nfr("lobby_request_list_reset", "", "", "",
    "clear the list of matched lobbies, so lobby_request_is_ready() returns false",
    [](StackPtr &, VM &) {
        return STEAM_BOOL_VALUE(steam->LobbyListReset());
    });

nfr("lobby_request_is_ready", "", "", "B",
    "returns true when a call to lobby_request_list() has finished",
    [](StackPtr &, VM &) {
        return STEAM_BOOL_VALUE(steam->LobbyListIsReady());
    });

nfr("lobby_request_get_lobbies", "", "", "I]",
    "returns the list of matched lobbies when lobby_request_list() has finished",
    [](StackPtr &, VM &vm) {
        auto *lobbies_vec = vm.NewVec(0, 0, TYPE_ELEM_VECTOR_OF_INT);

        #ifdef PLATFORM_STEAMWORKS
            if (steam && steam->matched_lobbies >= 0) {
                for (int i = 0; i < steam->matched_lobbies; ++i) {
                    auto id = steam->GetLobbyByIndex(i);
                    if (id.IsValid()) {
                        // NOTE: Can't use id.Render() because it seems to be unimplemented?
                        lobbies_vec->Push(vm, (iint)id.ConvertToUint64());
                    }
                }
            }
        #endif  // PLATFORM_STEAMWORKS

        return Value(lobbies_vec);
    });

nfr("lobby_get_game_server", "lobby_id", "I", "I", "get the game server associated with this lobby",
    [](StackPtr &, VM &, Value lobby_id) {
        return STEAM_IINT_VALUE(
            IIntFromSteamID(steam->GetLobbyGameServer(SteamIDFromValue(lobby_id))));
    });

nfr("lobby_set_game_server", "lobby_id,server_id", "II", "B",
    "set the game server associated with this lobby; only works if you are the owner",
    [](StackPtr &, VM &, Value lobby_id, Value server_id) {
        return STEAM_BOOL_VALUE(
            steam->SetLobbyGameServer(SteamIDFromValue(lobby_id), SteamIDFromValue(server_id)));
    });

nfr("workshop_open_page", "", "", "",
    "opens this game's steam workshop page in the steam overlay web browser, where the user"
    " can browse and (un)subscribe to content. Use workshop_sync() to get subscribed content"
    " into the game.",
    [](StackPtr &, VM &) {
        #ifdef PLATFORM_STEAMWORKS
            if (steam) steam->WorkshopOpenPage();
        #endif
    });

nfr("workshop_open_item_page", "fileid", "I", "",
    "opens the steam workshop page of the given workshop item in the steam overlay, e.g. to"
    " let the user view content they just uploaded.",
    [](StackPtr &, VM &, Value fileid) {
        #ifdef PLATFORM_STEAMWORKS
            if (steam) steam->WorkshopOpenItemPage(fileid.ival());
        #else
            (void)fileid;
        #endif
        return NilVal();
    });

nfr("workshop_open_legal_agreement", "", "", "",
    "opens the steam workshop legal agreement page in the steam overlay. A user must accept"
    " this agreement (once) before workshop content they upload can become publicly visible,"
    " see workshop_upload_status().",
    [](StackPtr &, VM &) {
        #ifdef PLATFORM_STEAMWORKS
            if (steam) steam->WorkshopOpenLegalAgreement();
        #endif
    });

nfr("workshop_sync", "dest_dir,own_subdirs", "SB", "IS]",
    "makes subscribed workshop content progress towards being downloaded, and copies each"
    " item's files into dest_dir (relative to the main write dir, or absolute) once its"
    " download completes. If own_subdirs is true, each item's files go into a subdirectory of"
    " dest_dir named after its workshop file id, otherwise all items' files are copied into"
    " dest_dir together. Files keep the names they were uploaded with either way (so with"
    " own_subdirs false, identically named files from different items overwrite each other)."
    " Call this regularly (e.g. once per frame, needs steam.update()) while you want content"
    " to come in. Returns a status: -1 = error copying files, 0 = all subscribed content is"
    " installed and copied (nothing left to do), 1 = downloads still in progress, 2 = done"
    " but some items failed to download. Second return value is the files copied by this"
    " call (full paths), so the game can pick up new/updated content the moment it arrives.",
    [](StackPtr &sp, VM &vm) {
        auto own_subdirs = Pop(sp);
        auto dest_dir = Pop(sp);
        auto *copied_vec = vm.NewVec(0, 0, TYPE_ELEM_VECTOR_OF_STRING);
        int status = -1;
        #ifdef PLATFORM_STEAMWORKS
            if (steam) {
                vector<string> copied_files;
                status = steam->WorkshopSync(dest_dir.sval()->strv(), own_subdirs.True(),
                                             copied_files);
                for (auto &f : copied_files) copied_vec->Push(vm, vm.NewString(f));
            }
        #else
            (void)own_subdirs;
            (void)dest_dir;
        #endif
        Push(sp, Value(status));
        Push(sp, Value(copied_vec));
    });

nfr("workshop_upload_start", "content_folder,title,description,metadata,changenote,"
    "preview_image,existing_fileid", "SSSSSSI", "B",
    "starts uploading workshop content for the current app; this is async, poll"
    " workshop_upload_status() (with steam.update() running) for the result."
    " content_folder (relative to the main write dir, or absolute) must contain exactly the"
    " file(s) making up this single workshop item, e.g. a staging folder the level editor"
    " saved the level file into; the original filenames inside it are preserved for"
    " downloaders (see workshop_sync()). title and description are user visible on the"
    " workshop. metadata is NOT user visible and may be empty; use it to store things the"
    " game may want to query later without downloading, e.g. the original filename or"
    " version info (max 5000 chars). changenote may be empty. preview_image optionally"
    " (empty = none) points to a jpg/png/gif under 1MB shown on the workshop page."
    " Pass 0 as existing_fileid to publish a NEW item, or the file id of previously uploaded"
    " content to update it; an update first verifies the item is owned by the current steam"
    " user and fails otherwise (status -2). On a successful new upload, the game MUST store"
    " the file id returned by workshop_upload_status() alongside the content (e.g. in the"
    " level file or its own sidecar/registry) and pass it back in here when the user uploads"
    " a new version of the same content, since steam has no other way to know it's the same"
    " item. Empty title/description/metadata leave the existing values unchanged when"
    " updating. Returns false if the upload could not be started (e.g. one is already in"
    " progress).",
    [](StackPtr &sp, VM &) {
        auto existing_fileid = Pop(sp);
        auto preview_image = Pop(sp);
        auto changenote = Pop(sp);
        auto metadata = Pop(sp);
        auto description = Pop(sp);
        auto title = Pop(sp);
        auto content_folder = Pop(sp);
        bool ok = false;
        #ifdef PLATFORM_STEAMWORKS
            if (steam) {
                ok = steam->WorkshopUploadStart(
                    content_folder.sval()->strv(), title.sval()->strv(),
                    description.sval()->strv(), metadata.sval()->strv(),
                    changenote.sval()->strv(), preview_image.sval()->strv(),
                    existing_fileid.ival());
            }
        #else
            (void)existing_fileid;
            (void)preview_image;
            (void)changenote;
            (void)metadata;
            (void)description;
            (void)title;
            (void)content_folder;
        #endif
        Push(sp, Value(ok));
    });

nfr("workshop_upload_status", "", "", "IIBII",
    "gets the status of the upload started with workshop_upload_start(): 0 = no upload"
    " started, 1 = in progress, 2 = done succesfully, -1 = failed, -2 = failed because the"
    " existing item is owned by a different steam user. Further return values are: the"
    " workshop file id (once known, 0 before; on success the game must store this with the"
    " content, see workshop_upload_start()), whether the user still needs to accept the"
    " steam workshop legal agreement (if true, the item remains hidden until they do; send"
    " them there with workshop_open_legal_agreement() or workshop_open_item_page()), and"
    " upload progress as bytes processed and bytes total (both may be 0 in early stages).",
    [](StackPtr &sp, VM &) {
        int status = 0;
        iint file_id = 0;
        bool needs_legal = false;
        iint processed = 0;
        iint total = 0;
        #ifdef PLATFORM_STEAMWORKS
            if (steam) status = steam->WorkshopUploadStatus(file_id, needs_legal, processed, total);
        #endif
        Push(sp, Value(status));
        Push(sp, Value(file_id));
        Push(sp, Value(needs_legal));
        Push(sp, Value(processed));
        Push(sp, Value(total));
    });

}  // AddSteam
