import std
import vec
import color
import gl
import imgui

let MAX_PLAYERS = 4
let HOST_PORT = 5101
let CLIENT_LISTEN_PORT = 5102

fatal(gl.window("Shooter Tutorial Co-op", 640, 480))

check(gl.set_font_name("data/fonts/US101/US101.ttf"), "can\'t load font!")

im.init(false, im.config_docking_enable, 3.0)
check(im.add_font("data/fonts/Droid_Sans/DroidSans.ttf", 20.0), "can\'t load font!")

let steamerr = steam.init(0, true, true)
if steamerr == 0:
    print "Steam init failed"
elif steamerr < 0:
    // Not started from Steam, exit this instance, Steam will launch again.
    // This only happens if there wasn't a steam.appid.txt file.
    print "Relaunching from Steam..."
    return
else:
    print "Welcome Steam user {steam.username()}"

var local_ident = ""
var host_ident = ""
var is_host = false

enum GameState:
    GS_INIT
    GS_CREATING_LOBBY
    GS_JOINING_LOBBY
    GS_IN_LOBBY
    GS_STARTING_IPADDR_GAME
    GS_HOST_WAITING_FOR_PEERS
    GS_PEER_WAITING_FOR_HOST
    GS_PEER_WAITING_FOR_PEERS
    GS_START_GAME
    GS_PLAYING
    GS_RESET

abstract class message

class m_start_game : message
    attribute serializable
    players:[string]

class m_host_welcome : message
    attribute serializable
    new_ident:string

class m_peer_joined : message
    attribute serializable
    new_ident:string

class m_end_game : message
    attribute serializable

class m_player_pos : message
    attribute serializable
    ident:string
    pos:float2
    dir:float2

class m_player_died : message
    attribute serializable
    ident:string

class m_new_enemy : message
    attribute serializable
    id:int
    pos:float2

class m_enemy_hit : message
    attribute serializable
    player:string
    id:int

class m_enemy_died : message
    attribute serializable
    killed_by:string
    id:int

class m_new_bullet : message
    attribute serializable
    ident:string
    pos:float2
    dir:float2


let worldsize = 20.0

class player:
    ident:string
    pos=float2_0
    dir=float2_0
    alive=true
    score=0

let playerspeed = 10
var localplayer = player{ local_ident }
var players = [ localplayer ]
var playerhealth = 0.0

class bullet:
    ident:string
    pos:float2
    dir:float2

let firerate = 0.1
let bulletspeed = 15
var bullets = []
var lastbullet = gl.time()

class enemy:
    id:int
    pos:float2
    hp:int

var nextenemyid = 0
var enemyrate = 1.0
let enemyspeed = 3
let enemymaxhp = 5
var enemies = []
var lastenemy = gl.time()

var game_state = GS_INIT

struct message_ident:
    msg:message
    ident:string

struct rename:
    old_ident:string
    new_ident:string

// Networking
var incoming = []::message_ident
var outgoing = []::message

def send(msg:message):
    incoming.push(message_ident { msg, local_ident })
    outgoing.push(msg)

def renderpointytriangle(pos, dir):
    gl.translate pos:
        gl.rotate_z dir:
            gl.polygon([ float2 { -0.5, 0.5 }, float2_x, float2 { -0.5, -0.5 } ])

while gl.frame():
    // Update networking
    steam.update()
    let peers = steam.p2p_get_connections()
    def has_peer(id):
        return id == local_ident or (find(peers): _ == id) != -1

    let incoming_data, incoming_idents = steam.p2p_receive_messages()
    // Deserialize the incoming messages
    for(incoming_data) bin, i:
        let lbval, lberr = lobster_binary_to_value(typeof [message], bin)
        assert lbval
        assert not lberr
        let ident = incoming_idents[i]
        for(lbval) msg:
            incoming.push(message_ident { msg, ident })
    // Handle the incoming messages
    // Queue up the renames since all messages will still be using the old name.
    let rename_list = []
    for(incoming) msg_peer:
        let msg = msg_peer.msg
        let ident = msg_peer.ident
        switch msg:
            case m_start_game:
                localplayer = player { local_ident }
                print "start_game: {msg.players}"
                var ok = true
                players = map(msg.players) c:
                    if c == local_ident:
                        localplayer
                    else:
                        // Only connect peer <-> peer, host is already connected to each peer.
                        if not is_host and c != host_ident:
                            // Pick one of the peers to make a connection to the other.
                            guard local_ident < c:
                                print "Skipping connection to \"{c}\""
                            print "Connecting to \"{c}\""
                            if c.find_string("ip:") == 0:
                                let ipaddr = c.substring(3, -1)
                                check(steam.p2p_connect_ipaddr(ipaddr), "Unable to connect to peer \'{c}\'")
                            elif c.find_string("steamid:") == 0:
                                check(steam.p2p_connect(c), "Unable to connect to peer \'{c}\'")
                            else:
                                ok = false
                            // Notify peers of our correct identity.
                            send(m_peer_joined { local_ident })
                        player { c }
                if ok:
                    game_state = GS_PEER_WAITING_FOR_PEERS
                else:
                    game_state = GS_RESET
            case m_peer_joined:
                guard ident != local_ident
                guard (is_host and game_state == GS_STARTING_IPADDR_GAME) or
                      (not is_host and game_state == GS_PEER_WAITING_FOR_PEERS):
                    print "Ignoring message: {msg}"
                rename_list.push(rename { ident, msg.new_ident })
                // Let the newly joined peer know the correct name of the host too.
                send(m_host_welcome { local_ident })
            case m_host_welcome:
                guard not is_host
                guard ident != local_ident
                guard game_state == GS_PEER_WAITING_FOR_HOST:
                    print "Ignoring message: {msg}"
                rename_list.push(rename { ident, msg.new_ident })
                host_ident = msg.new_ident
            case m_end_game:
                game_state = GS_RESET
            case m_player_pos:
                let pi = find(players): _.ident == msg.ident
                guard pi != -1
                players[pi].pos = msg.pos
                players[pi].dir = msg.dir
            case m_player_died:
                let pi = find(players): _.ident == msg.ident
                guard pi != -1
                players[pi].alive = false
            case m_new_enemy:
                enemies.push(enemy { msg.id, msg.pos, enemymaxhp })
                enemyrate *= 0.999
            case m_enemy_hit:
                let ei = find(enemies): _.id == msg.id
                guard ei != -1
                let e = enemies[ei]
                e.hp--
                if is_host and e.hp <= 0:
                    e.hp = 0
                    send(m_enemy_died { msg.player, msg.id })
            case m_enemy_died:
                let pi = find(players): _.ident == msg.killed_by
                guard pi != -1
                players[pi].score++
            case m_new_bullet:
                bullets.push(bullet { msg.ident, msg.pos, msg.dir })
    // Rename any peers that requested it.
    for(rename_list) r:
        print "renaming {r.old_ident} -> {r.new_ident}"
        steam.p2p_rename_peer(r.old_ident, r.new_ident)
    incoming = []

    gl.clear(color_black)
    gl.color(color_white)

    if game_state == GS_PLAYING:
        // Filter out any players that we lost connections to
        players = filter(players): has_peer(_.ident)
        // Quit if we lose connection to the host
        guard (is_host or has_peer(host_ident)) and gl.button("escape") != 1:
            game_state = GS_RESET

        let alive_players = filter(players): _.alive
        if alive_players.length:
            gl.translate gl.window_size() / 2:
                let scale = gl.window_size().y / worldsize
                gl.scale(scale)

                if localplayer.alive:
                    let dir = float2 { (gl.button("d") >= 1) - (gl.button("a") >= 1),
                                (gl.button("s") >= 1) - (gl.button("w") >= 1) }
                    let newpos = localplayer.pos + normalize(dir) * gl.delta_time() * playerspeed
                    if not any(abs(newpos) > float(gl.window_size()) / scale / 2.0):
                        localplayer.pos = newpos
                    localplayer.dir = normalize(gl.local_mouse_pos(0) - localplayer.pos)

                    if lastbullet < gl.time():
                        send(m_new_bullet { local_ident, localplayer.pos, localplayer.dir })
                        lastbullet += firerate

                    // just send our position to peers, since it was already updated above
                    outgoing.push(m_player_pos { local_ident, localplayer.pos, localplayer.dir })

                if is_host and lastenemy < gl.time():
                    send(m_new_enemy { nextenemyid++, sincos(rnd(360)) * worldsize * 2.0 })
                    lastenemy += enemyrate

                for(enemies) e:
                    // Move toward the closest alive player
                    let pi = find_best<float>(alive_players) p:
                        -magnitude(p.pos - e.pos)
                    assert pi != -1
                    let playerdir = normalize(alive_players[pi].pos - e.pos)
                    e.pos += playerdir * gl.delta_time() * enemyspeed
                    for(bullets) b:
                        if magnitude(b.pos - e.pos) < 1.0:
                            if b.ident == local_ident:
                                send(m_enemy_hit { local_ident, e.id })
                            b.pos = float2_x * worldsize * 10.0
                    // Check whether the enemy hit us (each player checks for themselves)
                    if magnitude(localplayer.pos - e.pos) < 1.0:
                        playerhealth -= gl.delta_time() * 50.0
                        if playerhealth <= 0.0:
                            playerhealth = 0.0
                            send(m_player_died { local_ident })
                    gl.color lerp(color_red, color_blue, div(e.hp, enemymaxhp)):
                        renderpointytriangle(e.pos, playerdir)

                enemies = filter(enemies): _.hp

                for(bullets) b:
                    b.pos += b.dir * gl.delta_time() * bulletspeed
                    gl.translate b.pos:
                        gl.color color_yellow:
                            gl.circle(0.2, 20)

                bullets = filter(bullets) b:
                    magnitude(b.pos) < worldsize * 2.0

                gl.translate gl.local_mouse_pos(0):
                    gl.line_mode 1:
                        gl.color color_grey:
                            gl.circle(0.5, 20)

                for(players) p:
                    guard p.alive
                    renderpointytriangle(p.pos, p.dir)
        elif is_host:
            send(m_end_game {})
        gl.set_font_size(32)
        gl.text("health: {ceiling(playerhealth)} - score: {localplayer.score}")
    else:
        im.frame():
            im.window("Shooter Tutorial Co-op", im.window_always_autoresize):
                let joined_lobby_ids = steam.lobby_get_all_joined()
                let joined_lobby_id = if joined_lobby_ids.length:
                    joined_lobby_ids[0]
                else:
                    0
                switch game_state:
                    case GS_INIT:
                        if im.button("Create Lobby"):
                            check(steam.p2p_listen(), "Unable to open listen socket")
                            steam.lobby_create(MAX_PLAYERS)
                            game_state = GS_CREATING_LOBBY
                        if im.button("Find Lobby"):
                            check(steam.p2p_listen(), "Unable to open listen, ip_addr socket")
                            steam.lobby_request_add_string_filter("game", "tut_net", 0)
                            steam.lobby_request_list()
                        im.separator()
                        if im.button("Host Game by IP Address..."):
                            im.popup_open("host_ip_game")
                        im.popup("host_ip_game"):
                            static local_ip_addr = "[::1]:{HOST_PORT}"
                            local_ip_addr = im.input_text("Host IP Address", local_ip_addr)
                            if im.button("Host"):
                                check(steam.p2p_listen_ipaddr(local_ip_addr), "Unable to open listen socket")
                                local_ident = steam.net_identity_ipaddr()
                                game_state = GS_STARTING_IPADDR_GAME
                                is_host = true
                        if im.button("Join Game at IP Address..."):
                            im.popup_open("join_ip_game")
                        im.popup("join_ip_game"):
                            static remote_ip_addr = "[::1]:{HOST_PORT}"
                            static local_ip_addr = "[::1]:{CLIENT_LISTEN_PORT}"
                            remote_ip_addr = im.input_text("Remote IP Address", remote_ip_addr)
                            local_ip_addr = im.input_text("Local IP Address", local_ip_addr)
                            im.tooltip("Can be any number, but must be unused on your local system")
                            if im.button("Join"):
                                check(steam.p2p_listen_ipaddr(local_ip_addr), "Unable to open listen socket")
                                check(steam.p2p_connect_ipaddr(remote_ip_addr), "Unable to open connection to host")
                                local_ident = steam.net_identity_ipaddr()
                                host_ident = remote_ip_addr
                                // Notify host of our remote identity (what other peers can use to join.)
                                print "My new identity is: {local_ident}"
                                send(m_peer_joined { local_ident })
                                game_state = GS_PEER_WAITING_FOR_HOST
                                is_host = false
                        im.separator()
                        if im.button("Quit"):
                            return
                        let matched = steam.lobby_request_get_lobbies()
                        let matched_idx = im.listbox("Matched Lobbies", map(matched): number_to_string(_, 16, 0), -2, 5)
                        if matched_idx >= 0 and im.button("Join Lobby"):
                            steam.lobby_join(matched[matched_idx])
                            steam.lobby_request_list_reset()
                            game_state = GS_JOINING_LOBBY
                    case GS_CREATING_LOBBY:
                        if joined_lobby_id != 0:
                            steam.lobby_set_data(joined_lobby_id, "game", "tut_net")
                            game_state = GS_IN_LOBBY
                            is_host = true
                    case GS_JOINING_LOBBY:
                        if joined_lobby_id != 0:
                            game_state = GS_IN_LOBBY
                            is_host = false
                    case GS_IN_LOBBY:
                        guard joined_lobby_id != 0:
                            game_state = GS_RESET
                        if im.button("Leave Lobby"):
                            steam.lobby_leave(joined_lobby_id)
                            game_state = GS_RESET
                        let members = steam.lobby_get_members(joined_lobby_id)
                        im.listbox("Members ({members.length})", map(members): number_to_string(_, 16, 0), -2, MAX_PLAYERS)
                        if is_host:
                            if im.button("Start Game"):
                                steam.lobby_set_joinable(joined_lobby_id, false)
                                steam.lobby_set_game_server(joined_lobby_id, steam.get_steam_id())
                                game_state = GS_HOST_WAITING_FOR_PEERS
                                host_ident = steam.net_identity()
                                players = map(members): player { steam.net_identity_from_steam_id(_) }
                        else:
                            let game_server_id = steam.lobby_get_game_server(joined_lobby_id)
                            if game_server_id != 0:
                                let net_ident = steam.net_identity_from_steam_id(game_server_id)
                                check(steam.p2p_connect(net_ident), "Unable to open connection to host")
                                host_ident = net_ident
                                game_state = GS_PEER_WAITING_FOR_HOST
                    case GS_STARTING_IPADDR_GAME:
                        let connections = steam.p2p_get_connections()
                        im.text("Network ID: {local_ident}")
                        im.listbox("Players ({connections.length})", connections, -2, MAX_PLAYERS)
                        if is_host:
                            if im.button("Start Game"):
                                game_state = GS_HOST_WAITING_FOR_PEERS
                                host_ident = steam.net_identity()
                                players = map(connections): player { _ }
                        if im.button("Leave Lobby"):
                            game_state = GS_RESET
                    case GS_HOST_WAITING_FOR_PEERS:
                        im.text("Waiting for peers...")
                        let ready = all(map(players): has_peer(_.ident))
                        guard ready
                        // Let all peers know that the game is starting
                        let peers_copy = copy(peers)
                        peers_copy.push(local_ident)
                        send(m_start_game { peers_copy })
                        game_state = GS_START_GAME
                    case GS_PEER_WAITING_FOR_HOST:
                        im.text("Waiting for host...")
                    case GS_PEER_WAITING_FOR_PEERS:
                        im.text("Waiting for peers...")
                        let ready = all(map(players): has_peer(_.ident))
                        guard ready
                        game_state = GS_START_GAME
                    case GS_START_GAME:
                        steam.lobby_leave(joined_lobby_id)
                        playerhealth = 100.0
                        bullets = []
                        lastbullet = gl.time()
                        enemyrate = 1.0
                        enemies = []
                        lastenemy = gl.time()
                        game_state = GS_PLAYING
                    case GS_PLAYING:
                        assert false
                    case GS_RESET:
                        for(peers) peer:
                            steam.p2p_close_connection(peer, false)
                        is_host = false
                        host_ident = ""
                        game_state = GS_INIT
                        check(steam.p2p_close_listen(), "Unable to close listen socket")

    // Send the outgoing messages
    if peers.length and outgoing.length:
        if steam.p2p_broadcast_message(lobster_value_to_binary(outgoing), true):
            // Only clear the outgoing messages if they were sent successfully.
            outgoing = []