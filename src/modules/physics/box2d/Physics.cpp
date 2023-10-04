/**
 * Copyright (c) 2006-2023 LOVE Development Team
 *
 * This software is provided 'as-is', without any express or implied
 * warranty.  In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 **/

#include "Physics.h"

// LOVE
#include "common/math.h"
#include "wrap_Body.h"

namespace love
{
namespace physics
{
namespace box2d
{

// TODO: Make this not static.
float Physics::meter = Physics::DEFAULT_METER;

Physics::Physics()
{
	meter = DEFAULT_METER;
}

Physics::~Physics()
{
}

const char *Physics::getName() const
{
	return "love.physics.box2d";
}

World *Physics::newWorld(float gx, float gy, bool sleep)
{
	return new World(b2Vec2(gx, gy), sleep);
}

Body *Physics::newBody(World *world, float x, float y, Body::Type type)
{
	return new Body(world, b2Vec2(x, y), type);
}

Body *Physics::newBody(World *world, Body::Type type)
{
	return new Body(world, b2Vec2(0, 0), type);
}

Body *Physics::newCircleBody(World *world, Body::Type type, float x, float y, float radius)
{
	StrongRef<Body> body(newBody(world, x, y, type), Acquire::NORETAIN);
	StrongRef<CircleShape> shape(newCircleShape(0, 0, radius), Acquire::NORETAIN);
	StrongRef<Fixture> fixture(newFixture(body, shape, 1.0f), Acquire::NORETAIN);
	body->retain();
	return body.get();
}

Body *Physics::newRectangleBody(World *world, Body::Type type, float x, float y, float w, float h, float angle)
{
	StrongRef<Body> body(newBody(world, x, y, type), Acquire::NORETAIN);
	StrongRef<PolygonShape> shape(newRectangleShape(0, 0, w, h, angle), Acquire::NORETAIN);
	StrongRef<Fixture> fixture(newFixture(body, shape, 1.0f), Acquire::NORETAIN);
	body->retain();
	return body.get();
}

Body *Physics::newPolygonBody(World *world, Body::Type type, const Vector2 *coords, int count)
{
	Vector2 origin(0, 0);

	for (int i = 0; i < count; i++)
		origin += coords[i] / count;

	std::vector<Vector2> localcoords;
	for (int i = 0; i < count; i++)
		localcoords.push_back(coords[i] - origin);

	StrongRef<Body> body(newBody(world, origin.x, origin.y, type), Acquire::NORETAIN);
	StrongRef<PolygonShape> shape(newPolygonShape(localcoords.data(), count), Acquire::NORETAIN);
	StrongRef<Fixture> fixture(newFixture(body, shape, 1.0f), Acquire::NORETAIN);
	body->retain();
	return body.get();
}

Body *Physics::newEdgeBody(World *world, Body::Type type, float x1, float y1, float x2, float y2, bool oneSided)
{
	float wx = (x2 - x1) / 2.0f;
	float wy = (y2 - y1) / 2.0f;
	StrongRef<Body> body(newBody(world, wx, wy, type), Acquire::NORETAIN);
	StrongRef<EdgeShape> shape(newEdgeShape(x1 - wx, y1 - wy, x2 - wx, y2 - wy, oneSided), Acquire::NORETAIN);
	StrongRef<Fixture> fixture(newFixture(body, shape, 1.0f), Acquire::NORETAIN);
	body->retain();
	return body.get();
}

Body *Physics::newChainBody(World *world, Body::Type type, bool loop, const Vector2 *coords, int count)
{
	Vector2 origin(0, 0);

	for (int i = 0; i < count; i++)
		origin += coords[i] / count;

	std::vector<Vector2> localcoords;
	for (int i = 0; i < count; i++)
		localcoords.push_back(coords[i] - origin);

	StrongRef<Body> body(newBody(world, origin.x, origin.y, type), Acquire::NORETAIN);
	StrongRef<ChainShape> shape(newChainShape(loop, localcoords.data(), count), Acquire::NORETAIN);
	StrongRef<Fixture> fixture(newFixture(body, shape, 1.0f), Acquire::NORETAIN);
	body->retain();
	return body.get();
}

CircleShape *Physics::newCircleShape(float x, float y, float radius)
{
	b2CircleShape *s = new b2CircleShape();
	s->m_p = Physics::scaleDown(b2Vec2(x, y));
	s->m_radius = Physics::scaleDown(radius);
	return new CircleShape(s);
}

PolygonShape *Physics::newRectangleShape(float x, float y, float w, float h, float angle)
{
	b2PolygonShape *s = new b2PolygonShape();
	s->SetAsBox(Physics::scaleDown(w/2.0f), Physics::scaleDown(h/2.0f), Physics::scaleDown(b2Vec2(x, y)), angle);
	return new PolygonShape(s);
}

EdgeShape *Physics::newEdgeShape(float x1, float y1, float x2, float y2, bool oneSided)
{
	b2EdgeShape *s = new b2EdgeShape();
	if (oneSided)
	{
		b2Vec2 v1 = Physics::scaleDown(b2Vec2(x1, y1));
		b2Vec2 v2 = Physics::scaleDown(b2Vec2(x2, y2));
		s->SetOneSided(v1, v1, v2, v2);
	}
	else
	{
		s->SetTwoSided(Physics::scaleDown(b2Vec2(x1, y1)), Physics::scaleDown(b2Vec2(x2, y2)));
	}
	return new EdgeShape(s);
}

PolygonShape *Physics::newPolygonShape(const Vector2 *coords, int count)
{
	// 3 to 8 (b2_maxPolygonVertices) vertices
	if (count < 3)
		throw love::Exception("Expected a minimum of 3 vertices, got %d.", count);
	else if (count > b2_maxPolygonVertices)
		throw love::Exception("Expected a maximum of %d vertices, got %d.", b2_maxPolygonVertices, count);

	b2Vec2 vecs[b2_maxPolygonVertices];

	for (int i = 0; i < count; i++)
		vecs[i] = Physics::scaleDown(b2Vec2(coords[i].x, coords[i].y));

	b2PolygonShape *s = new b2PolygonShape();

	try
	{
		s->Set(vecs, count);
	}
	catch (love::Exception &)
	{
		delete s;
		throw;
	}

	return new PolygonShape(s);
}

ChainShape *Physics::newChainShape(bool loop, const Vector2 *coords, int count)
{
	std::vector<b2Vec2> vecs;

	for (int i = 0; i < count; i++)
		vecs.push_back(Physics::scaleDown(b2Vec2(coords[i].x, coords[i].y)));

	b2ChainShape *s = new b2ChainShape();

	try
	{
		if (loop)
			s->CreateLoop(vecs.data(), count);
		else
			s->CreateChain(vecs.data(), count, vecs[0], vecs[count - 1]);
	}
	catch (love::Exception &)
	{
		delete s;
		throw;
	}

	return new ChainShape(s);
}

DistanceJoint *Physics::newDistanceJoint(Body *body1, Body *body2, float x1, float y1, float x2, float y2, bool collideConnected)
{
	return new DistanceJoint(body1, body2, x1, y1, x2, y2, collideConnected);
}

MouseJoint *Physics::newMouseJoint(Body *body, float x, float y)
{
	return new MouseJoint(body, x, y);
}

RevoluteJoint *Physics::newRevoluteJoint(Body *body1, Body *body2, float xA, float yA, float xB, float yB, bool collideConnected)
{
	return new RevoluteJoint(body1, body2, xA, yA, xB, yB, collideConnected);
}

RevoluteJoint *Physics::newRevoluteJoint(Body *body1, Body *body2, float xA, float yA, float xB, float yB, bool collideConnected, float referenceAngle)
{
	return new RevoluteJoint(body1, body2, xA, yA, xB, yB, collideConnected, referenceAngle);
}

PrismaticJoint *Physics::newPrismaticJoint(Body *body1, Body *body2, float xA, float yA, float xB, float yB, float ax, float ay, bool collideConnected)
{
	return new PrismaticJoint(body1, body2, xA, yA, xB, yB, ax, ay, collideConnected);
}

PrismaticJoint *Physics::newPrismaticJoint(Body *body1, Body *body2, float xA, float yA, float xB, float yB, float ax, float ay, bool collideConnected, float referenceAngle)
{
	return new PrismaticJoint(body1, body2, xA, yA, xB, yB, ax, ay, collideConnected, referenceAngle);
}

PulleyJoint *Physics::newPulleyJoint(Body *body1, Body *body2, b2Vec2 groundAnchor1, b2Vec2 groundAnchor2, b2Vec2 anchor1, b2Vec2 anchor2, float ratio, bool collideConnected)
{
	return new PulleyJoint(body1, body2, groundAnchor1, groundAnchor2, anchor1, anchor2, ratio, collideConnected);
}

GearJoint *Physics::newGearJoint(Joint *joint1, Joint *joint2, float ratio, bool collideConnected)
{
	return new GearJoint(joint1, joint2, ratio, collideConnected);
}

FrictionJoint *Physics::newFrictionJoint(Body *body1, Body *body2, float xA, float yA, float xB, float yB, bool collideConnected)
{
	return new FrictionJoint(body1, body2, xA, yA, xB, yB, collideConnected);
}

WeldJoint *Physics::newWeldJoint(Body *body1, Body *body2, float xA, float yA, float xB, float yB, bool collideConnected)
{
	return new WeldJoint(body1, body2, xA, yA, xB, yB, collideConnected);
}

WeldJoint *Physics::newWeldJoint(Body *body1, Body *body2, float xA, float yA, float xB, float yB, bool collideConnected, float referenceAngle)
{
	return new WeldJoint(body1, body2, xA, yA, xB, yB, collideConnected, referenceAngle);
}

WheelJoint *Physics::newWheelJoint(Body *body1, Body *body2, float xA, float yA, float xB, float yB, float ax, float ay, bool collideConnected)
{
	return new WheelJoint(body1, body2, xA, yA, xB, yB, ax, ay, collideConnected);
}

RopeJoint *Physics::newRopeJoint(Body *body1, Body *body2, float x1, float y1, float x2, float y2, float maxLength, bool collideConnected)
{
	return new RopeJoint(body1, body2, x1, y1, x2, y2, maxLength, collideConnected);
}

MotorJoint *Physics::newMotorJoint(Body *body1, Body *body2)
{
	return new MotorJoint(body1, body2);
}

MotorJoint *Physics::newMotorJoint(Body *body1, Body *body2, float correctionFactor, bool collideConnected)
{
	return new MotorJoint(body1, body2, correctionFactor, collideConnected);
}


Fixture *Physics::newFixture(Body *body, Shape *shape, float density)
{
	return new Fixture(body, shape, density);
}

int Physics::getDistance(lua_State *L)
{
	Fixture *fixtureA = luax_checktype<Fixture>(L, 1);
	Fixture *fixtureB = luax_checktype<Fixture>(L, 2);
	b2DistanceProxy pA, pB;
	b2DistanceInput i;
	b2DistanceOutput o;
	b2SimplexCache c;
	c.count = 0;

	luax_catchexcept(L, [&]() {
		pA.Set(fixtureA->fixture->GetShape(), 0);
		pB.Set(fixtureB->fixture->GetShape(), 0);
		i.proxyA = pA;
		i.proxyB = pB;
		i.transformA = fixtureA->fixture->GetBody()->GetTransform();
		i.transformB = fixtureB->fixture->GetBody()->GetTransform();
		i.useRadii = true;
		b2Distance(&o, &c, &i);
	});

	lua_pushnumber(L, Physics::scaleUp(o.distance));
	lua_pushnumber(L, Physics::scaleUp(o.pointA.x));
	lua_pushnumber(L, Physics::scaleUp(o.pointA.y));
	lua_pushnumber(L, Physics::scaleUp(o.pointB.x));
	lua_pushnumber(L, Physics::scaleUp(o.pointB.y));
	return 5;
}

void Physics::setMeter(float scale)
{
	if (scale < 1) throw love::Exception("Physics error: invalid meter");
	Physics::meter = scale;
}

float Physics::getMeter()
{
	return meter;
}

void Physics::scaleDown(float &x, float &y)
{
	x /= meter;
	y /= meter;
}

void Physics::scaleUp(float &x, float &y)
{
	x *= meter;
	y *= meter;
}

float Physics::scaleDown(float f)
{
	return f/meter;
}

float Physics::scaleUp(float f)
{
	return f*meter;
}

b2Vec2 Physics::scaleDown(const b2Vec2 &v)
{
	b2Vec2 t = v;
	scaleDown(t.x, t.y);
	return t;
}

b2Vec2 Physics::scaleUp(const b2Vec2 &v)
{
	b2Vec2 t = v;
	scaleUp(t.x, t.y);
	return t;
}

b2AABB Physics::scaleDown(const b2AABB &aabb)
{
	b2AABB t;
	t.lowerBound = scaleDown(aabb.lowerBound);
	t.upperBound = scaleDown(aabb.upperBound);
	return t;
}

b2AABB Physics::scaleUp(const b2AABB &aabb)
{
	b2AABB t;
	t.lowerBound = scaleUp(aabb.lowerBound);
	t.upperBound = scaleUp(aabb.upperBound);
	return t;
}

void Physics::computeLinearStiffness(float &stiffness, float &damping, float frequency, float dampingRatio, b2Body *bodyA, b2Body *bodyB)
{
	b2LinearStiffness(stiffness, damping, frequency, dampingRatio, bodyA, bodyB);
}

void Physics::computeLinearFrequency(float &frequency, float &ratio, float stiffness, float damping, b2Body *bodyA, b2Body *bodyB)
{
	float massA = bodyA->GetMass();
	float massB = bodyB->GetMass();
	float mass;
	if (massA > 0.0f && massB > 0.0f)
	{
		mass = massA * massB / (massA + massB);
	}
	else if (massA > 0.0f)
	{
		mass = massA;
	}
	else
	{
		mass = massB;
	}

	if (mass == 0.0f || stiffness <= 0.0f)
	{
		frequency = 0.0f;
		ratio = 0.0f;
		return;
	};

	float omega = b2Sqrt(stiffness / mass);
	frequency = omega / (2.0f * b2_pi);
	ratio = damping / (mass * 2.0f * omega);
}

void Physics::computeAngularStiffness(float &stiffness, float &damping, float frequency, float dampingRatio, b2Body *bodyA, b2Body *bodyB)
{
	b2AngularStiffness(stiffness, damping, frequency, dampingRatio, bodyA, bodyB);
}

void Physics::computeAngularFrequency(float &frequency, float &ratio, float stiffness, float damping, b2Body *bodyA, b2Body *bodyB)
{
	float IA = bodyA->GetInertia();
	float IB = bodyB->GetInertia();
	float I;
	if (IA > 0.0f && IB > 0.0f)
	{
		I = IA * IB / (IA + IB);
	}
	else if (IA > 0.0f)
	{
		I = IA;
	}
	else
	{
		I = IB;
	}

	if (I == 0.0f || stiffness <= 0.0f)
	{
		frequency = 0.0f;
		ratio = 0.0f;
		return;
	};

	float omega = b2Sqrt(stiffness / I);
	frequency = omega / (2.0f * b2_pi);
	ratio = damping / (I * 2.0f * omega);
}

} // box2d
} // physics
} // love
