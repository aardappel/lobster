// Copyright 2014 Wouter van Oortmerssen. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "stdafx.h"

#include "vmdata.h"
#include "natreg.h"

#include "glinterface.h"
#include "sdlinterface.h"

using namespace lobster;

int2 screensize = int2_0;

Primitive polymode = PRIM_FAN;

IntResourceManagerCompact<Mesh> *meshes = NULL;

map<string, uint> texturecache;

float4 curcolor = float4_0;

Shader *currentshader = NULL;
Shader *colorshader = NULL;

float3 lasthitsize = float3_0;
float3 lastframehitsize = float3_0;

bool graphics_initialized = false;

void GraphicsShutDown()  // should be safe to call even if it wasn't initialized partially or at all
{
    extern void CleanPhysics(); CleanPhysics();
    extern void MeshGenClear(); MeshGenClear();
    extern void FontCleanup(); FontCleanup();

    if (meshes)
    {
        delete meshes;
        meshes = NULL;
    }

    texturecache.clear();

    ShaderShutDown();
    currentshader = NULL;
    colorshader = NULL;

    SDLSoundClose();
    SDLShutdown();

    // we don't set this to false on most platforms, as currently SDL doesn't like being reinitialized
    #ifdef __ANDROID__
        // FIXME: really only allow this if the app has been killed
        graphics_initialized = false;
    #endif
}

void TestGL()
{
    if (!graphics_initialized)
        g_vm->BuiltinError("graphics system not initialized yet, call gl_window() first");
}

float2 localpos(const int2 &pos) { return (view2object * float4(float3(float2(pos), 0), 1)).xyz().xy(); }
float2 localfingerpos(int i) { return localpos(GetFinger(i, false)); }

struct transback { float4x4 view2object; float4x4 object2view; };

Value pushtrans(const float4x4 &forward, const float4x4 &backward, Value &body)
{
    if (body.type != V_NIL)
    {
        transback tb;
        tb.view2object = view2object;
        tb.object2view = object2view;
        g_vm->Push(Value(g_vm->NewString((char *)&tb, sizeof(transback))));
    }
    object2view *= forward;
    view2object = backward * view2object;
    return body;
}

Value poptrans(Value &ret)
{
    auto s = g_vm->Pop();
    assert(s.type == V_STRING && s.sval()->len == sizeof(transback));
    auto tb = (transback *)s.sval()->str();
    view2object = tb->view2object;
    object2view = tb->object2view;
    s.DECRT();
    return ret;
}

float2 transangle(Value &a)
{
    switch (a.type)
    {
        case V_VECTOR: return ValueDecTo<float3>(a).xy();
        case V_FLOAT:  return float2(cosf(a.fval() * RAD), sinf(a.fval() * RAD));
        case V_INT:    return float2(cosf(a.ival() * RAD), sinf(a.ival() * RAD));
        default: g_vm->BuiltinError("angle passed to rotation function must be int/float or vector"); return float2_0;
    }
}

Mesh *GetMesh(Value &i)
{
    auto m = meshes->Get(i.ival());
    if (!m) g_vm->BuiltinError("graphics: illegal mesh id");
    return m;
}

int GetSampler(Value &i)
{
    if (i.ival() < 0 || i.ival() >= Shader::MAX_SAMPLERS)
        g_vm->BuiltinError("graphics: illegal texture unit");
    return i.ival();
}

void AddGraphics()
{
    STARTDECL(gl_window) (Value &title, Value &xs, Value &ys, Value &fullscreen)
    {
        if (graphics_initialized)
            g_vm->BuiltinError("cannot call gl_window() twice");

        screensize = int2(xs.ival(), ys.ival());
        string err = SDLInit(title.sval()->str(), screensize, fullscreen.ival() != 0);
        title.DECRT();

        if (err.empty())
        {
            OpenGLInit();

            err = LoadMaterialFile("shaders/default.materials");
        }

        if (!err.empty())
        {
            Output(OUTPUT_INFO, err.c_str());
            return Value(g_vm->NewString(err));
        }

        colorshader = LookupShader("color");
        assert(colorshader);

        meshes = new IntResourceManagerCompact<Mesh>([](Mesh *m) { delete m; });

        Output(OUTPUT_INFO, "graphics fully initialized...");
        graphics_initialized = true;

        return Value(0, V_NIL);
    }
    ENDDECL4(gl_window, "title,xs,ys,fullscreen", "SIIi", "s",
        "opens a window for OpenGL rendering. returns error string if any problems, nil otherwise.");

    STARTDECL(gl_loadmaterials) (Value &fn)
    {
        TestGL();
        auto err = LoadMaterialFile(fn.sval()->str());
        fn.DECRT();
        return err[0] ? Value(g_vm->NewString(err)) : Value(0, V_NIL);
    }
    ENDDECL1(gl_loadmaterials, "filename", "S", "s",
        "loads an additional materials file (shader/default.materials is already loaded by default by gl_window())."
        " returns error string if any problems, nil otherwise.");

    STARTDECL(gl_frame) ()
    {
        TestGL();

        extern void CullFonts(); CullFonts();

        bool cb = SDLFrame(screensize);

        lastframehitsize = lasthitsize;
        lasthitsize = float3_0;

        OpenGLFrameStart(screensize);
        Set2DMode(screensize);

        currentshader = colorshader;

        g_vm->LogFrame();

        return Value(!cb);
    }
    ENDDECL0(gl_frame, "", "", "I",
        "advances rendering by one frame, swaps buffers, and collects new input events."
        " returns true if the closebutton on the window was pressed");

    STARTDECL(gl_shutdown) ()
    {
        GraphicsShutDown();
        return Value();
    }
    ENDDECL0(gl_shutdown, "", "", "",
        "shuts down the OpenGL window. you only need to call this function if you wish to close it before the end"
        " of the program");

    STARTDECL(gl_windowtitle) (Value &s)
    {
        TestGL();
        SDLTitle(s.sval()->str());
        return s;
    }
    ENDDECL1(gl_windowtitle, "title", "S", "S",
        "changes the window title.");

    STARTDECL(gl_visible) ()
    {
        return Value(!SDLIsMinimized());
    }
    ENDDECL0(gl_visible, "", "", "I",
        "checks if the window is currently visible (not minimized, or on mobile devices, in the foreground)."
        " If false, you should not render anything, nor run the frame's code.");

    STARTDECL(gl_cursor) (Value &on)
    {
        TestGL();
        return Value(SDLCursor(on.ival() != 0));
    }
    ENDDECL1(gl_cursor, "on", "I", "I",
        "default the cursor is visible, turn off for implementing FPS like control schemes. return wether it's on.");

    STARTDECL(gl_grab) (Value &on)
    {
        TestGL();
        return Value(SDLGrab(on.ival() != 0));
    }
    ENDDECL1(gl_grab, "on", "I", "I",
        "grabs the mouse when the window is active. return wether it's on.");

    STARTDECL(gl_wentdown) (Value &name)
    {
        auto ks = GetKS(name.sval()->str());
        name.DEC();
        return Value(ks.wentdown);
    }
    ENDDECL1(gl_wentdown, "name", "S", "I",
        "wether a key/mousebutton/finger went down this frame (pass a string like"
        " mouse1/mouse2/mouse3/escape/space/up/down/a/b/f1 etc. mouse11 and on are additional fingers)");

    STARTDECL(gl_wentup) (Value &name)
    {
        auto ks = GetKS(name.sval()->str());
        name.DEC();
        return Value(ks.wentup);
    }
    ENDDECL1(gl_wentup, "name", "S", "I",
        "wether a key/mousebutton/finger went up this frame");

    STARTDECL(gl_isdown) (Value &name)
    {
        auto ks = GetKS(name.sval()->str());
        name.DEC();
        return Value(ks.isdown);
    }
    ENDDECL1(gl_isdown, "name", "S", "I",
        "wether a key/mousebutton/finger is currently down");

    STARTDECL(gl_touchscreen) ()
    {
        #ifdef PLATFORM_MOBILE
            return Value(true);
        #else
            return Value(false);
        #endif
    }
    ENDDECL0(gl_touchscreen, "", "", "I",
        "wether a you\'re getting input from a touch screen (as opposed to mouse & keyboard)");

    STARTDECL(gl_windowsize) ()
    {
        return ToValue(screensize);
    }
    ENDDECL0(gl_windowsize, "", "", "I]:2",
        "a vector representing the size (in pixels) of the window, changes when the user resizes");

    STARTDECL(gl_mousepos) (Value &i)
    {
        return ToValue(GetFinger(i.ival(), false));
    }
    ENDDECL1(gl_mousepos, "i", "I", "I]:2",
        "the current mouse/finger position in pixels, pass a value other than 0 to read additional fingers"
        " (for touch screens only if the corresponding gl_isdown is true)");

    STARTDECL(gl_mousedelta) (Value &i)
    {
        return ToValue(GetFinger(i.ival(), true));
    }
    ENDDECL1(gl_mousedelta, "i", "I", "I]:2",
        "amount of pixels the mouse/finger has moved since the last frame. use this instead of substracting positions"
        " to correctly deal with lifted fingers and FPS mode (gl_cursor(0))");

    STARTDECL(gl_localmousepos) (Value &i)
    {
        return ToValue(localfingerpos(i.ival()));
    }
    ENDDECL1(gl_localmousepos, "i", "I", "F]:2",
        "the current mouse/finger position local to the current transform (gl_translate etc)"
        " (for touch screens only if the corresponding gl_isdown is true)");

    STARTDECL(gl_lastpos) (Value &name, Value &on)     // need a local version of this too?
    {
        auto p = GetKeyPos(name.sval()->str(), on.ival());
        name.DEC();
        return ToValue(p);
    }
    ENDDECL2(gl_lastpos, "name,down", "SI", "I]:2",
        "position (in pixels) key/mousebutton/finger last went down (true) or up (false)");

    STARTDECL(gl_locallastpos) (Value &name, Value &on)     // need a local version of this too?
    {
        auto p = localpos(GetKeyPos(name.sval()->str(), on.ival()));
        name.DEC();
        return ToValue(p);
    }
    ENDDECL2(gl_locallastpos, "name,down", "SI", "F]:2",
        "position (local to the current transform) key/mousebutton/finger last went down (true) or up (false)");

    STARTDECL(gl_mousewheeldelta) ()
    {
        return Value(SDLWheelDelta());
    }
    ENDDECL0(gl_mousewheeldelta, "", "", "I",
        "amount the mousewheel scrolled this frame, in number of notches");

    STARTDECL(gl_joyaxis) (Value &i)
    {
        return Value(GetJoyAxis(i.ival()));
    }
    ENDDECL1(gl_joyaxis, "i", "I", "F",
        "the current joystick orientation for axis i, as -1 to 1 value");

    STARTDECL(gl_deltatime) ()
    {
        return Value(SDLDeltaTime());
    }
    ENDDECL0(gl_deltatime, "", "", "F",
        "seconds since the last frame, updated only once per frame");

    STARTDECL(gl_time) ()
    {
        return Value(SDLTime());
    }
    ENDDECL0(gl_time, "", "", "F",
        "seconds since the start of the OpenGL subsystem, updated only once per frame (use seconds_elapsed()"
        " for continuous timing)");

    STARTDECL(gl_lasttime) (Value &name, Value &on)
    {
        auto t = GetKeyTime(name.sval()->str(), on.ival());
        name.DEC();
        return Value(t);
    }
    ENDDECL2(gl_lasttime, "name,down", "SI", "F",
        "time key/mousebutton/finger last went down (true) or up (false)");

    STARTDECL(gl_clear) (Value &col)
    {
        TestGL();
        ClearFrameBuffer(ValueDecTo<float3>(col));
        return Value();
    }
    ENDDECL1(gl_clear, "col", "F]", "",
        "clears the framebuffer (and depth buffer) to the given color");

    STARTDECL(gl_color) (Value &col, Value &body)
    {
        if (body.type != V_NIL) g_vm->Push(ToValue(curcolor));  // FIXME: maybe more efficient as an int
        curcolor = ValueDecTo<float4>(col);
        return body;
    }
    MIDDECL(gl_color) (Value &ret)
    {
        curcolor = ValueDecTo<float4>(g_vm->Pop());
        return ret;
    }
    ENDDECL2CONTEXIT(gl_color, "col,body", "Vc", "A",
        "sets the current color. when a body is given, restores the previous color afterwards");

    STARTDECL(gl_polygon) (Value &vl)
    {
        TestGL();

        if (vl.vval()->len < 3) g_vm->BuiltinError("polygon: must have at least 3 verts");

        auto vbuf = new BasicVert[vl.vval()->len];
        for (int i = 0; i < vl.vval()->len; i++) vbuf[i].pos = ValueTo<float3>(vl.vval()->at(i));

        auto v1 = vbuf[1].pos - vbuf[0].pos;
        auto v2 = vbuf[2].pos - vbuf[0].pos;
        auto norm = normalize(cross(v2, v1));
        for (int i = 0; i < vl.vval()->len; i++)
        {
            vbuf[i].norm = norm;
            vbuf[i].tc = vbuf[i].pos.xy();
            vbuf[i].col = byte4_255;
        }

        currentshader->Set();
        RenderArray(polymode, vl.vval()->len, "PNTC", sizeof(BasicVert), vbuf);

        delete[] vbuf;

        return vl;
    }
    ENDDECL1(gl_polygon, "vertlist", "V", "V",
        "renders a polygon using the list of points given. returns the argument.");

    STARTDECL(gl_circle) (Value &radius, Value &segments)
    {
        TestGL();

        auto vbuf = new float3[segments.ival()];

        float step = PI * 2 / segments.ival();
        for (int i = 0; i < segments.ival(); i++)
        {
            // + 1 to reduce "aliasing" from exact 0 / 90 degrees points
            vbuf[i] = float3(sinf(i * step + 1) * radius.fval(),
                             cosf(i * step + 1) * radius.fval(), 0);
        }

        currentshader->Set();
        RenderArray(polymode, segments.ival(), "P", sizeof(float3), vbuf);

        delete[] vbuf;

        return Value();
    }
    ENDDECL2(gl_circle, "radius,segments", "FI", "",
        "renders a circle");

    STARTDECL(gl_rotate_x) (Value &angle, Value &body)
    {
        auto a = transangle(angle);
        return pushtrans(rotationX(a), rotationX(a * float2(1, -1)), body);
    }
    MIDDECL(gl_rotate_x) (Value &ret)
    {
        return poptrans(ret);
    }
    ENDDECL2CONTEXIT(gl_rotate_x, "angle,body", "Ac", "A",
        "rotates the yz plane around the x axis. angle can also be a 2D vector. when a body is given,"
        " restores the previous transform afterwards");

    STARTDECL(gl_rotate_y) (Value &angle, Value &body)
    {
        auto a = transangle(angle);
        return pushtrans(rotationY(a), rotationY(a * float2(1, -1)), body);
    }
    MIDDECL(gl_rotate_y) (Value &ret)
    {
        return poptrans(ret);
    }
    ENDDECL2CONTEXIT(gl_rotate_y, "angle,body", "Ac", "A",
        "rotates the xz plane around the y axis. angle can also be a 2D vector."
        " when a body is given, restores the previous transform afterwards");

    STARTDECL(gl_rotate_z) (Value &angle, Value &body)
    {
        auto a = transangle(angle);
        return pushtrans(rotationZ(a), rotationZ(a * float2(1, -1)), body);
    }
    MIDDECL(gl_rotate_z) (Value &ret)
    {
        return poptrans(ret);
    }
    ENDDECL2CONTEXIT(gl_rotate_z, "angle,body", "Ac", "A",
        "rotates the xy plane around the z axis (used in 2D). angle can also be a 2D vector. when a body is given,"
        " restores the previous transform afterwards");

    STARTDECL(gl_translate) (Value &vec, Value &body)
    {
        auto v = ValueDecTo<float3>(vec);
        return pushtrans(translation(v), translation(-v), body);
    }
    MIDDECL(gl_translate) (Value &ret)
    {
        return poptrans(ret);
    }
    ENDDECL2CONTEXIT(gl_translate, "vec,body", "Vc", "A",
        "translates the current coordinate system along a vector. when a body is given,"
        " restores the previous transform afterwards");

    STARTDECL(gl_scale) (Value &vec, Value &body)
    {
        auto v = ValueDecTo<float3>(vec, 1);
        return pushtrans(float4x4(float4(v, 1)), float4x4(float4(float3_1 / v, 1)), body);
    }
    MIDDECL(gl_scale) (Value &ret)
    {
        return poptrans(ret);
    }
    ENDDECL2CONTEXIT(gl_scale, "factor,body", "Ac", "A",
        "scales the current coordinate system using a numerical factor (or a vector for individual factors per axis)."
        " when a body is given, restores the previous transform afterwards");

    STARTDECL(gl_origin) ()
    {
        auto pos = float2(object2view[3].x(), object2view[3].y());
        return ToValue(pos);
    }
    ENDDECL0(gl_origin, "", "", "F]:2",
        "returns a vector representing the current transform origin in pixels."
        " only makes sense in 2D mode (no gl_perspective called).");

    STARTDECL(gl_scaling) ()
    {
        auto sc = float2(object2view[0].x(), object2view[1].y());
        return ToValue(sc);
    }
    ENDDECL0(gl_scaling, "", "", "F]:2",
        "returns a vector representing the current transform scale in pixels."
        " only makes sense in 2D mode (no gl_perspective called).");

    STARTDECL(gl_linemode) (Value &on, Value &body)
    {
        if (body.type != V_NIL) g_vm->Push(Value((int)polymode));
        polymode = on.ival() ? PRIM_LOOP : PRIM_FAN;
        return body;
    }
    MIDDECL(gl_linemode) (Value &ret)
    {
        polymode = (Primitive)g_vm->Pop().ival();
        return ret;
    }
    ENDDECL2CONTEXIT(gl_linemode, "on,body", "IC", "A",
        "set line mode (true == on). when a body is given,"
        " restores the previous mode afterwards");

    STARTDECL(gl_hit) (Value &vec, Value &i)
    {
        auto size = ValueDecTo<float3>(vec);
        auto localmousepos = localfingerpos(i.ival());
        auto hit = localmousepos.x() >= 0 &&
                   localmousepos.y() >= 0 &&
                   localmousepos.x() < size.x() &&
                   localmousepos.y() < size.y();
        if (hit) lasthitsize = size;
        /*
        #ifdef PLATFORM_MOBILE
        // Inefficient for fingers other than 0, which is going to be rare. 
        auto ks = i ? GetKS((string("mouse1") + (char)('0' + i)).c_str()) : GetKS("mouse1");
        // On mobile, if the finger just went down, we wont have meaningfull lastframehitsize, so if the
        // programmer checks for the combination of gl_hit and gl_wentdown, that would fail.
        // Instead, we bypass that check.
        // PROBLEM: now we'll be returning true for overlapping elements.
        // if we can solve this, we can remove the frame delay from the input system.
        if (ks.wentdown && hit) return true;
        #endif
        */
        return Value(size == lastframehitsize && hit);
    }
    ENDDECL2(gl_hit, "vec,i", "VI", "I",
        "wether the mouse/finger is inside of the rectangle specified in terms of the current transform"
        " (for touch screens only if the corresponding gl_isdown is true). Only true if the last rectangle for"
        " which gl_hit was true last frame is of the same size as this one (allows you to safely test in most cases"
        " of overlapping rendering)");

    STARTDECL(gl_rect) (Value &vec)
    {
        TestGL();

        auto v = ValueTo<float3>(vec);

        static float tempquad_rect[20] =
        {
            0, 0, 0, 0, 0,
            0, 0, 0, 0, 1,
            0, 0, 0, 1, 1,
            0, 0, 0, 1, 0,
        };
        tempquad_rect[ 6] = tempquad_rect[11] = v.y();
        tempquad_rect[10] = tempquad_rect[15] = v.x();

        currentshader->Set();
        RenderArray(polymode, 4, "PT", sizeof(float) * 5, tempquad_rect);

        return vec;
    }
    ENDDECL1(gl_rect, "vec", "A]", "F]",   // FIXME: we'd prefer a numeric type
        "renders a rectangle of the given size. returns the argument.");

    STARTDECL(gl_line) (Value &start, Value &end, Value &thickness)
    {
        TestGL();

        auto v1 = ValueDecTo<float3>(start);
        auto v2 = ValueDecTo<float3>(end);

        float angle = atan2f(v2.y() - v1.y(), v2.x() - v1.x());
        float3 v = float3(sinf(angle), -cosf(angle), 0) * thickness.fval() / 2;

        currentshader->Set();
        RenderLine(polymode, v1, v2, v);

        return Value();
    }
    ENDDECL3(gl_line, "start,end,thickness", "VVF", "",
        "renders a line with the given thickness");

    STARTDECL(gl_perspective) (Value &fovy, Value &znear, Value &zfar)
    {
        Set3DMode(fovy.fval()*RAD, screensize.x() / (float)screensize.y(), znear.fval(), zfar.fval());
        return Value();
    }
    ENDDECL3(gl_perspective, "fovy,znear,zfar", "FFF", "",
        "changes from 2D mode (default) to 3D perspective mode with vertical fov (try 60), far plane"
        " (furthest you want to be able to render, try 1000) and near plane (try 1)");

    STARTDECL(gl_ortho) ()
    {
        Set2DMode(screensize);
        return Value();
    }
    ENDDECL0(gl_ortho, "", "", "",
        "changes back to 2D mode rendering with a coordinate system from (0,0) top-left to the screen size in pixels"
        " bottom right. this is the default at the start of a frame, use this call to get back to that after"
        " gl_perspective.");

    STARTDECL(gl_newmesh) (Value &indices, Value &positions, Value &colors, Value &texcoords, Value &normals)
    {
        TestGL();

        vector<int> idxs;
        for (int i = 0; i < indices.vval()->len; i++)
        {
            auto &e = indices.vval()->at(i);
            if (e.type != V_INT) g_vm->BuiltinError("newmesh: index list must be all integers");
            if (e.ival() < 0 || e.ival() >= positions.vval()->len)
                g_vm->BuiltinError("newmesh: index out of range of vertex list");
            idxs.push_back(e.ival());
        }
        indices.DECRT();

        int nverts = positions.vval()->len;

        BasicVert *verts = new BasicVert[nverts];
        BasicVert v = { float3_0, float3_0, float2_0, byte4_255 };

        for (int i = 0; i < nverts; i++)
        {
            v.pos  = ValueTo<float3>(positions.vval()->at(i), 0);
            v.col  = i < colors.vval()->len    ? quantizec(ValueTo<float4>(colors.vval()->at(i), 1)) : byte4_255;
            v.tc   = i < texcoords.vval()->len ? ValueTo<float3>(texcoords.vval()->at(i), 0).xy()    : v.pos.xy();
            v.norm = i < normals.vval()->len   ? ValueTo<float3>(normals.vval()->at(i), 0)           : float3_0;
            verts[i] = v;
        }

        if (!normals.vval()->len)
        {
            // if no normals were specified, generate them. if the user really doesn't use normals and this step is
            // somehow too expensive, he can always pass in the positions vector a second time to skip it
            normalize_mesh(&idxs[0], idxs.size(), verts, nverts);
        }

        positions.DECRT();
        colors.DECRT();
        texcoords.DECRT();
        normals.DECRT();

        auto m = new Mesh(new Geometry(verts, nverts, sizeof(BasicVert), "PNTC"));
        m->surfs.push_back(new Surface(&idxs[0], idxs.size()));

        delete[] verts;

        return Value((int)meshes->Add(m));
    }
    ENDDECL5(gl_newmesh, "indices,positions,colors,texcoords,normals", "VVVVV", "I",
        "creates a new vertex buffer and returns an integer id (1..) for it."
        " you may specify [] to get defaults for colors (white) / texcoords (position x & y) /"
        " normals (generated from adjacent triangles)");

    STARTDECL(gl_newmesh_iqm) (Value &fn)
    {
        TestGL();

        auto m = LoadIQM(fn.sval()->str());
        fn.DECRT();
        return Value(m ? (int)meshes->Add(m) : 0);
    }
    ENDDECL1(gl_newmesh_iqm, "filename", "S", "I",
        "load a .iqm file into a mesh, returns integer id (1..), or 0 on failure to load.");

    STARTDECL(gl_deletemesh) (Value &i)
    {
        meshes->Delete(i.ival());
        return Value();
    }
    ENDDECL1(gl_deletemesh, "i", "I", "",
        "free up memory for the given mesh id");

    STARTDECL(gl_meshparts) (Value &i)
    {
        auto m = GetMesh(i);
        auto v = g_vm->NewVector(m->surfs.size(), V_VECTOR);
        for (auto s : m->surfs) v->push(Value(g_vm->NewString(s->name)));
        return Value(v);
    }
    ENDDECL1(gl_meshparts, "i", "I", "S]",
        "returns an array of names of all parts of mesh i (names may be empty)");

    STARTDECL(gl_meshsize) (Value &i)
    {
        auto m = GetMesh(i);
        return Value((int)m->geom->nverts);
    }
    ENDDECL1(gl_meshsize, "i", "I", "I",
        "returns the number of verts in this mesh");

    STARTDECL(gl_animatemesh) (Value &i, Value &f)
    {
        GetMesh(i)->curanim = f.fval();
        return i;
    }
    ENDDECL2(gl_animatemesh, "i,frame", "IF", "",
        "set the frame for animated mesh i");

    STARTDECL(gl_rendermesh) (Value &i)
    {
        TestGL();
        GetMesh(i)->Render(currentshader);
        return i;
    }
    ENDDECL1(gl_rendermesh, "i", "I", "",
        "renders the specified mesh");

    STARTDECL(gl_setshader) (Value &shader)
    {
        TestGL();

        auto sh = LookupShader(shader.sval()->str());
        shader.DECRT();

        if (sh) currentshader = sh;

        return Value(sh != NULL);
    }
    ENDDECL1(gl_setshader, "shader", "S", "I",
        "changes the current shader. shaders must reside in the shaders folder, builtin ones are:"
        " color / textured / phong. returns if shader could be found.");

    STARTDECL(gl_setuniform) (Value &name, Value &vec)
    {
        TestGL();
        auto v = ValueDecTo<float4>(vec);
        currentshader->Activate();
        auto ok = currentshader->SetUniform(name.sval()->str(), v.begin(), vec.vval()->len);
        name.DECRT();
        return Value(ok);
    }
    ENDDECL2(gl_setuniform, "name,value", "SF]", "I",
             "set a uniform on the current shader. size of float vector must match size of uniform in the shader."
             " returns false on error.");

    STARTDECL(gl_blend) (Value &mode, Value &body)
    {
        TestGL();

        int old = SetBlendMode((BlendMode)mode.ival());
        if (body.type != V_NIL) g_vm->Push(Value(old));
        return body;
    }
    MIDDECL(gl_blend) (Value &ret)
    {
        auto m = g_vm->Pop();
        assert(m.type == V_INT);
        SetBlendMode((BlendMode)m.ival());
        return ret;
    }
    ENDDECL2CONTEXIT(gl_blend, "on,body", "Ic", "A",
        "changes the blending mode to 0: off, 1: alpha blend (default), 2: additive, 3: alpha additive,"
        " 4: multiplicative. when a body is given, restores the previous mode afterwards");

    STARTDECL(gl_loadtexture) (Value &name, Value &clamp, Value &nomip, Value &nearest)
    {
        TestGL();

        ValueRef nameref(name);
        uint id = 0;
        int2 dim(0);
        auto it = texturecache.find(name.sval()->str());
        if (it != texturecache.end())
        {
            id = it->second;
        }
        else
        {
            int tf = TF_NONE;
            if (clamp.True())   tf |= TF_CLAMP;
            if (nomip.True())   tf |= TF_NOMIPMAP;
            if (nearest.True()) tf |= TF_NEAREST;

            id = CreateTextureFromFile(name.sval()->str(), dim, tf);

            if (id) texturecache[name.sval()->str()] = id;
        }
        g_vm->Push(Value((int)id));
        return ToValue(dim);
    }
    ENDDECL4(gl_loadtexture, "name,clamp,nomip,nearest", "Siii", "II]:2",
        "returns texture id if succesfully loaded from file name, otherwise 0."
        " 3 optional booleans specify if you want clamping, turn of mipmapping, or nearest neighbor filtering."
        " Returns the size of the loaded textures in pixels as second return value on first load (xy_i),"
        " or (0, 0) otherwise."
        " Only loads from disk once if called again with the same name. Uses stb_image internally"
        " (see http://nothings.org/), loads JPEG Baseline, subsets of PNG, TGA, BMP, PSD, GIF, HDR, PIC.");

    STARTDECL(gl_setprimitivetexture) (Value &i, Value &id)
    {
        TestGL();

        SetTexture(GetSampler(i), id.ival());

        return Value();
    }
    ENDDECL2(gl_setprimitivetexture, "i,id", "II", "",
        "sets texture unit i to texture id (for use with rect/circle/polygon/line)");

    STARTDECL(gl_setmeshtexture) (Value &mid, Value &part, Value &i, Value &id)
    {
        auto m = GetMesh(mid);

        if (part.ival() < 0 || part.ival() >= (int)m->surfs.size())
            g_vm->BuiltinError("setmeshtexture: illegal part index");

        m->surfs[part.ival()]->textures[GetSampler(i)] = id.ival();

        return Value();
    }
    ENDDECL4(gl_setmeshtexture, "meshid,part,i,textureid", "IIII", "",
        "sets texture unit i to texture id for a mesh and part (0 if not a multi-part mesh)");

    STARTDECL(gl_createtexture) (Value &mat)
    {
        TestGL();

        LVector *cols = mat.vval();
        int x = cols->len;
        if (x && cols->at(0).type == V_VECTOR)
        {
            int y = cols->at(0).vval()->len;
            if (y)
            {
                auto buf = new byte4[x * y];
                memset(buf, 0, x * y * 4);
                for (int i = 0; i < x; i++) if (cols->at(i).type == V_VECTOR)
                {
                    LVector *row = cols->at(i).vval();
                    for (int j = 0; j < min(y, row->len); j++)
                    {
                        buf[j * x + i] = quantizec(ValueTo<float3>(row->at(j)));
                    }
                }
                mat.DECRT();
                uint id = CreateTexture((uchar *)buf, int2(x, y));
                delete[] buf;
                return Value((int)id);
            }
        }

        mat.DECRT();
        return Value(0);
    }
    ENDDECL1(gl_createtexture, "matrix", "F]]]", "I",
        "creates a texture from a 2d array of color vectors, returns texture id, or 0 if not a proper 2D array");

    STARTDECL(gl_deletetexture) (Value &i)
    {
        auto it = texturecache.begin();
        // this is potentially expensive, we're counting on gl_deletetexture not being needed often
        while (it != texturecache.end())
        {
            if (it->second == uint(i.ival())) texturecache.erase(it++);
            else ++it;
        }

        // the surfaces in meshes are still potentially referring to this texture,
        // but OpenGL doesn't care about illegal texture ids, so neither do we
        DeleteTexture(i.ival());

        return Value();
    }
    ENDDECL1(gl_deletetexture, "i", "I", "",
        "free up memory for the given texture id");

    STARTDECL(gl_light) (Value &pos)
    {
        Light l;
        l.pos = object2view * float4(ValueDecTo<float3>(pos), 1);
        lights.push_back(l);
        return Value();
    }
    ENDDECL1(gl_light, "pos", "F]", "",
        "sets up a light at the given position for this frame. make sure to call this after your camera transforms"
        " but before any object transforms (i.e. defined in \"worldspace\").");

    STARTDECL(gl_debug_grid) (Value &num, Value &dist, Value &thickness)
    {
        TestGL();

        float3 cp = view2object[3].xyz();
        auto m = ValueDecTo<float3>(num);
        auto step = ValueDecTo<float3>(dist);

        auto oldcolor = curcolor;
        curcolor = float4(0, 1, 0, 1); for (float z = 0; z <= m.z(); z += step.x()) for (float x = 0; x <= m.x(); x += step.x()) { currentshader->Set(); RenderLine3D(float3(x, 0, z), float3(x, m.y(), z), cp, thickness.fval()); }
        curcolor = float4(1, 0, 0, 1); for (float z = 0; z <= m.z(); z += step.y()) for (float y = 0; y <= m.y(); y += step.y()) { currentshader->Set(); RenderLine3D(float3(0, y, z), float3(m.x(), y, z), cp, thickness.fval()); }
        curcolor = float4(0, 0, 1, 1); for (float y = 0; y <= m.y(); y += step.z()) for (float x = 0; x <= m.x(); x += step.z()) { currentshader->Set(); RenderLine3D(float3(x, y, 0), float3(x, y, m.z()), cp, thickness.fval()); }
        curcolor = oldcolor;

        return Value();
    }
    ENDDECL3(gl_debug_grid, "num,dist,thickness", "VVF", "",
        "renders a grid in space for debugging purposes. num is the amount of lines in all 3 directions,"
        " and dist their spacing. thickness of the lines in the same units");
}

AutoRegister __ag("graphics", AddGraphics);
