var x, y = if seconds_elapsed() > 10000.0:
    (1, 2)
else:
    (3, 4)

assert x == 3 and y == 4

x, y = if false:
    (1, 2)
else:
    (3, 4)

assert x == 3 and y == 4

assert (if seconds_elapsed() > 10000.0: (1, 2) else: (3, 4)) == 3

