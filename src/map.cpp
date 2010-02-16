#include <iostream>
#include <SDL.h>
#include <math.h>
#include "rage.h"

using namespace std;


SDL_Surface *createDataSurface(int w, int h, Uint32 initial_data);


/**
* Load a file (simulated)
**/
int Map::load(string name)
{
	Area *a;
	
	this->width = 1000;
	this->height = 1000;
	
	// Default area
	a = new Area();
	a->x = 0;
	a->y = 0;
	a->width = this->width;
	a->height = this->height;
	a->angle = 0;
	a->type = getAreaTypeByID(0);
	this->areas.push_back(a);
	
	a = new Area();
	a->x = 300;
	a->y = 300;
	a->width = 100;
	a->height = 100;
	a->angle = 22;
	a->type = getAreaTypeByID(2);
	this->areas.push_back(a);
	
	a = new Area();
	a->x = 150;
	a->y = 170;
	a->width = 100;
	a->height = 100;
	a->angle = 0;
	a->type = getAreaTypeByID(2);
	this->areas.push_back(a);
	
	a = new Area();
	a->x = 0;
	a->y = 0;
	a->width = 200;
	a->height = 1000;
	a->angle = 2;
	a->type = getAreaTypeByID(3);
	this->areas.push_back(a);
	
	a = new Area();
	a->x = 400;
	a->y = 30;
	a->width = 200;
	a->height = 30;
	a->angle = 0;
	a->type = getAreaTypeByID(3);
	this->areas.push_back(a);
	
	return 1;
}


/**
* Render a single frame of the wall animation.
*
* It is the responsibility of the caller to free the returned surface.
*
* @param int frame The frame to render. Use frame -1 to get the data surface
**/
SDL_Surface* Map::renderWallFrame(int frame)
{
	bool return_data = false;
	if (frame == RENDER_FRAME_DATA) {
		frame = 0;
		return_data = true;
		
	} else if (frame == RENDER_FRAME_BG) {
		return tileSprite(this->areas[0]->type->surf, this->areas[0]->width, this->areas[0]->height);
	}
	
	SDL_Surface* surf = SDL_CreateRGBSurface(SDL_SWSURFACE, this->width, this->height, 32, 0,0,0,0);
	
	int colourkey = SDL_MapRGB(surf->format, 255, 0, 255);
	SDL_SetColorKey(surf, SDL_SRCCOLORKEY, colourkey);
	SDL_FillRect(surf, NULL, colourkey);
	
	Area *a;
	unsigned int i;
	SDL_Rect dest;
	SDL_Surface *datasurf;
	
	for (i = 1; i < this->areas.size(); i++) {
		a = this->areas[i];
		
		if (return_data) {
			datasurf = createDataSurface(a->width, a->height, a->type->id * 5000);
		}
		
		// Transforms (either streches or tiles)
		SDL_Surface *areasurf = a->type->surf;
		if (a->type->stretch)  {
			areasurf = rotozoomSurfaceXY(areasurf, 0, ((double)a->width) / ((double)areasurf->w), ((double)a->height) / ((double)areasurf->h), 0);
			if (areasurf == NULL) continue;
			
		} else {
			areasurf = tileSprite(areasurf, a->width, a->height);
			if (areasurf == NULL) continue;
		}
		
		// Rotates
		if (a->angle != 0)  {
			SDL_Surface* temp = areasurf;
			
			areasurf = rotozoomSurfaceXY(temp, a->angle, 1, 1, 0);
			SDL_FreeSurface(temp);
			if (areasurf == NULL) continue;
			
			if (return_data) {
				temp = datasurf;
				datasurf = rotozoomSurfaceXY(temp, a->angle, 1, 1, 0);
				SDL_FreeSurface(temp);
				if (datasurf == NULL) continue;
			}
		}
		
		dest.x = a->x;
		dest.y = a->y;
		
		if (return_data) {
			// Data surface
			cross_mask(datasurf, areasurf);
			SDL_BlitSurface(datasurf, NULL, surf, &dest);
			SDL_FreeSurface(datasurf);
			
		} else {
			// Video surface
			SDL_BlitSurface(areasurf, NULL, surf, &dest);
		}
		
		SDL_FreeSurface(areasurf);
	}
	
	return surf;
}


SDL_Surface *createDataSurface(int w, int h, Uint32 initial_data)
{
	SDL_Surface *surf = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 32, 0,0,0,0);
	
	SDL_FillRect(surf, NULL, initial_data);
	
	return surf;
}


