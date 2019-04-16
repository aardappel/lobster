import testing
import std
import vec

run_test("gradient"):

    rnd_seed(0)

    let N = 100
    let f = 2.0 / N

    def gradient_descent(x, d, mu, n_epochs):
        let y = map(N): 0.0
        var w0 = 0.0
        var w1 = 0.0
        for(n_epochs):
            var grad0 = 0.0
            var grad1 = 0.0
            for(N) i:
                let err = f * (d[i] - y[i])
                grad0 += err
                grad1 += err * x[i]
            w0 += mu * grad0
            w1 += mu * grad1
            for(N) i:
                y[i] = w0 + w1 * x[i]
        return [ w0, w1 ]

    let sigma = 0.1
    let mu = 0.001
    let n_epochs = 1000

    let x = map(N): 0.0
    let d = map(N): 0.0

    for(N) i:
        x[i] = f * float(i)
        d[i] = 3.0 + 2.0 * x[i] + sigma * rnd_gaussian()

    assert abs(gradient_descent(x, d, mu, n_epochs)[0] - 2.428925605728) < 0.01
