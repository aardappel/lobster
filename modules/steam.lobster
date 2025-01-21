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
