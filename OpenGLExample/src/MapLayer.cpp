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
#include <tmxlite/TileLayer.hpp>

MapLayer::MapLayer(const tmx::Map& map, std::size_t layerIdx, const std::vector<unsigned>& textures)
  : m_tilesetTextures   (textures)
{
    createSubsets(map, layerIdx);
}


MapLayer::~MapLayer()
{
    for(auto& ss : m_subsets)
    {
        if(ss.vbo)
        {
            glCheck(glDeleteBuffers(1, &ss.vbo));
        }
        if(ss.lookup)
        {
            glCheck(glDeleteTextures(1, &ss.lookup));
        }
        //don't delete the tileset textures as these are
        //shared and deleted elsewhere
    }
}

//public
void ::MapLayer::draw()
{    
    glCheck(glEnableVertexAttribArray(0));
    glCheck(glEnableVertexAttribArray(1));
    glCheck(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0));
    glCheck(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))));
    
    for(const auto& ss : m_subsets)
    {
        glCheck(glActiveTexture(GL_TEXTURE0));
        glCheck(glBindTexture(GL_TEXTURE_2D, ss.texture));
        
        glCheck(glActiveTexture(GL_TEXTURE1));
        glCheck(glBindTexture(GL_TEXTURE_2D, ss.lookup));
        
        glCheck(glBindBuffer(GL_ARRAY_BUFFER, ss.vbo));
        glCheck(glDrawArrays(GL_TRIANGLE_STRIP, 0, 4));
    }

    glCheck(glDisableVertexAttribArray(0));
    glCheck(glDisableVertexAttribArray(1));
}

//private
void MapLayer::createSubsets(const tmx::Map& map, std::size_t layerIdx)
{
    const auto& layers = map.getLayers();
    if(layerIdx >= layers.size() || (layers[layerIdx]->getType() != tmx::Layer::Type::Tile))
    {
        std::cout << "Invalid layer index or layer type, layer will be empty" << std::endl;
        return;
    }
    const auto layer = dynamic_cast<const tmx::TileLayer*>(layers[layerIdx].get());
    
    auto bounds = map.getBounds();
    float verts[] = 
    {
      bounds.left, bounds.top, 0.f, 0.f, 0.f,
      bounds.left + bounds.width, bounds.top, 0.f, 1.f, 0.f,
      bounds.left, bounds.top + bounds.height, 0.f, 0.f, 1.f,
      bounds.left + bounds.width, bounds.top + bounds.height, 0.f, 1.f, 1.f
    };
    
    const auto& mapSize = map.getTileCount();
    const auto& tilesets = map.getTilesets();
    for(auto i = 0u; i < tilesets.size(); ++i)
    {        
        //check each tile ID to see if it falls in the current tile set
        const auto& ts = tilesets[i];
        const auto& tileIDs = layer->getTiles();
        std::vector<std::uint16_t> pixelData;
        bool tsUsed = false;
        
        for(auto y = 0u; y < mapSize.y; ++y)
        {
            for(auto x = 0u; x < mapSize.x; ++x)
            {
                auto idx = y * mapSize.x + x;
                if(idx < tileIDs.size() && tileIDs[idx].ID >= ts.getFirstGID()
                    && tileIDs[idx].ID < (ts.getFirstGID() + ts.getTileCount()))
                {
                    pixelData.push_back(static_cast<std::uint16_t>((tileIDs[idx].ID - ts.getFirstGID()) + 1)); //red channel - making sure to index relative to the tileset
                    pixelData.push_back(static_cast<std::uint16_t>(tileIDs[idx].flipFlags)); //green channel - tile flips are performed on the shader
                    tsUsed = true;
                }
                else
                {
                    //pad with empty space
                    pixelData.push_back(0);
                    pixelData.push_back(0);
                }
            }
        }
        
        //if we have some data for this tile set, create the resources
        if(tsUsed)
        {
            m_subsets.emplace_back();
            m_subsets.back().texture = m_tilesetTextures[i];
     
            glCheck(glGenBuffers(1, &m_subsets.back().vbo));
            glCheck(glBindBuffer(GL_ARRAY_BUFFER, m_subsets.back().vbo));
            glCheck(glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW));
            
            glCheck(glGenTextures(1, &m_subsets.back().lookup));
            glCheck(glBindTexture(GL_TEXTURE_2D, m_subsets.back().lookup));
            glCheck(glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16UI, mapSize.x, mapSize.y, 0, GL_RG_INTEGER, GL_UNSIGNED_SHORT, (void*)pixelData.data()));

            glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
            glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
            glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
            glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
        }
    }    
}
