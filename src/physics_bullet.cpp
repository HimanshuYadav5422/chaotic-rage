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
	
	dynamicsWorld->setGravity(btVector3(0,-10,0));
}


/**
* This is post-map load, pre game run
**/
void PhysicsBullet::preGame()
{
	this->groundRigidBody = st->curr_map->createGroundBody();
	
	// If the map fails to return a terrain, we will create our own (flat one)
	if (! this->groundRigidBody) {
		btCollisionShape* groundShape = new btBoxShape(btVector3(this->st->curr_map->width/2.0f, 10.0f, this->st->curr_map->height/2.0f));
		
		btDefaultMotionState* groundMotionState = new btDefaultMotionState(btTransform(btQuaternion(0,0,0,1),btVector3(this->st->curr_map->width/2.0f, -10.0f, this->st->curr_map->height/2.0f)));
		btRigidBody::btRigidBodyConstructionInfo groundRigidBodyCI(
			0,
			groundMotionState,
			groundShape,
			btVector3(0,0,0)
		);
		
		this->groundRigidBody = new btRigidBody(groundRigidBodyCI);
	}
	
	this->groundRigidBody->setRestitution(0.f);
	this->groundRigidBody->setFriction(10.f);
	dynamicsWorld->addRigidBody(groundRigidBody);	//, COL_GROUND, COL_ALIVE | COL_DEAD
	
	collisionShapes = new btAlignedObjectArray<btCollisionShape*>();
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
* Add a rigid body which already exists
**/
void PhysicsBullet::addRigidBody(btRigidBody* body)
{
	dynamicsWorld->addRigidBody(body);	//, COL_ALIVE, COL_GROUND | COL_ALIVE
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
* Remove a rigid body from the game world, but keep the object around for adding later.
**/
void PhysicsBullet::remRigidBody(btRigidBody* body)
{
	dynamicsWorld->removeCollisionObject(body);
}


/**
* Delete a rigid body, and remove it from the game world too
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


/**
* Use a raytest to find an appropriate spawn location on the ground
**/
btVector3 PhysicsBullet::spawnLocation(float x, float z, float height)
{
	float y = 100.0f;
	
	btVector3 begin(x, y, z);
	btVector3 end(x, -y, z);
	
	btCollisionWorld::ClosestRayResultCallback cb(begin, end);
	dynamicsWorld->rayTest(begin, end, cb);
	
	if (cb.hasHit()) {
		y = cb.m_hitPointWorld.y() + height/2.0f;
	}
	
	return btVector3(x, y, z);
}


/**
* Convert a quaternion into euler angles
**/
void PhysicsBullet::QuaternionToEulerXYZ(const btQuaternion &quat, btVector3 &euler)
{
	float w=quat.getW(); float x=quat.getX(); float y=quat.getY(); float z=quat.getZ();
	double sqw = w*w; double sqx = x*x; double sqy = y*y; double sqz = z*z;
	
	euler.setZ((atan2(2.0 * (x*y + z*w),(sqx - sqy - sqz + sqw))));
	euler.setX((atan2(2.0 * (y*z + x*w),(-sqx - sqy + sqz + sqw))));
	euler.setY((asin(-2.0 * (x*z - y*w))));
}


/**
* Get the yaw from a quaternion
**/
float PhysicsBullet::QuaternionToYaw(const btQuaternion &quat)
{
	float fTx  = 2.0*quat.x();
	float fTy  = 2.0*quat.y();
	float fTz  = 2.0*quat.z();
	float fTwy = fTy*quat.w();
	float fTxx = fTx*quat.x();
	float fTxz = fTz*quat.x();
	float fTyy = fTy*quat.y();
	 
	return atan2(fTxz+fTwy, 1.0-(fTxx+fTyy));
}


