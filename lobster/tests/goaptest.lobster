include testing
include astar

run_test("goap"):

    enum wolves, skins, money, flour, bread, pizza, hungry

    goapactions := [
        goapaction { "kill wolf",  def(s): s[wolves] >= 1, def(s): s[wolves]--;   s[skins]++ },
        goapaction { "sell skin",  def(s): s[skins]  >= 1, def(s): s[money] += 2; s[skins]-- },
        goapaction { "buy pizza",  def(s): s[money]  >= 2, def(s): s[money] -= 2; s[pizza]++ },
        goapaction { "buy flour",  def(s): s[money]  >= 1, def(s): s[money]--;    s[flour]++ },
        goapaction { "bake bread", def(s): s[flour]  >= 1, def(s): s[flour]--;    s[bread]++ },
        goapaction {
            "eat",
            def(s): (s[bread] > 0 or s[pizza] > 0) and s[hungry] > 0,
            def(s): if s[bread] > 0: s[bread]-- else: s[pizza]--; s[hungry]--
        }
    ]

    goap_path :=
        astar_goap(goapactions, [ 3, 0, 0, 0, 0, 0, 2 ]) state:
            state[hungry]
        endcondition s:
            s[hungry] <= 0

    assert goap_path.length == 9
    goap_path.pop()
    assert equal(goap_path.map() n: if n.delta: n.delta.name else: "",
        ["eat","buy pizza","sell skin","kill wolf","eat","buy pizza","sell skin","kill wolf"])

