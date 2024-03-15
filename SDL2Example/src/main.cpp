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

#include "Texture.hpp"
#include "MapLayer.hpp"

#include <SDL.h>
#include <tmxlite/Map.hpp>

#include <iostream>

int main(int, char**)
{
    SDL_Window* window = nullptr;

    //init SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "Failed to initialise SDL: " << SDL_GetError() << "\n";
    }
    else
    {
        //create window
        window = SDL_CreateWindow("SDL2 Example", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_SHOWN);
        if (window == nullptr)
        {
            std::cerr << "Window could not be created: " << SDL_GetError() << "\n";
        }
        else
        {
            SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

            if (renderer)
            {
                std::vector<std::unique_ptr<Texture>> textures;
                std::vector<std::unique_ptr<MapLayer>> renderLayers;

                //load the tile map
                tmx::Map map;
                if (map.load("assets/demo.tmx"))
                {
                    //load the textures as they're shared between layers
                    const auto& tileSets = map.getTilesets();
                    assert(!tileSets.empty());
                    for (const auto& ts : tileSets)
                    {
                        textures.emplace_back(std::make_unique<Texture>());
                        if (!textures.back()->loadFromFile(ts.getImagePath(), renderer))
                        {
                            std::cerr << "Failed opening " << ts.getImagePath() << "\n";
                        }
                    }

                    //load the layers
                    const auto& mapLayers = map.getLayers();
                    for (auto i = 0u; i < mapLayers.size(); ++i)
                    {
                        if (mapLayers[i]->getType() == tmx::Layer::Type::Tile)
                        {
                            renderLayers.emplace_back(std::make_unique<MapLayer>());
                            renderLayers.back()->create(map, i, textures); //just cos we're using C++14
                        }
                    }
                }


                //enter loop...
                SDL_SetRenderDrawColor(renderer, 100, 149, 237, 255);

                bool running = true;
                while (running)
                {
                    //pump events
                    SDL_Event evt;
                    while (SDL_PollEvent(&evt))
                    {
                        if (evt.type == SDL_QUIT)
                        {
                            running = false;
                        }
                        else if (evt.type == SDL_KEYDOWN)
                        {
                            switch (evt.key.keysym.sym)
                            {
                            default: break;
                            case SDLK_ESCAPE:
                                running = false;
                                break;
                            }
                        }
                    }

                    //clear/draw/display
                    SDL_RenderClear(renderer);
                    for (const auto& l : renderLayers)
                    {
                        l->draw(renderer);
                    }
                    SDL_RenderPresent(renderer);
                }

                SDL_DestroyRenderer(renderer);
            }
        }
    }

    //tidy up
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}