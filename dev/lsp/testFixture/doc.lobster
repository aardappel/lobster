// Not about inc, a blank line is between.

// Returns x plus one.
// Never less.
def inc(x:int) -> int:
    return x + 1

class Counter:
    n:int
    // The count so far.
    def count():
        return n

//Only for this file.
private def hidden(): return 2

def bare(): return 3 // Not above it.

print(inc(1))
print(Counter { 1 }.count())
print(hidden())
print(bare())
