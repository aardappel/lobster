-- Lua implementation of the cross-language benchmarks, see ../SPEC.md.
--
--     lua main.lua [repeats] [name]
--     luajit main.lua [repeats] [name]
--
-- Written to run unchanged on Lua 5.5 and LuaJIT 2.1, so it avoids the integer division and
-- bitwise operators LuaJIT does not have, and indexes its arrays from zero with the lengths
-- tracked by hand.

local sqrt, floor, clock = math.sqrt, math.floor, os.clock

-- ------------------------------------------------------------------ shared

-- The 32-bit LCG from SPEC.md. The largest intermediate is under 2^53, so it is exact whether
-- the numbers are doubles (LuaJIT) or integers (Lua 5.5).
local Rnd = {}
Rnd.__index = Rnd

local function new_rnd(seed)
  return setmetatable({ state = seed }, Rnd)
end

function Rnd:next()
  self.state = (self.state * 1664525 + 1013904223) % 4294967296
  return self.state
end

function Rnd:nextf()
  return self:next() * (1.0 / 4294967296.0)
end

function Rnd:range(a, b)
  return a + self:nextf() * (b - a)
end

function Rnd:below(n)
  return self:next() % n
end

-- ------------------------------------------------------------------ 1. nbody

local function bench_nbody()
  local N, STEPS, DT, EPS2 = 256, 1000, 0.001, 0.5
  local rnd = new_rnd(12345)
  local px, py, pz = {}, {}, {}
  local vx, vy, vz = {}, {}, {}
  local mass = {}
  for i = 0, N - 1 do
    px[i] = rnd:range(-40.0, 40.0)
    py[i] = rnd:range(-40.0, 40.0)
    pz[i] = rnd:range(-40.0, 40.0)
    vx[i] = rnd:range(-0.5, 0.5)
    vy[i] = rnd:range(-0.5, 0.5)
    vz[i] = rnd:range(-0.5, 0.5)
    mass[i] = rnd:range(0.2, 1.2)
  end
  local ax, ay, az = {}, {}, {}
  for _ = 1, STEPS do
    for i = 0, N - 1 do
      local pix, piy, piz = px[i], py[i], pz[i]
      local sx, sy, sz = 0.0, 0.0, 0.0
      for j = 0, N - 1 do
        local dx = px[j] - pix
        local dy = py[j] - piy
        local dz = pz[j] - piz
        local r2 = dx * dx + dy * dy + dz * dz + EPS2
        -- The j == i term has d == 0, so it contributes exactly zero.
        local f = mass[j] / (r2 * sqrt(r2))
        sx = sx + dx * f
        sy = sy + dy * f
        sz = sz + dz * f
      end
      ax[i], ay[i], az[i] = sx, sy, sz
    end
    for i = 0, N - 1 do
      local nvx = vx[i] + ax[i] * DT
      local nvy = vy[i] + ay[i] * DT
      local nvz = vz[i] + az[i] * DT
      vx[i], vy[i], vz[i] = nvx, nvy, nvz
      px[i] = px[i] + nvx * DT
      py[i] = py[i] + nvy * DT
      pz[i] = pz[i] + nvz * DT
    end
  end
  local e = 0.0
  for i = 0, N - 1 do
    e = e + 0.5 * mass[i] * (vx[i] * vx[i] + vy[i] * vy[i] + vz[i] * vz[i])
  end
  for i = 0, N - 1 do
    for j = 0, N - 1 do
      if j > i then
        local dx = px[j] - px[i]
        local dy = py[j] - py[i]
        local dz = pz[j] - pz[i]
        e = e - mass[i] * mass[j] / sqrt(dx * dx + dy * dy + dz * dz + EPS2)
      end
    end
  end
  return e
end

-- ------------------------------------------------------------------ 2. raytrace

local RT_W, RT_H, NSPHERES, MAX_DEPTH = 640, 480, 48, 3
local EPS = 0.0001
-- tan(27.5 degrees).
local HALF = 0.5205670505517462
local AMBIENT = 0.12

local function normalize(x, y, z)
  local l = sqrt(x * x + y * y + z * z)
  if l == 0.0 then return 0.0, 0.0, 0.0 end
  return x / l, y / l, z / l
end

local function hit_sphere(s, rox, roy, roz, rdx, rdy, rdz)
  local ocx = s.cx - rox
  local ocy = s.cy - roy
  local ocz = s.cz - roz
  local b = ocx * rdx + ocy * rdy + ocz * rdz
  local c = ocx * ocx + ocy * ocy + ocz * ocz - s.r * s.r
  local disc = b * b - c
  if not (disc > 0.0) then return -1.0 end
  local sd = sqrt(disc)
  local t = b - sd
  if t > EPS then return t end
  local t2 = b + sd
  if t2 > EPS then return t2 end
  return -1.0
end

local function any_hit(spheres, n, rox, roy, roz, rdx, rdy, rdz)
  for i = 0, n - 1 do
    if hit_sphere(spheres[i], rox, roy, roz, rdx, rdy, rdz) > 0.0 then return true end
  end
  return false
end

local trace

trace = function(spheres, ns, lx, ly, lz, rox, roy, roz, rdx, rdy, rdz, depth)
  local best_t = -1.0
  local nx, ny, nz, arr, agg, abb, refl = 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0
  for i = 0, ns - 1 do
    local s = spheres[i]
    local t = hit_sphere(s, rox, roy, roz, rdx, rdy, rdz)
    if t > 0.0 and (best_t < 0.0 or t < best_t) then
      best_t = t
      nx, ny, nz = normalize(rox + rdx * t - s.cx, roy + rdy * t - s.cy, roz + rdz * t - s.cz)
      arr, agg, abb = s.r0, s.g0, s.b0
      refl = s.refl
    end
  end
  if rdy < -1.0e-9 then
    local t = -roy / rdy
    if t > EPS and (best_t < 0.0 or t < best_t) then
      local hx = rox + rdx * t
      local hz = roz + rdz * t
      local shade = 0.35
      if (floor(hx) + floor(hz)) % 2 == 0 then shade = 0.9 end
      best_t = t
      nx, ny, nz = 0.0, 1.0, 0.0
      arr, agg, abb = shade, shade, shade
      refl = 0.25
    end
  end
  if not (best_t > 0.0) then
    local sk = rdy
    if sk < 0.0 then sk = 0.0 end
    local f = 0.4 + 0.6 * sk
    return 0.35 * f, 0.52 * f, 0.85 * f
  end
  local ox = rox + rdx * best_t + nx * EPS
  local oy = roy + rdy * best_t + ny * EPS
  local oz = roz + rdz * best_t + nz * EPS
  local lit = 1.0
  if any_hit(spheres, ns, ox, oy, oz, lx, ly, lz) then lit = 0.0 end
  local nl = nx * lx + ny * ly + nz * lz
  if nl < 0.0 then nl = 0.0 end
  local k = AMBIENT + nl * lit
  local sr, sg, sb = arr * k, agg * k, abb * k
  if not (refl > 0.0 and depth > 0) then return sr, sg, sb end
  local d2 = 2.0 * (nx * rdx + ny * rdy + nz * rdz)
  local rr, rg, rb = trace(spheres, ns, lx, ly, lz, ox, oy, oz,
                           rdx - nx * d2, rdy - ny * d2, rdz - nz * d2, depth - 1)
  local inv = 1.0 - refl
  return sr * inv + rr * refl, sg * inv + rg * refl, sb * inv + rb * refl
end

local function bench_raytrace()
  local rnd = new_rnd(98765)
  local spheres = {}
  for i = 0, NSPHERES - 1 do
    local cx = rnd:range(-6.0, 6.0)
    local cy = rnd:range(0.4, 4.5)
    local cz = rnd:range(-6.0, 6.0)
    local r = rnd:range(0.35, 0.9)
    local r0 = rnd:range(0.2, 1.0)
    local g0 = rnd:range(0.2, 1.0)
    local b0 = rnd:range(0.2, 1.0)
    local refl = 0.0
    if i % 3 == 0 then refl = 0.55 end
    spheres[i] = { cx = cx, cy = cy, cz = cz, r = r, r0 = r0, g0 = g0, b0 = b0, refl = refl }
  end
  local lx, ly, lz = normalize(-0.6, 1.0, 0.45)
  local ex, ey, ez = 0.0, 2.6, 9.0
  local fx, fy, fz = normalize(0.0 - ex, 1.2 - ey, 0.0 - ez)
  -- cross(fwd, up) with up = (0, 1, 0), then cross(right, fwd).
  local rx, ry, rz = normalize(fy * 0.0 - fz * 1.0, fz * 0.0 - fx * 0.0, fx * 1.0 - fy * 0.0)
  local ux = ry * fz - rz * fy
  local uy = rz * fx - rx * fz
  local uz = rx * fy - ry * fx
  local aspect = RT_W / RT_H
  local total = 0.0
  for pyi = 0, RT_H - 1 do
    local v = 1.0 - (pyi + 0.5) / RT_H * 2.0
    local cv = v * HALF
    for pxi = 0, RT_W - 1 do
      local u = (pxi + 0.5) / RT_W * 2.0 - 1.0
      local cu = u * aspect * HALF
      local dx, dy, dz = normalize(fx + rx * cu + ux * cv,
                                   fy + ry * cu + uy * cv,
                                   fz + rz * cu + uz * cv)
      local cr, cg, cb = trace(spheres, NSPHERES, lx, ly, lz, ex, ey, ez, dx, dy, dz, MAX_DEPTH)
      total = total + (cr + cg + cb)
    end
  end
  return total
end

-- ------------------------------------------------------------------ 3. interp

local IP_DEPTH, IP_ROUNDS, NVARS = 14, 1500, 8
-- Node kinds, dispatched by a tag rather than by a metatable method, which is how Lua code
-- over a tree like this is normally written.
local K_CONST, K_VAR, K_NEG, K_ADD, K_SUB, K_MUL = 1, 2, 3, 4, 5, 6

local function build(rnd, d)
  if d == 0 then
    local k = rnd:below(3)
    if k == 0 then return { t = K_CONST, v = rnd:range(-1.0, 1.0) } end
    if k == 1 then return { t = K_VAR, i = rnd:below(NVARS) } end
    return { t = K_NEG, a = build(rnd, 0) }
  end
  -- Multiplication only near the leaves, so the value of the tree stays finite.
  local lim = 2
  if d <= 3 then lim = 3 end
  local k = rnd:below(lim)
  local a = build(rnd, d - 1)
  local b = build(rnd, d - 1)
  if k == 0 then return { t = K_ADD, a = a, b = b } end
  if k == 1 then return { t = K_SUB, a = a, b = b } end
  return { t = K_MUL, a = a, b = b }
end

local eval

eval = function(e, env)
  local t = e.t
  if t == K_CONST then return e.v end
  if t == K_VAR then return env[e.i] end
  if t == K_NEG then return -eval(e.a, env) end
  if t == K_ADD then return eval(e.a, env) + eval(e.b, env) end
  if t == K_SUB then return eval(e.a, env) - eval(e.b, env) end
  return eval(e.a, env) * eval(e.b, env)
end

local function bench_interp()
  local rnd = new_rnd(4242)
  local tree = build(rnd, IP_DEPTH)
  local env = {}
  local total = 0.0
  for r = 0, IP_ROUNDS - 1 do
    for k = 0, NVARS - 1 do
      env[k] = ((r * 7 + k * 13) % 101) / 50.0 - 1.0
    end
    total = total + eval(tree, env)
  end
  return total
end

-- ------------------------------------------------------------------ 4. sort

local SORT_N, SORT_ROUNDS = 400000, 6

-- The comparison arrives as a function value, which is what Lua code taking one looks like.
local function sort_rec(keys, ids, lo, hi, less)
  if hi - lo <= 1 then return end
  local pk, pi = keys[lo], ids[lo]
  local sp = lo + 1
  local ep = hi
  while sp < ep do
    local ck, ci = keys[sp], ids[sp]
    if less(ck, ci, pk, pi) then
      keys[sp - 1], ids[sp - 1] = keys[sp], ids[sp]
      sp = sp + 1
    else
      ep = ep - 1
      keys[sp], ids[sp] = keys[ep], ids[ep]
      keys[ep], ids[ep] = ck, ci
    end
  end
  sp = sp - 1
  keys[sp], ids[sp] = pk, pi
  sort_rec(keys, ids, lo, sp, less)
  sort_rec(keys, ids, ep, hi, less)
end

local function bench_sort()
  local rnd = new_rnd(777)
  local skeys, sids = {}, {}
  for i = 0, SORT_N - 1 do
    skeys[i] = rnd:range(-1000.0, 1000.0)
    sids[i] = i
  end
  local function less(ak, ai, bk, bi)
    return ak < bk or (ak == bk and ai < bi)
  end
  local keys, ids = {}, {}
  for _ = 1, SORT_ROUNDS do
    for i = 0, SORT_N - 1 do
      keys[i] = skeys[i]
      ids[i] = sids[i]
    end
    sort_rec(keys, ids, 0, SORT_N, less)
  end
  for i = 1, SORT_N - 1 do
    if less(keys[i], ids[i], keys[i - 1], ids[i - 1]) then
      print("# sort: not sorted at " .. i)
      return 0.0
    end
  end
  local total = 0.0
  for i = 0, SORT_N - 1 do
    total = total + (i % 97 + 1) * ids[i]
  end
  return total
end

-- ------------------------------------------------------------------ 5. astar

local AS_W, AS_H, AS_ROUNDS, WALL_P, INF = 512, 512, 20, 0.20, 0x3FFFFFFF

local function before(af, an, bf, bn)
  return af < bf or (af == bf and an < bn)
end

local function solve(seed)
  local rnd = new_rnd(seed)
  local cells = AS_W * AS_H
  local grid, gscore, closed = {}, {}, {}
  for i = 0, cells - 1 do
    if rnd:nextf() < WALL_P then grid[i] = 1 else grid[i] = 0 end
    gscore[i] = INF
    closed[i] = 0
  end
  local goal = cells - 1
  grid[0] = 0
  grid[1] = 0
  grid[AS_W] = 0
  grid[goal] = 0
  grid[goal - 1] = 0
  grid[goal - AS_W] = 0
  local hf, hn = {}, {}
  local hsize = 0
  local dx = { [0] = 1, 0, -1, 0 }
  local dy = { [0] = 0, 1, 0, -1 }
  local gx, gy = AS_W - 1, AS_H - 1
  gscore[0] = 0

  local function push(ef, en)
    local i = hsize
    hsize = hsize + 1
    while i > 0 do
      local p = floor((i - 1) / 2)
      if not before(ef, en, hf[p], hn[p]) then break end
      hf[i], hn[i] = hf[p], hn[p]
      i = p
    end
    hf[i], hn[i] = ef, en
  end

  local function pop()
    local top_n = hn[0]
    hsize = hsize - 1
    local last_f, last_n = hf[hsize], hn[hsize]
    if hsize > 0 then
      local i = 0
      while true do
        local l = i * 2 + 1
        if l >= hsize then break end
        local c = l
        local r = l + 1
        if r < hsize and before(hf[r], hn[r], hf[l], hn[l]) then c = r end
        if not before(hf[c], hn[c], last_f, last_n) then break end
        hf[i], hn[i] = hf[c], hn[c]
        i = c
      end
      hf[i], hn[i] = last_f, last_n
    end
    return top_n
  end

  push(gx + gy, 0)
  local expanded = 0
  local cost = -1
  while hsize > 0 do
    local n = pop()
    if closed[n] == 0 then
      closed[n] = 1
      expanded = expanded + 1
      if n == goal then
        cost = gscore[n]
        break
      end
      local y = floor(n / AS_W)
      local x = n - y * AS_W
      local g = gscore[n] + 1
      for k = 0, 3 do
        local nx = x + dx[k]
        local ny = y + dy[k]
        if nx >= 0 and ny >= 0 and nx < AS_W and ny < AS_H then
          local nb = n + dy[k] * AS_W + dx[k]
          if grid[nb] == 0 and closed[nb] == 0 and g < gscore[nb] then
            gscore[nb] = g
            push(g + (gx - nx) + (gy - ny), nb)
          end
        end
      end
    end
  end
  return cost, expanded
end

local function bench_astar()
  local total = 0
  for r = 0, AS_ROUNDS - 1 do
    local cost, expanded = solve(1000 + r)
    total = total + cost * 1000003 + expanded
  end
  return total + 0.0
end

-- ------------------------------------------------------------------ driver

local BENCHES = {
  { "nbody", bench_nbody },
  { "raytrace", bench_raytrace },
  { "interp", bench_interp },
  { "sort", bench_sort },
  { "astar", bench_astar },
}

local args = {}
for i = 1, #arg do
  if arg[i] ~= "--" then args[#args + 1] = arg[i] end
end
local repeats = 5
if args[1] then
  local n = tonumber(args[1])
  if n and n > 0 then repeats = math.floor(n) end
end
local only = args[2] or ""

local out = {}
for i = 1, #BENCHES do
  local name, fn = BENCHES[i][1], BENCHES[i][2]
  if only == "" or only == name then
    local best = math.huge
    local checksum = 0.0
    for _ = 1, repeats do
      local t0 = clock()
      checksum = fn()
      local dt = clock() - t0
      if dt < best then best = dt end
    end
    out[#out + 1] = string.format("bench\t%s\t%.7f\t%.17g", name, best, checksum)
  end
end
io.write(table.concat(out, "\n"), "\n")
