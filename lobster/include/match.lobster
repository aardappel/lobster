// match.lobster: pattern matching functionality similar to regexps, but more generally applicable
// as it works on vectors of any kind of values, and can also easily be used with (unicode) strings.

include std

private iterv := nil
private i := 0

// match with a particular vector, all matching functions must be used inside of the function
def match_with(it, f):
    iterv = it
    i = 0
    return f()

def match_elem(f):     // match single element
    iv := assert iterv  // if this fails, you're calling match() outside of a match_with() block
    return i < iv.length and f(iv[i]) and ++i

def match_end(): return i == iterv.length

def matched(f):    // return the sub-sequence matched by f as vector or string
    prev := i
    f()
    return slice(iterv, prev, i - prev)

// convenience functions:
def match(x):     return match_elem(): x == _
def match_not(x): return match_elem(): x != _

def match_any(f): while f(): nil

def match_sequence(v):
    prev := i  // backtrack to the start if f fails
    if (exists(v): !match(_)):
        i = prev
        return false
    return true

if program_name() == "match.lobster" or program_name() == "include/match.lobster":

    // example: match any number of A's, followed by either a B or C/D in sequence (and capture this
    // substring), followed by an E and then the end of the sequence:

    r := match_with(assert string_to_unicode("AAAAAAABCDCDBCDBBBE")):
        match_any(): match('A')
        print unicode_to_string(matched():
            match_any(): match('B') or match_sequence(assert string_to_unicode("CD")))
        match('E') and match_end()

    // prints out: BCDCDBCDBBB

    assert r
