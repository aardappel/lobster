//trace_bytecode(1)

struct test: [ a:int, b:string ]

function addrec(a, b): a + b

function add(a, b):
    addrec(a, b)

t := [ 1, "aaa" ]:test

i := and(t.a, 2)
l := length(t.b)
print(add(i, l))
print(add(3.0, 4.0))
//add(nil, nil)
