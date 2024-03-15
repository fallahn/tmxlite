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

#include "MapLayer.hpp"

#include <iostream>
#include <cassert>

MapLayer::MapLayer()
{

}

bool MapLayer::create(SDL_Renderer* renderer, const tmx::Layer::Ptr& layer)
{
    assert(layer->getType() == tmx::Layer::Type::Tile);

    //normally we would use some resource management for textures - otherwise this 
    //may load the same instance of an image multiple times (once for each layer)
    if (!m_texture.loadFromFile("assets/images/tilemap/tileset.png", renderer))
    {
        return false;
    }

    //normally we load this from the layer properties - using white here for brevity
    SDL_Color c = { 255,255,255,255 };
    m_vertexData =
    {
        {{0.f,     0.f}, c, {0.f, 0.f}},
        {{384.f,   0.f}, c, {1.f, 0.f}},
        {{0.f,   448.f}, c, {0.f, 1.f}},

        {{0.f,   448.f}, c, {0.f, 1.f}},
        {{384.f,   0.f}, c, {1.f, 0.f}},
        {{384.f, 448.f}, c, {1.f, 1.f}},
    };

    return true;
}

void MapLayer::draw(SDL_Renderer* renderer) const
{
    assert(renderer);
    SDL_RenderGeometry(renderer, m_texture, m_vertexData.data(), static_cast<std::int32_t>(m_vertexData.size()), nullptr, 0);
}