
for 60:
    log_frame()

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

