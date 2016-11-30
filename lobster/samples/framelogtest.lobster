
for 60:
    gl_logframe(1.0 / 60.0)

    def a():
        v ?= 0
        v++

    def b():
        i := a()
        if a() % 4:
            v, w ?= 0, 1
            v++
            print v + " | " + w
        else:
            nil
            print "--- " + i

    b()

