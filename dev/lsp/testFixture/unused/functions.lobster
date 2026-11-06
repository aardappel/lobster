let shown = 1
let _hidden = 2
let never = 3

def called(a:int) -> int: return a + shown
def uncalled(b,
             c:string):
    return b

enum level:
    low
    high

print called(low)
