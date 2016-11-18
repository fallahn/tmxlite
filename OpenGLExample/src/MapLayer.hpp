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

#ifndef MAPLAYER_HPP_
#define MAPLAYER_HPP_

#include <vector>

namespace tmx
{
    class Map;
}

class MapLayer final
{
public:
    MapLayer(const tmx::Map&, std::size_t, const std::vector<unsigned>&);
    ~MapLayer();
    
    MapLayer(const MapLayer&) = delete;
    MapLayer& operator = (const MapLayer&) = delete;

    void draw();

private:

    const std::vector<unsigned>& m_tilesetTextures;

    struct Subset final
    {
        unsigned vbo = 0;
        unsigned texture = 0;
        unsigned lookup = 0;
    };
    std::vector<Subset> m_subsets;

    void createSubsets(const tmx::Map&, std::size_t);
};

#endif //MAPLAYER_HPP_
