import testing
import vec

// A spring system that moves verts until they're evenly spaced, then computes polygons from them.

run_test("springs"):

    rnd_seed(0)

    class triangle:
        verts:[float2]
        neighbors:[int]

    class node:
        t1:triangle
        t2:triangle

    var verts = []::float2
    var edges = []::[int]
    var quads = []::[int]

    def wiggleverts():
        let disps = map(verts): float2_0
        for(verts) v:
            for(verts) w, i:
                let tow = w - v
                let dist = magnitude(tow)
                if(dist < 1.0 and v != w):
                    disps[i] += tow * (1.0 / dist) * (1.0 - dist)
        var totaldisp = 0.0
        for(disps) d, i:
            verts[i] += d
            totaldisp += magnitude(d)
        return totaldisp

    let wsize = int2_1 * 9
    verts = []
    edges = []
    forxy(wsize) p:
        verts.push(float(p) + rnd_float2() - float(wsize) / 2.0)
    while(wiggleverts() > 1.0): nil
    struct pedge:
        i1:int
        i2:int
        dist:float
    var pedges = []
    for(verts) v, i:
        for(verts) w, j:
            if(i < j):
                let dist = magnitude(w - v)
                if(dist < 2.0):
                    pedges.push(pedge { i, j, dist })
    pedges = qsort(pedges) (a, b): a.dist < b.dist
    let edgedistnocross = pedges[0].dist * sqrt(2.0)
    var vnbs = map(verts): []
    for(pedges) pe:
        if(pe.dist < edgedistnocross):
            edges.push([ pe.i1, pe.i2 ])
            vnbs[pe.i1].push(pe.i2)
            vnbs[pe.i2].push(pe.i1)
    pedges = []
    vnbs = map(vnbs) (nbl, i):
        qsort(nbl) (a, b):
            atan2(verts[a] - verts[i]) < atan2(verts[b] - verts[i])
    let tris = []::[int]
    quads = []
    for(verts.length) i:
        for(vnbs[i]) j:
            let shape = [ i, j ]
            var p = i
            while(shape.pop() != i):
                shape.push(j)
                let nbl = vnbs[j]
                let pidx = find(nbl): _ == p
                assert(pidx >= 0)
                p = j
                j = nbl[(pidx + 1) % nbl.length]
                shape.push(j)
            if shape.length <= 4:
                let start = reduce(shape): min(_a, _b)
                while(shape[0] != start):
                    shape.insert(0, shape.pop())
                if shape.length == 4:
                    if (not exists(quads): equal(_, shape)):
                        quads.push(shape)
                elif shape.length == 3:
                    if (not exists(tris): equal(_, shape)):
                        tris.push(shape)
    while(tris.length):
        var besti = -1
        var bestqc = -1
        for(tris) t, i:
            var qc = 0
            for(t) vi1:
                for(quads) q:
                    for(q) vi2:
                        if vi1 == vi2:
                            qc++
            if qc > bestqc:
                bestqc = qc
                besti = i
        let t1 = tris.remove(besti)
        var found = false
        for(t1) vi, vii:
            if not found:
                let ni = t1[(vii + 1) % 3]
                let li = t1[(vii + 2) % 3]
                for(tris) t:
                    if not found:
                        found = true
                        if t[0] == ni and t[1] == vi:
                            quads.push([ vi, t[2], ni, li ])
                        elif t[1] == ni and t[2] == vi:
                            quads.push([ vi, t[0], ni, li ])
                        elif t[2] == ni and t[0] == vi:
                            quads.push([ vi, t[1], ni, li ])
                        else:
                            found = false
                        if found:
                            tris.remove_obj(t)

