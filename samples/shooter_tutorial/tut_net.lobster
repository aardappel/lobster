import std
import vec
import color
import gl

fatal(gl_window("Shooter Tutorial Co-op", 640, 480))

check(gl_set_font_name("data/fonts/US101/US101.ttf") and gl_set_font_size(32), "can\'t load font!")

let args = command_line_arguments()
check(args.length >= 1, "Run with first arg as \"host\" or \"peer\"")

let is_host =
    if args[0] == "host":
        true
    elif args[0] == "peer":
        false
    else:
        print "Run with first arg as \"host\" or \"peer\""
        set_exit_code(1)
        return

let steamerr = steam_init(0, true, true)
if steamerr == 0:
    print "Steam init failed"
elif steamerr < 0:
    // Not started from Steam, exit this instance, Steam will launch again.
    // This only happens if there wasn't a steam_appid.txt file.
    print "Relaunching from Steam..."
    return
else:
    print "Welcome Steam user {steam_username()}"

let ident = steam_net_identity()
check(steam_p2p_listen(), "Unable to open listen socket")

var host_ident = ""
if is_host:
    host_ident = ident
    print "Host: Your identity is = {ident}"
else:
    host_ident = args[1]
    check(args.length >= 2, "Peer needs remote identity argument")
    check(steam_p2p_connect(host_ident), "Unable to open connection to host")
    print "Peer: Your identity is = {ident}"

abstract class message

class m_start_game : message
    attribute serializable
    players:[string]

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
var localplayer = player{ ident }
var players = [ localplayer ]
var playerhealth = 0.0

class bullet:
    ident:string
    pos:float2
    dir:float2

let firerate = 0.1
let bulletspeed = 15
var bullets = []
var lastbullet = gl_time()

class enemy:
    id:int
    pos:float2
    hp:int

var nextenemyid = 0
var enemyrate = 1.0
let enemyspeed = 3
let enemymaxhp = 5
var enemies = []
var lastenemy = gl_time()

var playing = false
var waiting_for_players = false

// Networking
var incoming = []::message
var outgoing = []::message

def send(msg:message):
    incoming.push(msg)
    outgoing.push(msg)

def renderpointytriangle(pos, dir):
    gl_translate pos:
        gl_rotate_z dir:
            gl_polygon([ float2 { -0.5, 0.5 }, float2_x, float2 { -0.5, -0.5 } ])

while gl_frame() and gl_button("escape") != 1:
    // Update networking
    steam_update()
    let peers = steam_p2p_get_connections()
    def has_peer(id):
        return id == ident or (find(peers): _ == id) != -1

    let incoming_data = steam_p2p_receive_messages()
    // Deserialize the incoming messages
    for(incoming_data) bin:
        let lbval, lberr = lobster_binary_to_value(typeof [message], bin)
        assert lbval
        assert not lberr
        append_into(incoming, lbval)
    // Handle the incoming messages
    for(incoming) msg:
        switch msg:
            case m_start_game:
                localplayer = player { ident }
                players = map(msg.players) c:
                    if c == ident:
                        localplayer
                    else:
                        // Only connect peer <-> peer, host is already connected to each peer.
                        if not is_host and c != host_ident:
                            check(steam_p2p_connect(c), "Unable to connect to peer \'{c}\'")
                        player { c }
                waiting_for_players = true
            case m_end_game:
                playing = false
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
    incoming = []

    gl_clear(color_black)
    gl_color(color_white)

    if playing:
        // Filter out any players that we lost connections to
        players = filter(players): has_peer(_.ident)
        // Quit if we lose connection to the host
        check(is_host or has_peer(host_ident), "Lost connection to the host")

        let alive_players = filter(players): _.alive
        if alive_players.length:
            gl_translate float(gl_window_size()) / 2.0:
                let scale = gl_window_size().y / worldsize
                gl_scale(scale)

                if localplayer.alive:
                    let dir = float2 { (gl_button("d") >= 1) - (gl_button("a") >= 1),
                                (gl_button("s") >= 1) - (gl_button("w") >= 1) }
                    let newpos = localplayer.pos + normalize(dir) * gl_delta_time() * playerspeed
                    if not any(abs(newpos) > float(gl_window_size()) / scale / 2.0):
                        localplayer.pos = newpos
                    localplayer.dir = normalize(gl_local_mouse_pos(0) - localplayer.pos)

                    if lastbullet < gl_time():
                        send(m_new_bullet { ident, localplayer.pos, localplayer.dir })
                        lastbullet += firerate

                    // just send our position to peers, since it was already updated above
                    outgoing.push(m_player_pos { ident, localplayer.pos, localplayer.dir })

                if is_host and lastenemy < gl_time():
                    send(m_new_enemy { nextenemyid++, sincos(rnd(360)) * worldsize * 2.0 })
                    lastenemy += enemyrate

                for(enemies) e:
                    // Move toward the closest alive player
                    let pi = find_best<float>(alive_players) p:
                        -magnitude(p.pos - e.pos)
                    assert pi != -1
                    let playerdir = normalize(alive_players[pi].pos - e.pos)
                    e.pos += playerdir * gl_delta_time() * enemyspeed
                    for(bullets) b:
                        if magnitude(b.pos - e.pos) < 1.0:
                            if b.ident == ident:
                                send(m_enemy_hit { ident, e.id })
                            b.pos = float2_x * worldsize * 10.0
                    // Check whether the enemy hit us (each player checks for themselves)
                    if magnitude(localplayer.pos - e.pos) < 1.0:
                        playerhealth -= gl_delta_time() * 50.0
                        if playerhealth <= 0.0:
                            playerhealth = 0.0
                            send(m_player_died { ident })
                    gl_color lerp(color_red, color_blue, div(e.hp, enemymaxhp)):
                        renderpointytriangle(e.pos, playerdir)

                enemies = filter(enemies): _.hp

                for(bullets) b:
                    b.pos += b.dir * gl_delta_time() * bulletspeed
                    gl_translate b.pos:
                        gl_color color_yellow:
                            gl_circle(0.2, 20)

                bullets = filter(bullets) b:
                    magnitude(b.pos) < worldsize * 2.0

                gl_translate gl_local_mouse_pos(0):
                    gl_line_mode 1:
                        gl_color color_grey:
                            gl_circle(0.5, 20)

                for(players) p:
                    guard p.alive
                    renderpointytriangle(p.pos, p.dir)
        elif is_host:
            send(m_end_game {})
    elif waiting_for_players:
        // We're ready if we have a connection to each of the other players
        let ready = all(map(players): has_peer(_.ident))
        if ready:
            playerhealth = 100.0
            bullets = []
            lastbullet = gl_time()
            enemyrate = 1.0
            enemies = []
            lastenemy = gl_time()
            waiting_for_players = false
            playing = true

        let msg = "waiting for all players to be ready..."
        gl_translate float(gl_window_size() - gl_text_size(msg)) / 2.0:
            gl_text(msg)
    else:
        let msg = if is_host:
            "press space to play! ({peers.length + 1} players)"
        else:
            "waiting for host to start game..."
        gl_translate float(gl_window_size() - gl_text_size(msg)) / 2.0:
            gl_text(msg)
        if is_host and gl_button("space") == 1:
            let peers_copy = copy(peers)
            peers_copy.push(ident)
            send(m_start_game { peers_copy })
    gl_text("health: {ceiling(playerhealth)} - score: {localplayer.score}")

    // Send the outgoing messages
    steam_p2p_broadcast_message(lobster_value_to_binary(outgoing))
    outgoing = []