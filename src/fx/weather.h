// This file is part of Chaotic Rage (c) 2010 Josh Heidenreich
//
// kate: tab-width 4; indent-width 4; space-indent off; word-wrap off;

#pragma once
#include "../rage.h"

class GameState;

// Class forward references
namespace SPK
{
	class Zone;
	class Emitter;
	class Group;
	class Model;
}


/**
* How often to recalculate the weather
**/
#define RANDOM_UPDATE_TIME 30000.0f


/**
* Weather such as rain etc
**/
class Weather
{
	private:
		GameState* st;
		float delta;

		/**
		* Where all weather comes from
		**/
		SPK::Zone* sky;

		/**
		* Rain
		**/
		SPK::Emitter* rain_emitter;
		SPK::Group* rain_group;
		SPK::Model* rain_model;

	public:
		Weather(GameState* st, float map_size_x, float map_size_z);
		~Weather();

	public:
		/**
		* Update the internal clock
		**/
		void update(float delta);

		/**
		* The internal clock will do random weather at regular intervals
		**/
		void random();

		/**
		* Specific control for rain
		**/
		void startRain(int flow);
		void stopRain();
};

