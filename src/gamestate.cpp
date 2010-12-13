// This file is part of Chaotic Rage (c) 2010 Josh Heidenreich
//
// kate: tab-width 4; indent-width 4; space-indent off; word-wrap off;

#include <string.h>
#include <iostream>
#include <map>
#include <algorithm>
#include <SDL.h>
#include "rage.h"

using namespace std;


static GameState *g_st;


static bool EntityEraser(Entity *e);



/**
* Don't use unless if you are in global code
*
* Should this be removed?
**/
GameState * getGameState()
{
	return g_st;
}


/**
* New game state
**/
GameState::GameState()
{
	this->anim_frame = 0;
	this->game_time = 0;
	
	this->hud = new HUD();
	this->hud->st = this;
	
	this->render = NULL;
	
	g_st = this;
}

GameState::~GameState()
{
	delete(this->hud);
}


/**
* Add a unit
**/
void GameState::addUnit(Unit* unit)
{
	this->entities_add.push_back(unit);
	this->units.push_back(unit);
}

/**
* Add a particle
**/
void GameState::addParticle(Particle* particle)
{
	this->entities_add.push_back(particle);
}

/**
* Add a particle generator
**/
void GameState::addParticleGenerator(ParticleGenerator* generator)
{
	this->entities_add.push_back(generator);
}

/**
* Add a wall
**/
void GameState::addWall(Wall* wall)
{
	this->entities_add.push_back(wall);
	this->walls.push_back(wall);
}


/**
* Used for filtering
**/
static bool EntityEraser(Entity *e)
{
	return e->del;
}



/**
* Updates the state of everything
**/
void GameState::update(int delta)
{
	DEBUG("Updating gamestate using delta: %i\n", delta);
	
	vector<Entity*>::iterator it;
	vector<Entity*>::iterator newend;
	
	// Add new entities
	for (it = this->entities_add.begin(); it < this->entities_add.end(); it++) {
		Entity *e = (*it);
		this->entities.push_back(e);
	}
	this->entities_add.clear();
	
	// Update entities
	for (it = this->entities.begin(); it < this->entities.end(); it++) {
		Entity *e = (*it);
		if (! e->del) {
			e->update(delta);
		}
	}
	
	this->doCollisions();
	
	// Remove entities
	// I'm fairly sure this leaks because it doesn't actually delete the entity
	newend = remove_if(this->entities.begin(), this->entities.end(), EntityEraser);
	this->entities.erase(newend, this->entities.end());
	
	// Update time
	this->game_time += delta;
	this->anim_frame = (int) floor(this->game_time * ANIMATION_FPS / 1000.0);
}


/**
* Look for collissions of entities
**/
void GameState::doCollisions()
{
	float dist;
	
	vector<Entity*>::iterator it1;
	vector<Entity*>::iterator it2;
	list<Entity*>::iterator it3;
	map<Entity*,Entity*>::iterator it4;
	map<Entity*,Entity*> rem;
	
	for (it1 = this->entities.begin(); it1 != this->entities.end(); it1++) {
		Entity *e1 = (*it1);
		if (! e1->collide) continue;
		
		for (it2 = it1; it2 != this->entities.end(); it2++) {
			Entity *e2 = (*it2);
			if (e2 == e1) continue;
			if (! e2->collide) continue;
			
			dist = sqrt(((e1->x - e2->x) * (e1->x - e2->x)) + ((e1->y - e2->y) * (e1->y - e2->y)));
			
			if (dist <= e1->radius + e2->radius) {
				if (find(e1->hits.begin(), e1->hits.end(), e2) == e1->hits.end()) {
					e1->hasHit(e2);
					e2->hasHit(e1);
					
					Event *ev = new Event();
					ev->type = ENTITY_HIT;
					ev->e1 = e1;
					ev->e2 = e2;
					fireEvent(ev);
				}
			}
		}
		
		if (rem.find(e1) == rem.end()) {
			for (it3 = e1->hits.begin(); it3 != e1->hits.end(); it3++) {
				Entity *e2 = (*it3);
			
				dist = sqrt(((e1->x - e2->x) * (e1->x - e2->x)) + ((e1->y - e2->y) * (e1->y - e2->y)));
				if (dist > e1->radius + e2->radius) {
					rem[e1] = e2;
					rem[e2] = e1;
					
					Event *ev = new Event();
					ev->type = ENTITY_UNHIT;
					ev->e1 = e1;
					ev->e2 = e2;
					fireEvent(ev);
				}
			}
		}
	}
	
	for (it4 = rem.begin(); it4 != rem.end(); it4++) {
		(*it4).first->hits.remove((*it4).second);
	}
}


/**
* Add a mod to the list
**/
void GameState::addMod(Mod * mod)
{
	mods.push_back(mod);
}

/**
* Get a mod
**/
Mod * GameState::getMod(int id)
{
	return mods.at(id);
}


