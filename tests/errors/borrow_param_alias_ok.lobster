// Writes thru a borrowed parameter to a deeper field than the one borrowed, or to another
// field of the same object, are fine and must keep compiling.
// EXPECT-OK

class Node:
    val:int
    left:Node? = nil
    right:Node? = nil

def tree_insert(n:Node, v:int) -> void:
    if v < n.val:
        if n.left: tree_insert(n.left, v)
        else: n.left = Node { v }
    else:
        if n.right: tree_insert(n.right, v)
        else: n.right = Node { v }

def count(n:Node?) -> int:
    if not n: return 0
    return 1 + count(n.left) + count(n.right)

let root = Node { 50 }
for(20) i: tree_insert(root, (i * 37) % 100)
assert count(root) == 21

class Pair:
    a:string
    b:string

def set_b(p:Pair, s:string):
    p.b = s
    return p.a.length

let pr = Pair { "x", "yy" }
assert set_b(pr, pr.a) == 1
