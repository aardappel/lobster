include testing
include floodfillpath
include vec

run_test("flood"):
    assert (floodfillpath(xy_1i * 20, xy_1i * 5) v: true)
