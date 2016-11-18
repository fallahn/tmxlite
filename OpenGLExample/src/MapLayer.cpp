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

#include <MapLayer.hpp>
#include <OpenGL.hpp>
#include <GLCheck.hpp>

#include <tmxlite/Map.hpp>

MapLayer::MapLayer(const tmx::Map& map, int layer, const std::vector<unsigned>& textures)
  : m_tilesetTextures   (textures),
  m_vbo                 (0)
{
    createResources(map);
}


MapLayer::~MapLayer()
{
    if(m_vbo)
    {
        glCheck(glDeleteBuffers(1, &m_vbo));
    }
}

//public
void ::MapLayer::draw()
{
    glCheck(glActiveTexture(GL_TEXTURE0));
    glCheck(glBindTexture(GL_TEXTURE_2D, m_tilesetTextures[0]));
    
    glCheck(glEnableVertexAttribArray(0));
    glCheck(glEnableVertexAttribArray(1));
    glCheck(glBindBuffer(GL_ARRAY_BUFFER, m_vbo));
    glCheck(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0));
    glCheck(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))));
    glCheck(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
    glCheck(glDisableVertexAttribArray(0));
    glCheck(glDisableVertexAttribArray(1));
}

//private
void MapLayer::createResources(const tmx::Map& map)
{
    //TODO we want to check how many tile sets are used and create a quad for each
    auto bounds = map.getBounds();
    float verts[] = 
    {
      bounds.left, bounds.top, 0.f, 0.f, 0.f,
      bounds.left + bounds.width, bounds.top, 0.f, 1.f, 0.f,
      bounds.left, bounds.top + bounds.height, 0.f, 0.f, 1.f,
      bounds.left + bounds.width, bounds.top + bounds.height, 0.f, 1.f, 1.f
      //0.f, 0.f, 0.f,
      //0.f, 1.f, 0.f,
      //1.f, 0.f, 0.f,
      //1.f, 1.f, 0.f
    };
    
    glCheck(glGenBuffers(1, &m_vbo));
    glCheck(glBindBuffer(GL_ARRAY_BUFFER, m_vbo));
    glCheck(glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW));
    
}
