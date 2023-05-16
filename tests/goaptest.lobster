import testing
import astar

run_test("goap"):

    class resources:
        wolves:int
        skins:int
        money:int
        flour:int
        bread:int
        pizza:int
        hungry:int

    class KillWolf : goapaction
        def precondition(s):
            return s.wolves >= 1
        def effect(s):
            s.wolves--
            s.skins++

    class SellSkin : goapaction
        def precondition(s):
            return s.skins >= 1
        def effect(s):
            s.money += 2
            s.skins--

    class BuyPizza : goapaction
        def precondition(s):
            return s.money >= 2
        def effect(s):
            s.money -= 2
            s.pizza++

    class BuyFlour : goapaction
        def precondition(s):
            return s.money >= 1
        def effect(s):
            s.money--
            s.flour++

    class BakeBread : goapaction
        def precondition(s):
            return s.flour >= 1
        def effect(s):
            s.flour--
            s.bread++

    class Eat : goapaction
        def precondition(s): 
            return s.hungry > 0 and (s.bread > 0 or s.pizza > 0)
        def effect(s):
            if s.bread > 0:
                s.bread--
            else:
                s.pizza--
            s.hungry--

    let goapactions = [
        KillWolf {},
        SellSkin {},
        BuyPizza {},
        BuyFlour {},
        BakeBread {},
        Eat {},
    ]

    class spec_node = goap_node<resources>

    let goap_path =
        astar_goap(goapactions, resources { 3, 0, 0, 0, 0, 0, 2 }) state:
            state.hungry
        fn s:
            s.hungry <= 0

    assert goap_path.length == 9
    goap_path.pop()
    assert equal(goap_path.map() n: if n.delta: type_string(n.delta) else: "",
        ["Eat", "BuyPizza", "SellSkin", "KillWolf", "Eat", "BuyPizza", "SellSkin", "KillWolf"])

