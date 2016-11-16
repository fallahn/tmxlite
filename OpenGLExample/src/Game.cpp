/*********************************************************************
Matt Marchant 2016
http://trederia.blogspot.com

tmxlite - Zlib license.

This software is provided 'as-is', without any express or
implied warranty. In no event will the authors be held
liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.

2. Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any
source distribution.
*********************************************************************/

#include <Game.hpp>
#include <GLExtensions.hpp>
#include <GLCheck.hpp>

#include <cassert>
#include <array>

namespace
{
    bool running = true;
}


Game::Game()
{
    
}

Game::~Game()
{
    
}

//public
void Game::run(SDL_Window* window)
{
    assert(window);
    
    if(ogl_LoadFunctions() == ogl_LOAD_FAILED)
    {
        std::cout << "Failed to load OpenGL functions" << std::endl;
        return;
    }
    
    while(running)
    {
        doEvents();
        update(0.f);
        draw(window);
    }
}

//private
void Game::doEvents()
{
    SDL_Event evt;
    while(SDL_PollEvent(&evt))
    {
        if(evt.type == SDL_QUIT) running = false;
        
        if(evt.type == SDL_KEYUP)
        {
            switch(evt.key.keysym.sym)
            {
            default:break;
            case SDLK_ESCAPE: running = false; break;
            case SDLK_r: 
                glCheck(glClearColor(1.f, 0.f, 0.f, 1.f));
                break;
            case SDLK_g: 
                glCheck(glClearColor(0.f, 1.f, 0.f, 1.f));
                break;
            case SDLK_b: 
                glCheck(glClearColor(0.f, 0.f, 1.f, 1.f));
                break;                                
            }
        }
    }
}

void Game::update(float)
{
    
}

void Game::draw(SDL_Window* window)
{
    glCheck(glClear(GL_COLOR_BUFFER_BIT));
    SDL_GL_SwapWindow(window);
}
