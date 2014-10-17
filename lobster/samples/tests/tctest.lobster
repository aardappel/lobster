//trace_bytecode(1)

function addrec(a, b): a + b

function add(a, b):
    addrec(a, b)

i := and(1, 2)
l := length("aaa")
print(add(i, l))
print(add(3.0, 4.0))
//add(nil, nil)
