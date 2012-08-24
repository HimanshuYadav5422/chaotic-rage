// This file is part of Chaotic Rage (c) 2010 Josh Heidenreich
//
// kate: tab-width 4; indent-width 4; space-indent off; word-wrap off;

#include <iostream>
#include <algorithm>
#include <SDL.h>
#include <btBulletDynamicsCommon.h>
#include "rage.h"

using namespace std;



PhysicsBullet::PhysicsBullet(GameState * st)
{
	this->st = st;
	st->physics = this;
}

PhysicsBullet::~PhysicsBullet()
{
}



/**
* This is run before map load to set up stuff
**/
void PhysicsBullet::init()
{
	collisionConfiguration = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collisionConfiguration);
	overlappingPairCache = new btDbvtBroadphase();
	solver = new btSequentialImpulseConstraintSolver();
	
	dynamicsWorld = new btDiscreteDynamicsWorld(
		dispatcher,
		overlappingPairCache,
		solver,
		collisionConfiguration
	);
	
	dynamicsWorld->setGravity(btVector3(0,0,-10));
}


/**
* This is post-map load, pre game run
**/
void PhysicsBullet::preGame()
{
	btCollisionShape* groundShape = new btBoxShape(btVector3(100.f, 100.f, 10.0f));
	
	// Ground
	btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(100.0f,100.0f,-10.0f)));
	btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(
		0,
		groundMotionState,
		groundShape,
		btVector3(0,0,0)
	);
	
	this->groundRigidBody = new btRigidBody(groundRigidBodyCI);
	this->groundRigidBody->setRestitution(0.f);
	this->groundRigidBody->setFriction(10.f);
	dynamicsWorld->addRigidBody(groundRigidBody);	//, COL_GROUND, COL_ALIVE | COL_DEAD
	
	// This leaks. I'm using pointers because of MS compiler doesn't like the alignment or something.
	this->addBoundaryPlane(new btVector3(1, 0, 0), new btVector3(0, 0, 0));
	this->addBoundaryPlane(new btVector3(0, 1, 0), new btVector3(0, 0, 0));
	this->addBoundaryPlane(new btVector3(-1, 0, 0), new btVector3(this->st->curr_map->width, this->st->curr_map->height, 0));
	this->addBoundaryPlane(new btVector3(0, -1, 0), new btVector3(this->st->curr_map->width, this->st->curr_map->height, 0));
	
	collisionShapes = new btAlignedObjectArray<btCollisionShape*>();
}


/**
* Create a boundary plane (invisible wall)
* There is normally four of these around the edges of the map.
**/
void PhysicsBullet::addBoundaryPlane(btVector3 * axis, btVector3 * loc)
{
	btCollisionShape* shape = new btStaticPlaneShape(*axis, 0);
	
	btDefaultMotionState* motionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1), *loc));
	btRigidBody::btRigidBodyConstructionInfo rigidBodyCI(
		0,
		motionState,
		shape,
		btVector3(0,0,0)
	);
	
	btRigidBody *rigidBody = new btRigidBody(rigidBodyCI);
	
	dynamicsWorld->addRigidBody(rigidBody);	//, COL_GROUND, COL_ALIVE | COL_DEAD
}


/**
* Tear down the physics world
**/
void PhysicsBullet::postGame()
{
	// TODO: Free other stuff properly, see HelloWorld demo
	
	delete collisionShapes;
	delete dynamicsWorld;
	delete solver;
	delete overlappingPairCache;
	delete dispatcher;
	delete collisionConfiguration;
}


/**
* Returns the world
**/
btDiscreteDynamicsWorld* PhysicsBullet::getWorld()
{
	return dynamicsWorld;
}


/**
* Create and add a rigid body
*
* colShape = The collission shape of the body
* m = mass
* x,y,z = origin position
**/
btRigidBody* PhysicsBullet::addRigidBody(btCollisionShape* colShape, float m, float x, float y, float z)
{
	btDefaultMotionState* myMotionState =
		new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(x,y,z)));
	
	btScalar mass(m);
	bool isDynamic = (mass != 0.f);
	
	btVector3 localInertia(0,0,0);
	if (isDynamic)
		colShape->calculateLocalInertia(mass,localInertia);
	
	btRigidBody::btRigidBodyConstructionInfo rbInfo(
		mass,
		myMotionState,
		colShape,
		localInertia
	);
	btRigidBody* body = new btRigidBody(rbInfo);
	
	dynamicsWorld->addRigidBody(body);	//, COL_ALIVE, COL_GROUND | COL_ALIVE
	
	return body;
}


/**
* Create and add a rigid body
*
* colShape = The collission shape of the body
* m = mass
* motionState = origin motion state (position and rotation)
**/
btRigidBody* PhysicsBullet::addRigidBody(btCollisionShape* colShape, float m, btDefaultMotionState* motionState)
{
	btScalar mass(m);
	bool isDynamic = (mass != 0.f);
	
	btVector3 localInertia(0,0,0);
	if (isDynamic)
		colShape->calculateLocalInertia(mass,localInertia);
	
	btRigidBody::btRigidBodyConstructionInfo rbInfo(
		mass,
		motionState,
		colShape,
		localInertia
	);
	btRigidBody* body = new btRigidBody(rbInfo);
	
	dynamicsWorld->addRigidBody(body);	//, COL_ALIVE, COL_GROUND | COL_ALIVE
	
	return body;
}


/**
* Add a vehicle
**/
void PhysicsBullet::addVehicle(btRaycastVehicle* vehicle)
{
	dynamicsWorld->addVehicle(vehicle);
}


/**
* Remove and reinsert a rigid body, but with different flags
**/
void PhysicsBullet::markDead(btRigidBody* body)
{
	if (! body) return;
	
	dynamicsWorld->removeCollisionObject(body);
	dynamicsWorld->addRigidBody(body);//, COL_DEAD, COL_GROUND
}


/**
* Remove a rigid body from the game world
**/
void PhysicsBullet::delRigidBody(btRigidBody* body)
{
	if (body && body->getMotionState()) {
		delete body->getMotionState();
	}
	
	dynamicsWorld->removeCollisionObject(body);
	
	delete body;
}


/**
* Step the physics forward by the given amount of time
**/
void PhysicsBullet::stepTime(int ms)
{
	dynamicsWorld->stepSimulation( ((float)ms) / 1000.0f , 10);
}



/**
* Look for collissions of entities
**/
void PhysicsBullet::doCollisions()
{
	int i;
	
	int numManifolds = dynamicsWorld->getDispatcher()->getNumManifolds();
	
	DEBUG("coll", "Num manifolds: %i", numManifolds);
	
	for (i = 0; i < numManifolds; i++) {
		btPersistentManifold* contactManifold = dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
		
		btCollisionObject* obA = static_cast<btCollisionObject*>(contactManifold->getBody0());
		btCollisionObject* obB = static_cast<btCollisionObject*>(contactManifold->getBody1());
		
		if (obA == this->groundRigidBody || obB == this->groundRigidBody) continue;
		
		DEBUG("coll", "Collision: %p %p", obA, obB);
		
		Entity* entA = static_cast<Entity*>(obA->getUserPointer());
		Entity* entB = static_cast<Entity*>(obB->getUserPointer());
		
		if (entA == NULL || entB == NULL) continue;

		entA->hasBeenHit(entB);
		entB->hasBeenHit(entA);
		
		/*int numContacts = contactManifold->getNumContacts();
		
		for (j = 0; j < numContacts; j++) {
			btManifoldPoint& pt = contactManifold->getContactPoint(j);
			
			glBegin(GL_LINES);
			glColor3f(0, 0, 0);
			
			btVector3 ptA = pt.getPositionWorldOnA();
			btVector3 ptB = pt.getPositionWorldOnB();
			
			glVertex3d(ptA.x(),ptA.y(),ptA.z());
			glVertex3d(ptB.x(),ptB.y(),ptB.z());
			glEnd();
		}*/
		
		//you can un-comment out this line, and then all points are removed
		//contactManifold->clearManifold();	
	}
}


void PhysicsBullet::QuaternionToEulerXYZ(const btQuaternion &quat, btVector3 &euler)
{
	float w=quat.getW(); float x=quat.getX(); float y=quat.getY(); float z=quat.getZ();
	double sqw = w*w; double sqx = x*x; double sqy = y*y; double sqz = z*z;
	
	euler.setZ((atan2(2.0 * (x*y + z*w),(sqx - sqy - sqz + sqw))));
	euler.setX((atan2(2.0 * (y*z + x*w),(-sqx - sqy + sqz + sqw))));
	euler.setY((asin(-2.0 * (x*z - y*w))));
}

