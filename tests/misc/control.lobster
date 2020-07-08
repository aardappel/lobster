
def F():
    for(10) i:
        if i:
            return G()
        else:
            return G()
        if i:
            break
    switch 1:
        case 1: print 1
        case 2: print 2
        case 3: print 3
        case 4: print 4
    switch "a":
        case "b": print 3
        case "c": print 4
    switch 1:
        case 1..3, 5..7, 9: print 1
        case 111: print 2  // force non-jumptable impl
        default: print 3
    return 1

def G():
    var n = 10
    while n--:
        if n:
            break
        return 2 from F
    return H() + 1

def H():
    if []:
        return 4
    return 3 from G

assert F() == 5
