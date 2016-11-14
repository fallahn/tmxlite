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
This is an example of drawing with SFML - better use of vertex
arrays as well as resource management by the user is encouraged.
*/

#ifndef SFML_ORTHO_HPP_
#define SFML_ORTHO_HPP_

#include <tmxlite/Map.hpp>
#include <tmxlite/TileLayer.hpp>

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <memory>
#include <vector>
#include <array>
#include <map>
#include <string>
#include <limits>

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
            std::cout << "Not a valid othogonal layer, nothing will be drawn." << std::endl;
        }
    }

    ~MapLayer() = default;
    MapLayer(const MapLayer&) = delete;
    MapLayer& operator = (const MapLayer&) = delete;

    const sf::FloatRect& getGlobalBounds() const { return m_globalBounds; }

private:

    sf::Vector2f m_chunkSize = sf::Vector2f(1024.f, 1024.f);
    sf::Vector2u m_chunkCount;
    sf::FloatRect m_globalBounds;

    using TextureResource = std::map<std::string, std::unique_ptr<sf::Texture>>;
    TextureResource m_textureResource;

    class Chunk final : public sf::Transformable, public sf::Drawable 
    {
    public:
        using Ptr = std::unique_ptr<Chunk>;
        using Tile = std::array<sf::Vertex, 4u>;
        Chunk(const tmx::TileLayer, std::vector<const tmx::Tileset*> tilesets, const sf::Vector2f& position)
        {
            //go through the tiles and create the approriate arrays
            //TODO remember to set vertex colours / visibility from layer property
            
            setPosition(position);
        }
        ~Chunk() = default;
        Chunk(const Chunk&) = delete;
        Chunk& operator = (const Chunk&) = delete;

        bool empty() const { return m_chunkArrays.empty(); }
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

        private:
            const sf::Texture& m_texture;
            std::vector<sf::Vertex> m_vertices;
            void draw(sf::RenderTarget& rt, sf::RenderStates states) const override
            {
                states.texture = &m_texture;
                rt.draw(m_vertices.data(), m_vertices.size(), sf::Quads, states);
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
            std::unique_ptr<sf::Texture> newTexture = std::make_unique<sf::Texture>();
            if (!newTexture->loadFromFile(path))
            {
                newTexture->loadFromImage(fallback);
            }
            m_textureResource.insert(std::make_pair(path, std::move(newTexture)));
        }

        //calculate the number of chunks in the layer
        //and create each one
        const auto bounds = map.getBounds();
        m_chunkCount.x = static_cast<sf::Uint32>(std::ceil(bounds.width / m_chunkSize.x));
        m_chunkCount.y = static_cast<sf::Uint32>(std::ceil(bounds.height / m_chunkSize.y));

        for (auto y = 0u; y < m_chunkCount.y; ++y)
        {
            for (auto x = 0u; x < m_chunkCount.x; ++x)
            {
                m_chunks.emplace_back(std::make_unique<Chunk>(layer, usedTileSets, sf::Vector2f(x * m_chunkSize.x, y * m_chunkSize.y)));
            }
        }
    }

    void updateVisibility(const sf::View& view) const
    {
        sf::Vector2f viewCorner = view.getCenter();
        viewCorner -= view.getSize() / 2.f;
        
        int posX = static_cast<int>(std::floor(viewCorner.x / m_chunkSize.x));
        int posY = static_cast<int>(std::floor(viewCorner.y / m_chunkSize.y));

        std::vector<const Chunk*> visible;
        for (auto y = posY; y < posY + 2; ++y)
        {
            for (auto x = posX; x < posX + 2; ++x)
            {
                auto idx = y * m_chunkCount.x + x;
                if (idx > 0 && idx < m_chunks.size() && !m_chunks[idx]->empty())
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
        for (const auto c : m_visibleChunks)
        {
            rt.draw(*c, states);
        }
    }
};

#endif //SFML_ORTHO_HPP_