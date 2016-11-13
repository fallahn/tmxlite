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

#ifndef TMXLITE_TILELAYER_HPP_
#define TMXLITE_TILELAYER_HPP_

#include <tmxlite/Layer.hpp>

namespace tmx
{
    /*!
    \brief A layer made up from a series of tile sets
    */
    class TMXLITE_EXPORT_API TileLayer final : public Layer
    {
    public:
        /*!
        \brief Tile information for a layer
        */
        struct Tile final
        {
            std::uint32_t ID = 0; //< Global ID of the tile
            std::uint8_t flipFlags = 0; //< Flags marking if the tile should be flipped when drawn
        };

        /*!
        \brief Flags used to tell if a tile is flipped when drawn
        */
        enum FlipFlag
        {
            Horizontal = 0x8,
            Vertical = 0x4,
            Diagonal = 0x2
        };
            
        explicit TileLayer(std::size_t);
        ~TileLayer() = default;

        Type getType() const override { return Layer::Type::Tile; }
        void parse(const pugi::xml_node&) override;

        /*!
        \brief Returns the list of tiles used to make up the layer
        */
        const std::vector<Tile>& getTiles() const { return m_tiles; }

    private:
        std::vector<Tile> m_tiles;
        std::size_t m_tileCount;

        void parseBase64(const pugi::xml_node&);
        void parseCSV(const pugi::xml_node&);
        void parseUnencoded(const pugi::xml_node&);

        void createTiles(const std::vector<std::uint32_t>&);
    };
}

#endif //TMXLITE_TILE_LAYER_HPP_