// This file is part of Chaotic Rage (c) 2010 Josh Heidenreich
//
// kate: tab-width 4; indent-width 4; space-indent off; word-wrap off;

#pragma once
#include <iostream>
#include <SDL.h>
#include <guichan.hpp>
#include "../rage.h"

using namespace std;



class VectorListModel: public gcn::ListModel
{
	private:
		vector<string> * v;
		
	public:
		VectorListModel(vector<string> * vec) {
			this->v = vec;
		}
		
		std::string getElementAt(int i)
		{
			return v->at(i);
		}
		
		int getNumberOfElements()
		{
			return v->size();
		}
};


class MapRegistryListModel: public gcn::ListModel
{
	private:
		MapRegistry * mapreg;
		
	public:
		MapRegistryListModel(MapRegistry * mapreg) : mapreg(mapreg) {}
		
		std::string getElementAt(int i)
		{
			return mapreg->titleAt(i);
		}
		
		int getNumberOfElements()
		{
			return mapreg->size();
		}
};


class GametypeListModel: public gcn::ListModel
{
	private:
		vector<GameType*> * gametypes;
		
	public:
		GametypeListModel(vector<GameType*> * gametypes) : gametypes(gametypes) {}
		~GametypeListModel() { delete(gametypes); }
		
		std::string getElementAt(int i)
		{
			return gametypes->at(i)->title;
		}
		
		int getNumberOfElements()
		{
			return gametypes->size();
		}
};


/**
* Base dialog class
**/
class Dialog {
	friend class Menu;
	
	protected:
		gcn::Container * c;
		Menu * m;
		
	public:
		virtual gcn::Container * setup() = 0;
		virtual string getName() { return "?"; }
		gcn::Container * getContainer() { return this->c; };
};


/**
* Dialog complaining about not being implemented yet.
**/
class DialogNull : public Dialog {
	public:
		virtual gcn::Container * setup();
};


/**
* Asks if the user wants to quit.
**/
class DialogQuit : public Dialog, public gcn::ActionListener {
	private:
		GameState * st;
		
	public:
		DialogQuit(GameState * st);
		virtual gcn::Container * setup();
		virtual string getName() { return "quit"; }
		virtual void action(const gcn::ActionEvent& actionEvent);
};

