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

/*
Creates an SFML drawable from an Orthogonal tmx map layer.
This is an example of drawing with SFML - not all features
are implemented.
*/

#ifndef SFML_ORTHO_HPP_
#define SFML_ORTHO_HPP_

#include <tmxlite/Map.hpp>
#include <tmxlite/TileLayer.hpp>
#include <tmxlite/detail/Log.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Transformable.hpp>

#include <memory>
#include <vector>
#include <array>
#include <map>
#include <string>
#include <limits>
#include <iostream>
#include <cmath>


class MapLayer final : public sf::Drawable
{
public:


    MapLayer(const tmx::Map& map, std::size_t idx)
    {
        const auto& layers = map.getLayers();
        if (map.getOrientation() == tmx::Orientation::Orthogonal && 
            idx < layers.size() && layers[idx]->getType() == tmx::Layer::Type::Tile)
        {
            //round the chunk size to the nearest tile
            const auto tileSize = map.getTileSize();
            m_chunkSize.x = std::floor(m_chunkSize.x / tileSize.x) * tileSize.x;
            m_chunkSize.y = std::floor(m_chunkSize.y / tileSize.y) * tileSize.y;

            const auto& layer = *dynamic_cast<const tmx::TileLayer*>(layers[idx].get());
            createChunks(map, layer);

            auto mapSize = map.getBounds();
            m_globalBounds.width = mapSize.width;
            m_globalBounds.height = mapSize.height;
        }
        else
        {
            std::cout << "Not a valid orthogonal layer, nothing will be drawn." << std::endl;
        }
    }

    ~MapLayer() = default;
    MapLayer(const MapLayer&) = delete;
    MapLayer& operator = (const MapLayer&) = delete;

    const sf::FloatRect& getGlobalBounds() const { return m_globalBounds; }

private:
	//increasing m_chunkSize by 4; fixes render problems when mapsize != chunksize
	//sf::Vector2f m_chunkSize = sf::Vector2f(1024.f, 1024.f);
    sf::Vector2f m_chunkSize = sf::Vector2f(4096.f, 4096.f);
    sf::Vector2u m_chunkCount;
    sf::FloatRect m_globalBounds;

    using TextureResource = std::map<std::string, std::unique_ptr<sf::Texture>>;
    TextureResource m_textureResource;

    class Chunk final : public sf::Transformable, public sf::Drawable 
    {
    public:
        using Ptr = std::unique_ptr<Chunk>;
		
		// the Android OpenGL driver isn't capable of rendering quads,
		// so we need to use two triangles per tile instead
#ifdef __ANDROID__        
		using Tile = std::array<sf::Vertex, 6u>;
#endif		
#ifndef __ANDROID__
		using Tile = std::array<sf::Vertex, 4u>;
#endif		
        Chunk(const tmx::TileLayer& layer, std::vector<const tmx::Tileset*> tilesets,
            const sf::Vector2f& position, const sf::Vector2f& tileCount, std::size_t rowSize,  TextureResource& tr)
        {
            auto opacity = static_cast<sf::Uint8>(layer.getOpacity() /  1.f * 255.f);
            sf::Color vertColour = sf::Color::White;
            vertColour.a = opacity;

            auto offset = layer.getOffset();
            sf::Vector2f layerOffset(offset.x, offset.y);

            const auto& tileIDs = layer.getTiles();
            
            //go through the tiles and create the appropriate arrays
            for (const auto ts : tilesets)
            {
                if(ts->getImagePath().empty())
                {
                    tmx::Logger::log("This example does not support Collection of Images tilesets", tmx::Logger::Type::Info);
                    tmx::Logger::log("Chunks using " + ts->getName() + " will not be created", tmx::Logger::Type::Info);
                    continue;
                }

                bool chunkArrayCreated = false;
                auto tileSize = ts->getTileSize();

                sf::Vector2u tsTileCount;

                std::size_t xPos = static_cast<std::size_t>(position.x / tileSize.x);
                std::size_t yPos = static_cast<std::size_t>(position.y / tileSize.y);

                for (auto y = yPos; y < yPos + tileCount.y; ++y)
                {
                    for (auto x = xPos; x < xPos + tileCount.x; ++x)
                    {
                        auto idx = (y * rowSize + x);
                        if (idx < tileIDs.size() && tileIDs[idx].ID >= ts->getFirstGID()
                            && tileIDs[idx].ID < (ts->getFirstGID() + ts->getTileCount()))
                        {
                            //ID must belong to this set - so add a tile
                            if (!chunkArrayCreated)
                            {
								// replace all usages of make_unique with unique_ptr,
								// to be able to compile with C++11 (make_unique is the only function of c++14 that is used)
                                
								//m_chunkArrays.emplace_back(std::make_unique<ChunkArray>(*tr.find(ts->getImagePath())->second));
								m_chunkArrays.emplace_back(std::unique_ptr<ChunkArray>(new ChunkArray(*tr.find(ts->getImagePath())->second)));
                                auto texSize = m_chunkArrays.back()->getTextureSize();
                                tsTileCount.x = texSize.x / tileSize.x;
                                tsTileCount.y = texSize.y / tileSize.y;
                                chunkArrayCreated = true;
                            }
                            auto& ca = m_chunkArrays.back();
                            sf::Vector2f tileOffset(x * tileSize.x, y * tileSize.y);
                            
                            auto idIndex = tileIDs[idx].ID - ts->getFirstGID();
                            sf::Vector2f tileIndex(idIndex % tsTileCount.x, idIndex / tsTileCount.x);
                            tileIndex.x *= tileSize.x;
                            tileIndex.y *= tileSize.y;
                            Tile tile = 
                            {
#ifndef __ANDROID__								
                                sf::Vertex(tileOffset, vertColour, tileIndex),
                                sf::Vertex(tileOffset + sf::Vector2f(tileSize.x, 0.f), vertColour, tileIndex + sf::Vector2f(tileSize.x, 0.f)),
                                sf::Vertex(tileOffset + sf::Vector2f(tileSize.x, tileSize.y), vertColour, tileIndex + sf::Vector2f(tileSize.x, tileSize.y)),
                                sf::Vertex(tileOffset + sf::Vector2f(0.f, tileSize.y), vertColour, tileIndex + sf::Vector2f(0.f, tileSize.y))
#endif
#ifdef __ANDROID__								
								sf::Vertex(tileOffset, vertColour, tileIndex),
								sf::Vertex(tileOffset + sf::Vector2f(tileSize.x, 0.f), vertColour, tileIndex + sf::Vector2f(tileSize.x, 0.f)),
								sf::Vertex(tileOffset + sf::Vector2f(tileSize.x, tileSize.y), vertColour, tileIndex + sf::Vector2f(tileSize.x, tileSize.y)),
								sf::Vertex(tileOffset, vertColour, tileIndex),
								sf::Vertex(tileOffset + sf::Vector2f(0.f, tileSize.y), vertColour, tileIndex + sf::Vector2f(0.f, tileSize.y)),
								sf::Vertex(tileOffset + sf::Vector2f(tileSize.x, tileSize.y), vertColour, tileIndex + sf::Vector2f(tileSize.x, tileSize.y))
#endif
                            };
                            doFlips(tileIDs[idx].flipFlags,&tile[0].texCoords,&tile[1].texCoords,&tile[2].texCoords,&tile[3].texCoords);
                            ca->addTile(tile);
                        }
                    }
                }
            }
        }
        ~Chunk() = default;
        Chunk(const Chunk&) = delete;
        Chunk& operator = (const Chunk&) = delete;

        bool empty() const { return m_chunkArrays.empty(); }
        void flipY(sf::Vector2f *v0, sf::Vector2f *v1, sf::Vector2f *v2, sf::Vector2f *v3)
        {
            //Flip Y
            sf::Vector2f tmp = *v0;
            v0->y = v2->y;
            v1->y = v2->y;
            v2->y = tmp.y ;
            v3->y = v2->y  ;
        }

        void flipX(sf::Vector2f *v0, sf::Vector2f *v1, sf::Vector2f *v2, sf::Vector2f *v3)
        {
            //Flip X
            sf::Vector2f tmp = *v0;
            v0->x = v1->x;
            v1->x = tmp.x;
            v2->x = v3->x;
            v3->x = v0->x ;
        }

        void flipD(sf::Vector2f *v0, sf::Vector2f *v1, sf::Vector2f *v2, sf::Vector2f *v3)
        {
            //Diagonal flip
            sf::Vector2f tmp = *v1;
            v1->x = v3->x;
            v1->y = v3->y;
            v3->x = tmp.x;
            v3->y = tmp.y;
        }

        void doFlips(std::uint8_t bits, sf::Vector2f *v0, sf::Vector2f *v1, sf::Vector2f *v2, sf::Vector2f *v3)
        {
            //0000 = no change
            //0100 = vertical = swap y axis
            //1000 = horizontal = swap x axis
            //1100 = horiz + vert = swap both axes = horiz+vert = rotate 180 degrees
            //0010 = diag = rotate 90 degrees right and swap x axis
            //0110 = diag+vert = rotate 270 degrees right
            //1010 = horiz+diag = rotate 90 degrees right
            //1110 = horiz+vert+diag = rotate 90 degrees right and swap y axis
            if(!(bits & tmx::TileLayer::FlipFlag::Horizontal) &&
               !(bits & tmx::TileLayer::FlipFlag::Vertical) &&
               !(bits & tmx::TileLayer::FlipFlag::Diagonal) )
            {
                //Shortcircuit tests for nothing to do
                return;
            }
            else if(!(bits & tmx::TileLayer::FlipFlag::Horizontal) &&
                (bits & tmx::TileLayer::FlipFlag::Vertical) &&
               !(bits & tmx::TileLayer::FlipFlag::Diagonal) )
            {
                //0100
                flipY(v0,v1,v2,v3);
            }
            else if((bits & tmx::TileLayer::FlipFlag::Horizontal) &&
               !(bits & tmx::TileLayer::FlipFlag::Vertical) &&
               !(bits & tmx::TileLayer::FlipFlag::Diagonal) )
            {
                //1000
                flipX(v0,v1,v2,v3);
            }
            else if((bits & tmx::TileLayer::FlipFlag::Horizontal) &&
               (bits & tmx::TileLayer::FlipFlag::Vertical) &&
               !(bits & tmx::TileLayer::FlipFlag::Diagonal) )
            {
                //1100
                flipY(v0,v1,v2,v3);
                flipX(v0,v1,v2,v3);
            }
            else if(!(bits & tmx::TileLayer::FlipFlag::Horizontal) &&
               !(bits & tmx::TileLayer::FlipFlag::Vertical) &&
               (bits & tmx::TileLayer::FlipFlag::Diagonal) )
            {
                //0010
                flipD(v0,v1,v2,v3);
            }
            else if(!(bits & tmx::TileLayer::FlipFlag::Horizontal) &&
                (bits & tmx::TileLayer::FlipFlag::Vertical) &&
                (bits & tmx::TileLayer::FlipFlag::Diagonal) )
            {
                //0110
                flipX(v0,v1,v2,v3);
                flipD(v0,v1,v2,v3);
            }
            else if((bits & tmx::TileLayer::FlipFlag::Horizontal) &&
                !(bits & tmx::TileLayer::FlipFlag::Vertical) &&
                (bits & tmx::TileLayer::FlipFlag::Diagonal) )
            {
                //1010
                flipY(v0,v1,v2,v3);
                flipD(v0,v1,v2,v3);
           }
            else if((bits & tmx::TileLayer::FlipFlag::Horizontal) &&
               (bits & tmx::TileLayer::FlipFlag::Vertical) &&
               (bits & tmx::TileLayer::FlipFlag::Diagonal) )
            {
                //1110
                flipY(v0,v1,v2,v3);
                flipX(v0,v1,v2,v3);
                flipD(v0,v1,v2,v3);
            }
        }
    private:
        class ChunkArray final : public sf::Drawable
        {
        public:
            using Ptr = std::unique_ptr<ChunkArray>;
            explicit ChunkArray(const sf::Texture& t)
                : m_texture(t) {}
            ~ChunkArray() = default;
            ChunkArray(const ChunkArray&) = delete;
            ChunkArray& operator = (const ChunkArray&) = delete;

            void addTile(const Chunk::Tile& tile)
            {
                for (const auto& v : tile)
                {
                    m_vertices.push_back(v);
                }
            }
            sf::Vector2u getTextureSize() const { return m_texture.getSize(); }

        private:
            const sf::Texture& m_texture;
            std::vector<sf::Vertex> m_vertices;
            void draw(sf::RenderTarget& rt, sf::RenderStates states) const override
            {
                states.texture = &m_texture;
#ifndef __ANDROID__
                rt.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);
#endif
#ifdef __ANDROID__				
				rt.draw(m_vertices.data(), m_vertices.size(), sf::Triangles, states);
#endif				
            }
        };

        std::vector<ChunkArray::Ptr> m_chunkArrays;
        void draw(sf::RenderTarget& rt, sf::RenderStates states) const override
        {
            states.transform *= getTransform();
            for (const auto& a : m_chunkArrays)
            {
                rt.draw(*a, states);
            }
        }
    };

    std::vector<Chunk::Ptr> m_chunks;
    mutable std::vector<const Chunk*> m_visibleChunks;
    void createChunks(const tmx::Map& map, const tmx::TileLayer& layer)
    {
        //look up all the tile sets and load the textures
        const auto& tileSets = map.getTilesets();
        const auto& layerIDs = layer.getTiles();
        std::uint32_t maxID = std::numeric_limits<std::uint32_t>::max();
        std::vector<const tmx::Tileset*> usedTileSets;

        for (auto i = tileSets.rbegin(); i != tileSets.rend(); ++i)
        {
            for (const auto& tile : layerIDs)
            {
                if (tile.ID >= i->getFirstGID() && tile.ID < maxID)
                {
                    usedTileSets.push_back(&(*i));
                    break;
                }
            }
            maxID = i->getFirstGID();
        }

        sf::Image fallback;
        fallback.create(2, 2, sf::Color::Magenta);
        for (const auto ts : usedTileSets)
        {
            const auto& path = ts->getImagePath();
            //std::unique_ptr<sf::Texture> newTexture = std::make_unique<sf::Texture>();
			std::unique_ptr<sf::Texture> newTexture = std::unique_ptr<sf::Texture>(new sf::Texture());
            sf::Image img;
            if (!img.loadFromFile(path))
            {
                newTexture->loadFromImage(fallback);
            }
            else
            {
                if (ts->hasTransparency())
                {
                    auto transparency = ts->getTransparencyColour();
                    img.createMaskFromColor({ transparency.r, transparency.g, transparency.b, transparency.a });
                }
                newTexture->loadFromImage(img);
            }
            m_textureResource.insert(std::make_pair(path, std::move(newTexture)));
        }

        //calculate the number of chunks in the layer
        //and create each one
        const auto bounds = map.getBounds();
        m_chunkCount.x = static_cast<sf::Uint32>(std::ceil(bounds.width / m_chunkSize.x));
        m_chunkCount.y = static_cast<sf::Uint32>(std::ceil(bounds.height / m_chunkSize.y));

        sf::Vector2f tileCount(m_chunkSize.x / map.getTileSize().x, m_chunkSize.y / map.getTileSize().y);

        for (auto y = 0u; y < m_chunkCount.y; ++y)
        {
            for (auto x = 0u; x < m_chunkCount.x; ++x)
            {
                // calculate size of each Chunk (clip against map)
                if ((x+1) * m_chunkSize.x > bounds.width)
                {
                    tileCount.x = (bounds.width - x * m_chunkSize.x) /  map.getTileSize().x;
                }
                if ((y+1) * m_chunkSize.y * map.getTileSize().y > bounds.height)
                {
                    tileCount.y = (bounds.height - y * m_chunkSize.y) /  map.getTileSize().y;
                }
                //m_chunks.emplace_back(std::make_unique<Chunk>(layer, usedTileSets, 
                //    sf::Vector2f(x * m_chunkSize.x, y * m_chunkSize.y), tileCount, map.getTileCount().x, m_textureResource));
				m_chunks.emplace_back(std::unique_ptr<Chunk>(new Chunk(layer, usedTileSets, 
                    sf::Vector2f(x * m_chunkSize.x, y * m_chunkSize.y), tileCount, map.getTileCount().x, m_textureResource)));	
            }
        }
    }

    void updateVisibility(const sf::View& view) const
    {
        sf::Vector2f viewCorner = view.getCenter();
        viewCorner -= view.getSize() / 2.f;
        
        int posX = static_cast<int>(std::floor(viewCorner.x / m_chunkSize.x));
        int posY = static_cast<int>(std::floor(viewCorner.y / m_chunkSize.y));
        int posX2 = static_cast<int>(std::ceil((viewCorner.x + view.getSize().x) / m_chunkSize.x));
        int posY2 = static_cast<int>(std::ceil((viewCorner.y + view.getSize().x)/ m_chunkSize.y));

        std::vector<const Chunk*> visible;
        for (auto y = posY; y < posY2; ++y)
        {
            for (auto x = posX; x < posX2; ++x)
            {
                auto idx = y * int(m_chunkCount.x) + x;
                if (idx >= 0u && idx < m_chunks.size() && !m_chunks[idx]->empty())
                {
                    visible.push_back(m_chunks[idx].get());
                }
            }
        }

        std::swap(m_visibleChunks, visible);
    }

    void draw(sf::RenderTarget& rt, sf::RenderStates states) const override
    {
        //calc view coverage and draw nearest chunks
        updateVisibility(rt.getView());
        for (const auto& c : m_visibleChunks)
        {
            rt.draw(*c, states);
        }
    }
};

#endif //SFML_ORTHO_HPP_
