// This file is part of Chaotic Rage (c) 2010 Josh Heidenreich
//
// kate: tab-width 4; indent-width 4; space-indent off; word-wrap off;

#pragma once
#include <string>
#include <SDL2/SDL.h>

class Cmd
{
    public:
        Cmd(std::string x): x(x) {
            lock = SDL_CreateMutex();
            complete = SDL_CreateCond();
        };

        void exec() {
            SDL_LockMutex(lock);
            this->resp = this->doWork();
            SDL_CondBroadcast(complete);
            SDL_UnlockMutex(lock);
        };

        std::string doWork() {
            return "url is " + x;
        };

        std::string waitDone() {
            SDL_LockMutex(lock);
            SDL_CondWait(complete, lock);
            SDL_UnlockMutex(lock);
            return this->resp;
        };

    private:
        SDL_mutex* lock;
        SDL_cond* complete;
        std::string resp;

        std::string x;
};
