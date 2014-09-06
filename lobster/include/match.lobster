/*

match.lobster: pattern matching functionality similar to regexps, but more generally applicable as it works on
(unicode) strings, vectors of any kind of values, and even coroutines

example: match any amount of A's, followed by either a B or C/D in sequence (and capture this substring),
followed by an E and then the end of the sequence:

    r := match_with("AAAAAAABCDCDBCDBBBE"):
        match_any(): match('A')
        print(matched(): match_any(): match('B') | match_sequence("CD"))
        match('E') & match_end()

    assert(r)

prints out: BCDCDBCDBBB

You could replace the string by coroutine for("AAAAAAABCDCDBCDBBBE") and it would give the same result.

*/


private iterable := nil
private iterv := nil
private i := 0

private function start(ito, itv, f):
    iterable <- ito
    iterv <- itv
    i <- 0
    f()

private function grow(it): false
private function grow(it:coroutine):
    x := it.returnvalue
    it.active & it.resume & iterv.push(x)

private function isend(it:coroutine): !it.active
private function isend(it): i == iterv.length

private function repr(it:string, v): unicode2string(v)
private function repr(it,        v): v

// match with a particular iterable, all matching functions must be used inside of the function
// you can use nested match_with calls
function match_with(it:vector,    f): start(it, it, f)
function match_with(it:string,    f): start(it, string2unicode(it), f)
function match_with(it:coroutine, f): start(it, [], f)

function match_elem(f):     // match single element
    assert(iterv)           // if this fails, you're calling match() outside of a match_with() block
    (i < iterv.length | grow(iterable)) & f(iterv[i]) & ++i

function match_end(): isend(iterable)

function guard(f):          // backtrack to the start if f fails
    prev := i
    f() | ((i = prev) & false)

function matched(f):    // return the sub-sequence matched by f as vector or string
    prev := i
    f()
    repr(iterable, slice(iterv, prev, i - prev))

// convenience functions:
function match(x):     match_elem(): x == _
function match_not(x): match_elem(): x != _

function match_any(f): while(f()): nil

function match_sequence(s:string): match_sequence(string2unicode(s))
function match_sequence(v:vector): guard(): !exists(v): !match(_)
