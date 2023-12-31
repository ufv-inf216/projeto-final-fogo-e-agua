//
// Created by Lucas N. Ferreira on 07/12/23.
//

#pragma once

#include <SDL_stdinc.h>
#include "../Math.h"

class Scene
{
public:
    Scene(class Game* game);

    virtual void Load();
    virtual void ProcessInput(const Uint8* keyState);

    class Game* GetGame() { return mGame; }

protected:
    class Game* mGame;
};
