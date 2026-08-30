// Part of the relative import test: a file whose last (and only) statement is
// an import, which the lexer ends without generating a linefeed for. The
// import is also a duplicate of one done elsewhere, so nothing gets included.
import .inner2
