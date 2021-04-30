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

#pragma once

#include <tmxlite/Config.hpp>
#include <tmxlite/Property.hpp>
#include <tmxlite/Types.hpp>

#include <string>
#include <memory>
#include <vector>

namespace pugi
{
    class xml_node;
}

namespace tmx
{
    class Map;
    class TileLayer;
    class ObjectGroup;
    class ImageLayer;
    class LayerGroup;
    /*!
    \brief Represents a layer of a tmx format tile map.
    This is an abstract base class from which all layer
    types are derived.
    */
    class TMXLITE_EXPORT_API Layer
    {
    public:
        using Ptr = std::unique_ptr<Layer>;

        Layer() : m_opacity(1.f), m_visible(true) {};
        virtual ~Layer() = default;

        /*!
        \brief Layer type as returned by getType()
        Tile: this layer is a TileLayer type
        Object: This layer is an ObjectGroup type
        Image: This layer is an ImageLayer type
        Group: This layer is a LayerGroup type
        */
        enum class Type
        {
            Tile,
            Object,
            Image,
            Group
        };

        /*!
        \brief Returns a Type value representing the concrete type.
        Use this when deciding which conrete layer type to use when
        calling the templated function getLayerAs<T>()
        */
        virtual Type getType() const = 0;

        /*!
        \brief Use this to get a reference to the concrete layer type
        which this layer points to.
        Use getType() to return the type value of this layer and determine
        if the concrete type is TileLayer, ObjectGroup, ImageLayer, or LayerGroup
        */
        template <typename T>
        T& getLayerAs();
        /*{
            throw("Not a valid layer type");
            return *dynamic_cast<T*>(this);
        }*/

        template <typename T>
        const T& getLayerAs() const { return getLayerAs<T>(); }

        /*!
        \brief Attempts to parse the specific node layer type
        */
        virtual void parse(const pugi::xml_node&, Map* = nullptr) = 0;

        /*!
        \brief Returns the name of the layer
        */
        const std::string& getName() const { return m_name; }

        /*!
        \brief Returns the opacity value for the layer
        */
        float getOpacity() const { return m_opacity; }

        /*!
        \brief Returns whether this layer is visible or not
        */
        bool getVisible() const { return m_visible; }

        /*!
        \brief Returns the offset from the top left corner
        of the layer, in pixels
        */
        const Vector2i& getOffset() const { return m_offset; }

        /*!
        \brief Returns the size of the layer, in pixels.
        This will be the same as the map size for fixed size maps.
        */
        const Vector2u& getSize() const { return m_size; }

        /*!
        \brief Returns the list of properties of this layer
        */
        const std::vector<Property>& getProperties() const { return m_properties; }

    protected:

        void setName(const std::string& name) { m_name = name; }
        void setOpacity(float opacity) { m_opacity = opacity; }
        void setVisible(bool visible) { m_visible = visible; }
        void setOffset(std::int32_t x, std::int32_t y) { m_offset = Vector2i(x, y); }
        void setSize(std::uint32_t width, std::uint32_t height) { m_size = Vector2u(width, height); }
        void addProperty(const pugi::xml_node& node) { m_properties.emplace_back(); m_properties.back().parse(node); }

    private:
        std::string m_name;
        float m_opacity;
        bool m_visible;
        Vector2i m_offset;
        Vector2u m_size;

        std::vector<Property> m_properties;
    };
}
