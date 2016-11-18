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

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <iostream>

namespace
{
    const int WIDTH = 800;
    const int HEIGHT = 600;
}

int main()
{
    SDL_Window* window = nullptr;
    SDL_GLContext context = nullptr;
    
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cout << "Failed to init SDL!" << std::endl;
    }
    else
    {
        window = SDL_CreateWindow("SDL Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WIDTH, HEIGHT, SDL_WINDOW_OPENGL);
        if(!window)
        {
            std::cout << "Failed to create window *sadface*" << std::endl;
        }
        else
        {
			context = SDL_GL_CreateContext(window);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
			
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
			
			SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
			
			SDL_GL_SetSwapInterval(1); //vsync
			
			int value = 0;
			SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &value);
			std::cout << "Got context with OpenGL Version: " << value;
			SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &value);
			std::cout << "." << value << std::endl;
			
            IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG);
            
			Game game;
			game.run(window);
		}
    }
    
    IMG_Quit();
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    
    
    return 0;
}
