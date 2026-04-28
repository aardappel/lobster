// Constants for use with SteamWorks steam functions.

namespace steam

// https://partner.steamgames.com/doc/api/ISteamMatchmaking#ELobbyType
enum lobby_type:
    lobby_type_private = 0
    lobby_type_friends_only = 1
    lobby_type_public = 2
    lobby_type_invisible = 3

// https://partner.steamgames.com/doc/api/ISteamFriends#EFriendFlags
enum friend_flag:
    friend_none                    = 0x00   // None.
    friend_blocked                 = 0x01   // Users that the// current user has blocked from contacting.
    friend_friendship_requested    = 0x02   // Users that have sent a friend invite to the current user.
    friend_immediate               = 0x04   // The current user's "regular" friends.
    friend_clan_member             = 0x08   // Users that are in one of the same (small) Steam groups as the current user.
    friend_on_game_server          = 0x10   // Users that are on the same game server; as set by SetPlayedWith.
    friend_requesting_friendship   = 0x80   // Users that the current user has sent friend invites to.
    friend_requesting_info         = 0x100  // Users that are currently sending additional info about themselves after a call to RequestUserInformation
    friend_ignored                 = 0x200  // Users that the current user has ignored from contacting them.
    friend_ignored_friend          = 0x400  // Users that have ignored the current user; but the current user still knows about them.
    friend_chat_member             = 0x1000 // Users in one of the same chats.

// https://partner.steamgames.com/doc/api/steam_api#EResult
enum result:
    result_none = 0                 // no result
    result_ok	= 1                   // success
    result_no_connection = 3        // no/failed network connection
    result_invalid_param = 8        // a parameter is incorrect
    result_invalid_state = 11       // called object was in an invalid state
    result_limit_exceeded = 25      // Too much of a good thing
    result_ignored = 41             // target is ignoring sender

enum global_config_value:
    /// [global float, 0--100] Randomly discard N pct of packets instead of sending/recv
    fake_packet_loss_send = 2
    fake_packet_loss_recv = 3
    /// [global int32].  Delay all outbound/inbound packets by N ms
    fake_packet_lag_send = 4
    fake_packet_lag_recv = 5
    /// [global float] 0-100 Percentage of packets we will add additional delay
    /// to (causing them to be reordered)
    fake_packet_reorder_send = 6
    fake_packet_reorder_recv = 7
    /// [global int32] Extra delay, in ms, to apply to reordered packets.
    fake_packet_reorder_time = 8
    /// [global float 0--100] Globally duplicate some percentage of packets we send
    fake_packet_dup_send = 26
    fake_packet_dup_recv = 27
    /// [global int32] Amount of delay, in ms, to delay duplicated packets.
    /// (We chose a random delay between 0 and this value)
    fake_packet_dup_time_max = 28

