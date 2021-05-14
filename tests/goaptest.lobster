import testing
import astar

run_test("goap"):

    enum resources:
        wolves
        skins
        money
        flour
        bread
        pizza
        hungry

    let goapactions = [
        goapaction { "kill wolf",  fn s: s[wolves] >= 1, fn s: s[wolves]--;   s[skins]++ },
        goapaction { "sell skin",  fn s: s[skins]  >= 1, fn s: s[money] += 2; s[skins]-- },
        goapaction { "buy pizza",  fn s: s[money]  >= 2, fn s: s[money] -= 2; s[pizza]++ },
        goapaction { "buy flour",  fn s: s[money]  >= 1, fn s: s[money]--;    s[flour]++ },
        goapaction { "bake bread", fn s: s[flour]  >= 1, fn s: s[flour]--;    s[bread]++ },
        goapaction {
            "eat",
            fn s: (s[bread] > 0 or s[pizza] > 0) and s[hungry] > 0,
            fn s: (if s[bread] > 0: s[bread]-- else: s[pizza]--); s[hungry]--
        }
    ]

    let goap_path =
        astar_goap(goapactions, [ 3, 0, 0, 0, 0, 0, 2 ]) state:
            state[hungry]
        fn s:
            s[hungry] <= 0

    assert goap_path.length == 9
    goap_path.pop()
    assert equal(goap_path.map() n: if n.delta: n.delta.name else: "",
        ["eat","buy pizza","sell skin","kill wolf","eat","buy pizza","sell skin","kill wolf"])

