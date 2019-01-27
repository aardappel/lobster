include testing
include std
include vec

run_test("gradient"):

    rnd_seed(0)

    N :== 100
    f :== 2.0 / N

    def gradient_descent(x, d, mu, n_epochs):
        y := map(N): 0.0
        w0 := 0.0
        w1 := 0.0
        for(n_epochs):
            grad0 := 0.0
            grad1 := 0.0
            for(N) i:
                err := f * (d[i] - y[i])
                grad0 += err
                grad1 += err * x[i]
            w0 += mu * grad0
            w1 += mu * grad1
            for(N) i:
                y[i] = w0 + w1 * x[i]
        return [ w0, w1 ]

    sigma :== 0.1
    mu :== 0.001
    n_epochs :== 1000

    x := map(N): 0.0
    d := map(N): 0.0

    for(N) i:
        x[i] = f * float(i)
        d[i] = 3.0 + 2.0 * x[i] + sigma * rnd_gaussian()

    assert abs(gradient_descent(x, d, mu, n_epochs)[0] - 2.428925605728) < 0.01
