
for 60:
    gl_log_frame(1.0 / 60.0)

    def a():
        var v ?= 0
        return v++

    def b():
        let i = a()
        if a() % 4:
            var v, w ?= 0, 1
            v++
            print v + " | " + w
        else:
            nil
            print "--- " + i

    b()

