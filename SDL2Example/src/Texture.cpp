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

#include "Texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include <cstdint>

Texture::Texture()
    : m_texture(nullptr)
{

}

Texture::~Texture()
{
    if (m_texture)
    {
        SDL_DestroyTexture(m_texture);
    }
}

//public
bool Texture::loadFromFile(const std::string& path, SDL_Renderer* renderer)
{
    assert(renderer != nullptr);
    assert(!path.empty());

    std::int32_t x = 0;
    std::int32_t y = 0;
    std::int32_t c = 0;
    unsigned char* data = stbi_load(path.c_str(), &x, &y, &c, 0);

    if (data)
    {
        std::int32_t pitch = x * c;
        pitch = (pitch + 3) & ~3;

        constexpr std::int32_t rmask = 0x000000ff;
        constexpr std::int32_t gmask = 0x0000ff00;
        constexpr std::int32_t bmask = 0x00ff0000;
        const std::int32_t amask = c == 4 ? 0xff000000 : 0;

        auto* surface = SDL_CreateRGBSurfaceFrom(data, x, y, c * 8, pitch, rmask, gmask, bmask, amask);

        if (!surface)
        {
            std::cerr << "Unable to create texture surface: " << SDL_GetError() << "\n";
            stbi_image_free(data);
            return false;
        }

        m_texture = SDL_CreateTextureFromSurface(renderer, surface);

        SDL_FreeSurface(surface);
        stbi_image_free(data);

        if (!m_texture)
        {
            std::cerr << "Failed to create texture for " << path << "\n";
            return false;
        }

        //sets alpha blending
        //SDL_SetTextureBlendMode(m_texture, SDL_BLENDMODE_BLEND);
        m_size.x = x;
        m_size.y = y;

        return true;
    }

    return false;
}
