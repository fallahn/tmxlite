/*********************************************************************
(c) Matt Marchant & contributors 2016 - 2024
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

#include <SFML/Graphics.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>

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
        if (map.getOrientation() != tmx::Orientation::Orthogonal)
        {
            std::cout << "Map is not orthogonal - nothing will be drawn" << std::endl;
        }
        else if (idx >= layers.size())
        {
            std::cout << "Layer index " << idx << " is out of range, layer count is " << layers.size() << std::endl;
        }
        else if (layers[idx]->getType() != tmx::Layer::Type::Tile)
        {
            std::cout << "layer " << idx << " is not a Tile layer..." << std::endl;
        }

        else
        {
            //round the chunk size to the nearest tile
            const auto tileSize = map.getTileSize();
            m_chunkSize.x = std::floor(m_chunkSize.x / tileSize.x) * tileSize.x;
            m_chunkSize.y = std::floor(m_chunkSize.y / tileSize.y) * tileSize.y;
            m_mapTileSize.x = map.getTileSize().x;
            m_mapTileSize.y = map.getTileSize().y;
            const auto& layer = layers[idx]->getLayerAs<tmx::TileLayer>();
            createChunks(map, layer);

            auto mapSize = map.getBounds();
            m_globalBounds.size.x = mapSize.width;
            m_globalBounds.size.y = mapSize.height;
        }
    }

    ~MapLayer() = default;
    MapLayer(const MapLayer&) = delete;
    MapLayer& operator = (const MapLayer&) = delete;

    const sf::FloatRect& getGlobalBounds() const { return m_globalBounds; }

    void setTile(std::int32_t tileX, std::int32_t tileY, tmx::TileLayer::Tile tile, bool refresh = true)
    {
        sf::Vector2u chunkLocale;
        const auto& selectedChunk = getChunkAndTransform(tileX, tileY, chunkLocale);
        selectedChunk->setTile(chunkLocale.x, chunkLocale.y, tile, refresh);
    }

    tmx::TileLayer::Tile getTile(std::int32_t tileX, std::int32_t tileY)
    {
        sf::Vector2u chunkLocale;
        const auto& selectedChunk = getChunkAndTransform(tileX, tileY, chunkLocale);
        return selectedChunk->getTile(chunkLocale.x, chunkLocale.y);
    }
    void setColor(std::int32_t tileX, std::int32_t tileY, sf::Color color, bool refresh = true)
    {
        sf::Vector2u chunkLocale;
        const auto& selectedChunk = getChunkAndTransform(tileX, tileY, chunkLocale);
        selectedChunk->setColor(chunkLocale.x, chunkLocale.y, color, refresh);
    }

    sf::Color getColor(std::int32_t tileX, std::int32_t tileY)
    {
        sf::Vector2u chunkLocale;
        const auto& selectedChunk = getChunkAndTransform(tileX, tileY, chunkLocale);
        return selectedChunk->getColor(chunkLocale.x, chunkLocale.y);
    }

    void setOffset(sf::Vector2f offset) { m_offset = offset; }
    sf::Vector2f getOffset() const { return m_offset; }

    void update(sf::Time elapsed)
    {
        for (auto& c : m_visibleChunks)
        {
            for (AnimationState& as : c->getActiveAnimations())
            {
                as.currentTime += elapsed;

                tmx::TileLayer::Tile tile;
                std::int32_t animTime = 0;
                auto x = as.animTile.animation.frames.begin();
                while (animTime < as.currentTime.asMilliseconds())
                {
                    if (x == as.animTile.animation.frames.end())
                    {
                        x = as.animTile.animation.frames.begin();
                        as.currentTime -= sf::milliseconds(animTime);
                        animTime = 0;
                    }

                    tile.ID = x->tileID;
                    animTime += x->duration;
                    x++;
                }

                setTile(as.tileCords.x, as.tileCords.y, tile);
            }
        }
    }

private:
    //increasing m_chunkSize by 4; fixes render problems when mapsize != chunksize
    //sf::Vector2f m_chunkSize = sf::Vector2f(1024.f, 1024.f);
    sf::Vector2f m_chunkSize = sf::Vector2f(512.f, 512.f);
    sf::Vector2u m_chunkCount;
    sf::Vector2u m_mapTileSize;   // general Tilesize of Map
    sf::FloatRect m_globalBounds;
    sf::Vector2f m_offset;

    using TextureResource = std::map<std::string, std::unique_ptr<sf::Texture>>;
    TextureResource m_textureResource;

    struct AnimationState
    {
        sf::Vector2u tileCords;
        sf::Time startTime;
        sf::Time currentTime;
        tmx::Tileset::Tile animTile;
        std::uint8_t flipFlags;
    };

    class Chunk final : public sf::Transformable, public sf::Drawable
    {
    public:
        using Ptr = std::unique_ptr<Chunk>;
        using Tile = std::array<sf::Vertex, 6u>;

        Chunk(const tmx::TileLayer& layer, std::vector<const tmx::Tileset*> tilesets,
            const sf::Vector2f& position, const sf::Vector2f& tileCount,
            const sf::Vector2u& tileSize, std::size_t rowSize,
            TextureResource& tr, const std::map<std::uint32_t, tmx::Tileset::Tile>& animTiles)
            : m_animTiles(animTiles)
        {
            setPosition(position);
            layerOpacity = static_cast<std::uint8_t>(layer.getOpacity() / 1.f * 255.f);

            sf::Color vertColour = sf::Color(200, 200, 200, layerOpacity);
            auto offset = layer.getOffset();
            layerOffset = { static_cast<float>(offset.x), static_cast<float>(offset.y) };
            chunkTileCount = { tileCount.x, tileCount.y };
            mapTileSize = tileSize;

            const auto& tileIDs = layer.getTiles();

            //go through the tiles and create all arrays (for latter manipulation)
            for (const auto& ts : tilesets)
            {
                if (ts->getImagePath().empty())
                {
                    tmx::Logger::log("This example does not support Collection of Images tilesets", tmx::Logger::Type::Info);
                    tmx::Logger::log("Chunks using " + ts->getName() + " will not be created", tmx::Logger::Type::Info);
                    continue;
                }
                m_chunkArrays.emplace_back(std::make_unique<ChunkArray>(*tr.find(ts->getImagePath())->second, *ts));
            }
            std::size_t xPos = static_cast<std::size_t>(position.x / tileSize.x);
            std::size_t yPos = static_cast<std::size_t>(position.y / tileSize.y);
            for (auto y = yPos; y < yPos + tileCount.y; ++y)
            {
                for (auto x = xPos; x < xPos + tileCount.x; ++x)
                {
                    auto idx = (y * rowSize + x);
                    m_chunkTileIDs.emplace_back(tileIDs[idx]);
                    m_chunkColors.emplace_back(vertColour);
                }
            }
            generateTiles(true);
        }

        void generateTiles(bool registerAnimation = false)
        {
            if (registerAnimation)
            {
                m_activeAnimations.clear();
            }
            for (const auto& ca : m_chunkArrays)
            {
                std::uint32_t idx = 0;
                std::uint32_t xPos = static_cast<std::uint32_t>(getPosition().x / mapTileSize.x);
                std::uint32_t yPos = static_cast<std::uint32_t>(getPosition().y / mapTileSize.y);
                for (auto y = yPos; y < yPos + chunkTileCount.y; ++y)
                {
                    for (auto x = xPos; x < xPos + chunkTileCount.x; ++x)
                    {
                        if (idx < m_chunkTileIDs.size() && m_chunkTileIDs[idx].ID >= ca->m_firstGID
                            && m_chunkTileIDs[idx].ID <= ca->m_lastGID)
                        {
                            if (registerAnimation && m_animTiles.find(m_chunkTileIDs[idx].ID) != m_animTiles.end())
                            {
                                AnimationState as;
                                as.animTile = m_animTiles[m_chunkTileIDs[idx].ID];
                                as.startTime = sf::milliseconds(0);
                                as.tileCords = sf::Vector2u(x, y);
                                m_activeAnimations.push_back(as);
                            }

                            sf::Vector2f tileOffset(static_cast<float>(x) * mapTileSize.x, static_cast<float>(y) * mapTileSize.y + mapTileSize.y - ca->tileSetSize.y);

                            auto idIndex = m_chunkTileIDs[idx].ID - ca->m_firstGID;
                            sf::Vector2f tileIndex(sf::Vector2i(idIndex % ca->tsTileCount.x, idIndex / ca->tsTileCount.x));
                            tileIndex.x *= ca->tileSetSize.x;
                            tileIndex.y *= ca->tileSetSize.y;
                            Tile tile =
                            {
                                sf::Vertex{tileOffset - getPosition(), m_chunkColors[idx], tileIndex},
                                sf::Vertex{tileOffset - getPosition() + sf::Vector2f(static_cast<float>(ca->tileSetSize.x), 0.f), m_chunkColors[idx], tileIndex + sf::Vector2f(static_cast<float>(ca->tileSetSize.x), 0.f)},
                                sf::Vertex{tileOffset - getPosition() + sf::Vector2f(sf::Vector2u(ca->tileSetSize.x, ca->tileSetSize.y)), m_chunkColors[idx], tileIndex + sf::Vector2f(sf::Vector2u(ca->tileSetSize.x, ca->tileSetSize.y))},
                                sf::Vertex{tileOffset - getPosition(), m_chunkColors[idx], tileIndex},
                                sf::Vertex{tileOffset - getPosition() + sf::Vector2f(sf::Vector2u(ca->tileSetSize.x, ca->tileSetSize.y)), m_chunkColors[idx], tileIndex + sf::Vector2f(sf::Vector2u(ca->tileSetSize.x, ca->tileSetSize.y))},
                                sf::Vertex{tileOffset - getPosition() + sf::Vector2f(0.f,static_cast<float>(ca->tileSetSize.y)), m_chunkColors[idx], tileIndex + sf::Vector2f(0.f, static_cast<float>(ca->tileSetSize.y))}
                            };
                            doFlips(m_chunkTileIDs[idx].flipFlags, &tile[0].texCoords, &tile[1].texCoords, &tile[2].texCoords, &tile[3].texCoords, &tile[4].texCoords, &tile[5].texCoords);
                            ca->addTile(tile);
                        }
                        idx++;
                    }
                }
            }
        }
        ~Chunk() = default;
        Chunk(const Chunk&) = delete;
        Chunk& operator = (const Chunk&) = delete;

        std::vector<AnimationState>& getActiveAnimations()
        {
            return m_activeAnimations;
        }

        tmx::TileLayer::Tile getTile(std::int32_t x, std::int32_t y) const
        {
            return m_chunkTileIDs[calcIndexFrom(x, y)];
        }

        void setTile(std::int32_t x, std::int32_t y, tmx::TileLayer::Tile tile, bool refresh)
        {
            m_chunkTileIDs[calcIndexFrom(x, y)] = tile;
            maybeRegenerate(refresh);
        }

        sf::Color getColor(std::int32_t x, std::int32_t y) const
        {
            return m_chunkColors[calcIndexFrom(x, y)];
        }

        void setColor(std::int32_t x, std::int32_t y, sf::Color color, bool refresh)
        {
            m_chunkColors[calcIndexFrom(x, y)] = color;
            maybeRegenerate(refresh);
        }

        void maybeRegenerate(bool refresh)
        {
            if (refresh)
            {
                for (const auto& ca : m_chunkArrays)
                {
                    ca->reset();
                }
                generateTiles();
            }
        }

        std::int32_t calcIndexFrom(std::int32_t x, std::int32_t y) const
        {
            return x + y * static_cast<std::int32_t>(chunkTileCount.x);
        }

        bool empty() const
        {
            return m_chunkArrays.empty();
        }

        void flipY(sf::Vector2f* v0, sf::Vector2f* v1, sf::Vector2f* v2, sf::Vector2f* v3, sf::Vector2f* v4, sf::Vector2f* v5)
        {
            //Flip Y
            sf::Vector2f tmp0 = *v0;
            v0->y = v5->y;
            v3->y = v5->y;
            v5->y = tmp0.y;
            sf::Vector2f tmp2 = *v2;
            v2->y = v1->y;
            v4->y = v1->y;
            v1->y = tmp2.y;
        }

        void flipX(sf::Vector2f* v0, sf::Vector2f* v1, sf::Vector2f* v2, sf::Vector2f* v3, sf::Vector2f* v4, sf::Vector2f* v5)
        {
            //Flip X
            sf::Vector2f tmp0 = *v0;
            v0->x = v1->x;
            v3->x = v1->x;
            v1->x = tmp0.x;
            sf::Vector2f tmp2 = *v2;
            v2->x = v5->x;
            v4->x = v5->x;
            v5->x = tmp2.x;
        }

        void flipD(sf::Vector2f* v0, sf::Vector2f* v1, sf::Vector2f* v2, sf::Vector2f* v3, sf::Vector2f* v4, sf::Vector2f* v5)
        {
            //Diagonal flip
            sf::Vector2f tmp2 = *v2;
            *v2 = *v4;
            *v4 = tmp2;
            sf::Vector2f tmp0 = *v0;
            *v0 = *v3;
            *v3 = tmp0;
            sf::Vector2f tmp1 = *v1;
            *v1 = *v5;
            *v5 = tmp1;
        }

        void doFlips(std::uint8_t bits, sf::Vector2f* v0, sf::Vector2f* v1, sf::Vector2f* v2, sf::Vector2f* v3, sf::Vector2f* v4, sf::Vector2f* v5)
        {
            //0000 = no change
            //0100 = vertical = swap y axis
            //1000 = horizontal = swap x axis
            //1100 = horiz + vert = swap both axes = horiz+vert = rotate 180 degrees
            //0010 = diag = rotate 90 degrees right and swap x axis
            //0110 = diag+vert = rotate 270 degrees right
            //1010 = horiz+diag = rotate 90 degrees right
            //1110 = horiz+vert+diag = rotate 90 degrees right and swap y axis
            if (!(bits & tmx::TileLayer::FlipFlag::Horizontal) &&
                !(bits & tmx::TileLayer::FlipFlag::Vertical) &&
                !(bits & tmx::TileLayer::FlipFlag::Diagonal))
            {
                //Shortcircuit tests for nothing to do
                return;
            }
            else if (!(bits & tmx::TileLayer::FlipFlag::Horizontal) &&
                (bits & tmx::TileLayer::FlipFlag::Vertical) &&
                !(bits & tmx::TileLayer::FlipFlag::Diagonal))
            {
                //0100
                flipY(v0, v1, v2, v3, v4, v5);
            }
            else if ((bits & tmx::TileLayer::FlipFlag::Horizontal) &&
                !(bits & tmx::TileLayer::FlipFlag::Vertical) &&
                !(bits & tmx::TileLayer::FlipFlag::Diagonal))
            {
                //1000
                flipX(v0, v1, v2, v3, v4, v5);
            }
            else if ((bits & tmx::TileLayer::FlipFlag::Horizontal) &&
                (bits & tmx::TileLayer::FlipFlag::Vertical) &&
                !(bits & tmx::TileLayer::FlipFlag::Diagonal))
            {
                //1100
                flipY(v0, v1, v2, v3, v4, v5);
                flipX(v0, v1, v2, v3, v4, v5);
            }
            else if (!(bits & tmx::TileLayer::FlipFlag::Horizontal) &&
                !(bits & tmx::TileLayer::FlipFlag::Vertical) &&
                (bits & tmx::TileLayer::FlipFlag::Diagonal))
            {
                //0010
                flipD(v0, v1, v2, v3, v4, v5);
            }
            else if (!(bits & tmx::TileLayer::FlipFlag::Horizontal) &&
                (bits & tmx::TileLayer::FlipFlag::Vertical) &&
                (bits & tmx::TileLayer::FlipFlag::Diagonal))
            {
                //0110
                flipX(v0, v1, v2, v3, v4, v5);
                flipD(v0, v1, v2, v3, v4, v5);
            }
            else if ((bits & tmx::TileLayer::FlipFlag::Horizontal) &&
                !(bits & tmx::TileLayer::FlipFlag::Vertical) &&
                (bits & tmx::TileLayer::FlipFlag::Diagonal))
            {
                //1010
                flipY(v0, v1, v2, v3, v4, v5);
                flipD(v0, v1, v2, v3, v4, v5);
            }
            else if ((bits & tmx::TileLayer::FlipFlag::Horizontal) &&
                (bits & tmx::TileLayer::FlipFlag::Vertical) &&
                (bits & tmx::TileLayer::FlipFlag::Diagonal))
            {
                //1110
                flipY(v0, v1, v2, v3, v4, v5);
                flipX(v0, v1, v2, v3, v4, v5);
                flipD(v0, v1, v2, v3, v4, v5);
            }
        }

    private:
        class ChunkArray final : public sf::Drawable
        {
        public:
            using Ptr = std::unique_ptr<ChunkArray>;

            tmx::Vector2u tileSetSize;
            sf::Vector2u tsTileCount;
            std::uint32_t m_firstGID, m_lastGID;

            explicit ChunkArray(const sf::Texture& t, const tmx::Tileset& ts)
                : m_texture(t)
            {
                auto texSize = getTextureSize();
                tileSetSize = ts.getTileSize();
                tsTileCount.x = texSize.x / tileSetSize.x;
                tsTileCount.y = texSize.y / tileSetSize.y;
                m_firstGID = ts.getFirstGID();
                m_lastGID = ts.getLastGID();
            }

            ~ChunkArray() = default;
            ChunkArray(const ChunkArray&) = delete;
            ChunkArray& operator = (const ChunkArray&) = delete;

            void reset()
            {
                m_vertices.clear();
            }

            void addTile(const Chunk::Tile& tile)
            {
                for (const auto& v : tile)
                {
                    m_vertices.push_back(v);
                }
            }

            sf::Vector2u getTextureSize() const
            {
                return m_texture.getSize();
            }

        private:
            const sf::Texture& m_texture;
            std::vector<sf::Vertex> m_vertices;
            void draw(sf::RenderTarget& rt, sf::RenderStates states) const override
            {
                states.texture = &m_texture;
                rt.draw(m_vertices.data(), m_vertices.size(), sf::PrimitiveType::Triangles, states);
            }
        };

        std::int8_t layerOpacity;     // opacity of the layer
        sf::Vector2f layerOffset;   // Layer offset
        sf::Vector2u mapTileSize;   // general Tilesize of Map
        sf::Vector2f chunkTileCount;   // chunk tilecount
        std::vector<tmx::TileLayer::Tile> m_chunkTileIDs; // stores all tiles in this chunk for later manipulation
        std::vector<sf::Color> m_chunkColors; // stores colors for extended color effects
        std::map<std::uint32_t, tmx::Tileset::Tile> m_animTiles;    // animation catalogue
        std::vector<AnimationState> m_activeAnimations;     // Animations to be done in this chunk
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
    mutable std::vector<Chunk*> m_visibleChunks;

    Chunk::Ptr& getChunkAndTransform(std::int32_t x, std::int32_t y, sf::Vector2u& chunkRelative)
    {
        std::uint32_t chunkX = (x * m_mapTileSize.x) / static_cast<std::uint32_t>(m_chunkSize.x);
        std::uint32_t chunkY = (y * m_mapTileSize.y) / static_cast<std::uint32_t>(m_chunkSize.y);
        chunkRelative.x = ((x * m_mapTileSize.x) - chunkX * static_cast<std::uint32_t>(m_chunkSize.x)) / m_mapTileSize.x;
        chunkRelative.y = ((y * m_mapTileSize.y) - chunkY * static_cast<std::uint32_t>(m_chunkSize.y)) / m_mapTileSize.y;
        return  m_chunks[chunkX + chunkY * m_chunkCount.x];
    }

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
        fallback.resize(sf::Vector2u{ 2, 2 }, sf::Color::Magenta);
        for (const auto& ts : usedTileSets)
        {
            const auto& path = ts->getImagePath();

            std::unique_ptr<sf::Texture> newTexture = std::make_unique<sf::Texture>();
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
        m_chunkCount.x = static_cast<std::uint32_t>(std::ceil(bounds.width / m_chunkSize.x));
        m_chunkCount.y = static_cast<std::uint32_t>(std::ceil(bounds.height / m_chunkSize.y));

        sf::Vector2u tileSize(map.getTileSize().x, map.getTileSize().y);

        for (auto y = 0u; y < m_chunkCount.y; ++y)
        {
            sf::Vector2f tileCount(m_chunkSize.x / tileSize.x, m_chunkSize.y / tileSize.y);
            for (auto x = 0u; x < m_chunkCount.x; ++x)
            {
                // calculate size of each Chunk (clip against map)
                if ((x + 1) * m_chunkSize.x > bounds.width)
                {
                    tileCount.x = (bounds.width - x * m_chunkSize.x) / map.getTileSize().x;
                }
                if ((y + 1) * m_chunkSize.y > bounds.height)
                {
                    tileCount.y = (bounds.height - y * m_chunkSize.y) / map.getTileSize().y;
                }
                //m_chunks.emplace_back(std::make_unique<Chunk>(layer, usedTileSets,
                //    sf::Vector2f(x * m_chunkSize.x, y * m_chunkSize.y), tileCount, map.getTileCount().x, m_textureResource));
                m_chunks.emplace_back(std::make_unique<Chunk>(layer, usedTileSets,
                    sf::Vector2f(x * m_chunkSize.x, y * m_chunkSize.y), tileCount, tileSize, map.getTileCount().x, m_textureResource, map.getAnimatedTiles()));
            }
        }
    }

    void updateVisibility(const sf::View& view) const
    {
        sf::Vector2f viewCorner = view.getCenter();
        viewCorner -= view.getSize() / 2.f;

        std::int32_t posX = static_cast<std::int32_t>(std::floor(viewCorner.x / m_chunkSize.x));
        std::int32_t posY = static_cast<std::int32_t>(std::floor(viewCorner.y / m_chunkSize.y));
        std::int32_t posX2 = static_cast<std::int32_t>(std::ceil((viewCorner.x + view.getSize().x) / m_chunkSize.x));
        std::int32_t posY2 = static_cast<std::int32_t>(std::ceil((viewCorner.y + view.getSize().x) / m_chunkSize.y));

        std::vector<Chunk*> visible;
        for (auto y = posY; y < posY2; ++y)
        {
            for (auto x = posX; x < posX2; ++x)
            {
                std::size_t idx = y * std::int32_t(m_chunkCount.x) + x;
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
        states.transform.translate(m_offset);

        //calc view coverage and draw nearest chunks
        updateVisibility(rt.getView());
        for (const auto& c : m_visibleChunks)
        {
            rt.draw(*c, states);
        }
    }
};

#endif //SFML_ORTHO_HPP_