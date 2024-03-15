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

#pragma once

/*
Encapsulates SDL_Texture with image loading from stb_image
*/

#include <SDL.h>

#include <string>

class Texture final
{
public:
    Texture();
    ~Texture();

    Texture(const Texture&) = delete;
    Texture(Texture&&) = delete;

    Texture& operator = (const Texture&) = delete;
    Texture& operator = (Texture&&) = delete;

    bool loadFromFile(const std::string&, SDL_Renderer*);
    SDL_Point getSize() const { return m_size; }

    operator SDL_Texture* () { return m_texture; }

private:

    SDL_Texture* m_texture;
    SDL_Point m_size;
};