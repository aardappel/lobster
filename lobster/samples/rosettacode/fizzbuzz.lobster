include "std.lobster"

forbias(100, 1) i:
    fb := (i % 3 == 0 and "fizz" or "") +
          (i % 5 == 0 and "buzz" or "")
    print fb.length and fb or "" + i
