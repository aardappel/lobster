include testing
include vec

// A spring system that moves verts until they're evenly spaced, then computes polygons from them.

run_test("springs"):

    rnd_seed(0)

    class triangle { verts:[xy_f], neighbors:[int], used:int }
    class node { t1:triangle, t2:triangle }

    verts := []::xy_f
    edges := []::[int]
    quads := []::[int]

    def wiggleverts():
        disps := map(verts): xy_0
        for(verts) v:
            for(verts) (w, i):
                tow := w - v
                dist := magnitude(tow)
                if(dist < 1 and v != w):
                    disps[i] += tow * (1 / dist) * (1 - dist)
        totaldisp := 0.0
        for(disps) (d, i):
            verts[i] += d
            totaldisp += magnitude(d)
        return totaldisp

    wsize :== xy_1i * 9
    verts = []
    edges = []
    forxy(wsize) p:
        verts.push(float(p) + xy_rnd() - float(wsize) / 2.0)
    while(wiggleverts() > 1): nil
    struct pedge { i1:int, i2:int, dist:float }
    pedges := []
    for(verts) (v, i):
        for(verts) (w, j):
            if(i < j):
                dist := magnitude(w - v)
                if(dist < 2):
                    pedges.push(pedge { i, j, dist })
    pedges = qsort(pedges) (a, b): a.dist < b.dist
    edgedistnocross := pedges[0].dist * sqrt(2)
    vnbs := map(verts): []
    for(pedges) pe:
        if(pe.dist < edgedistnocross):
            edges.push([ pe.i1, pe.i2 ])
            vnbs[pe.i1].push(pe.i2)
            vnbs[pe.i2].push(pe.i1)
    pedges = []
    vnbs = map(vnbs) (nbl, i):
        qsort(nbl) (a, b):
            atan2(verts[a] - verts[i]) < atan2(verts[b] - verts[i])
    tris := []::[int]
    quads = []
    for(verts) (v, i):
        for(vnbs[i]) j:
            shape := [ i, j ]
            p := i
            while(shape.pop() != i):
                shape.push(j)
                nbl := vnbs[j]
                pidx := find(nbl): _ == p
                assert(pidx >= 0)
                p = j
                j = nbl[(pidx + 1) % nbl.length]
                shape.push(j)
            if shape.length <= 4:
                start := reduce(shape): min(_a, _b)
                while(shape[0] != start):
                    shape.insert(0, shape.pop())
                if shape.length == 4:
                    if (!exists(quads): equal(_, shape)):
                        quads.push(shape)
                else: if shape.length == 3:
                    if (!exists(tris): equal(_, shape)):
                        tris.push(shape)
    while(tris.length):
        besti := -1
        bestqc := -1
        for(tris) (t, i):
            qc := 0
            for(t) vi1:
                for(quads) q:
                    for(q) vi2:
                        if vi1 == vi2:
                            qc++
            if qc > bestqc:
                bestqc = qc
                besti = i
        t1 := tris.remove(besti)
        found := false
        for(t1) vi, vii:
            if !found:
                ni := t1[(vii + 1) % 3]
                li := t1[(vii + 2) % 3]
                for(tris) t:
                    if !found:
                        found = true
                        if t[0] == ni and t[1] == vi:
                            quads.push([ vi, t[2], ni, li ])
                        else: if t[1] == ni and t[2] == vi:
                            quads.push([ vi, t[0], ni, li ])
                        else: if t[2] == ni and t[0] == vi:
                            quads.push([ vi, t[1], ni, li ])
                        else:
                            found = false
                        if found:
                            tris.remove_obj(t)

