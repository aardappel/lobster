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

#include "stdafx.h"

#include "vmdata.h"
#include "natreg.h"

#include "Box2D/Box2D.h"

#include "glinterface.h"

using namespace lobster;

struct Renderable : Textured
{
	float4 color;
	Shader *sh;

	Renderable(const char *shname) : color(float4_1), sh(LookupShader(shname))
	{
		assert(sh);
	}

	void Set()
	{
		sh->Set();
		sh->SetTextures(textures);
	}
};

b2World *world = nullptr;
IntResourceManagerCompact<b2Fixture> *fixtures = nullptr;
b2ParticleSystem *particlesystem = nullptr;
Renderable *particlematerial = nullptr;

void CleanPhysics()
{
	if (fixtures) delete fixtures;
	fixtures = nullptr;

	if (world) delete world;
	world = nullptr;
	particlesystem = nullptr;

	delete particlematerial;
	particlematerial = nullptr;
}

void InitPhysics(const float2 &gv)
{
	// FIXME: check that shaders are initialized, since renderables depend on that
	CleanPhysics();
	world = new b2World(b2Vec2(gv.x(), gv.y()));
	fixtures = new IntResourceManagerCompact<b2Fixture>([](b2Fixture *fixture)
	{
		delete (Renderable *)fixture->GetUserData();
	});
}

void CheckPhysics()
{
	if (!world) InitPhysics(float2(0, -10));
}

void CheckParticles(float size = 0.1f)
{
	CheckPhysics();
	if (!particlesystem)
	{
		b2ParticleSystemDef psd;
		psd.radius = size;
		particlesystem = world->CreateParticleSystem(&psd);
		particlematerial = new Renderable("color_attr");
	}
}

b2Vec2 Float2ToB2(const float2 &v) { return b2Vec2(v.x(), v.y()); }
float2 B2ToFloat2(const b2Vec2 &v) { return float2(v.x, v.y); }

b2Vec2 ValueDecToB2(Value &vec)
{
	auto v = ValueDecToF<2>(vec);
	return Float2ToB2(v);
}

b2Body &GetBody(Value &id, Value &position)
{
	CheckPhysics();
	b2Body *body = nullptr;
	if (id.True())
	{
		auto other_fixture = fixtures->Get(id.ival());
		if (other_fixture) body = other_fixture->GetBody();
	}
	auto wpos = ValueDecToF<2>(position);
	if (!body)
	{
		b2BodyDef bd;
		bd.type = b2_staticBody;
		bd.position.Set(wpos.x(), wpos.y());
		body = world->CreateBody(&bd);
	}
	return *body;
}

Value CreateFixture(b2Body &body, b2Shape &shape)
{
	auto fixture = body.CreateFixture(&shape, 1.0f);
	auto r = new Renderable("color");
	fixture->SetUserData(r);
	return Value((int)fixtures->Add(fixture));
}

b2Vec2 OptionalOffset(Value &offset) { return offset.True() ? ValueDecToB2(offset) : b2Vec2_zero; }

Renderable *GetRenderable(int id)
{
	CheckPhysics();
    if (!id) return particlematerial;
	auto fixture = fixtures->Get(id);
	return fixture ? (Renderable *)fixture->GetUserData() : nullptr;
}

extern int GetSampler(Value &i); // from graphics

void AddPhysics()
{
	STARTDECL(ph_initialize) (Value &gravity)
	{
		InitPhysics(ValueDecToF<2>(gravity));
		return Value();
	}
	ENDDECL1(ph_initialize, "gravityvector", "F]", "",
        "initializes or resets the physical world, gravity typically [0, -10].");

	STARTDECL(ph_createbox) (Value &position, Value &size, Value &offset, Value &rot, Value &other_id)
	{
		auto &body = GetBody(other_id, position);
		auto sz = ValueDecToF<2>(size);
		auto r = rot.fval();
		b2PolygonShape shape;
		shape.SetAsBox(sz.x(), sz.y(), OptionalOffset(offset), r * RAD);
		return CreateFixture(body, shape);
	}
	ENDDECL5(ph_createbox, "position,size,offset,rotation,attachto", "F]:2F]:2F]:2?F?I?", "I",
        "creates a physical box shape in the world at position, with size the half-extends around the center,"
        " offset from the center if needed, at a particular rotation (in degrees)."
        " attachto is a previous physical object to attach this one to, to become a combined physical body.");

	STARTDECL(ph_createcircle) (Value &position, Value &radius, Value &offset, Value &other_id)
	{
		auto &body = GetBody(other_id, position);
		b2CircleShape shape;
		auto off = OptionalOffset(offset);
		shape.m_p.Set(off.x, off.y);
		shape.m_radius = radius.fval();
		return CreateFixture(body, shape);
	}
	ENDDECL4(ph_createcircle, "position,radius,offset,attachto", "F]:2FF]:2?I?", "I",
        "creates a physical circle shape in the world at position, with the given radius, offset from the center if"
        " needed. attachto is a previous physical object to attach this one to, to become a combined physical body.");

	STARTDECL(ph_createpolygon) (Value &position, Value &vertices, Value &other_id)
	{
		auto &body = GetBody(other_id, position);
        b2PolygonShape shape;
		auto verts = new b2Vec2[vertices.eval()->Len()];
        for (int i = 0; i < vertices.eval()->Len(); i++)
        {
            auto vert = ValueToF<2>(vertices.eval()->At(i));
            verts[i] = Float2ToB2(vert);
        }
		shape.Set(verts, vertices.eval()->Len());
		delete[] verts;
		vertices.DECRT();
		return CreateFixture(body, shape);
	}
	ENDDECL3(ph_createpolygon, "position,vertices,attachto", "F]:2F]]I?", "I",
        "creates a polygon circle shape in the world at position, with the given list of vertices."
        " attachto is a previous physical object to attach this one to, to become a combined physical body.");

	STARTDECL(ph_dynamic) (Value &fixture_id, Value &on)
	{
		CheckPhysics();
		auto fixture = fixtures->Get(fixture_id.ival());
		if (fixture) fixture->GetBody()->SetType(on.ival() ? b2_dynamicBody : b2_staticBody);
		return fixture_id;
	}
	ENDDECL2(ph_dynamic, "shape,on", "II", "I",
        "makes a shape dynamic (on = true) or not. returns shape.");

	STARTDECL(ph_deleteshape) (Value &fixture_id)
	{
		CheckPhysics();
		auto fixture = fixtures->Get(fixture_id.ival());
		if (fixture)
		{
			auto body = fixture->GetBody();
			body->DestroyFixture(fixture);
			if (!body->GetFixtureList()) world->DestroyBody(body);
			fixtures->Delete(fixture_id.ival());
		}
		return Value();
	}
	ENDDECL1(ph_deleteshape, "id", "I", "",
        "removes a shape from the physical world.");

	STARTDECL(ph_setcolor) (Value &fixture_id, Value &color)
	{
		auto r = GetRenderable(fixture_id.ival());
		auto c = ValueDecToF<4>(color);
		if (r) r->color = c;
		return Value();
	}
	ENDDECL2(ph_setcolor, "id,color", "IF]:4", "",
        "sets a shape (or 0 for particles) to be rendered with a particular color.");

	STARTDECL(ph_setshader) (Value &fixture_id, Value &shader)
	{
		auto r = GetRenderable(fixture_id.ival());
		auto sh = LookupShader(shader.sval()->str());
		shader.DECRT();
		if (r && sh) r->sh = sh;
		return Value();
	}
	ENDDECL2(ph_setshader, "id,shadername", "IS", "",
        "sets a shape (or 0 for particles) to be rendered with a particular shader.");

	STARTDECL(ph_settexture) (Value &fixture_id, Value &tex_id, Value &tex_unit)
	{
		auto r = GetRenderable(fixture_id.ival());
		if (r) r->textures[GetSampler(tex_unit)] = tex_id.ival();
		return Value();
	}
	ENDDECL3(ph_settexture, "id,texid,texunit", "III?", "",
        "sets a shape (or 0 for particles) to be rendered with a particular texture"
        " (assigned to a texture unit, default 0).");

	STARTDECL(ph_createparticlecircle) (Value &position, Value &radius, Value &color, Value &type)
	{
		CheckParticles();
		b2ParticleGroupDef pgd;
		b2CircleShape shape;
		shape.m_radius = radius.fval();
		pgd.shape = &shape;
		pgd.flags = type.ival();
		pgd.position = ValueDecToB2(position);
		auto c = ValueDecToF<3>(color);
		pgd.color.Set(b2Color(c.x(), c.y(), c.z()));
		particlesystem->CreateParticleGroup(pgd);
		return Value();
	}
	ENDDECL4(ph_createparticlecircle, "position,radius,color,flags", "F]:2FF]:4I?", "",
        "creates a circle filled with particles. For flags, see include/physics.lobster");

	STARTDECL(ph_initializeparticles) (Value &size)
	{
		CheckParticles(size.fval());
		return Value();
	}
	ENDDECL1(ph_initializeparticles, "radius", "F", "",
        "initializes the particle system with a given particle radius.");

	STARTDECL(ph_step) (Value &delta)
	{
		CheckPhysics();
		world->Step(min(delta.fval(), 0.1f), 8, 3);
		return Value();
	}
	ENDDECL1(ph_step, "seconds", "F", "",
        "simulates the physical world for the given period (try: gl_deltatime()).");

	STARTDECL(ph_render) ()
	{
		CheckPhysics();
		auto oldobject2view = otransforms.object2view;
		auto oldcolor = curcolor;
		for (b2Body *body = world->GetBodyList(); body; body = body->GetNext())
		{
			auto pos = body->GetPosition();
			auto mat = translation(float3(pos.x, pos.y, 0)) * rotationZ(body->GetAngle());
            otransforms.object2view = oldobject2view * mat;

			for (b2Fixture *fixture = body->GetFixtureList(); fixture; fixture = fixture->GetNext())
			{
				auto shapetype = fixture->GetType();
				auto r = (Renderable *)fixture->GetUserData();
				curcolor = r->color;
				switch (shapetype)
				{
					case b2Shape::e_polygon:
					{
                        r->Set();
                        auto polyshape = (b2PolygonShape *)fixture->GetShape();
						RenderArraySlow(PRIM_FAN, polyshape->m_count, polyshape->m_count, "pn", sizeof(b2Vec2), polyshape->m_vertices, nullptr, 
							                                              sizeof(b2Vec2), polyshape->m_normals);
						break;
					}
					case b2Shape::e_circle:
					{
                        r->sh->SetTextures(r->textures);  // FIXME
                        auto polyshape = (b2CircleShape *)fixture->GetShape();
                        Transform2D(translation(float3(B2ToFloat2(polyshape->m_p), 0)), [&]()
                        {
                            RenderCircle(r->sh, PRIM_FAN, 20, polyshape->m_radius);
                        });
						break;
					}
					case b2Shape::e_edge:
					case b2Shape::e_chain:
                    case b2Shape::e_typeCount:
						assert(0);
						break;
				}
			}
		}
        otransforms.object2view = oldobject2view;
		curcolor = oldcolor;
		return Value();
	}
	ENDDECL0(ph_render, "", "", "",
        "renders all rigid body objects.");

    STARTDECL(ph_renderparticles) (Value &particlescale)
    {
        CheckPhysics();
        if (!particlesystem) return Value();

        //Output(OUTPUT_DEBUG, "rendering particles: %d", particlesystem->GetParticleCount());
        auto verts = (float2 *)particlesystem->GetPositionBuffer();
        auto colors = (byte4 *)particlesystem->GetColorBuffer();
        auto scale = fabs(otransforms.object2view[0].x());
        SetPointSprite(scale * particlesystem->GetRadius() * particlescale.fval());
        particlematerial->Set();
        RenderArraySlow(PRIM_POINT, particlesystem->GetParticleCount(), particlesystem->GetParticleCount(), "pC", sizeof(float2), verts, nullptr, sizeof(byte4), colors);
        return Value();
    }
    ENDDECL1(ph_renderparticles, "scale", "F", "",
        "render all particles, with the given scale.");

}
