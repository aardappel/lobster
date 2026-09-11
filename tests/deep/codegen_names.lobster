import std


// Source identifiers must stay distinct after becoming C locals or members.

def codegen_name_tests():
    suite("codegen names")

    // Field and nested-vector assignments require the compiler's lo/lidx/lvec
    // scratch locals in the same function as these legal source names.
    class NameBox:
        value:int
    let lo = NameBox { 1 }
    lo.value = 2
    test_eq(lo.value, 2)
    let lidx = rnd(1)
    let lvec = [[3]]
    lvec[int2 { lidx, 0 }] += 4
    test_eq(lvec[0][0], 7)

    // A reserved prefix needs changing at the front: repeatedly appending a
    // suffix to udt_ would never stop colliding with the reserved namespace.
    let udt_local = [ 11 ]
    let vudt_local = [ 19 ]
    udt_local[0]++
    test_eq(udt_local[0], 12)
    test_eq(vudt_local[0], 19)

    // A local must not hide the typedef needed by an uninlined multi-return
    // call in its scope. Recursion keeps this call out of the inliner.
    def named_pair(n:int) -> int, int:
        if n: return named_pair(n - 1)
        return 20, 21
    let ret_ii = [ 22 ]
    let first, second = named_pair(1)
    test_eq(first, 20)
    test_eq(second, 21)
    test_eq(ret_ii[0], 22)
    // Even a name without the separator can enter a reserved namespace when
    // suffixed: ret is reserved on its own, and ret_2 is reserved by prefix.
    let ret = [ 23 ]
    test_eq(ret[0], 23)
    // Sibling scopes reuse source names in one emitted function. The second
    // instance also needs a suffix that stays outside the elem_ namespace.
    for(1):
        let elem = [ 24 ]
        test_eq(elem[0], 24)
    for(1):
        let elem = [ 25 ]
        test_eq(elem[0], 25)

    struct NamePair:
        x:int
        text:string
    class NameMembers:
        top:int
        top_:int
        top_2:int
        typeinfo:int
        typeinfo_2:int
        refc:int
        refc_2:int
        pair:NamePair
        pair_x:int
        pair_text:string
        _hidden:int
        v_hidden:int
        udt_value:int
        vudt_value:int
    // Keywords/helper names, the object header, flattened nested fields, and
    // names already ending in the allocator's suffix all share one namespace.
    def check_members(m:NameMembers):
        test_seq([ m.top, m.top_, m.top_2, m.typeinfo, m.typeinfo_2,
                   m.refc, m.refc_2, m.pair.x, m.pair_x, m._hidden,
                   m.v_hidden, m.udt_value, m.vudt_value ],
                 [ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 ])
        test_eq(m.pair.text, "nested")
        test_eq(m.pair_text, "flat")
    let members = NameMembers {
        1, 2, 3, 4, 5, 6, 7, NamePair { 8, "nested" }, 9, "flat", 10, 11, 12, 13
    }
    check_members(members)
    members.top += 20
    members.typeinfo += 20
    members.refc += 20
    members.pair.text += "!"
    members.pair_text += "?"
    test_seq([ members.top, members.top_, members.top_2,
               members.typeinfo, members.typeinfo_2, members.refc, members.refc_2 ],
             [ 21, 2, 3, 24, 5, 26, 7 ])
    test_eq(members.pair.text, "nested!")
    test_eq(members.pair_text, "flat?")

    // Access via the base type must agree with construction via the subclass.
    class NameSub : NameMembers
        extra = 14
    let sub = NameSub {
        1, 2, 3, 4, 5, 6, 7, NamePair { 8, "nested" }, 9, "flat", 10, 11, 12, 13
    }
    check_members(sub)
    test_eq(sub.extra, 14)

    // Mixed structs use named C members when stored in a vector as well.
    struct NameRecord:
        pair:NamePair
        pair_x:int
        pair_text:string
        top:int
        top_:string
        typeinfo:int
        refc:int
    let records = [ NameRecord { NamePair { 15, "inner" }, 16, "outer", 17, "last", 18, 19 } ]
    records[0].pair.x += 20
    records[0].pair_text += "!"
    test_seq([ records[0].pair.x, records[0].pair_x, records[0].top,
               records[0].typeinfo, records[0].refc ], [ 35, 16, 17, 18, 19 ])
    test_eq(records[0].pair.text, "inner")
    test_eq(records[0].pair_text, "outer!")
    test_eq(records[0].top_, "last")

codegen_name_tests()
