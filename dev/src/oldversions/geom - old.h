#define PI 3.1415927f
#define RAD (PI/180.0f)

template <typename T> struct vec
{
    T x, y, z;
    
    vec() { x = y = z = 0; }

    vec(T _x, T _y)       : x(   _x), y(   _y), z(    0) {}
    template<typename U>
    vec(U _x, U _y, U _z) : x((T)_x), y((T)_y), z((T)_z) {}
    vec(T _x, T _y, T _z) : x(   _x), y(   _y), z(   _z) {}

    vec(T d[3]) : x(d[0]), y(d[1]), z(d[2]) {}
    
    template<typename U> vec(vec<U> d) : x((T)d.x), y((T)d.y), z((T)d.z) {}
    operator T *() { return &x; }
    
    bool operator==(const vec &o) { return x==o.x && y==o.y && z==o.z; }
    bool operator!=(const vec &o) { return x!=o.x || y!=o.y || z!=o.z; }
    
    vec operator+(const vec &o)   { return vec(x+o.x, y+o.y, z+o.z); }
    vec operator+(const T d)      { return vec(x+d, y+d, z+d); }
    vec operator-(const vec &o)   { return vec(x-o.x, y-o.y, z-o.z); }
    vec operator-(const T d)      { return vec(x-d, y-d, z-d); }
    vec operator-()               { return vec(-x, -y, -z); }
    vec operator*(T d)            { return vec(x*d, y*d, z*d); }
    vec operator/(T d)            { return vec(x/d, y/d, z/d); }

    vec operator+=(const vec &o)  { return *this = vec(x+o.x, y+o.y, z+o.z); }
    vec operator-=(const vec &o)  { return *this = vec(x-o.x, y-o.y, z-o.z); }
    vec operator*=(T d)           { return *this = vec(x*d, y*d, z*d); }
    vec operator/=(T d)           { return *this = vec(x/d, y/d, z/d); }
    vec operator+=(T d)           { return *this = vec(x+d, y+d, z+d); }
    vec operator-=(T d)           { return *this = vec(x-d, y-d, z-d); }
    
    T dot(const vec &o) { return x*o.x + y*o.y + z*o.z; }
    float len()         { return sqrtf((float)dot(*this)); }
    vec normalize()     { return *this /  (T)len(); }
    vec normalize_it()  { return *this /= (T)len(); }
    T dist(vec &o)      { vec t = *this-o; return t.len(); }
    
    vec cross(vec &o)         { return vec(y*o.z-z*o.y, z*o.x-x*o.z, x*o.y-y*o.x); }


    vec rotateZaxis(float q) { return float3(x*cosf(q)+y*sinf(q), y*cosf(q)-x*sinf(q), z); }
    vec rotateYaxis(float q) { return float3(z*sinf(q)+x*cosf(q), y, z*cosf(q)-x*sinf(q)); }
    vec rotateXaxis(float q) { return float3(x, y*cosf(q)-z*sinf(q), y*sinf(q)+z*cosf(q)); }

    void quantizec(unsigned char *norm) { for (int i = 0; i < 3; i++) norm[i] = (unsigned char)(((*this)[i])*255); norm[3] = 0xFF; }

};


template <typename T> struct vec4 : vec<T>
{
    T w;

    vec4()                       : vec(),           w( 0) {}
    template<typename U>
    vec4(U _x, U _y, U _z, U _w) : vec(_x, _y, _z), w(_w) {}
    vec4(T _x, T _y, T _z, T _w) : vec(_x, _y, _z), w(_w) {}
    vec4(vec &o, float _w) {
        *(vec *)this = o;
        w = _w; }
};


typedef vec<float> float3;
typedef vec<int>   int3;

typedef vec4<float> float4;
typedef vec4<int>   int4;


