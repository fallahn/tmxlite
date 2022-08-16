/*********************************************************************
Matt Marchant 2016 - 2021
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

#include "detail/pugixml.hpp"
#include <tmxlite/Tileset.hpp>
#include <tmxlite/FreeFuncs.hpp>
#include <tmxlite/detail/Log.hpp>

#include <ctype.h>

using namespace tmx;

Tileset::Tileset(const std::string& workingDir)
    : m_workingDir          (workingDir),
    m_firstGID              (0),
    m_spacing               (0),
    m_margin                (0),
    m_tileCount             (0),
    m_columnCount           (0),
    m_objectAlignment       (ObjectAlignment::Unspecified),
    m_transparencyColour    (0, 0, 0, 0),
    m_hasTransparency       (false)
{

}

//public
void Tileset::parse(pugi::xml_node node, Map* map)
{
    assert(map);

    std::string attribString = node.name();
    if (attribString != "tileset")
    {
        Logger::log(attribString + ": not a tileset node! Node will be skipped.", Logger::Type::Warning);
        return;
    }
    
    m_firstGID = node.attribute("firstgid").as_int();
    if (m_firstGID == 0)
    {
        Logger::log("Invalid first GID in tileset. Tileset node skipped.", Logger::Type::Warning);
        return;
    }

    pugi::xml_document tsxDoc; //need to keep this in scope
    if (node.attribute("source"))
    {
        //parse TSX doc
        std::string path = node.attribute("source").as_string();
        path = resolveFilePath(path, m_workingDir);

        //as the TSX file now dictates the image path, the working
        //directory is now that of the tsx file
        auto position = path.find_last_of('/');
        if (position != std::string::npos)
        {
            m_workingDir = path.substr(0, position);
        }
        else
        {
            m_workingDir = "";
        }

        //see if doc can be opened
        auto result = tsxDoc.load_file(path.c_str());
        if (!result)
        {
            Logger::log("Failed opening tsx file for tile set, tile set will be skipped", Logger::Type::Error);
            return reset();
        }

        //if it can then replace the current node with tsx node
        node = tsxDoc.child("tileset");
        if (!node)
        {
            Logger::log("tsx file does not contain a tile set node, tile set will be skipped", Logger::Type::Error);
            return reset();
        }
    }

    m_name = node.attribute("name").as_string();
    LOG("found tile set " + m_name, Logger::Type::Info);

    m_tileSize.x = node.attribute("tilewidth").as_int();
    m_tileSize.y = node.attribute("tileheight").as_int();
    if (m_tileSize.x == 0 || m_tileSize.y == 0)
    {
        Logger::log("Invalid tile size found in tile set node. Node will be skipped.", Logger::Type::Error);
        return reset();
    }

    m_spacing = node.attribute("spacing").as_int();
    m_margin = node.attribute("margin").as_int();
    m_tileCount = node.attribute("tilecount").as_int();
    m_columnCount = node.attribute("columns").as_int();

    std::string objectAlignment = node.attribute("objectalignment").as_string();
    if (!objectAlignment.empty())
    {
        if (objectAlignment == "unspecified")
        {
            m_objectAlignment = ObjectAlignment::Unspecified;
        }
        else if (objectAlignment == "topleft")
        {
            m_objectAlignment = ObjectAlignment::TopLeft;
        }
        else if (objectAlignment == "top")
        {
            m_objectAlignment = ObjectAlignment::Top;
        }
        else if (objectAlignment == "topright")
        {
            m_objectAlignment = ObjectAlignment::TopRight;
        }
        else if (objectAlignment == "left")
        {
            m_objectAlignment = ObjectAlignment::Left;
        }
        else if (objectAlignment == "center")
        {
            m_objectAlignment = ObjectAlignment::Center;
        }
        else if (objectAlignment == "right")
        {
            m_objectAlignment = ObjectAlignment::Right;
        }
        else if (objectAlignment == "bottomleft")
        {
            m_objectAlignment = ObjectAlignment::BottomLeft;
        }
        else if (objectAlignment == "bottom")
        {
            m_objectAlignment = ObjectAlignment::Bottom;
        }
        else if (objectAlignment == "bottomright")
        {
            m_objectAlignment = ObjectAlignment::BottomRight;
        }
    }

    const auto& children = node.children();
    for (const auto& node : children)
    {
        std::string name = node.name();
        if (name == "image")
        {
            //TODO this currently doesn't cover embedded images
            //mostly because I can't figure out how to export them
            //from the Tiled editor... but also resource handling
            //should be handled by the renderer, not the parser.
            attribString = node.attribute("source").as_string();
            if (attribString.empty())
            {
                Logger::log("Tileset image node has missing source property, tile set not loaded", Logger::Type::Error);
                return reset();
            }
            m_imagePath = resolveFilePath(attribString, m_workingDir);
            if (node.attribute("trans"))
            {
                attribString = node.attribute("trans").as_string();
                m_transparencyColour = colourFromString(attribString);
                m_hasTransparency = true;
            }
            if (node.attribute("width") && node.attribute("height"))
            {
                m_imageSize.x = node.attribute("width").as_int();
                m_imageSize.y = node.attribute("height").as_int();
            }
        }
        else if (name == "tileoffset")
        {
            parseOffsetNode(node);
        }
        else if (name == "properties")
        {
            parsePropertyNode(node);
        }
        else if (name == "terraintypes")
        {
            parseTerrainNode(node);
        }
        else if (name == "tile")
        {
            parseTileNode(node, map);
        }
    }

    //if the tsx file does not declare every tile, we create the missing ones
    if (m_tiles.size() != getTileCount())
    {
        for (std::uint32_t ID = 0 ; ID < getTileCount() ; ID++)
        {
            createMissingTile(ID);
        }
    }

    //sort these just to make sure when we request last GID we get the corrtect value
    std::sort(m_tiles.begin(), m_tiles.end(), [](const Tile& t1, const Tile& t2) {return t1.ID < t2.ID; });
}

std::uint32_t Tileset::getLastGID() const
{
    assert(!m_tiles.empty());
    return m_firstGID + m_tiles.back().ID;
}

const Tileset::Tile* Tileset::getTile(std::uint32_t id) const
{
    if (!hasTile(id))
    {
        return nullptr;
    }
    
    //corrects the ID. Indices and IDs are different.
    id = (getLastGID() - m_firstGID) - (getLastGID() - id);
    
    const auto itr = std::find_if(m_tiles.begin(), m_tiles.end(), 
        [id](const Tile& tile)
    {
            return tile.ID == id;
    });

    return (itr == m_tiles.end()) ? nullptr : &(*itr);
}

//private
void Tileset::reset()
{
    m_firstGID = 0;
    m_source = "";
    m_name = "";
    m_tileSize = { 0,0 };
    m_spacing = 0;
    m_margin = 0;
    m_tileCount = 0;
    m_columnCount = 0;
    m_objectAlignment = ObjectAlignment::Unspecified;
    m_tileOffset = { 0,0 };
    m_properties.clear();
    m_imagePath = "";
    m_transparencyColour = { 0, 0, 0, 0 };
    m_hasTransparency = false;
    m_terrainTypes.clear();
    m_tiles.clear();
}

void Tileset::parseOffsetNode(const pugi::xml_node& node)
{
    m_tileOffset.x = node.attribute("x").as_int();
    m_tileOffset.y = node.attribute("y").as_int();
}

void Tileset::parsePropertyNode(const pugi::xml_node& node)
{
    const auto& children = node.children();
    for (const auto& child : children)
    {
        m_properties.emplace_back();
        m_properties.back().parse(child);
    }
}

void Tileset::parseTerrainNode(const pugi::xml_node& node)
{
    const auto& children = node.children();
    for (const auto& child : children)
    {
        std::string name = child.name();
        if (name == "terrain")
        {
            m_terrainTypes.emplace_back();
            auto& terrain = m_terrainTypes.back();
            terrain.name = child.attribute("name").as_string();
            terrain.tileID = child.attribute("tile").as_int();
            auto properties = child.child("properties");
            if (properties)
            {
                for (const auto& p : properties)
                {
                    name = p.name();
                    if (name == "property")
                    {
                        terrain.properties.emplace_back();
                        terrain.properties.back().parse(p);
                    }
                }
            }
        }
    }
}

void Tileset::parseTileNode(const pugi::xml_node& node, Map* map)
{
    assert(map);

    Tile tile;
    tile.ID = node.attribute("id").as_int();
    if (node.attribute("terrain"))
    {
        std::string data = node.attribute("terrain").as_string();
        bool lastWasChar = true;
        std::size_t idx = 0u;
        for (auto i = 0u; i < data.size() && idx < tile.terrainIndices.size(); ++i)
        {
            if (isdigit(data[i]))
            {
                tile.terrainIndices[idx++] = std::atoi(&data[i]);
                lastWasChar = false;
            }
            else
            {
                if (!lastWasChar)
                {
                    lastWasChar = true;
                }
                else
                {
                    tile.terrainIndices[idx++] = -1;
                    lastWasChar = false;
                }
            }
        }
        if (lastWasChar)
        {
            tile.terrainIndices[idx] = -1;
        }
    }

    tile.probability = node.attribute("probability").as_int(100);

    tile.Class = node.attribute("type").as_string();
	if (tile.Class.empty())
	{
        tile.Class = node.attribute("class").as_string();
	}
    
    //by default we set the tile's values as in an Image tileset
    tile.imagePath = m_imagePath;
    tile.imageSize = m_tileSize;

    if (m_columnCount != 0) 
    {
        std::int32_t rowIndex = tile.ID % m_columnCount;
        std::int32_t columnIndex = tile.ID / m_columnCount;
        tile.imagePosition.x = m_margin + rowIndex * (m_tileSize.x + m_spacing);
        tile.imagePosition.y = m_margin + columnIndex * (m_tileSize.y + m_spacing);
    }

    const auto& children = node.children();
    for (const auto& child : children)
    {
        std::string name = child.name();
        if (name == "properties")
        {
            for (const auto& prop : child.children())
            {
                tile.properties.emplace_back();
                tile.properties.back().parse(prop);
            }
        }
        else if (name == "objectgroup")
        {
            tile.objectGroup.parse(child, map);
        }
        else if (name == "image")
        {
            std::string attribString = child.attribute("source").as_string();
            if (attribString.empty())
            {
                Logger::log("Tile image path missing", Logger::Type::Warning);
                continue;
            }
            tile.imagePath = resolveFilePath(attribString, m_workingDir);

            tile.imagePosition = tmx::Vector2u(0, 0);

            if (child.attribute("trans"))
            {
                attribString = child.attribute("trans").as_string();
                m_transparencyColour = colourFromString(attribString);
                m_hasTransparency = true;
            }
            if (child.attribute("width"))
            {
                tile.imageSize.x = child.attribute("width").as_uint();
            }
            if (child.attribute("height"))
            {
                tile.imageSize.y = child.attribute("height").as_uint();
            }
        }
        else if (name == "animation")
        {
            for (const auto& frameNode : child.children())
            {
                Tile::Animation::Frame frame;
                frame.duration = frameNode.attribute("duration").as_int();
                frame.tileID = frameNode.attribute("tileid").as_int() + m_firstGID;
                tile.animation.frames.push_back(frame);
            }
        }
    }
    m_tiles.push_back(tile);
}

void Tileset::createMissingTile(std::uint32_t ID)
{
    //first, we check if the tile does not yet exist
    for (const auto& tile : m_tiles)
    {
        if (tile.ID == ID)
        {
            return;
        }
    }

    Tile tile;
    tile.ID = ID;
    tile.imagePath = m_imagePath;
    tile.imageSize = m_tileSize;

    std::int32_t rowIndex = ID % m_columnCount;
    std::int32_t columnIndex = ID / m_columnCount;
    tile.imagePosition.x = m_margin + rowIndex * (m_tileSize.x + m_spacing);
    tile.imagePosition.y = m_margin + columnIndex * (m_tileSize.y + m_spacing);

    m_tiles.push_back(tile);
}
