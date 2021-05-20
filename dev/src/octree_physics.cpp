/* This is phsyics.cpp (and parts of ents.h) from Cube 2: Sauerbraten (!)
http://cubeengine.com/

This code is mostly as-is, adapted and reformatted to fit the Lobster project,
and to work with octree.h/.cpp code instead of the octree that was in Cube 2.
At the bottom of this file are some Lobster specific bindings.

Interestingly the original version of this physics engine (including its very
doom/quake-like feel) was written/designed by myself (Wouter), though later greatly
enhanced / improved by Lee Salzman, as he took over maintenance of the project.

I am including it in the Lobster engine because it is about the smallest 3D
(particle) physics engine that is still very useful for making 3D games, and
I like the "feel" of it. It also fits the cubical worlds in most game prototypes
I tend to make.

It of course lacks rigid/soft body functionality.

*/

#include "lobster/stdafx.h"
#include "lobster/natreg.h"
#include "lobster/octree.h"
#include "lobster/sdlinterface.h"

OcTree *oc = nullptr;

// -------------------------------------------------------------------------------------------------
// tools.h: FIXME remove

#define loop(v, m) for (int v = 0; v < int(m); ++v)
#define loopi(m) loop(i, m)
#define loopj(m) loop(j, m)
#define loopk(m) loop(k, m)
#define loopl(m) loop(l, m)
#define looprev(v, m) for (int v = int(m); --v >= 0;)
#define loopirev(m) looprev(i, m)
#define loopjrev(m) looprev(j, m)
#define loopkrev(m) looprev(k, m)
#define looplrev(m) looprev(l, m)


// -------------------------------------------------------------------------------------------------
// geom.h: parts to facilitate porting.

// FIXME: would be good to remove this and replace with lobster/geom.h types.
struct vec3 {
    union {
        struct {
            float x, y, z;
        };
        float v[3];
    };

    vec3() {}
    explicit vec3(int a) : x(float(a)), y(float(a)), z(float(a)) {}
    explicit vec3(float a) : x(a), y(a), z(a) {}
    vec3(float a, float b, float c) : x(a), y(b), z(c) {}
    explicit vec3(int v[3]) : x(float(v[0])), y(float(v[1])), z(float(v[2])) {}
    explicit vec3(const float *v) : x(v[0]), y(v[1]), z(v[2]) {}
    explicit vec3(const float2 &v, float z = 0) : x(v.x), y(v.y), z(z) {}
    explicit vec3(const float3 &v) : x(v.x), y(v.y), z(v.z) {}
    explicit vec3(const float4 &v) : x(v.x), y(v.y), z(v.z) {}
    explicit vec3(const int3 &v) : x(float(v.x)), y(float(v.y)), z(float(v.z)) {}

    vec3(float yaw, float pitch)
        : x(-sinf(yaw) * cosf(pitch)), y(cosf(yaw) * cosf(pitch)), z(sinf(pitch)) {}

    float &operator[](int i) { return v[i]; }
    float operator[](int i) const { return v[i]; }

    vec3 &set(int i, float f) {
        v[i] = f;
        return *this;
    }

    bool operator==(const vec3 &o) const { return x == o.x && y == o.y && z == o.z; }
    bool operator!=(const vec3 &o) const { return x != o.x || y != o.y || z != o.z; }

    bool iszero() const { return x == 0 && y == 0 && z == 0; }
    float squaredlen() const { return x * x + y * y + z * z; }
    template<class T> float dot2(const T &o) const { return x * o.x + y * o.y; }
    float dot(const vec3 &o) const { return x * o.x + y * o.y + z * o.z; }
    float absdot(const vec3 &o) const { return fabs(x * o.x) + fabs(y * o.y) + fabs(z * o.z); }
    vec3 &pow(float f) {
        x = ::pow(x, f);
        y = ::pow(y, f);
        z = ::pow(z, f);
        return *this;
    }
    vec3 &exp() {
        x = ::exp(x);
        y = ::exp(y);
        z = ::exp(z);
        return *this;
    }
    vec3 &exp2() {
        x = ::exp2(x);
        y = ::exp2(y);
        z = ::exp2(z);
        return *this;
    }
    vec3 &sqrt() {
        x = sqrtf(x);
        y = sqrtf(y);
        z = sqrtf(z);
        return *this;
    }
    vec3 &mul(const vec3 &o) {
        x *= o.x;
        y *= o.y;
        z *= o.z;
        return *this;
    }
    vec3 &mul(float f) {
        x *= f;
        y *= f;
        z *= f;
        return *this;
    }
    vec3 &square() {
        mul(*this);
        return *this;
    }
    vec3 &div(const vec3 &o) {
        x /= o.x;
        y /= o.y;
        z /= o.z;
        return *this;
    }
    vec3 &div(float f) {
        x /= f;
        y /= f;
        z /= f;
        return *this;
    }
    vec3 &recip() {
        x = 1 / x;
        y = 1 / y;
        z = 1 / z;
        return *this;
    }
    vec3 &add(const vec3 &o) {
        x += o.x;
        y += o.y;
        z += o.z;
        return *this;
    }
    vec3 &add(float f) {
        x += f;
        y += f;
        z += f;
        return *this;
    }
    vec3 &add2(float f) {
        x += f;
        y += f;
        return *this;
    }
    vec3 &addz(float f) {
        z += f;
        return *this;
    }
    vec3 &sub(const vec3 &o) {
        x -= o.x;
        y -= o.y;
        z -= o.z;
        return *this;
    }
    vec3 &sub(float f) {
        x -= f;
        y -= f;
        z -= f;
        return *this;
    }
    vec3 &sub2(float f) {
        x -= f;
        y -= f;
        return *this;
    }
    vec3 &subz(float f) {
        z -= f;
        return *this;
    }
    vec3 &neg2() {
        x = -x;
        y = -y;
        return *this;
    }
    vec3 &neg() {
        x = -x;
        y = -y;
        z = -z;
        return *this;
    }
    vec3 &min(const vec3 &o) {
        x = ::min(x, o.x);
        y = ::min(y, o.y);
        z = ::min(z, o.z);
        return *this;
    }
    vec3 &max(const vec3 &o) {
        x = ::max(x, o.x);
        y = ::max(y, o.y);
        z = ::max(z, o.z);
        return *this;
    }
    vec3 &min(float f) {
        x = ::min(x, f);
        y = ::min(y, f);
        z = ::min(z, f);
        return *this;
    }
    vec3 &max(float f) {
        x = ::max(x, f);
        y = ::max(y, f);
        z = ::max(z, f);
        return *this;
    }
    vec3 &clamp(float f, float h) {
        x = std::clamp(x, f, h);
        y = std::clamp(y, f, h);
        z = std::clamp(z, f, h);
        return *this;
    }
    vec3 &abs() {
        x = fabs(x);
        y = fabs(y);
        z = fabs(z);
        return *this;
    }
    float magnitude2() const { return sqrtf(dot2(*this)); }
    float magnitude() const { return sqrtf(squaredlen()); }
    vec3 &normalize() {
        div(magnitude());
        return *this;
    }
    vec3 &safenormalize() {
        float m = magnitude();
        if (m) div(m);
        return *this;
    }
    bool isnormalized() const {
        float m = squaredlen();
        return (m > 0.99f && m < 1.01f);
    }
    float squaredist(const vec3 &e) const { return vec3(*this).sub(e).squaredlen(); }
    float dist(const vec3 &e) const {
        vec3 t;
        return dist(e, t);
    }
    float dist(const vec3 &e, vec3 &t) const {
        t = *this;
        t.sub(e);
        return t.magnitude();
    }
    float dist2(const vec3 &o) const {
        float dx = x - o.x, dy = y - o.y;
        return sqrtf(dx * dx + dy * dy);
    }
    bool reject(const vec3 &o, float r) {
        return x > o.x + r || x < o.x - r || y > o.y + r || y < o.y - r;
    }
    template<class A, class B> vec3 &cross(const A &a, const B &b) {
        x = a.y * b.z - a.z * b.y;
        y = a.z * b.x - a.x * b.z;
        z = a.x * b.y - a.y * b.x;
        return *this;
    }
    vec3 &cross(const vec3 &o, const vec3 &a, const vec3 &b) {
        return cross(vec3(a).sub(o), vec3(b).sub(o));
    }
    float scalartriple(const vec3 &a, const vec3 &b) const {
        return x * (a.y * b.z - a.z * b.y) + y * (a.z * b.x - a.x * b.z) +
               z * (a.x * b.y - a.y * b.x);
    }
    vec3 &reflectz(float rz) {
        z = 2 * rz - z;
        return *this;
    }
    vec3 &reflect(const vec3 &n) {
        float k = 2 * dot(n);
        x -= k * n.x;
        y -= k * n.y;
        z -= k * n.z;
        return *this;
    }
    vec3 &project(const vec3 &n) {
        float k = dot(n);
        x -= k * n.x;
        y -= k * n.y;
        z -= k * n.z;
        return *this;
    }
    vec3 &projectxydir(const vec3 &n) {
        if (n.z) z = -(x * n.x / n.z + y * n.y / n.z);
        return *this;
    }
    vec3 &projectxy(const vec3 &n) {
        float m = squaredlen(), k = dot(n);
        projectxydir(n);
        rescale(sqrtf(::max(m - k * k, 0.0f)));
        return *this;
    }
    vec3 &projectxy(const vec3 &n, float threshold) {
        float m = squaredlen(), k = ::min(dot(n), threshold);
        projectxydir(n);
        rescale(sqrtf(::max(m - k * k, 0.0f)));
        return *this;
    }
    vec3 &lerp(const vec3 &b, float t) {
        x += (b.x - x) * t;
        y += (b.y - y) * t;
        z += (b.z - z) * t;
        return *this;
    }
    vec3 &lerp(const vec3 &a, const vec3 &b, float t) {
        x = a.x + (b.x - a.x) * t;
        y = a.y + (b.y - a.y) * t;
        z = a.z + (b.z - a.z) * t;
        return *this;
    }
    template<class B> vec3 &madd(const vec3 &a, const B &b) { return add(vec3(a).mul(b)); }
    template<class B> vec3 &msub(const vec3 &a, const B &b) { return sub(vec3(a).mul(b)); }

    vec3 &rescale(float k) {
        float mag = magnitude();
        if (mag > 1e-6f) mul(k / mag);
        return *this;
    }

    vec3 &rotate_around_z(float c, float s) {
        float rx = x, ry = y;
        x = c * rx - s * ry;
        y = c * ry + s * rx;
        return *this;
    }
    vec3 &rotate_around_x(float c, float s) {
        float ry = y, rz = z;
        y = c * ry - s * rz;
        z = c * rz + s * ry;
        return *this;
    }
    vec3 &rotate_around_y(float c, float s) {
        float rx = x, rz = z;
        x = c * rx + s * rz;
        z = c * rz - s * rx;
        return *this;
    }

    vec3 &rotate_around_z(float angle) { return rotate_around_z(cosf(angle), sinf(angle)); }
    vec3 &rotate_around_x(float angle) { return rotate_around_x(cosf(angle), sinf(angle)); }
    vec3 &rotate_around_y(float angle) { return rotate_around_y(cosf(angle), sinf(angle)); }

    vec3 &rotate_around_z(const float2 &sc) { return rotate_around_z(sc.x, sc.y); }
    vec3 &rotate_around_x(const float2 &sc) { return rotate_around_x(sc.x, sc.y); }
    vec3 &rotate_around_y(const float2 &sc) { return rotate_around_y(sc.x, sc.y); }

    vec3 &rotate(float c, float s, const vec3 &d) {
        *this = vec3(x * (d.x * d.x * (1 - c) + c) + y * (d.x * d.y * (1 - c) - d.z * s) +
                        z * (d.x * d.z * (1 - c) + d.y * s),
                    x * (d.y * d.x * (1 - c) + d.z * s) + y * (d.y * d.y * (1 - c) + c) +
                        z * (d.y * d.z * (1 - c) - d.x * s),
                    x * (d.x * d.z * (1 - c) - d.y * s) + y * (d.y * d.z * (1 - c) + d.x * s) +
                        z * (d.z * d.z * (1 - c) + c));
        return *this;
    }
    vec3 &rotate(float angle, const vec3 &d) { return rotate(cosf(angle), sinf(angle), d); }
    vec3 &rotate(const float2 &sc, const vec3 &d) { return rotate(sc.x, sc.y, d); }

    void orthogonal(const vec3 &d) {
        *this = fabs(d.x) > fabs(d.z) ? vec3(-d.y, d.x, 0) : vec3(0, -d.z, d.y);
    }

    void orthonormalize(vec3 &s, vec3 &t) const {
        s.sub(vec3(*this).mul(dot(s)));
        t.sub(vec3(*this).mul(dot(t))).sub(vec3(s).mul(s.dot(t)));
    }

    template<class T> bool insidebb(const T &bbmin, const T &bbmax) const {
        return x >= bbmin.x && x <= bbmax.x && y >= bbmin.y && y <= bbmax.y && z >= bbmin.z &&
               z <= bbmax.z;
    }

    template<class T, class U> bool insidebb(const T &o, U size) const {
        return x >= o.x && x <= o.x + size && y >= o.y && y <= o.y + size && z >= o.z &&
               z <= o.z + size;
    }

    template<class T> float dist_to_bb(const T &min, const T &max) const {
        float sqrdist = 0;
        loopi(3) {
            if (v[i] < min[i]) {
                float delta = v[i] - min[i];
                sqrdist += delta * delta;
            } else if (v[i] > max[i]) {
                float delta = max[i] - v[i];
                sqrdist += delta * delta;
            }
        }
        return sqrtf(sqrdist);
    }

    template<class T, class S> float dist_to_bb(const T &o, S size) const {
        return dist_to_bb(o, T(o).add(size));
    }
};


struct plane : vec3 {
    float offset;

    float dist(const vec3 &p) const { return dot(p) + offset; }
    //float dist(const float4 &p) const { return p.dot3(*this) + p.w * offset; }
    bool operator==(const plane &p) const {
        return x == p.x && y == p.y && z == p.z && offset == p.offset;
    }
    bool operator!=(const plane &p) const {
        return x != p.x || y != p.y || z != p.z || offset != p.offset;
    }

    plane() {}
    plane(const vec3 &c, float off) : vec3(c), offset(off) {}
    plane(const float4 &p) : vec3(p), offset(p.w) {}
    plane(int d, float off) {
        x = y = z = 0.0f;
        v[d] = 1.0f;
        offset = -off;
    }
    plane(float a, float b, float c, float d) : vec3(a, b, c), offset(d) {}

    void toplane(const vec3 &n, const vec3 &p) {
        x = n.x;
        y = n.y;
        z = n.z;
        offset = -dot(p);
    }

    bool toplane(const vec3 &a, const vec3 &b, const vec3 &c) {
        cross(vec3(b).sub(a), vec3(c).sub(a));
        float mag = magnitude();
        if (!mag) return false;
        div(mag);
        offset = -dot(a);
        return true;
    }

    bool rayintersect(const vec3 &o, const vec3 &ray, float &dist) {
        float cosalpha = dot(ray);
        if (cosalpha == 0) return false;
        float deltac = offset + dot(o);
        dist -= deltac / cosalpha;
        return true;
    }

    plane &reflectz(float rz) {
        offset += 2 * rz * z;
        z = -z;
        return *this;
    }

    plane &invert() {
        neg();
        offset = -offset;
        return *this;
    }

    plane &scale(float k) {
        mul(k);
        return *this;
    }

    plane &translate(const vec3 &p) {
        offset += dot(p);
        return *this;
    }

    plane &normalize() {
        float mag = magnitude();
        div(mag);
        offset /= mag;
        return *this;
    }

    float zintersect(const vec3 &p) const { return -(x * p.x + y * p.y + offset) / z; }
    float zdelta(const vec3 &p) const { return -(x * p.x + y * p.y) / z; }
    float zdist(const vec3 &p) const { return p.z - zintersect(p); }
};


struct matrix3 {
    vec3 a, b, c;

    matrix3() {}
    matrix3(const vec3 &a, const vec3 &b, const vec3 &c) : a(a), b(b), c(c) {}
    explicit matrix3(float angle, const vec3 &axis) { rotate(angle, axis); }
    explicit matrix3(const quat &q) {
        float x = q.x, y = q.y, z = q.z, w = q.w, tx = 2 * x, ty = 2 * y, tz = 2 * z, txx = tx * x,
              tyy = ty * y, tzz = tz * z, txy = tx * y, txz = tx * z, tyz = ty * z, twx = w * tx,
              twy = w * ty, twz = w * tz;
        a = vec3(1 - (tyy + tzz), txy + twz, txz - twy);
        b = vec3(txy - twz, 1 - (txx + tzz), tyz + twx);
        c = vec3(txz + twy, tyz - twx, 1 - (txx + tyy));
    }
    //explicit matrix3(const matrix4x3 &m);
    //explicit matrix3(const matrix4 &m);

    void mul(const matrix3 &m, const matrix3 &n) {
        a = vec3(m.a).mul(n.a.x).madd(m.b, n.a.y).madd(m.c, n.a.z);
        b = vec3(m.a).mul(n.b.x).madd(m.b, n.b.y).madd(m.c, n.b.z);
        c = vec3(m.a).mul(n.c.x).madd(m.b, n.c.y).madd(m.c, n.c.z);
    }
    void mul(const matrix3 &n) { mul(matrix3(*this), n); }

    void multranspose(const matrix3 &m, const matrix3 &n) {
        a = vec3(m.a).mul(n.a.x).madd(m.b, n.b.x).madd(m.c, n.c.x);
        b = vec3(m.a).mul(n.a.y).madd(m.b, n.b.y).madd(m.c, n.c.y);
        c = vec3(m.a).mul(n.a.z).madd(m.b, n.b.z).madd(m.c, n.c.z);
    }
    void multranspose(const matrix3 &n) { multranspose(matrix3(*this), n); }

    void transposemul(const matrix3 &m, const matrix3 &n) {
        a = vec3(m.a.dot(n.a), m.b.dot(n.a), m.c.dot(n.a));
        b = vec3(m.a.dot(n.b), m.b.dot(n.b), m.c.dot(n.b));
        c = vec3(m.a.dot(n.c), m.b.dot(n.c), m.c.dot(n.c));
    }
    void transposemul(const matrix3 &n) { transposemul(matrix3(*this), n); }

    void transpose() {
        swap(a.y, b.x);
        swap(a.z, c.x);
        swap(b.z, c.y);
    }

    template<class M> void transpose(const M &m) {
        a = vec3(m.a.x, m.b.x, m.c.x);
        b = vec3(m.a.y, m.b.y, m.c.y);
        c = vec3(m.a.z, m.b.z, m.c.z);
    }

    void invert(const matrix3 &o) {
        vec3 unscale(1 / o.a.squaredlen(), 1 / o.b.squaredlen(), 1 / o.c.squaredlen());
        transpose(o);
        a.mul(unscale);
        b.mul(unscale);
        c.mul(unscale);
    }
    void invert() { invert(matrix3(*this)); }

    void normalize() {
        a.normalize();
        b.normalize();
        c.normalize();
    }

    void scale(float k) {
        a.mul(k);
        b.mul(k);
        c.mul(k);
    }

    void rotate(float angle, const vec3 &axis) { rotate(cosf(angle), sinf(angle), axis); }

    void rotate(float ck, float sk, const vec3 &axis) {
        a = vec3(axis.x * axis.x * (1 - ck) + ck, axis.x * axis.y * (1 - ck) + axis.z * sk,
                axis.x * axis.z * (1 - ck) - axis.y * sk);
        b = vec3(axis.x * axis.y * (1 - ck) - axis.z * sk, axis.y * axis.y * (1 - ck) + ck,
                axis.y * axis.z * (1 - ck) + axis.x * sk);
        c = vec3(axis.x * axis.z * (1 - ck) + axis.y * sk, axis.y * axis.z * (1 - ck) - axis.x * sk,
                axis.z * axis.z * (1 - ck) + ck);
    }

    void setyaw(float ck, float sk) {
        a = vec3(ck, sk, 0);
        b = vec3(-sk, ck, 0);
        c = vec3(0, 0, 1);
    }

    void setyaw(float angle) { setyaw(cosf(angle), sinf(angle)); }

    float trace() const { return a.x + b.y + c.z; }

    bool calcangleaxis(float tr, float &angle, vec3 &axis, float threshold = 1e-16f) const {
        if (tr <= -1) {
            if (a.x >= b.y && a.x >= c.z) {
                float r = 1 + a.x - b.y - c.z;
                if (r <= threshold) return false;
                r = sqrtf(r);
                axis.x = 0.5f * r;
                axis.y = b.x / r;
                axis.z = c.x / r;
            } else if (b.y >= c.z) {
                float r = 1 + b.y - a.x - c.z;
                if (r <= threshold) return false;
                r = sqrtf(r);
                axis.y = 0.5f * r;
                axis.x = b.x / r;
                axis.z = c.y / r;
            } else {
                float r = 1 + b.y - a.x - c.z;
                if (r <= threshold) return false;
                r = sqrtf(r);
                axis.z = 0.5f * r;
                axis.x = c.x / r;
                axis.y = c.y / r;
            }
            angle = PI;
        } else if (tr >= 3) {
            axis = vec3(0, 0, 1);
            angle = 0;
        } else {
            axis = vec3(b.z - c.y, c.x - a.z, a.y - b.x);
            float r = axis.squaredlen();
            if (r <= threshold) return false;
            axis.mul(1 / sqrtf(r));
            angle = acosf(0.5f * (tr - 1));
        }
        return true;
    }

    bool calcangleaxis(float &angle, vec3 &axis, float threshold = 1e-16f) const {
        return calcangleaxis(trace(), angle, axis, threshold);
    }

    vec3 transform(const vec3 &o) const { return vec3(a).mul(o.x).madd(b, o.y).madd(c, o.z); }
    vec3 transposedtransform(const vec3 &o) const { return vec3(a.dot(o), b.dot(o), c.dot(o)); }
    vec3 abstransform(const vec3 &o) const {
        return vec3(a).mul(o.x).abs().add(vec3(b).mul(o.y).abs()).add(vec3(c).mul(o.z).abs());
    }
    vec3 abstransposedtransform(const vec3 &o) const {
        return vec3(a.absdot(o), b.absdot(o), c.absdot(o));
    }

    void identity() {
        a = vec3(1, 0, 0);
        b = vec3(0, 1, 0);
        c = vec3(0, 0, 1);
    }

    void rotate_around_x(float ck, float sk) {
        vec3 rb = vec3(b).mul(ck).madd(c, sk), rc = vec3(c).mul(ck).msub(b, sk);
        b = rb;
        c = rc;
    }
    void rotate_around_x(float angle) { rotate_around_x(cosf(angle), sinf(angle)); }
    void rotate_around_x(const float2 &sc) { rotate_around_x(sc.x, sc.y); }

    void rotate_around_y(float ck, float sk) {
        vec3 rc = vec3(c).mul(ck).madd(a, sk), ra = vec3(a).mul(ck).msub(c, sk);
        c = rc;
        a = ra;
    }
    void rotate_around_y(float angle) { rotate_around_y(cosf(angle), sinf(angle)); }
    void rotate_around_y(const float2 &sc) { rotate_around_y(sc.x, sc.y); }

    void rotate_around_z(float ck, float sk) {
        vec3 ra = vec3(a).mul(ck).madd(b, sk), rb = vec3(b).mul(ck).msub(a, sk);
        a = ra;
        b = rb;
    }
    void rotate_around_z(float angle) { rotate_around_z(cosf(angle), sinf(angle)); }
    void rotate_around_z(const float2 &sc) { rotate_around_z(sc.x, sc.y); }

    vec3 transform(const float2 &o) { return vec3(a).mul(o.x).madd(b, o.y); }
    vec3 transposedtransform(const float2 &o) const { return vec3(a.dot2(o), b.dot2(o), c.dot2(o)); }

    vec3 rowx() const { return vec3(a.x, b.x, c.x); }
    vec3 rowy() const { return vec3(a.y, b.y, c.y); }
    vec3 rowz() const { return vec3(a.z, b.z, c.z); }
};

struct clipplanes {
    vec3 o, r, v[8];
    plane p[12];
    uint8_t side[12];
    uint8_t size, visible;
    int version;
};



// -------------------------------------------------------------------------------------------------
// ents.h: parts of it that pertain to physics.

enum { CS_ALIVE = 0, CS_DEAD, CS_SPAWNING, CS_LAGGED, CS_EDITING, CS_SPECTATOR };

enum {
    PHYS_FLOAT = 0,
    PHYS_FALL,
    PHYS_SLIDE,
    PHYS_SLOPE,
    PHYS_FLOOR,
    PHYS_STEP_UP,
    PHYS_STEP_DOWN,
    PHYS_BOUNCE
};

enum { ENT_PLAYER = 0, ENT_AI, ENT_INANIMATE, ENT_CAMERA, ENT_BOUNCE };

enum { COLLIDE_NONE = 0,
       COLLIDE_ELLIPSE,         // Players
       COLLIDE_ELLIPSE_PRECISE  // Grenades
     };

enum {
    RAY_BB = 1,
    RAY_POLY = 3,
    RAY_ALPHAPOLY = 7,
    RAY_ENTS = 9,
    RAY_CLIPMAT = 16,
    RAY_SKIPFIRST = 32,
    RAY_EDITMAT = 64,
    RAY_SHADOW = 128,
    RAY_PASS = 256,
    RAY_SKIPSKY = 512,
    RAY_SKYTEX = 1024
};

// base entity type, can be affected by physics
struct physent {
    vec3 o = vec3(0, 0, 0);
    vec3 vel = vec3(0, 0, 0), falling = vec3(0, 0, 0);
    vec3 deltapos = vec3(0, 0, 0);  // movement interpolation
    vec3 newpos = vec3(0, 0, 0);
    float yaw = 0, pitch = 0, roll = 0;
    float maxspeed = 100;                // cubes per second, 100 for player
    float radius = 4.1f, eyeheight = 14, aboveeye = 1;  // bounding box size
    float xradius = 4.1f, yradius = 4.1f, zmargin = 0;
    vec3 floor = vec3(0, 0, 1);  // the normal of floor the physent is on

    uint16_t timeinair = 0;
    uint8_t inwater = 0;
    bool jumping = false;
    int8_t move = 0, strafe = 0;

    uint8_t physstate = PHYS_FALL;                   // one of PHYS_* above
    uint8_t state = CS_ALIVE;                        // one of CS_* above
    uint8_t type = ENT_PLAYER;                       // one of ENT_* above
    uint8_t collidetype = COLLIDE_ELLIPSE;           // one of COLLIDE_* above

    bool blocked = false;  // used by physics to signal ai

    const char *event_id = nullptr;

    void resetinterp() {
        newpos = o;
        deltapos = vec3(0, 0, 0);
    }

    void reset() {
        inwater = 0;
        timeinair = 0;
        jumping = false;
        strafe = move = 0;
        physstate = PHYS_FALL;
        vel = falling = vec3(0, 0, 0);
        floor = vec3(0, 0, 1);
    }

    vec3 feetpos(float offset = 0) const { return vec3(o).add(vec3(0, 0, offset - eyeheight)); }
    vec3 headpos(float offset = 0) const { return vec3(o).add(vec3(0, 0, offset)); }

    bool maymove() const {
        return timeinair || physstate < PHYS_FLOOR || vel.squaredlen() > 1e-4f ||
               deltapos.squaredlen() > 1e-4f;
    }
};

enum {
    MATF_INDEX_SHIFT = 0,
    MATF_VOLUME_SHIFT = 2,
    MATF_CLIP_SHIFT = 5,
    MATF_FLAG_SHIFT = 8,

    MATF_INDEX = 3 << MATF_INDEX_SHIFT,
    MATF_VOLUME = 7 << MATF_VOLUME_SHIFT,
    MATF_CLIP = 7 << MATF_CLIP_SHIFT,
    MATF_FLAGS = 0xFF << MATF_FLAG_SHIFT
};

enum  // cube empty-space materials
{
    MAT_AIR = 0,                         // the default, fill the empty space with air
    MAT_WATER = 1 << MATF_VOLUME_SHIFT,  // fill with water, showing waves at the surface
    MAT_LAVA = 2 << MATF_VOLUME_SHIFT,   // fill with lava
    MAT_GLASS = 3 << MATF_VOLUME_SHIFT,  // behaves like clip but is blended blueish

    MAT_NOCLIP = 1 << MATF_CLIP_SHIFT,    // collisions always treat cube as empty
    MAT_CLIP = 2 << MATF_CLIP_SHIFT,      // collisions always treat cube as solid
    MAT_GAMECLIP = 3 << MATF_CLIP_SHIFT,  // game specific clip material

    MAT_DEATH = 1 << MATF_FLAG_SHIFT,  // force player suicide
    MAT_ALPHA = 4 << MATF_FLAG_SHIFT   // alpha blended
};

#define isliquid(mat) ((mat) == MAT_WATER || (mat) == MAT_LAVA)
#define isclipped(mat) ((mat) == MAT_GLASS)
#define isdeadly(mat) ((mat) == MAT_LAVA)


// -------------------------------------------------------------------------------------------------
// mpr.h:

// This code is based off the Minkowski Portal Refinement algorithm by Gary Snethen in XenoCollide &
// Game Programming Gems 7.

namespace mpr {
struct CubePlanes {
    const clipplanes &p;

    CubePlanes(const clipplanes &p) : p(p) {}

    vec3 center() const { return p.o; }

    vec3 supportpoint(const vec3 &n) const {
        int besti = 7;
        float bestd = n.dot(p.v[7]);
        loopi(7) {
            float d = n.dot(p.v[i]);
            if (d > bestd) {
                besti = i;
                bestd = d;
            }
        }
        return p.v[besti];
    }
};

struct SolidCube {
    vec3 o;
    int size;

    SolidCube(float x, float y, float z, int size) : o(x, y, z), size(size) {}
    SolidCube(const vec3 &o, int size) : o(o), size(size) {}
    SolidCube(const int3 &o, int size) : o(o), size(size) {}

    vec3 center() const { return vec3(o).add(size / 2.0f); }

    vec3 supportpoint(const vec3 &n) const {
        vec3 p(o);
        if (n.x > 0) p.x += size;
        if (n.y > 0) p.y += size;
        if (n.z > 0) p.z += size;
        return p;
    }
};

struct Ent {
    physent *ent;

    Ent(physent *ent) : ent(ent) {}

    vec3 center() const {
        return vec3(ent->o.x, ent->o.y, ent->o.z + (ent->aboveeye - ent->eyeheight) / 2);
    }
};

struct EntFuzzy : Ent {
    EntFuzzy(physent *ent) : Ent(ent) {}

    float left() const { return ent->o.x - ent->radius; }
    float right() const { return ent->o.x + ent->radius; }
    float back() const { return ent->o.y - ent->radius; }
    float front() const { return ent->o.y + ent->radius; }
    float bottom() const { return ent->o.z - ent->eyeheight; }
    float top() const { return ent->o.z + ent->aboveeye; }
};

struct EntCapsule : EntFuzzy {
    EntCapsule(physent *ent) : EntFuzzy(ent) {}

    vec3 supportpoint(const vec3 &n) const {
        vec3 p(ent->o);
        if (n.z > 0)
            p.z += ent->aboveeye - ent->radius;
        else
            p.z -= ent->eyeheight - ent->radius;
        p.add(vec3(n).mul(ent->radius / n.magnitude()));
        return p;
    }
};

struct EntEllipsoid : EntFuzzy {
    EntEllipsoid(physent *ent) : EntFuzzy(ent) {}

    vec3 supportpoint(const vec3 &dir) const {
        vec3 p(ent->o), n = vec3(dir).normalize();
        p.x += ent->radius * n.x;
        p.y += ent->radius * n.y;
        p.z += (ent->aboveeye + ent->eyeheight) / 2 * (1 + n.z) - ent->eyeheight;
        return p;
    }
};

struct Model {
    vec3 o, radius;
    matrix3 orient;

    Model(const vec3 &ent, const vec3 &center, const vec3 &radius, int yaw) : o(ent), radius(radius) {
        orient.setyaw(yaw * RAD);
        o.add(orient.transposedtransform(center));
    }

    vec3 center() const { return o; }
};

struct ModelOBB : Model {
    ModelOBB(const vec3 &ent, const vec3 &center, const vec3 &radius, int yaw)
        : Model(ent, center, radius, yaw) {}

    vec3 contactface(const vec3 &wn, const vec3 &wdir) const {
        vec3 n = orient.transform(wn).div(radius), dir = orient.transform(wdir),
            an(fabs(n.x), fabs(n.y), dir.z ? fabs(n.z) : 0), fn(0, 0, 0);
        if (an.x > an.y) {
            if (an.x > an.z)
                fn.x = n.x * dir.x < 0.0f ? (n.x > 0.0f ? 1.0f : -1.0f) : 0.0f;
            else if (an.z > 0.0f)
                fn.z = n.z * dir.z < 0.0f ? (n.z > 0.0f ? 1.0f : -1.0f) : 0.0f;
        } else if (an.y > an.z)
            fn.y = n.y * dir.y < 0.0f ? (n.y > 0.0f ? 1.0f : -1.0f) : 0.0f;
        else if (an.z > 0)
            fn.z = n.z * dir.z < 0.0f ? (n.z > 0.0f ? 1.0f : -1.0f) : 0.0f;
        return orient.transposedtransform(fn);
    }

    vec3 supportpoint(const vec3 &n) const {
        vec3 ln = orient.transform(n), p(0, 0, 0);
        if (ln.x > 0)
            p.x += radius.x;
        else
            p.x -= radius.x;
        if (ln.y > 0)
            p.y += radius.y;
        else
            p.y -= radius.y;
        if (ln.z > 0)
            p.z += radius.z;
        else
            p.z -= radius.z;
        return orient.transposedtransform(p).add(o);
    }
};

struct ModelEllipse : Model {
    ModelEllipse(const vec3 &ent, const vec3 &center, const vec3 &radius, int yaw)
        : Model(ent, center, radius, yaw) {}

    vec3 contactface(const vec3 &wn, const vec3 &wdir) const {
        vec3 n = orient.transform(wn).div(radius), dir = orient.transform(wdir);
        float dxy = n.dot2(n), dz = n.z * n.z;
        vec3 fn(0, 0, 0);
        if (dz > dxy && dir.z)
            fn.z = n.z * dir.z < 0.0f ? (n.z > 0.0f ? 1.0f : -1.0f) : 0.0f;
        else if (n.dot2(dir) < 0) {
            fn.x = n.x * radius.y;
            fn.y = n.y * radius.x;
            fn.normalize();
        }
        return orient.transposedtransform(fn);
    }

    vec3 supportpoint(const vec3 &n) const {
        vec3 ln = orient.transform(n), p(0, 0, 0);
        if (ln.z > 0)
            p.z += radius.z;
        else
            p.z -= radius.z;
        if (ln.x || ln.y) {
            float r = ln.magnitude2();
            p.x += ln.x * radius.x / r;
            p.y += ln.y * radius.y / r;
        }
        return orient.transposedtransform(p).add(o);
    }
};

const float boundarytolerance = 1e-3f;

template<class T, class U> bool collide(const T &p1, const U &p2) {
    // v0 = center of Minkowski difference
    vec3 v0 = p2.center().sub(p1.center());
    if (v0.iszero()) return true;  // v0 and origin overlap ==> hit

    // v1 = support in direction of origin
    vec3 n = vec3(v0).neg();
    vec3 v1 = p2.supportpoint(n).sub(p1.supportpoint(vec3(n).neg()));
    if (v1.dot(n) <= 0) return false;  // origin outside v1 support plane ==> miss

    // v2 = support perpendicular to plane containing origin, v0 and v1
    n.cross(v1, v0);
    if (n.iszero())
        return true;  // v0, v1 and origin colinear (and origin inside v1 support plane) == > hit
    vec3 v2 = p2.supportpoint(n).sub(p1.supportpoint(vec3(n).neg()));
    if (v2.dot(n) <= 0) return false;  // origin outside v2 support plane ==> miss

    // v3 = support perpendicular to plane containing v0, v1 and v2
    n.cross(v0, v1, v2);

    // If the origin is on the - side of the plane, reverse the direction of the plane
    if (n.dot(v0) > 0) {
        swap(v1, v2);
        n.neg();
    }

    ///
    // Phase One: Find a valid portal

    loopi(100) {
        // Obtain the next support point
        vec3 v3 = p2.supportpoint(n).sub(p1.supportpoint(vec3(n).neg()));
        if (v3.dot(n) <= 0) return false;  // origin outside v3 support plane ==> miss

        // If origin is outside (v1,v0,v3), then portal is invalid -- eliminate v2 and find new
        // support outside face
        vec3 v3xv0;
        v3xv0.cross(v3, v0);
        if (v1.dot(v3xv0) < 0) {
            v2 = v3;
            n.cross(v0, v1, v3);
            continue;
        }

        // If origin is outside (v3,v0,v2), then portal is invalid -- eliminate v1 and find new
        // support outside face
        if (v2.dot(v3xv0) > 0) {
            v1 = v3;
            n.cross(v0, v3, v2);
            continue;
        }

        ///
        // Phase Two: Refine the portal

        for (int j = 0;; j++) {
            // Compute outward facing normal of the portal
            n.cross(v1, v2, v3);

            // If the origin is inside the portal, we have a hit
            if (n.dot(v1) >= 0) return true;

            n.normalize();

            // Find the support point in the direction of the portal's normal
            vec3 v4 = p2.supportpoint(n).sub(p1.supportpoint(vec3(n).neg()));

            // If the origin is outside the support plane or the boundary is thin enough, we have a
            // miss
            if (v4.dot(n) <= 0 || vec3(v4).sub(v3).dot(n) <= boundarytolerance || j > 100)
                return false;

            // Test origin against the three planes that separate the new portal candidates:
            // (v1,v4,v0) (v2,v4,v0) (v3,v4,v0) Note:  We're taking advantage of the triple product
            // identities here as an optimization
            //        (v1 % v4) * v0 == v1 * (v4 % v0)    > 0 if origin inside (v1, v4, v0)
            //        (v2 % v4) * v0 == v2 * (v4 % v0)    > 0 if origin inside (v2, v4, v0)
            //        (v3 % v4) * v0 == v3 * (v4 % v0)    > 0 if origin inside (v3, v4, v0)
            vec3 v4xv0;
            v4xv0.cross(v4, v0);
            if (v1.dot(v4xv0) > 0) {
                if (v2.dot(v4xv0) > 0)
                    v1 = v4;  // Inside v1 & inside v2 ==> eliminate v1
                else
                    v3 = v4;  // Inside v1 & outside v2 ==> eliminate v3
            } else {
                if (v3.dot(v4xv0) > 0)
                    v2 = v4;  // Outside v1 & inside v3 ==> eliminate v2
                else
                    v1 = v4;  // Outside v1 & outside v3 ==> eliminate v1
            }
        }
    }
    return false;
}

template<class T, class U>
bool collide(const T &p1, const U &p2, vec3 *contactnormal, vec3 *contactpoint1, vec3 *contactpoint2) {
    // v0 = center of Minkowski sum
    vec3 v01 = p1.center();
    vec3 v02 = p2.center();
    vec3 v0 = vec3(v02).sub(v01);

    // Avoid case where centers overlap -- any direction is fine in this case
    if (v0.iszero()) v0 = vec3(0, 0, 1e-5f);

    // v1 = support in direction of origin
    vec3 n = vec3(v0).neg();
    vec3 v11 = p1.supportpoint(vec3(n).neg());
    vec3 v12 = p2.supportpoint(n);
    vec3 v1 = vec3(v12).sub(v11);
    if (v1.dot(n) <= 0) {
        if (contactnormal) *contactnormal = n;
        return false;
    }

    // v2 - support perpendicular to v1,v0
    n.cross(v1, v0);
    if (n.iszero()) {
        n = vec3(v1).sub(v0);
        n.normalize();
        if (contactnormal) *contactnormal = n;
        if (contactpoint1) *contactpoint1 = v11;
        if (contactpoint2) *contactpoint2 = v12;
        return true;
    }
    vec3 v21 = p1.supportpoint(vec3(n).neg());
    vec3 v22 = p2.supportpoint(n);
    vec3 v2 = vec3(v22).sub(v21);
    if (v2.dot(n) <= 0) {
        if (contactnormal) *contactnormal = n;
        return false;
    }

    // Determine whether origin is on + or - side of plane (v1,v0,v2)
    n.cross(v0, v1, v2);
    assert(!n.iszero());
    // If the origin is on the - side of the plane, reverse the direction of the plane
    if (n.dot(v0) > 0) {
        swap(v1, v2);
        swap(v11, v21);
        swap(v12, v22);
        n.neg();
    }

    ///
    // Phase One: Identify a portal

    loopi(100) {
        // Obtain the support point in a direction perpendicular to the existing plane
        // Note: This point is guaranteed to lie off the plane
        vec3 v31 = p1.supportpoint(vec3(n).neg());
        vec3 v32 = p2.supportpoint(n);
        vec3 v3 = vec3(v32).sub(v31);
        if (v3.dot(n) <= 0) {
            if (contactnormal) *contactnormal = n;
            return false;
        }

        // If origin is outside (v1,v0,v3), then eliminate v2 and loop
        vec3 v3xv0;
        v3xv0.cross(v3, v0);
        if (v1.dot(v3xv0) < 0) {
            v2 = v3;
            v21 = v31;
            v22 = v32;
            n.cross(v0, v1, v3);
            continue;
        }

        // If origin is outside (v3,v0,v2), then eliminate v1 and loop
        if (v2.dot(v3xv0) > 0) {
            v1 = v3;
            v11 = v31;
            v12 = v32;
            n.cross(v0, v3, v2);
            continue;
        }

        bool hit = false;

        ///
        // Phase Two: Refine the portal

        // We are now inside of a wedge...
        for (int j = 0;; j++) {
            // Compute normal of the wedge face
            n.cross(v1, v2, v3);

            // Can this happen???  Can it be handled more cleanly?
            if (n.iszero()) {
                assert(0);
                return true;
            }

            n.normalize();

            // If the origin is inside the wedge, we have a hit
            if (n.dot(v1) >= 0 && !hit) {
                if (contactnormal) *contactnormal = n;

                // Compute the barycentric coordinates of the origin
                if (contactpoint1 || contactpoint2) {
                    float b0 = v3.scalartriple(v1, v2), b1 = v0.scalartriple(v3, v2),
                          b2 = v3.scalartriple(v0, v1), b3 = v0.scalartriple(v2, v1),
                          sum = b0 + b1 + b2 + b3;
                    if (sum <= 0) {
                        b0 = 0;
                        b1 = n.scalartriple(v2, v3);
                        b2 = n.scalartriple(v3, v1);
                        b3 = n.scalartriple(v1, v2);
                        sum = b1 + b2 + b3;
                    }
                    if (contactpoint1)
                        *contactpoint1 = (vec3(v01)
                                              .mul(b0)
                                              .add(vec3(v11).mul(b1))
                                              .add(vec3(v21).mul(b2))
                                              .add(vec3(v31).mul(b3)))
                                             .mul(1.0f / sum);
                    if (contactpoint2)
                        *contactpoint2 = (vec3(v02)
                                              .mul(b0)
                                              .add(vec3(v12).mul(b1))
                                              .add(vec3(v22).mul(b2))
                                              .add(vec3(v32).mul(b3)))
                                             .mul(1.0f / sum);
                }

                // HIT!!!
                hit = true;
            }

            // Find the support point in the direction of the wedge face
            vec3 v41 = p1.supportpoint(vec3(n).neg());
            vec3 v42 = p2.supportpoint(n);
            vec3 v4 = vec3(v42).sub(v41);

            // If the boundary is thin enough or the origin is outside the support plane for the
            // newly discovered vertex, then we can terminate
            if (v4.dot(n) <= 0 || vec3(v4).sub(v3).dot(n) <= boundarytolerance || j > 100) {
                if (contactnormal) *contactnormal = n;
                return hit;
            }

            // Test origin against the three planes that separate the new portal candidates:
            // (v1,v4,v0) (v2,v4,v0) (v3,v4,v0) Note:  We're taking advantage of the triple product
            // identities here as an optimization
            //        (v1 % v4) * v0 == v1 * (v4 % v0)    > 0 if origin inside (v1, v4, v0)
            //        (v2 % v4) * v0 == v2 * (v4 % v0)    > 0 if origin inside (v2, v4, v0)
            //        (v3 % v4) * v0 == v3 * (v4 % v0)    > 0 if origin inside (v3, v4, v0)
            vec3 v4xv0;
            v4xv0.cross(v4, v0);
            if (v1.dot(v4xv0) > 0)  // Compute the tetrahedron dividing face d1 = (v4,v0,v1)
            {
                if (v2.dot(v4xv0) > 0)  // Compute the tetrahedron dividing face d2 = (v4,v0,v2)
                {
                    // Inside d1 & inside d2 ==> eliminate v1
                    v1 = v4;
                    v11 = v41;
                    v12 = v42;
                } else {
                    // Inside d1 & outside d2 ==> eliminate v3
                    v3 = v4;
                    v31 = v41;
                    v32 = v42;
                }
            } else {
                if (v3.dot(v4xv0) > 0)  // Compute the tetrahedron dividing face d3 = (v4,v0,v3)
                {
                    // Outside d1 & inside d3 ==> eliminate v2
                    v2 = v4;
                    v21 = v41;
                    v22 = v42;
                } else {
                    // Outside d1 & outside d3 ==> eliminate v1
                    v1 = v4;
                    v11 = v41;
                    v12 = v42;
                }
            }
        }
    }
    return false;
}
}


// -------------------------------------------------------------------------------------------------
// physics.cpp: no physics books were hurt nor consulted in the construction of this code.
// All physics computations and constants were invented on the fly and simply tweaked until
// they "felt right", and have no basis in reality. Collision detection is simplistic but
// very robust (uses discrete steps at fixed fps).

bool isempty(OcVal c) { return c.LeafData() == 0; }
bool isentirelysolid(OcVal c) { return c.LeafData() != 0; }

namespace game {

bool allowmove(physent *) { return true; }

void bounced(physent *, const vec3 &) {}

void playsound(const char *id, physent *d) { d->event_id = id; }

void physicstrigger(physent *d, bool /*local*/, int floorlevel, int waterlevel, int material = 0) {
    if (d->type == ENT_INANIMATE) return;
    if (waterlevel > 0) {
        if (material != MAT_LAVA) playsound("water_out", d);
    } else if (waterlevel < 0) {
        playsound(material == MAT_LAVA ? "lava_in" : "water_in", d);
    }
    if (floorlevel > 0) {
        playsound("jump", d);
    } else if (floorlevel < 0) {
        playsound("land", d);
    }
}

void suicide(physent *) {}  // touched MAT_DEATH.

int scaletime(int t) { return t * 100; }

}

// This function only for empty space, not solids.
int lookupmaterial(const vec3 &v) {
    // FIXME!!!
    return v.z < 64 + 1 + 0.1f ? MAT_WATER : MAT_AIR;
}

/////////////////////////  ray - cube collision ///////////////////////////////////////////////

vec3 hitsurface;

bool raycubeintersect(const int3 &co, int size, const vec3 &v, const vec3 &ray, const vec3 &invray,
                      float &dist) {
    auto bbmin = float3(co);
    auto bbmax = bbmin + float(size);
    auto rayo = float3(v.v);
    auto reciprocal_raydir = float3(invray.v);
    auto v1 = (bbmin - rayo) * reciprocal_raydir;
    auto v2 = (bbmax - rayo) * reciprocal_raydir;
    auto n = min(v1, v2);
    auto f = max(v1, v2);
    auto enter = max(n.x, max(n.y, n.z));
    auto exit = min(f.x, min(f.y, f.z));
    dist = max(enter, 0.0f);
    auto hit = exit >= dist;
    if (!hit) return false;
    auto hitpoint = rayo + float3(ray.v) * dist;
    auto p = (hitpoint - bbmin) / float(size);
    auto r = p - 0.5f;
    auto ar = abs(r);
    hitsurface = vec3(0.0f);
    // Derive cube normal from vector.
    int dim = ar.x > ar.y && ar.x > ar.z ? 0 : (ar.y > ar.z ? 1 : 2);
    hitsurface[dim] = float(r[dim] >= 0) * 2.0f - 1.0f;
    return true;
}

float hitentdist;
int hitent, hitorient;

static inline bool insideworld(const vec3 &o, int worldsize) {
    return o.x >= 0 && o.x < worldsize && o.y >= 0 && o.y < worldsize && o.z >= 0 &&
           o.z < worldsize;
}

vector<OcVal *> levels;

float raycube(const vec3 &o, const vec3 &ray, float radius = 0, int mode = RAY_CLIPMAT,
              int size = 0) {
    if (ray.iszero()) return 0;
    float dist = 0, dent = radius > 0 ? radius : 1e16f;
    vec3 v(o);
    vec3 invray(ray.x ? 1 / ray.x : 1e16f, ray.y ? 1 / ray.y : 1e16f, ray.z ? 1 / ray.z : 1e16f);
    int worldsize = 1 << oc->world_bits;
    int lshift = oc->world_bits;
    levels.resize(lshift + 1);
    levels[lshift] = &oc->nodes[OcTree::ROOT_INDEX];
    int3 lsizemask(invray.x > 0 ? 1 : 0, invray.y > 0 ? 1 : 0, invray.z > 0 ? 1 : 0);
    if (!insideworld(o, worldsize)) {
        float disttoworld = 0, exitworld = 1e16f;
        loopi(3) {
            float c = v[i];
            if (c < 0 || c >= worldsize) {
                float d = ((invray[i] > 0 ? 0 : worldsize) - c) * invray[i];
                if (d < 0) return (radius > 0 ? radius : -1);
                disttoworld = max(disttoworld, 0.1f + d);
            }
            float e = ((invray[i] > 0 ? worldsize : 0) - c) * invray[i];
            exitworld = min(exitworld, e);
        }
        if (disttoworld > exitworld) return (radius > 0 ? radius : -1);
        v.add(vec3(ray).mul(disttoworld));
        dist += disttoworld;
    }
    int closest = -1, x = int(v.x), y = int(v.y), z = int(v.z);
    for (;;) {
        auto lc = levels[lshift];
        for (;;) {
            lshift--;
            assert(lshift >= 0);
            lc = lc + oc->Step(int3(x, y, z), lshift);
            if (lc->IsLeaf()) break;
            lc = &oc->nodes[lc->NodeIdx()];
            levels[lshift] = lc;
        }
        int lsize = 1 << lshift;
        auto c = *lc;
        if ((dist > 0 || !(mode & RAY_SKIPFIRST)) &&
            (((mode & RAY_EDITMAT) && !isempty(c)) ||
             (!(mode & RAY_PASS) && lsize == size && !isempty(c)) || isentirelysolid(c) ||
             dent < dist)) {
            if (closest >= 0) {
                hitsurface = vec3(0, 0, 0);
                hitsurface[closest] = ray[closest] > 0 ? -1.0f : 1.0f;
            }
            return min(dent, dist);
        }
        int3 lo(x & (~0U << lshift), y & (~0U << lshift), z & (~0U << lshift));
        if (!isempty(c)) {
            float f = 0;
            if (raycubeintersect(lo, lsize, v, ray, invray, f) &&
                (dist + f > 0 || !(mode & RAY_SKIPFIRST)))
                return min(dent, dist + f);
        }
        float dx = (lo.x + (lsizemask.x << lshift) - v.x) * invray.x,
              dy = (lo.y + (lsizemask.y << lshift) - v.y) * invray.y,
              dz = (lo.z + (lsizemask.z << lshift) - v.z) * invray.z;
        float disttonext = dx;
        closest = 0;
        if (dy < disttonext) {
            disttonext = dy;
            closest = 1;
        }
        if (dz < disttonext) {
            disttonext = dz;
            closest = 2;
        }
        disttonext += 0.1f;
        v.add(vec3(ray).mul(disttonext));
        dist += disttonext;
        if (radius > 0 && dist >= radius) return min(dent, dist);
        x = int(v.x);
        y = int(v.y);
        z = int(v.z);
        uint32_t diff = uint32_t(lo.x ^ x) | uint32_t(lo.y ^ y) | uint32_t(lo.z ^ z);
        if (diff >= uint32_t(worldsize)) return min(dent, radius > 0 ? radius : dist);
        diff >>= lshift;
        if (!diff) return min(dent, radius > 0 ? radius : dist);
        do {
            lshift++;
            diff >>= 1;
        } while (diff);
    }
}

float raycubepos(const vec3 &o, const vec3 &ray, vec3 &hitpos, float radius, int mode = RAY_CLIPMAT,
                 int size = 0) {
    hitpos = ray;
    float dist = raycube(o, ray, radius, mode, size);
    if (radius > 0 && dist >= radius) dist = radius;
    hitpos.mul(dist).add(o);
    return dist;
}

bool raycubelos(const vec3 &o, const vec3 &dest, vec3 &hitpos) {
    vec3 ray(dest);
    ray.sub(o);
    float mag = ray.magnitude();
    ray.mul(1 / mag);
    float distance = raycubepos(o, ray, hitpos, mag, RAY_CLIPMAT | RAY_POLY);
    return distance >= mag;
}

float rayfloor(const vec3 &o, vec3 &floor, int mode = 0, float radius = 0) {
    if (o.z <= 0) return -1;
    hitsurface = vec3(0, 0, 1);
    float dist = raycube(o, vec3(0, 0, -1), radius, mode);
    if (dist < 0 || (radius > 0 && dist >= radius)) return dist;
    floor = hitsurface;
    return dist;
}


/////////////////////////  entity collision  ///////////////////////////////////////////////

// info about collisions
int collideinside = 0;       // whether an internal collision happened
physent *collideplayer = nullptr;  // whether the collection hit a player
vec3 collidewall = vec3(0, 0, 0);         // just the normal vectors.

// FIXME!!!
const float STAIRHEIGHT = 4.1f / 4;
const float FLOORZ = 0.867f / 4;
const float SLOPEZ = 0.5f / 4;
const float WALLZ = 0.2f / 4;
const float JUMPVEL = 125.0f / 4;
const float GRAVITY = 200.0f / 4;

/*

bool ellipsecollide(physent *d, const vec3 &dir, const vec3 &o, const vec3 &center, float yaw,
                    float xr, float yr, float hi, float lo) {
    float below = (o.z + center.z - lo) - (d->o.z + d->aboveeye),
          above = (d->o.z - d->eyeheight) - (o.z + center.z + hi);
    if (below >= 0 || above >= 0) return false;
    vec3 yo(center);
    yo.rotate_around_z(yaw * RAD);
    yo.add(o);
    float x = yo.x - d->o.x, y = yo.y - d->o.y;
    float angle = atan2f(y, x), dangle = angle - (d->yaw + 90) * RAD,
          eangle = angle - (yaw + 90) * RAD;
    float dx = d->xradius * cosf(dangle), dy = d->yradius * sinf(dangle);
    float ex = xr * cosf(eangle), ey = yr * sinf(eangle);
    float dist = sqrtf(x * x + y * y) - sqrtf(dx * dx + dy * dy) - sqrtf(ex * ex + ey * ey);
    if (dist < 0) {
        if (dist > (d->o.z < yo.z ? below : above) && (dir.iszero() || x * dir.x + y * dir.y > 0)) {
            collidewall = vec3(-x, -y, 0);
            if (!collidewall.iszero()) collidewall.normalize();
            return true;
        }
        if (d->o.z < yo.z) {
            if (dir.iszero() ||
                (dir.z > 0 && (d->type >= ENT_INANIMATE ||
                               below >= d->zmargin - (d->eyeheight + d->aboveeye) / 4.0f))) {
                collidewall = vec3(0, 0, -1);
                return true;
            }
        } else if (dir.iszero() ||
                   (dir.z < 0 && (d->type >= ENT_INANIMATE ||
                                  above >= d->zmargin - (d->eyeheight + d->aboveeye) / 3.0f))) {
            collidewall = vec3(0, 0, 1);
            return true;
        }
        collideinside++;
    }
    return false;
}

template<class E, class O> bool plcollide(physent *d, const vec3 &dir, physent *o) {
    E entvol(d);
    O obvol(o);
    vec3 cp;
    if (mpr::collide(entvol, obvol, NULL, NULL, &cp)) {
        vec3 wn = vec3(cp).sub(obvol.center());
        collidewall = obvol.contactface(wn, dir.iszero() ? vec3(wn).neg() : dir);
        if (!collidewall.iszero()) return true;
        collideinside++;
    }
    return false;
}

bool plcollide(physent *d, const vec3 &dir, physent *o) {
    switch (d->collidetype) {
        case COLLIDE_ELLIPSE:
        case COLLIDE_ELLIPSE_PRECISE:
            eturn ellipsecollide(d, dir, o->o, vec3(0, 0, 0), o->yaw, o->xradius, o->yradius,
                                 o->aboveeye, o->eyeheight);
        default: return false;
    }
}

bool plcollide(physent *d, const vec3 &dir, bool insideplayercol)  // collide with player or monster
{
    if (d->type == ENT_CAMERA || d->state != CS_ALIVE) return false;
    int lastinside = collideinside;
    physent *insideplayer = NULL;
    loopdynentcache(x, y, d->o, d->radius) {
        const vector<physent *> &dynents = checkdynentcache(x, y);
        loopv(dynents) {
            physent *o = dynents[i];
            if (o == d || d->o.reject(o->o, d->radius + o->radius)) continue;
            if (plcollide(d, dir, o)) {
                collideplayer = o;
                game::dynentcollide(d, o, collidewall);
                return true;
            }
            if (collideinside > lastinside) {
                lastinside = collideinside;
                insideplayer = o;
            }
        }
    }
    if (insideplayer && insideplayercol) {
        collideplayer = insideplayer;
        game::dynentcollide(d, insideplayer, vec3(0, 0, 0));
        return true;
    }
    return false;
}

*/

// collide with solid cube geometry
template<class E>
bool fuzzycollidesolid(physent *d, const vec3 &dir, float cutoff, const int3 &co, int size) {
    float crad = size / 2.0f;
    if (fabs(d->o.x - co.x - crad) > d->radius + crad ||
        fabs(d->o.y - co.y - crad) > d->radius + crad || d->o.z + d->aboveeye < co.z ||
        d->o.z - d->eyeheight > co.z + size)
        return false;
    E entvol(d);
    collidewall = vec3(0, 0, 0);
    float bestdist = -1e10f;
    #define CHECKSIDE(distval, dotval, margin, normal)                             \
    do {                                                                           \
            float dist = distval;                                                  \
            if (dist > 0)                                                          \
                return false;                                                      \
            if (dist <= bestdist) continue;                                        \
            if (!dir.iszero()) {                                                   \
                if (dotval >= -cutoff * dir.magnitude()) continue;                 \
                if (d->type < ENT_CAMERA && dotval < 0 && dist < margin) continue; \
            }                                                                      \
            collidewall = normal;                                                  \
            bestdist = dist;                                                       \
    } while (0)
    CHECKSIDE(co.x - (d->o.x + d->radius), -dir.x, -d->radius, vec3(-1, 0, 0));
    CHECKSIDE(d->o.x - d->radius - (co.x + size), dir.x, -d->radius, vec3(1, 0, 0));
    CHECKSIDE(co.y - (d->o.y + d->radius), -dir.y, -d->radius, vec3(0, -1, 0));
    CHECKSIDE(d->o.y - d->radius - (co.y + size), dir.y, -d->radius, vec3(0, 1, 0));
    CHECKSIDE(co.z - (d->o.z + d->aboveeye), -dir.z,
              d->zmargin - (d->eyeheight + d->aboveeye) / 4.0f, vec3(0, 0, -1));
    CHECKSIDE(d->o.z - d->eyeheight - (co.z + size), dir.z,
              d->zmargin - (d->eyeheight + d->aboveeye) / 3.0f, vec3(0, 0, 1));
    if (collidewall.iszero()) {
        collideinside++;
        return false;
    }
    return true;
}

template<class E>
bool clampcollide(const clipplanes &p, const E &entvol, const plane &w, const vec3 &pw) {
    if (w.x && (w.y || w.z) && fabs(pw.x - p.o.x) > p.r.x) {
        vec3 c = entvol.center();
        float fv = pw.x < p.o.x ? p.o.x - p.r.x : p.o.x + p.r.x,
              fdist = (w.x * fv + w.y * c.y + w.z * c.z + w.offset) / (w.y * w.y + w.z * w.z);
        vec3 fdir(fv - c.x, -w.y * fdist, -w.z * fdist);
        if ((pw.y - c.y - fdir.y) * w.y + (pw.z - c.z - fdir.z) * w.z >= 0 &&
            entvol.supportpoint(fdir).squaredist(c) < fdir.squaredlen())
            return true;
    }
    if (w.y && (w.x || w.z) && fabs(pw.y - p.o.y) > p.r.y) {
        vec3 c = entvol.center();
        float fv = pw.y < p.o.y ? p.o.y - p.r.y : p.o.y + p.r.y,
              fdist = (w.x * c.x + w.y * fv + w.z * c.z + w.offset) / (w.x * w.x + w.z * w.z);
        vec3 fdir(-w.x * fdist, fv - c.y, -w.z * fdist);
        if ((pw.x - c.x - fdir.x) * w.x + (pw.z - c.z - fdir.z) * w.z >= 0 &&
            entvol.supportpoint(fdir).squaredist(c) < fdir.squaredlen())
            return true;
    }
    if (w.z && (w.x || w.y) && fabs(pw.z - p.o.z) > p.r.z) {
        vec3 c = entvol.center();
        float fv = pw.z < p.o.z ? p.o.z - p.r.z : p.o.z + p.r.z,
              fdist = (w.x * c.x + w.y * c.y + w.z * fv + w.offset) / (w.x * w.x + w.y * w.y);
        vec3 fdir(-w.x * fdist, -w.y * fdist, fv - c.z);
        if ((pw.x - c.x - fdir.x) * w.x + (pw.y - c.y - fdir.y) * w.y >= 0 &&
            entvol.supportpoint(fdir).squaredist(c) < fdir.squaredlen())
            return true;
    }
    return false;
}

// collide with solid cube geometry
template<class E>
bool cubecollidesolid(physent *d, const vec3 &dir, float cutoff, const int3 &co, int size) {
    float crad = size / 2.0f;
    if (fabs(d->o.x - co.x - crad) > d->radius + crad ||
        fabs(d->o.y - co.y - crad) > d->radius + crad || d->o.z + d->aboveeye < co.z ||
        d->o.z - d->eyeheight > co.z + size)
        return false;
    E entvol(d);
    bool collided = mpr::collide(mpr::SolidCube(co, size), entvol);
    if (!collided)
        return false;
    collidewall = vec3(0, 0, 0);
    float bestdist = -1e10f;
    CHECKSIDE(co.x - entvol.right(), -dir.x, -d->radius, vec3(-1, 0, 0));
    CHECKSIDE(entvol.left() - (co.x + size), dir.x, -d->radius, vec3(1, 0, 0));
    CHECKSIDE(co.y - entvol.front(), -dir.y, -d->radius, vec3(0, -1, 0));
    CHECKSIDE(entvol.back() - (co.y + size), dir.y, -d->radius, vec3(0, 1, 0));
    CHECKSIDE(co.z - entvol.top(), -dir.z,
              d->zmargin - (d->eyeheight + d->aboveeye) / 4.0f, vec3(0, 0, -1));
    CHECKSIDE(entvol.bottom() - (co.z + size), dir.z,
              d->zmargin - (d->eyeheight + d->aboveeye) / 3.0f, vec3(0, 0, 1));
    if (collidewall.iszero()) {
        collideinside++;
        return false;
    }
    return true;
}

bool cubecollide(physent *d, const vec3 &dir, float cutoff, const int3 &co, int size) {
    switch (d->collidetype) {
        case COLLIDE_ELLIPSE:
            return fuzzycollidesolid<mpr::EntCapsule>(d, dir, cutoff, co, size);
        case COLLIDE_ELLIPSE_PRECISE:
            return cubecollidesolid<mpr::EntCapsule>(d, dir, cutoff, co, size);
        default:
            return false;
    }
}

uint8_t octaboxoverlap(const int3 &o, int size, const int3 &bbmin, const int3 &bbmax) {
    uint8_t p = 0xFF;  // bitmask of possible collisions with octants. 0 bit = 0 octant, etc
    int3 mid = int3(o) + size;
    if (mid.z <= bbmin.z)
        p &= 0xF0;  // not in a -ve Z octant
    else if (mid.z >= bbmax.z)
        p &= 0x0F;  // not in a +ve Z octant
    if (mid.y <= bbmin.y)
        p &= 0xCC;  // not in a -ve Y octant
    else if (mid.y >= bbmax.y)
        p &= 0x33;  // etc..
    if (mid.x <= bbmin.x)
        p &= 0xAA;
    else if (mid.x >= bbmax.x)
        p &= 0x55;
    return p;
}

int3 c_int3(int i, const int3 &co, int size) {
    return int3(co.x + ((i & 1) >> 0) * size,
                co.y + ((i & 2) >> 1) * size,
                co.z + ((i & 4) >> 2) * size);
}

// collide with octants
bool octacollide(physent *d, const vec3 &dir, float cutoff, const int3 &bo, const int3 &bs,
                 const OcVal *c, const int3 &cor, int size) {
    auto possible = octaboxoverlap(cor, size, bo, bs);
    loopi(8) if (possible & (1 << i)) {
        int3 o = c_int3(i, cor, size);
        if (!c[i].IsLeaf()) {
            if (octacollide(d, dir, cutoff, bo, bs, &oc->nodes[c[i].NodeIdx()], o, size >> 1))
                return true;
        } else {
            if (isempty(c[i]))
                continue;
            if (cubecollide(d, dir, cutoff, o, size))
                return true;
        }
    }
    return false;
}

bool octacollide(physent *d, const vec3 &dir, float cutoff, const int3 &bo, const int3 &bs) {
    int diff = (bo.x ^ bs.x) | (bo.y ^ bs.y) | (bo.z ^ bs.z);
    int scale = oc->world_bits - 1;
    int worldsize = 1 << oc->world_bits;
    auto worldroot = &oc->nodes[OcTree::ROOT_INDEX];
    if (diff & ~((1 << scale) - 1) ||
        uint32_t(bo.x | bo.y | bo.z | bs.x | bs.y | bs.z) >= uint32_t(worldsize))
        return octacollide(d, dir, cutoff, bo, bs, worldroot, int3(0, 0, 0), worldsize >> 1);
    auto c = oc->nodes[oc->Step(bo, scale) + OcTree::ROOT_INDEX];
    scale--;
    while (!c.IsLeaf() && !(diff & (1 << scale))) {
        c = oc->nodes[oc->Step(bo, scale) + c.NodeIdx()];
        scale--;
    }
    if (!c.IsLeaf())
        return octacollide(d, dir, cutoff, bo, bs, &oc->nodes[c.NodeIdx()], int3(bo) & (~((2 << scale) - 1)),
                           1 << scale);
    if (isempty(c))
        return false;
    int csize = 2 << scale, cmask = ~(csize - 1);
    return cubecollide(d, dir, cutoff, int3(bo) & cmask, csize);
}

// all collision happens here
bool collide(physent *d, const vec3 &dir = vec3(0, 0, 0), float cutoff = 0.0f, bool /*playercol*/ = true,
             bool /*insideplayercol*/ = false) {
    collideinside = 0;
    collideplayer = NULL;
    collidewall = vec3(0, 0, 0);
    int3 bo(int(d->o.x - d->radius), int(d->o.y - d->radius), int(d->o.z - d->eyeheight)),
         bs(int(d->o.x + d->radius), int(d->o.y + d->radius), int(d->o.z + d->aboveeye));
    bs += 1;  // guard space for rounding errors
    return octacollide(d, dir, cutoff, bo, bs);
    // || (playercol && plcollide(d, dir, insideplayercol));
}

void recalcdir(physent *d, const vec3 &oldvel, vec3 &dir) {
    float speed = oldvel.magnitude();
    if (speed > 1e-6f) {
        float step = dir.magnitude();
        dir = d->vel;
        dir.add(d->falling);
        dir.mul(step / speed);
    }
}

void slideagainst(physent *d, vec3 &dir, const vec3 &obstacle, bool foundfloor, bool slidecollide) {
    vec3 wall(obstacle);
    if (foundfloor ? wall.z > 0 : slidecollide) {
        wall.z = 0;
        if (!wall.iszero()) wall.normalize();
    }
    vec3 oldvel(d->vel);
    oldvel.add(d->falling);
    d->vel.project(wall);
    d->falling.project(wall);
    recalcdir(d, oldvel, dir);
}

void switchfloor(physent *d, vec3 &dir, const vec3 &floor) {
    if (floor.z >= FLOORZ) d->falling = vec3(0, 0, 0);

    vec3 oldvel(d->vel);
    oldvel.add(d->falling);
    if (dir.dot(floor) >= 0) {
        if (d->physstate < PHYS_SLIDE || fabs(dir.dot(d->floor)) > 0.01f * dir.magnitude()) return;
        d->vel.projectxy(floor, 0.0f);
    } else
        d->vel.projectxy(floor);
    d->falling.project(floor);
    recalcdir(d, oldvel, dir);
}

bool trystepup(physent *d, vec3 &dir, const vec3 &obstacle, float maxstep, const vec3 &floor) {
    vec3 old(d->o),
        stairdir = (obstacle.z >= 0 && obstacle.z < SLOPEZ ? vec3(-obstacle.x, -obstacle.y, 0)
                                                           : vec3(dir.x, dir.y, 0))
                       .rescale(1);
    bool cansmooth = true;
    /* check if there is space atop the stair to move to */
    if (d->physstate != PHYS_STEP_UP) {
        vec3 checkdir = stairdir;
        checkdir.mul(0.1f);
        checkdir.z += maxstep + 0.1f;
        d->o.add(checkdir);
        if (collide(d)) {
            d->o = old;
            if (!collide(d, vec3(0, 0, -1), SLOPEZ)) return false;
            cansmooth = false;
        }
    }

    if (cansmooth) {
        vec3 checkdir = stairdir;
        checkdir.z += 1;
        checkdir.mul(maxstep);
        d->o = old;
        d->o.add(checkdir);
        int scale = 2;
        if (collide(d, checkdir)) {
            if (!collide(d, vec3(0, 0, -1), SLOPEZ)) {
                d->o = old;
                return false;
            }
            d->o.add(checkdir);
            if (collide(d, vec3(0, 0, -1), SLOPEZ)) scale = 1;
        }
        if (scale != 1) {
            d->o = old;
            d->o.sub(checkdir.mul(vec3(2, 2, 1)));
            if (!collide(d, vec3(0, 0, -1), SLOPEZ)) scale = 1;
        }

        d->o = old;
        vec3 smoothdir(dir.x, dir.y, 0);
        float magxy = smoothdir.magnitude();
        if (magxy > 1e-9f) {
            if (magxy > scale * dir.z) {
                smoothdir.mul(1 / magxy);
                smoothdir.z = 1.0f / scale;
                smoothdir.mul(dir.magnitude() / smoothdir.magnitude());
            } else
                smoothdir.z = dir.z;
            d->o.add(smoothdir);
            d->o.z += maxstep + 0.1f;
            if (!collide(d, smoothdir)) {
                d->o.z -= maxstep + 0.1f;
                if (d->physstate == PHYS_FALL || d->floor != floor) {
                    d->timeinair = 0;
                    d->floor = floor;
                    switchfloor(d, dir, d->floor);
                }
                d->physstate = PHYS_STEP_UP;
                return true;
            }
        }
    }

    /* try stepping up */
    d->o = old;
    d->o.z += dir.magnitude();
    if (!collide(d, vec3(0, 0, 1))) {
        if (d->physstate == PHYS_FALL || d->floor != floor) {
            d->timeinair = 0;
            d->floor = floor;
            switchfloor(d, dir, d->floor);
        }
        if (cansmooth) d->physstate = PHYS_STEP_UP;
        return true;
    }
    d->o = old;
    return false;
}

bool trystepdown(physent *d, vec3 &dir, float step, float xy, float z, bool init = false) {
    vec3 stepdir(dir.x, dir.y, 0);
    stepdir.z = -stepdir.magnitude2() * z / xy;
    if (!stepdir.z) return false;
    stepdir.normalize();

    vec3 old(d->o);
    d->o.add(vec3(stepdir).mul(STAIRHEIGHT / fabs(stepdir.z))).z -= STAIRHEIGHT;
    d->zmargin = -STAIRHEIGHT;
    if (collide(d, vec3(0, 0, -1), SLOPEZ)) {
        d->o = old;
        d->o.add(vec3(stepdir).mul(step));
        d->zmargin = 0;
        if (!collide(d, vec3(0, 0, -1))) {
            vec3 stepfloor(stepdir);
            stepfloor.mul(-stepfloor.z).z += 1;
            stepfloor.normalize();
            if (d->physstate >= PHYS_SLOPE && d->floor != stepfloor) {
                // prevent alternating step-down/step-up states if player would keep bumping into
                // the same floor
                vec3 stepped(d->o);
                d->o.z -= 0.5f;
                d->zmargin = -0.5f;
                if (collide(d, stepdir) && collidewall == d->floor) {
                    d->o = old;
                    if (!init) {
                        d->o.x += dir.x;
                        d->o.y += dir.y;
                        if (dir.z <= 0 || collide(d, dir)) d->o.z += dir.z;
                    }
                    d->zmargin = 0;
                    d->physstate = PHYS_STEP_DOWN;
                    d->timeinair = 0;
                    return true;
                }
                d->o = init ? old : stepped;
                d->zmargin = 0;
            } else if (init)
                d->o = old;
            switchfloor(d, dir, stepfloor);
            d->floor = stepfloor;
            d->physstate = PHYS_STEP_DOWN;
            d->timeinair = 0;
            return true;
        }
    }
    d->o = old;
    d->zmargin = 0;
    return false;
}

bool trystepdown(physent *d, vec3 &dir, bool init = false) {
    if ((!d->move && !d->strafe) || !game::allowmove(d)) return false;
    vec3 old(d->o);
    d->o.z -= STAIRHEIGHT;
    d->zmargin = -STAIRHEIGHT;
    if (!collide(d, vec3(0, 0, -1), SLOPEZ)) {
        d->o = old;
        d->zmargin = 0;
        return false;
    }
    d->o = old;
    d->zmargin = 0;
    float step = dir.magnitude();
    #if 1
        // weaker check, just enough to avoid hopping up slopes
        if (trystepdown(d, dir, step, 4, 1, init)) return true;
    #else
        if (trystepdown(d, dir, step, 2, 1, init)) return true;
        if (trystepdown(d, dir, step, 1, 1, init)) return true;
        if (trystepdown(d, dir, step, 1, 2, init)) return true;
    #endif
    return false;
}

void falling(physent *d, vec3 &dir, const vec3 &floor) {
    if (floor.z > 0.0f && floor.z < SLOPEZ) {
        if (floor.z >= WALLZ) switchfloor(d, dir, floor);
        d->timeinair = 0;
        d->physstate = PHYS_SLIDE;
        d->floor = floor;
    } else if (d->physstate < PHYS_SLOPE || dir.dot(d->floor) > 0.01f * dir.magnitude() ||
               (floor.z != 0.0f && floor.z != 1.0f) || !trystepdown(d, dir, true))
        d->physstate = PHYS_FALL;
}

void landing(physent *d, vec3 &dir, const vec3 &floor, bool collided) {
    #if 0
    if(d->physstate == PHYS_FALL)
    {
        d->timeinair = 0;
        if(dir.z < 0.0f) dir.z = d->vel.z = 0.0f;
    }
    #endif
    switchfloor(d, dir, floor);
    d->timeinair = 0;
    if ((d->physstate != PHYS_STEP_UP && d->physstate != PHYS_STEP_DOWN) || !collided)
        d->physstate = uint8_t(floor.z >= FLOORZ ? PHYS_FLOOR : PHYS_SLOPE);
    d->floor = floor;
}

bool findfloor(physent *d, bool collided, const vec3 &obstacle, bool &slide, vec3 &floor) {
    bool found = false;
    vec3 moved(d->o);
    d->o.z -= 0.1f;
    if (collide(d, vec3(0, 0, -1),
                d->physstate == PHYS_SLOPE || d->physstate == PHYS_STEP_DOWN ? SLOPEZ : FLOORZ)) {
        floor = collidewall;
        found = true;
    } else if (collided && obstacle.z >= SLOPEZ) {
        floor = obstacle;
        found = true;
        slide = false;
    } else if (d->physstate == PHYS_STEP_UP || d->physstate == PHYS_SLIDE) {
        if (collide(d, vec3(0, 0, -1)) && collidewall.z > 0.0f) {
            floor = collidewall;
            if (floor.z >= SLOPEZ) found = true;
        }
    } else if (d->physstate >= PHYS_SLOPE && d->floor.z < 1.0f) {
        if (collide(d, vec3(d->floor).neg(), 0.95f) || collide(d, vec3(0, 0, -1))) {
            floor = collidewall;
            if (floor.z >= SLOPEZ && floor.z < 1.0f) found = true;
        }
    }
    if (collided && (!found || obstacle.z > floor.z)) {
        floor = obstacle;
        slide = !found && (floor.z < WALLZ || floor.z >= SLOPEZ);
    }
    d->o = moved;
    return found;
}

bool move(physent *d, vec3 &dir) {
    vec3 old(d->o);
    bool collided = false, slidecollide = false;
    vec3 obstacle;
    d->o.add(dir);
    if (collide(d, dir) ||
        ((d->type == ENT_AI || d->type == ENT_INANIMATE) && collide(d, vec3(0, 0, 0), 0, false))) {
        obstacle = collidewall;
        /* check to see if there is an obstacle that would prevent this one from being used as a
         * floor (or ceiling bump) */
        if (d->type == ENT_PLAYER &&
            ((collidewall.z >= SLOPEZ && dir.z < 0) || (collidewall.z <= -SLOPEZ && dir.z > 0)) &&
            (dir.x || dir.y) && collide(d, vec3(dir.x, dir.y, 0))) {
            if (collidewall.dot(dir) >= 0) slidecollide = true;
            obstacle = collidewall;
        }
        d->o = old;
        d->o.z -= STAIRHEIGHT;
        d->zmargin = -STAIRHEIGHT;
        if (d->physstate == PHYS_SLOPE || d->physstate == PHYS_FLOOR ||
            (collide(d, vec3(0, 0, -1), SLOPEZ) &&
             (d->physstate == PHYS_STEP_UP || d->physstate == PHYS_STEP_DOWN ||
              collidewall.z >= FLOORZ))) {
            d->o = old;
            d->zmargin = 0;
            if (trystepup(d, dir, obstacle, STAIRHEIGHT,
                          d->physstate == PHYS_SLOPE || d->physstate == PHYS_FLOOR
                              ? d->floor
                              : vec3(collidewall)))
                return true;
        } else {
            d->o = old;
            d->zmargin = 0;
        }
        /* can't step over the obstacle, so just slide against it */
        collided = true;
    } else if (d->physstate == PHYS_STEP_UP) {
        if (collide(d, vec3(0, 0, -1), SLOPEZ)) {
            d->o = old;
            if (trystepup(d, dir, vec3(0, 0, 1), STAIRHEIGHT, vec3(collidewall))) return true;
            d->o.add(dir);
        }
    } else if (d->physstate == PHYS_STEP_DOWN && dir.dot(d->floor) <= 1e-6f) {
        vec3 moved(d->o);
        d->o = old;
        if (trystepdown(d, dir)) return true;
        d->o = moved;
    }
    vec3 floor(0, 0, 0);
    bool slide = collided, found = findfloor(d, collided, obstacle, slide, floor);
    if (slide || (!collided && floor.z > 0 && floor.z < WALLZ)) {
        slideagainst(d, dir, slide ? obstacle : floor, found, slidecollide);
        // if(d->type == ENT_AI || d->type == ENT_INANIMATE)
        d->blocked = true;
    }
    if (found)
        landing(d, dir, floor, collided);
    else
        falling(d, dir, floor);
    return !collided;
}

bool bounce(physent *d, float secs, float elasticity, float waterfric, float grav) {
    // make sure bouncers don't start inside geometry
    if (d->physstate != PHYS_BOUNCE && collide(d, vec3(0, 0, 0), 0, false)) return true;
    int mat = lookupmaterial(vec3(d->o.x, d->o.y, d->o.z + (d->aboveeye - d->eyeheight) / 2));
    bool water = isliquid(mat);
    if (water) {
        d->vel.z -= grav * GRAVITY / 16 * secs;
        d->vel.mul(max(1.0f - secs / waterfric, 0.0f));
    } else
        d->vel.z -= grav * GRAVITY * secs;
    vec3 old(d->o);
    loopi(2) {
        vec3 dir(d->vel);
        dir.mul(secs);
        d->o.add(dir);
        if (!collide(d, dir, 0, true, true)) {
            if (collideinside) {
                d->o = old;
                d->vel.mul(-elasticity);
            }
            break;
        } else if (collideplayer)
            break;
        d->o = old;
        game::bounced(d, collidewall);
        float c = collidewall.dot(d->vel), k = 1.0f + (1.0f - elasticity) * c / d->vel.magnitude();
        d->vel.mul(k);
        d->vel.sub(vec3(collidewall).mul(elasticity * 2.0f * c));
    }
    if (d->physstate != PHYS_BOUNCE) {
        // make sure bouncers don't start inside geometry
        if (d->o == old) return !collideplayer;
        d->physstate = PHYS_BOUNCE;
    }
    return collideplayer != NULL;
}

void avoidcollision(physent *d, const vec3 &dir, physent *obstacle, float space) {
    float rad = obstacle->radius + d->radius;
    vec3 bbmin(obstacle->o);
    bbmin.x -= rad;
    bbmin.y -= rad;
    bbmin.z -= obstacle->eyeheight + d->aboveeye;
    bbmin.sub(space);
    vec3 bbmax(obstacle->o);
    bbmax.x += rad;
    bbmax.y += rad;
    bbmax.z += obstacle->aboveeye + d->eyeheight;
    bbmax.add(space);

    loopi(3) if (d->o[i] <= bbmin[i] || d->o[i] >= bbmax[i]) return;

    float mindist = 1e16f;
    loopi(3) if (dir[i] != 0) {
        float dist = ((dir[i] > 0 ? bbmax[i] : bbmin[i]) - d->o[i]) / dir[i];
        mindist = min(mindist, dist);
    }
    if (mindist >= 0.0f && mindist < 1e15f) d->o.add(vec3(dir).mul(mindist));
}

bool movecamera(physent *pl, const vec3 &dir, float dist, float stepdist) {
    int steps = (int)ceil(dist / stepdist);
    if (steps <= 0) return true;

    vec3 d(dir);
    d.mul(dist / steps);
    loopi(steps) {
        vec3 oldpos(pl->o);
        pl->o.add(d);
        if (collide(pl, vec3(0, 0, 0), 0, false)) {
            pl->o = oldpos;
            return false;
        }
    }
    return true;
}

bool insideworld(const vec3 &o) {
    int worldsize = 1 << oc->world_bits;
    return o.x >= 0 && o.x < worldsize && o.y >= 0 && o.y < worldsize && o.z >= 0 &&
           o.z < worldsize;
}

bool droptofloor(vec3 &o, float radius, float height) {
    static struct dropent : physent {
        dropent() {
            type = ENT_BOUNCE;
            vel = vec3(0, 0, -1);
        }
    } d;
    d.o = o;
    int worldsize = 1 << oc->world_bits;
    if (!insideworld(d.o)) {
        if (d.o.z < worldsize) return false;
        d.o.z = worldsize - 1e-3f;
        if (!insideworld(d.o)) return false;
    }
    vec3 v(0.0001f, 0.0001f, -1);
    v.normalize();
    assert(false);//if (raycube(d.o, v, worldsize) >= worldsize) return false;
    d.radius = d.xradius = d.yradius = radius;
    d.eyeheight = height;
    d.aboveeye = radius;
    if (!movecamera(&d, d.vel, (float)worldsize, 1)) {
        o = d.o;
        return true;
    }
    return false;
}

void vecfromyawpitch(float yaw, float pitch, int move, int strafe, vec3 &m) {
    if (move) {
        m.x = move * -sinf(RAD * yaw);
        m.y = move * cosf(RAD * yaw);
    } else
        m.x = m.y = 0;

    if (pitch) {
        m.x *= cosf(RAD * pitch);
        m.y *= cosf(RAD * pitch);
        m.z = move * sinf(RAD * pitch);
    } else
        m.z = 0;

    if (strafe) {
        m.x += strafe * cosf(RAD * yaw);
        m.y += strafe * sinf(RAD * yaw);
    }
}

void vectoyawpitch(const vec3 &v, float &yaw, float &pitch) {
    if (v.iszero())
        yaw = pitch = 0;
    else {
        yaw = -atan2(v.x, v.y) / RAD;
        pitch = asin(v.z / v.magnitude()) / RAD;
    }
}

const int PHYSFRAMETIME = 5;

int maxroll = 0;  // 0..20
float straferoll = 0.033f;  // 0..90
float faderoll = 0.95f;  // 0..1
int floatspeed = 100;  // 1..10000

void modifyvelocity(physent *pl, bool local, bool water, bool floating, int curtime,
                    bool local_controlled) {
    bool allowmove = game::allowmove(pl);
    if (floating) {
        if (pl->jumping && allowmove) {
            pl->jumping = false;
            pl->vel.z = max(pl->vel.z, JUMPVEL);
        }
    } else if (pl->physstate >= PHYS_SLOPE || water) {
        if (water && !pl->inwater) pl->vel.div(8);
        if (pl->jumping && allowmove) {
            pl->jumping = false;

            pl->vel.z = max(pl->vel.z, JUMPVEL);  // physics impulse upwards
            if (water) {
                pl->vel.x /= 8.0f;
                pl->vel.y /= 8.0f;
            }  // dampen velocity change even harder, gives correct water feel

            game::physicstrigger(pl, local, 1, 0);
        }
    }
    if (!floating && pl->physstate == PHYS_FALL)
        pl->timeinair = (uint16_t)min(pl->timeinair + curtime, 1000);

    vec3 m(0.0f, 0.0f, 0.0f);
    if ((pl->move || pl->strafe) && allowmove) {
        vecfromyawpitch(pl->yaw, floating || water || pl->type == ENT_CAMERA ? pl->pitch : 0,
                        pl->move, pl->strafe, m);

        if (!floating && pl->physstate >= PHYS_SLOPE) {
            /* move up or down slopes in air
             * but only move up slopes in water
             */
            float dz = -(m.x * pl->floor.x + m.y * pl->floor.y) / pl->floor.z;
            m.z = water ? max(m.z, dz) : dz;
        }

        m.normalize();
    }

    vec3 d(m);
    d.mul(pl->maxspeed);
    if (pl->type == ENT_PLAYER) {
        if (floating) {
            if (local_controlled) d.mul(floatspeed / 100.0f);
        } else if (!water && allowmove)
            d.mul((pl->move && !pl->strafe ? 1.3f : 1.0f) *
                  (pl->physstate < PHYS_SLOPE ? 1.3f : 1.0f));
    }
    float fric =
        water && !floating ? 20.0f : (pl->physstate >= PHYS_SLOPE || floating ? 6.0f : 30.0f);
    pl->vel.lerp(d, pl->vel, pow(1 - 1 / fric, curtime / 20.0f));
    // old fps friction
    //    float friction = water && !floating ? 20.0f : (pl->physstate >= PHYS_SLOPE || floating
    //    ? 6.0f : 30.0f); float fpsfric = min(curtime/(20.0f*friction), 1.0f);
    //    pl->vel.lerp(pl->vel, d, fpsfric);
}

void modifygravity(physent *pl, bool water, int curtime) {
    float secs = curtime / 1000.0f;
    vec3 g(0, 0, 0);
    if (pl->physstate == PHYS_FALL)
        g.z -= GRAVITY * secs;
    else if (pl->floor.z > 0 && pl->floor.z < FLOORZ) {
        g.z = -1;
        g.project(pl->floor);
        g.normalize();
        g.mul(GRAVITY * secs);
    }
    if (!water || !game::allowmove(pl) || (!pl->move && !pl->strafe)) pl->falling.add(g);

    if (water || pl->physstate >= PHYS_SLOPE) {
        float fric = water ? 2.0f : 6.0f;
        float c = water ? 1.0f : std::clamp((pl->floor.z - SLOPEZ) / (FLOORZ - SLOPEZ), 0.0f, 1.0f);
        pl->falling.mul(pow(1 - c / fric, curtime / 20.0f));
        // old fps friction
        //        float friction = water ? 2.0f : 6.0f,
        //              fpsfric = friction/curtime*20.0f,
        //              c = water ? 1.0f : std::clamp((pl->floor.z - SLOPEZ)/(FLOORZ-SLOPEZ),
        //              0.0f, 1.0f);
        //        pl->falling.mul(1 - c/fpsfric);
    }
}

// main physics routine, moves a player/monster for a curtime step
// moveres indicated the physics precision (which is lower for monsters and multiplayer prediction)
// local is false for multiplayer prediction

bool moveplayer(physent *pl, int moveres, bool local, int curtime, bool local_controlled) {
    int material =
        lookupmaterial(vec3(pl->o.x, pl->o.y, pl->o.z + (3 * pl->aboveeye - pl->eyeheight) / 4));
    bool water = isliquid(material & MATF_VOLUME);
    bool floating =
        pl->type == ENT_PLAYER && (pl->state == CS_EDITING || pl->state == CS_SPECTATOR);
    float secs = curtime / 1000.f;

    // apply gravity
    if (!floating) modifygravity(pl, water, curtime);
    // apply any player generated changes in velocity
    modifyvelocity(pl, local, water, floating, curtime, local_controlled);

    vec3 d(pl->vel);
    if (!floating && water) d.mul(0.5f);
    d.add(pl->falling);
    d.mul(secs);

    pl->blocked = false;

    if (floating)  // just apply velocity
    {
        if (pl->physstate != PHYS_FLOAT) {
            pl->physstate = PHYS_FLOAT;
            pl->timeinair = 0;
            pl->falling = vec3(0, 0, 0);
        }
        pl->o.add(d);
    } else  // apply velocity with collision
    {
        const float f = 1.0f / moveres;
        const int timeinair = pl->timeinair;
        int collisions = 0;

        d.mul(f);
        loopi(moveres) if (!move(pl, d) && ++collisions < 5)
            i--;  // discrete steps collision detection & sliding
        if (timeinair > 800 && !pl->timeinair &&
            !water)  // if we land after long time must have been a high jump, make thud sound
        {
            game::physicstrigger(pl, local, -1, 0);
        }
    }

    // automatically apply smooth roll when strafing

    if (pl->strafe && maxroll)
        pl->roll =
            std::clamp(pl->roll - pow(std::clamp(1.0f + pl->strafe * pl->roll / maxroll, 0.0f, 1.0f), 0.33f) *
                                 pl->strafe * curtime * straferoll,
                       (float)-maxroll, (float)maxroll);
    else
        pl->roll *=
            curtime == PHYSFRAMETIME ? faderoll : pow(faderoll, curtime / float(PHYSFRAMETIME));

    // play sounds on water transitions

    if (pl->inwater && !water) {
        material =
            lookupmaterial(vec3(pl->o.x, pl->o.y, pl->o.z + (pl->aboveeye - pl->eyeheight) / 2));
        water = isliquid(material & MATF_VOLUME);
    }
    if (!pl->inwater && water)
        game::physicstrigger(pl, local, 0, -1, material & MATF_VOLUME);
    else if (pl->inwater && !water)
        game::physicstrigger(pl, local, 0, 1, pl->inwater);
    pl->inwater = water ? material & MATF_VOLUME : MAT_AIR;

    if (pl->state == CS_ALIVE && (pl->o.z < 0 || material & MAT_DEATH)) game::suicide(pl);

    return true;
}

int physsteps = 0, physframetime = PHYSFRAMETIME, lastphysframe = 0;

void physicsframe(int lastmillis)  // optimally schedule physics frames inside the graphics frames
{
    int diff = lastmillis - lastphysframe;
    if (diff <= 0)
        physsteps = 0;
    else {
        physframetime = std::clamp(game::scaletime(PHYSFRAMETIME) / 100, 1, PHYSFRAMETIME);
        physsteps = (diff + physframetime - 1) / physframetime;
        lastphysframe += physsteps * physframetime;
    }
}

void interppos(physent *pl, int lastmillis) {
    pl->o = pl->newpos;

    int diff = lastphysframe - lastmillis;
    if (diff <= 0) return;

    vec3 deltapos(pl->deltapos);
    deltapos.mul(min(diff, physframetime) / float(physframetime));
    pl->o.add(deltapos);
}

void moveplayer_int(physent *pl, int moveres, bool local, bool local_controlled, int lastmillis) {
    if (physsteps <= 0) {
        if (local) interppos(pl, lastmillis);
        return;
    }

    if (local) pl->o = pl->newpos;
    loopi(physsteps - 1) moveplayer(pl, moveres, local, physframetime, local_controlled);
    if (local) pl->deltapos = pl->o;
    moveplayer(pl, moveres, local, physframetime, local_controlled);
    if (local) {
        pl->newpos = pl->o;
        pl->deltapos.sub(pl->newpos);
        interppos(pl, lastmillis);
    }
}

bool bounce(physent *d, float elasticity, float waterfric, float grav, int lastmillis) {
    if (physsteps <= 0) {
        interppos(d, lastmillis);
        return false;
    }

    d->o = d->newpos;
    bool hitplayer = false;
    loopi(physsteps - 1) {
        if (bounce(d, physframetime / 1000.0f, elasticity, waterfric, grav)) hitplayer = true;
    }
    d->deltapos = d->o;
    if (bounce(d, physframetime / 1000.0f, elasticity, waterfric, grav)) hitplayer = true;
    d->newpos = d->o;
    d->deltapos.sub(d->newpos);
    interppos(d, lastmillis);
    return hitplayer;
}

void updatephysstate(physent *d) {
    if (d->physstate == PHYS_FALL) return;
    d->timeinair = 0;
    vec3 old(d->o);
    /* Attempt to reconstruct the floor state.
     * May be inaccurate since movement collisions are not considered.
     * If good floor is not found, just keep the old floor and hope it's correct enough.
     */
    switch (d->physstate) {
        case PHYS_SLOPE:
        case PHYS_FLOOR:
        case PHYS_STEP_DOWN:
            d->o.z -= 0.15f;
            if (collide(
                    d, vec3(0, 0, -1),
                    d->physstate == PHYS_SLOPE || d->physstate == PHYS_STEP_DOWN ? SLOPEZ : FLOORZ))
                d->floor = collidewall;
            break;

        case PHYS_STEP_UP:
            d->o.z -= STAIRHEIGHT + 0.15f;
            if (collide(d, vec3(0, 0, -1), SLOPEZ)) d->floor = collidewall;
            break;

        case PHYS_SLIDE:
            d->o.z -= 0.15f;
            if (collide(d, vec3(0, 0, -1)) && collidewall.z < SLOPEZ) d->floor = collidewall;
            break;
    }
    if (d->physstate > PHYS_FALL && d->floor.z <= 0) d->floor = vec3(0, 0, 1);
    d->o = old;
}

// brute force but effective way to find a free spawn spot in the map
bool entinmap(physent *d, bool avoidplayers) {
    RandomNumberGenerator<PCG32> rnd;
    d->o.z += d->eyeheight;  // pos specified is at feet
    vec3 orig = d->o;
    loopi(100)  // try max 100 times
    {
        if (i) {
            d->o = orig;
            d->o.x += (rnd(21) - 10) * i / 5;  // increasing distance
            d->o.y += (rnd(21) - 10) * i / 5;
            d->o.z += (rnd(21) - 10) * i / 5;
        }

        if (!collide(d) && !collideinside) {
            if (collideplayer) {
                if (!avoidplayers) continue;
                d->o = orig;
                d->resetinterp();
                return false;
            }

            d->resetinterp();
            return true;
        }
    }
    // leave ent at original pos, possibly stuck
    d->o = orig;
    d->resetinterp();
    //conoutf(CON_WARN, "can't find entity spawn spot! (%.1f, %.1f, %.1f)", d->o.x, d->o.y, d->o.z);
    return false;
}


// -------------------------------------------------------------------------------------------------

// Lobster-specific bindings follow here.

namespace lobster {

ResourceType *GetPhysEntType() {
    static ResourceType physent_type = {
        "physent",
        [](void *ph) { delete (physent *)ph; }
    };
    return &physent_type;
}

physent &GetPhysEnt(VM &vm, const Value &res) {
    return *GetResourceDec<physent *>(vm, res, GetPhysEntType());
}

}  // namespace lobster

using namespace lobster;

void AddOcTreePhysics(NativeRegistry &nfr) {

nfr("oc_new_physent", "pos,maxspeed,rad,eyeheight,aboveeye", "F}:3FFFF", "R", "",
    [](StackPtr &sp, VM &vm) {
        auto ph = new physent();
        ph->aboveeye = Pop(sp).fltval();
        ph->eyeheight = Pop(sp).fltval();
        ph->radius = Pop(sp).fltval();
        ph->xradius = ph->radius;
        ph->yradius = ph->radius;
        ph->maxspeed = Pop(sp).fltval();
        ph->o = vec3(PopVec<float3>(sp));
        ph->resetinterp();
        Push(sp, Value(vm.NewResource(ph, GetPhysEntType())));
    });

nfr("oc_update_physent", "octree,ent,dpadvec,yaw,pitch,jumping,flying", "RRI}:2FFII", "F}:3S?", "",
    [](StackPtr &sp, VM &vm) {
        auto flying = Pop(sp).intval();
        auto jumping = Pop(sp).intval();
        auto pitch = Pop(sp).fltval();
        auto yaw = Pop(sp).fltval();
        auto dpad = PopVec<int2>(sp);
        auto &ph = GetPhysEnt(vm, Pop(sp).xval());
        ph.strafe = (int8_t)dpad.x;
        ph.move = (int8_t)dpad.y;
        ph.yaw = yaw;
        ph.pitch = pitch;
        ph.state = uint8_t(flying ? CS_EDITING : CS_ALIVE);
        if (jumping == 1 || jumping == 0) ph.jumping = jumping;
        ph.event_id = nullptr;
        oc = &GetOcTree(vm, Pop(sp).xval());
        moveplayer_int(&ph, 10, true, true, int(SDLTime() * 1000));
        oc = nullptr;
        PushVec(sp, float3(ph.o.v));
        Push(sp, ph.event_id ? Value(vm.NewString(ph.event_id)) : NilVal());
    });

nfr("oc_physics_frame", "", "", "", "",
    [](StackPtr &, VM &) {
        physicsframe(int(SDLTime() * 1000));
    });

nfr("oc_raycast", "octree,rayo,raydir,rad", "RF}:3F}:3F", "F}:3F", "",
    [](StackPtr &sp, VM &vm) {
        auto radius = Pop(sp).fltval();
        auto ray = vec3(PopVec<float3>(sp));
        auto o = vec3(PopVec<float3>(sp));
        oc = &GetOcTree(vm, Pop(sp).xval());
        vec3 hitpos;
        auto dist = raycubepos(o, ray, hitpos, radius);
        oc = nullptr;
        PushVec(sp, float3(hitpos.v));
        Push(sp, dist);
    });

}
