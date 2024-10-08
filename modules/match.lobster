// match.lobster: pattern matching functionality similar to regexps, but more generally applicable
// as it works on vectors of any kind of values, and can also easily be used with (unicode) strings.

import std

private var iterv = nil
private var i = 0

// match with a particular vector, all matching functions must be used inside of the function
def match_with(it, f):
    assert not iterv
    iterv = it
    i = 0
    let result = f()
    iterv = nil
    return result

def match_elem(f):     // match single element
    return i < iterv.length and f(iterv[i]) and ++i

def match_end(): return i == iterv.length

def matched(f):    // return the sub-sequence matched by f as vector or string
    let prev = i
    f()
    return slice(iterv, prev, i - prev)

// convenience functions:
def match(x):     return match_elem(): x == _
def match_not(x): return match_elem(): x != _

def match_any(f): while f(): nil

def match_sequence(v):
    let prev = i  // backtrack to the start if f fails
    if (exists(v): not match(_)):
        i = prev
        return false
    return true

if program_name() == "match.lobster" or program_name() == "modules/match.lobster":

    // example: match any number of A's, followed by either a B or C/D in sequence (and capture this
    // substring), followed by an E and then the end of the sequence:

    let r = match_with(string_to_unicode("AAAAAAABCDCDBCDBBBE")):
        match_any(): match('A')
        print unicode_to_string(matched():
            match_any(): match('B') or match_sequence(string_to_unicode("CD")))
        match('E') and match_end()

    // prints out: BCDCDBCDBBB

    assert r
