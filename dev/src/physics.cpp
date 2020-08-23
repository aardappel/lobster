// Copyright 2014 Google Inc. All rights reserved.
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

#include "lobster/stdafx.h"

#include "lobster/natreg.h"

#include "lobster/glinterface.h"

#include "Box2D/Box2D.h"

using namespace lobster;

struct Renderable : Textured {
    float4 color = float4_1;
    Shader *sh;

    Renderable(const char *shname) : sh(LookupShader(shname)) { assert(sh); }

    void Set() {
        sh->Set();
        sh->SetTextures(textures);
    }
};

b2World *world = nullptr;
b2ParticleSystem *particlesystem = nullptr;
Renderable *particlematerial = nullptr;

b2Vec2 Float2ToB2(const float2 &v) { return b2Vec2(v.x, v.y); }
float2 B2ToFloat2(const b2Vec2 &v) { return float2(v.x, v.y); }

b2Vec2 PopB2(StackPtr &sp) {
    auto v = PopVec<float2>(sp);
    return Float2ToB2(v);
}

struct PhysicsObject {
    Renderable r;
    b2Fixture *fixture;
    vector<int> *particle_contacts;

    PhysicsObject(const Renderable &_r, b2Fixture *_f)
        : r(_r), fixture(_f), particle_contacts(nullptr) {}
    ~PhysicsObject() {
        auto body = fixture->GetBody();
        body->DestroyFixture(fixture);
        if (!body->GetFixtureList()) world->DestroyBody(body);
        if (particle_contacts) delete particle_contacts;
    }
    float2 Pos() { return B2ToFloat2(fixture->GetBody()->GetPosition()); }
};

static ResourceType physics_type = { "physical", [](void *v) { delete ((PhysicsObject *)v); } };

PhysicsObject &GetObject(StackPtr &sp, VM &vm, const Value &res) {
    return *GetResourceDec<PhysicsObject *>(sp, vm, res, &physics_type);
}

void CleanPhysics() {
    if (world) delete world;
    world = nullptr;
    particlesystem = nullptr;
    delete particlematerial;
    particlematerial = nullptr;
}

void InitPhysics(const float2 &gv) {
    // FIXME: check that shaders are initialized, since renderables depend on that
    CleanPhysics();
    world = new b2World(b2Vec2(gv.x, gv.y));
}

void CheckPhysics() {
    if (!world) InitPhysics(float2(0, -10));
}

void CheckParticles(float size = 0.1f) {
    CheckPhysics();
    if (!particlesystem) {
        b2ParticleSystemDef psd;
        psd.radius = size;
        particlesystem = world->CreateParticleSystem(&psd);
        particlematerial = new Renderable("color_attr");
    }
}

b2Body &GetBody(StackPtr &sp, VM &vm, Value &id, float2 wpos) {
    CheckPhysics();
    b2Body *body = id.True() ? GetObject(sp, vm, id).fixture->GetBody() : nullptr;
    if (!body) {
        b2BodyDef bd;
        bd.type = b2_staticBody;
        bd.position.Set(wpos.x, wpos.y);
        body = world->CreateBody(&bd);
    }
    return *body;
}

Value CreateFixture(VM &vm, b2Body &body, b2Shape &shape) {
    auto fixture = body.CreateFixture(&shape, 1.0f);
    auto po = new PhysicsObject(Renderable("color"), fixture);
    fixture->SetUserData(po);
    return Value(vm.NewResource(po, &physics_type));
}

b2Vec2 OptionalOffset(StackPtr &sp) {
    return Top(sp).True() ? PopB2(sp) : (Pop(sp), b2Vec2_zero);
}

Renderable &GetRenderable(StackPtr &sp, VM &vm, const Value &id) {
    CheckPhysics();
    return id.True() ? GetObject(sp, vm, id).r : *particlematerial;
}

extern int GetSampler(StackPtr sp, VM &vm, Value &i);  // from graphics

void AddPhysics(NativeRegistry &nfr) {

nfr("ph_initialize", "gravityvector", "F}:2", "",
    "initializes or resets the physical world, gravity typically [0, -10].",
    [](StackPtr &sp, VM &) {
        InitPhysics(PopVec<float2>(sp));
    });

nfr("ph_create_box", "position,size,offset,rotation,attachto", "F}:2F}:2F}:2?F?R?", "R",
    "creates a physical box shape in the world at position, with size the half-extends around"
    " the center, offset from the center if needed, at a particular rotation (in degrees)."
    " attachto is a previous physical object to attach this one to, to become a combined"
    " physical body.",
    [](StackPtr &sp, VM &vm) {
        auto other_id = Pop(sp);
        auto rot = Pop(sp).fltval();
        auto offset = OptionalOffset(sp);
        auto sz = PopVec<float2>(sp);
        auto wp = PopVec<float2>(sp);
        auto &body = GetBody(sp, vm, other_id, wp);
        b2PolygonShape shape;
        shape.SetAsBox(sz.x, sz.y, offset, rot * RAD);
        Push(sp,  CreateFixture(vm, body, shape));
    });

nfr("ph_create_circle", "position,radius,offset,attachto", "F}:2FF}:2?R?", "R",
    "creates a physical circle shape in the world at position, with the given radius, offset"
    " from the center if needed. attachto is a previous physical object to attach this one to,"
    " to become a combined physical body.",
    [](StackPtr &sp, VM &vm) {
        auto other_id = Pop(sp);
        auto offset = OptionalOffset(sp);
        auto radius = Pop(sp).fltval();
        auto wp = PopVec<float2>(sp);
        auto &body = GetBody(sp, vm, other_id, wp);
        b2CircleShape shape;
        shape.m_p.Set(offset.x, offset.y);
        shape.m_radius = radius;
        Push(sp,  CreateFixture(vm, body, shape));
    });

nfr("ph_create_polygon", "position,vertices,attachto", "F}:2F}:2]R?", "R",
    "creates a polygon circle shape in the world at position, with the given list of vertices."
    " attachto is a previous physical object to attach this one to, to become a combined"
    " physical body.",
    [](StackPtr &sp, VM &vm) {
        auto other_id = Pop(sp);
        auto vertices = Pop(sp).vval();
        auto wp = PopVec<float2>(sp);
        auto &body = GetBody(sp, vm, other_id, wp);
        b2PolygonShape shape;
        auto verts = new b2Vec2[vertices->len];
        for (int i = 0; i < vertices->len; i++) {
            auto vert = ValueToFLT<2>(vertices->AtSt(i), vertices->width);
            verts[i] = Float2ToB2(vert);
        }
        shape.Set(verts, (int)vertices->len);
        delete[] verts;
        Push(sp,  CreateFixture(vm, body, shape));
    });

nfr("ph_dynamic", "shape,on", "RB", "",
    "makes a shape dynamic (on = true) or not.",
    [](StackPtr &sp, VM &vm, Value &fixture_id, Value &on) {
        CheckPhysics();
        GetObject(sp, vm, fixture_id)
            .fixture->GetBody()
            ->SetType(on.ival() ? b2_dynamicBody : b2_staticBody);
        return Value();
    });

nfr("ph_set_color", "id,color", "R?F}:4", "",
    "sets a shape (or nil for particles) to be rendered with a particular color.",
    [](StackPtr &sp, VM &vm) {
        auto c = PopVec<float4>(sp);
        auto id = Pop(sp);
        auto &r = GetRenderable(sp, vm, id);
        r.color = c;
    });

nfr("ph_set_shader", "id,shadername", "R?S", "",
    "sets a shape (or nil for particles) to be rendered with a particular shader.",
    [](StackPtr &sp, VM &vm, Value &fixture_id, Value &shader) {
        auto &r = GetRenderable(sp, vm, fixture_id);
        auto sh = LookupShader(shader.sval()->strv());
        if (sh) r.sh = sh;
        return Value();
    });

nfr("ph_set_texture", "id,tex,texunit", "R?RI?", "",
    "sets a shape (or nil for particles) to be rendered with a particular texture"
    " (assigned to a texture unit, default 0).",
    [](StackPtr &sp, VM &vm, Value &fixture_id, Value &tex, Value &tex_unit) {
        auto &r = GetRenderable(sp, vm, fixture_id);
        extern Texture GetTexture(StackPtr sp, VM &vm, const Value &res);
        r.Get(GetSampler(sp, vm, tex_unit)) = GetTexture(sp, vm, tex);
        return Value();
    });

nfr("ph_get_position", "id", "R", "F}:2",
    "gets a shape's position.",
    [](StackPtr &sp, VM &vm) {
        auto id = Pop(sp);
        PushVec(sp, GetObject(sp, vm, id).Pos());
    });

nfr("ph_create_particle", "position,velocity,color,flags", "F}:2F}:2F}:4I?", "I",
    "creates an individual particle. For flags, see include/physics.lobster",
    [](StackPtr &sp, VM &) {
        CheckParticles();
        b2ParticleDef pd;
        pd.flags = Pop(sp).intval();
        auto c = PopVec<float3>(sp);
        pd.color.Set(b2Color(c.x, c.y, c.z));
        pd.velocity = PopB2(sp);
        pd.position = PopB2(sp);
        Push(sp,  particlesystem->CreateParticle(pd));
    });

nfr("ph_create_particle_circle", "position,radius,color,flags", "F}:2FF}:4I?", "",
    "creates a circle filled with particles. For flags, see include/physics.lobster",
    [](StackPtr &sp, VM &) {
        CheckParticles();
        b2ParticleGroupDef pgd;
        b2CircleShape shape;
        pgd.shape = &shape;
        pgd.flags = Pop(sp).intval();
        auto c = PopVec<float3>(sp);
        pgd.color.Set(b2Color(c.x, c.y, c.z));
        shape.m_radius = Pop(sp).fltval();
        pgd.position = PopB2(sp);
        particlesystem->CreateParticleGroup(pgd);
    });

nfr("ph_initialize_particles", "radius", "F", "",
    "initializes the particle system with a given particle radius.",
    [](StackPtr &, VM &, Value &size) {
        CheckParticles(size.fltval());
        return Value();
    });

nfr("ph_step", "seconds,viter,piter", "FII", "",
    "simulates the physical world for the given period (try: gl_delta_time()). You can specify"
    " the amount of velocity/position iterations per step, more means more accurate but also"
    " more expensive computationally (try 8 and 3).",
    [](StackPtr &, VM &, Value &delta, Value &viter, Value &piter) {
        CheckPhysics();
        world->Step(min(delta.fltval(), 0.1f), viter.intval(), piter.intval());
        if (particlesystem) {
            for (b2Body *body = world->GetBodyList(); body; body = body->GetNext()) {
                for (b2Fixture *fixture = body->GetFixtureList(); fixture;
                     fixture = fixture->GetNext()) {
                    auto pc = ((PhysicsObject *)fixture->GetUserData())->particle_contacts;
                    if (pc) pc->clear();
                }
            }
            auto contacts = particlesystem->GetBodyContacts();
            for (int i = 0; i < particlesystem->GetBodyContactCount(); i++) {
                auto &c = contacts[i];
                auto pc = ((PhysicsObject *)c.fixture->GetUserData())->particle_contacts;
                if (pc) pc->push_back(c.index);
            }
        }
        return Value();
    });

nfr("ph_particle_contacts", "id", "R", "I]",
    "gets the particle indices that are currently contacting a giving physics object."
    " Call after step(). Indices may be invalid after next step().",
    [](StackPtr &sp, VM &vm, Value &id) {
        CheckPhysics();
        auto &po = GetObject(sp, vm, id);
        if (!po.particle_contacts) po.particle_contacts = new vector<int>();
        auto numelems = (int)po.particle_contacts->size();
        auto v = vm.NewVec(numelems, numelems, TYPE_ELEM_VECTOR_OF_INT);
        for (int i = 0; i < numelems; i++) v->At(i) = Value((*po.particle_contacts)[i]);
        return Value(v);
    });

nfr("ph_raycast", "p1,p2,n", "F}:2F}:2I", "I]",
    "returns a vector of the first n particle ids that intersect a ray from p1 to p2,"
    " not including particles that overlap p1.",
    [](StackPtr &sp, VM &vm) {
        CheckPhysics();
        auto n = Pop(sp).ival();
        auto p2v = PopB2(sp);
        auto p1v = PopB2(sp);
        auto v = vm.NewVec(0, max(n, 1_L), TYPE_ELEM_VECTOR_OF_INT);
        if (!particlesystem) { Push(sp,  v); return; }
        struct callback : b2RayCastCallback {
            LVector *v;
            VM &vm;
            float ReportParticle(const b2ParticleSystem *, int i, const b2Vec2 &, const b2Vec2 &,
                                 float) {
                v->Push(vm, Value(i));
                return v->len == v->maxl ? -1.0f : 1.0f;
            }
            float ReportFixture(b2Fixture *, const b2Vec2 &, const b2Vec2 &, float) {
                return -1.0f;
            }
            callback(LVector *_v, VM &vm) : v(_v), vm(vm) {}
        } cb(v, vm);
        particlesystem->RayCast(&cb, p1v, p2v);
        Push(sp,  v);
    });

nfr("ph_delete_particle", "i", "I", "",
    "deletes given particle. Deleting particles causes indices to be invalidated at next"
    " step().",
    [](StackPtr &, VM &, Value &i) {
        CheckPhysics();
        particlesystem->DestroyParticle(i.intval());
        return Value();
    });

nfr("ph_getparticle_position", "i", "I", "F}:2",
    "gets a particle's position.",
    [](StackPtr &sp, VM &) {
        CheckPhysics();
        auto pos = B2ToFloat2(particlesystem->GetPositionBuffer()[Pop(sp).ival()]);
        PushVec(sp, pos);
    });

nfr("ph_render", "", "", "",
    "renders all rigid body objects.",
    [](StackPtr &, VM &) {
        CheckPhysics();
        auto oldobject2view = otransforms.object2view;
        auto oldcolor = curcolor;
        for (b2Body *body = world->GetBodyList(); body; body = body->GetNext()) {
            auto pos = body->GetPosition();
            auto mat = translation(float3(pos.x, pos.y, 0)) * rotationZ(body->GetAngle());
            otransforms.object2view = oldobject2view * mat;
            for (b2Fixture *fixture = body->GetFixtureList(); fixture;
                 fixture = fixture->GetNext()) {
                auto shapetype = fixture->GetType();
                auto &r = ((PhysicsObject *)fixture->GetUserData())->r;
                curcolor = r.color;
                switch (shapetype) {
                    case b2Shape::e_polygon: {
                        r.Set();
                        auto polyshape = (b2PolygonShape *)fixture->GetShape();
                        RenderArraySlow(
                            PRIM_FAN, make_span(polyshape->m_vertices, polyshape->m_count), "pn",
                            span<int>(), make_span(polyshape->m_normals, polyshape->m_count));
                        break;
                    }
                    case b2Shape::e_circle: {
                        r.sh->SetTextures(r.textures);  // FIXME
                        auto polyshape = (b2CircleShape *)fixture->GetShape();
                        Transform2D(translation(float3(B2ToFloat2(polyshape->m_p), 0)), [&]() {
                            geomcache->RenderCircle(r.sh, PRIM_FAN, 20, polyshape->m_radius);
                        });
                        break;
                    }
                    case b2Shape::e_edge:
                    case b2Shape::e_chain:
                    case b2Shape::e_typeCount: assert(0); break;
                }
            }
        }
        otransforms.object2view = oldobject2view;
        curcolor = oldcolor;
        return Value();
    });

nfr("ph_render_particles", "scale", "F", "",
    "render all particles, with the given scale.",
    [](StackPtr &, VM &, Value &particlescale) {
        CheckPhysics();
        if (!particlesystem) return Value();
        // LOG_DEBUG("rendering particles: ", particlesystem->GetParticleCount());
        auto verts = (float2 *)particlesystem->GetPositionBuffer();
        auto colors = (byte4 *)particlesystem->GetColorBuffer();
        auto scale = length(otransforms.object2view[0].xy());
        SetPointSprite(scale * particlesystem->GetRadius() * particlescale.fltval());
        particlematerial->Set();
        RenderArraySlow(PRIM_POINT, make_span(verts, particlesystem->GetParticleCount()), "pC",
                        span<int>(), make_span(colors, particlesystem->GetParticleCount()));
        return Value();
    });

}  // AddPhysics
