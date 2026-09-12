import std


// Lifetime / reference-count analysis edge cases: re-assignments from owned
// containers, temporaries, swaps, and self-referential updates.

class DLBox:
    s:string

class DLDispatchEntry:
    name:string

class DLDispatchOwner:
    entry:DLDispatchEntry

class DLDispatchBase:
    def read(entry:DLDispatchEntry, owner:DLDispatchOwner) -> string:
        return entry.name

class DLDispatchReader : DLDispatchBase
    def read(entry:DLDispatchEntry, owner:DLDispatchOwner) -> string:
        return entry.name

class DLDispatchWriter : DLDispatchBase
    def read(entry:DLDispatchEntry, owner:DLDispatchOwner) -> string:
        owner.entry = DLDispatchEntry { "replacement" + "" }
        return entry.name

class DLDispatchInheritedWriter : DLDispatchWriter

// Declare the mutating subclass before the other subclass this time. Assigning
// the parameter makes it owning in every implementation of this dispatch.
class DLDispatchOwningBase:
    def read(entry:DLDispatchEntry, owner:DLDispatchOwner) -> string:
        return entry.name

class DLDispatchOwningWriter : DLDispatchOwningBase
    def read(entry:DLDispatchEntry, owner:DLDispatchOwner) -> string:
        owner.entry = DLDispatchEntry { "replacement" + "" }
        let name = entry.name
        entry = owner.entry
        return name + entry.name

class DLDispatchOwningReader : DLDispatchOwningBase
    def read(entry:DLDispatchEntry, owner:DLDispatchOwner) -> string:
        return entry.name

def lifetime_tests():
    suite("lifetime")

    // Re-assigning a var from a vector element (borrow then replace).
    let sv = [ "a", "b" ]
    var k = sv[0]
    k = sv[1]
    test_eq(k, "b")
    let iv = [ 1, 2 ]
    var n = iv[0]
    n = iv[1]
    test_eq(n, 2)

    // Overwriting vector elements that hold references.
    sv[0] = sv[1]
    test_eq(sv[0], "b")
    sv[1] = "c" + sv[0]
    test_eq(sv[1], "cb")

    // Swapping references through multi-assignment.
    var x1 = "one"
    var x2 = "two"
    x1, x2 = x2, x1
    test_eq(x1, "two")
    test_eq(x2, "one")

    // String temporaries in expressions, and building in a loop.
    var acc = ""
    for(3) i:
        acc += "{i}," + ""
    test_eq(acc, "0,1,2,")
    test_eq(("a" + "b") + ("c" + "d"), "abcd")

    // Passing owned values through functions that return them.
    def passthru(s:string): return s
    let pt = passthru("p" + "q")
    test_eq(pt, "pq")
    test_eq(passthru(passthru(pt)), "pq")

    // Recursion keeps these as actual function exits. Their only return transfers a local
    // or an assigned parameter, including all reference slots of a struct.
    def owned_local(depth:int) -> string:
        let value = if depth: owned_local(depth - 1) + "!" else: "local" + string(depth)
        return value
    def owned_arg(value:string, depth:int) -> string:
        if depth: value = owned_arg(value, depth - 1) + "!"
        return value
    struct ReturnedRefs:
        text:string
        items:[int]
    def owned_struct(depth:int) -> ReturnedRefs:
        let value = if depth: owned_struct(depth - 1) else: ReturnedRefs { "struct" + string(depth), [depth] }
        return value
    test_eq(owned_local(3), "local0!!!")
    test_eq(owned_arg("arg" + string(0), 3), "arg0!!!")
    let returned = owned_struct(3)
    test_eq(returned.text, "struct0")
    test_seq(returned.items, [0])

    // Inlined returns keep their ordinary owned result. Later copies can overwrite the
    // shared local, while results from earlier copies remain alive in the vector.
    def inline_owned(i:int):
        let value = [i]
        return value
    let saved = [[-1]]
    for(4) i:
        saved.push(inline_owned(i))
        saved.push(inline_owned(i + 10))
    for(4) i:
        test_seq(saved[i * 2 + 1], [i])
        test_seq(saved[i * 2 + 2], [i + 10])

    // The same reference passed to multiple parameters of one call.
    def both_lens(a, b): return a.length + b.length
    let shared_s = "xyz"
    test_eq(both_lens(shared_s, shared_s), 6)
    let shared_v = [ 1, 2 ]
    test_eq(both_lens(shared_v, shared_v), 4)

    // Overwriting a vector element with itself.
    let selfsv = [ "only" ]
    selfsv[0] = selfsv[0]
    test_eq(selfsv[0], "only")

    // An object stays alive through a var when its vector slot goes away.
    let keeper = [ [ 42 ] ]
    let kept = keeper[0]
    keeper.pop()
    test_eq(keeper.length, 0)
    test_seq(kept, [ 42 ])

    // Conditionally returning one of two owned locals.
    def pick_of_two(c):
        let o1 = "one" + ""
        let o2 = "two" + ""
        return if c: o1 else: o2
    test_eq(pick_of_two(1), "one")
    test_eq(pick_of_two(0), "two")

    // Structs holding refs copied between vars: both copies usable.
    struct SRef:
        s:string
    var sr1 = SRef { "orig" }
    let sr2 = sr1
    sr1 = SRef { "new" }
    test_eq(sr1.s, "new")
    test_eq(sr2.s, "orig")

    // Ownership specialization: the same function borrows a var in one call
    // and consumes an owned temporary in another.
    def measure(x): return x.length
    let borrowed = [ 1, 2 ]
    test_eq(measure([ 1, 2, 3 ]), 3)
    test_eq(measure(borrowed), 2)

    // Arguments that are assigned to become owned by the function: the
    // caller's value is unaffected.
    def reassign_arg(y):
        y = y + "!"
        return y
    let unchanged = "o"
    test_eq(reassign_arg(unchanged), "o!")
    test_eq(unchanged, "o")

    // Fields holding references, overwritten repeatedly.
    let box = DLBox { "init" }
    for(3) i:
        box.s = "v{i}"
    test_eq(box.s, "v2")
    box.s = box.s + "!"
    test_eq(box.s, "v2!")

    // A vector appended to itself (copies its elements, must not loop).
    var selfv = [ 1, 2 ]
    selfv = append(selfv, selfv)
    test_seq(selfv, [ 1, 2, 1, 2 ])

    // Anonymous values consumed immediately.
    test_eq([ "x", "y" ].pop(), "y")
    test_eq((DLBox { "temp" }).s, "temp")

    // Conditional ownership: both branches produce a reference.
    def pick_ref(c):
        return if c: "yes" + "" else: "no" + ""
    test_eq(pick_ref(1), "yes")
    test_eq(pick_ref(0), "no")

    // References captured as free variables and reassigned inside blocks.
    var freev = "f0"
    do():
        freev = "f1"
    test_eq(freev, "f1")
    for(2) i:
        if i == 1: freev = "f2"
    test_eq(freev, "f2")

    // Only a later dispatch implementation overwrites the field supplying the
    // borrowed argument. The caller must retain it across the whole call.
    let dispatchers = [ DLDispatchBase {}, DLDispatchReader {}, DLDispatchWriter {},
                        DLDispatchInheritedWriter {} ]
    let owner1 = DLDispatchOwner { DLDispatchEntry { "first" + "" } }
    test_eq(dispatchers[2].read(owner1.entry, owner1), "first")
    test_eq(owner1.entry.name, "replacement")

    // Reuse the same dispatch with a different borrowed field and an inherited
    // implementation, then with an implementation that leaves the field alone.
    let owner2 = DLDispatchOwner { DLDispatchEntry { "second" + "" } }
    test_eq(dispatchers[3].read(owner2.entry, owner2), "second")
    test_eq(owner2.entry.name, "replacement")
    let owner3 = DLDispatchOwner { DLDispatchEntry { "third" + "" } }
    test_eq(dispatchers[1].read(owner3.entry, owner3), "third")
    test_eq(owner3.entry.name, "third")

    def read_dispatch(receiver:DLDispatchBase, owner:DLDispatchOwner) -> string:
        return receiver.read(owner.entry, owner)
    let owner4 = DLDispatchOwner { DLDispatchEntry { "fourth" + "" } }
    test_eq(read_dispatch(dispatchers[2], owner4), "fourth")
    test_eq(read_dispatch(dispatchers[2], owner4), "replacement")

    // A reused dispatch must adjust owning arguments before replaying the
    // subclass's writes, just as it does when first checking that subclass.
    let owning_dispatchers = [ DLDispatchOwningBase {}, DLDispatchOwningWriter {},
                               DLDispatchOwningReader {} ]
    let owner5 = DLDispatchOwner { DLDispatchEntry { "fifth" + "" } }
    test_eq(owning_dispatchers[1].read(owner5.entry, owner5), "fifthreplacement")
    let owner6 = DLDispatchOwner { DLDispatchEntry { "sixth" + "" } }
    test_eq(owning_dispatchers[1].read(owner6.entry, owner6), "sixthreplacement")

lifetime_tests()
