#include "stdafx.h"

#include "openglsup.h"
#include "mctables.h"

/*
struct Function2
{
    //vec3 trans;
	int3 a, b, c, d, e, f, g, h;
    //vec3 scale;

	Function2()
	{
		printf("a\n");
		printf("a\n");
		printf("a\n");
	}

    virtual void Size() = 0;
    virtual void FillGrid() = 0;
    virtual void FindEdge() = 0;
};

template<typename T> struct FunctionImpl2 : Function2
{
	FunctionImpl2()
	{
		printf("a\n");
		printf("a\n");
		printf("a\n");
	}

    void FillGrid()
    {
    }

    void FindEdge()
    {

    }
};



struct Sphere2 : FunctionImpl2<Sphere2>
{
    static INL bool Eval(const vec3 &pos) { return dot(pos, pos) < 1.0f; }
    
    void Size() {  }
};


void Test()
{
	Sphere2 sp2;
}

*/

Surface *surf = NULL;
Geometry *geom = NULL;

struct edge
{
    vec3 mid, col;

    edge() {}
    edge(const vec3 &_mid, const vec3 &_col) : mid(_mid), col(_col) {}
};

struct cell
{
    vec3 accum;
    vec3 col;
    int n;

    cell() : n(0), accum(0), col(0) {}
};

struct vert
{
    vec3 pos;
    vec3 norm;
    unsigned char col[4];
};

int3 corners[8] =
{
    int3(0, 0, 0),
    int3(1, 0, 0),
    int3(1, 1, 0),
    int3(0, 1, 0),
    int3(0, 0, 1),
    int3(1, 0, 1),
    int3(1, 1, 1),
    int3(0, 1, 1),
};

struct fcell
{
	char edges[3];
	bool solid;
};

INL int Sample(const int3 &v, const int3 &gridsize)
{
    return v.x * gridsize.y * gridsize.z +
           v.y * gridsize.y +
           v.z;
}

float gridscale;
vec3 gridtrans;

struct Function
{
    float a[6];

	Function()
	{
		printf("hi!\n");
	}
};

void mesh_do()
{
    Function sp;
}

void mesh_render(mat4 &mvp, vec4 &curcolor)
{
    phongshader->set(mvp, curcolor);
    //extern GLenum polymode;
    geom->Render(surf, GL_TRIANGLES);
}
