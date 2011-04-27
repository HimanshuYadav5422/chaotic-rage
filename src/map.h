// This file is part of Chaotic Rage (c) 2010 Josh Heidenreich
//
// kate: tab-width 4; indent-width 4; space-indent off; word-wrap off;

#pragma once
#include <iostream>
#include <SDL.h>
#include "rage.h"

using namespace std;


#define RENDER_FRAME_DATA -1
#define RENDER_FRAME_BG -2


typedef struct {
	FloorType * type;
	Uint8 hp;
	char pad[3];
} data_pixel;


class Area {
	public:
		int x;
		int y;
		int width;
		int height;
		int angle;
		FloorType * type;
		AnimPlay * anim;
		int health;
		
	public:
		Area(FloorType * type);
		~Area();
};


class Map {
	friend class RenderSDL;
	friend class RenderOpenGL;
	
	private:
		vector<Area*> areas;
		vector<Zone*> zones;
		Render * render;
		GameState * st;
		
	public:
		data_pixel * data;
		SpritePtr background;
		int width;
		int height;
		
	public:
		Map(GameState * st);
		~Map();
		
	private:
		SDL_Surface * renderDataSurface();
		
	public:
		int load(string name, Render * render);
		SDL_Surface * renderFrame(int frame, bool wall);
		
		data_pixel getDataAt(int x, int y);
		void setDataHP(int x, int y, int newhp);
		
		Zone * getSpawnZone(Faction f);
		Zone * getPrisonZone(Faction f);
		Zone * getCollectZone(Faction f);
		Zone * getDestZone(Faction f);
		Zone * getNearbaseZone(Faction f);
};
