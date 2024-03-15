/*********************************************************************
(c) Matt Marchant 2024
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

/*
Example code for rendering a map layer with SDL2
*/

#include <SDL.h>

#include <iostream>

int main(int, char**)
{
    SDL_Window* window = nullptr;
    SDL_Surface* screenSurface = nullptr;

    //init SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << "\n";
    }
    else
    {
        //create window
        window = SDL_CreateWindow("SDL2 Example", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN);
        if (window == nullptr)
        {
            std::cout << "Window could not be created! SDL_Error: " << SDL_GetError() << "\n";
        }
        else
        {
            screenSurface = SDL_GetWindowSurface(window);
            SDL_FillRect(screenSurface, nullptr, SDL_MapRGB(screenSurface->format, 0xFF, 0xFF, 0xFF));
            SDL_UpdateWindowSurface(window);

            //pump events
            SDL_Event evt;
            bool running = true;
            while (running)
            {
                while (SDL_PollEvent(&evt)) 
                {
                    if (evt.type == SDL_QUIT)
                    {
                        running = false;
                    }
                }
            }
        }
    }

    //tidy up
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}