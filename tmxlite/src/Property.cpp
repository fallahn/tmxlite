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

#include <tmxlite/Property.hpp>
#include "detail/pugixml.hpp"
#include <tmxlite/detail/Log.hpp>
#include <tmxlite/FreeFuncs.hpp>

using namespace tmx;

Property::Property()
    : m_type(Type::Undef)
{

}

//public
void Property::parse(const pugi::xml_node& node)
{
    std::string attribData = node.name();
    if (attribData != "property")
    {
        Logger::log("Node was not a valid property, node will be skipped", Logger::Type::Error);
        return;
    }

    m_name = node.attribute("name").as_string();

    attribData = node.attribute("type").as_string("string");
    if (attribData == "bool")
    {
        attribData = node.attribute("value").as_string("false");
        m_boolValue = (attribData == "true");
        m_type = Type::Boolean;
        return;
    }
    else if (attribData == "int")
    {
        m_intValue = node.attribute("value").as_int(0);
        m_type = Type::Int;
        return;
    }
    else if (attribData == "float")
    {
        m_floatValue = node.attribute("value").as_float(0.f);
        m_type = Type::Float;
        return;
    }
    else if(attribData == "string")
    {
        m_stringValue = node.attribute("value").as_string();
        m_type = Type::String;
        return;
    }
    else if (attribData == "color")
    {
        m_colourValue = colourFromString(node.attribute("value").as_string("#FFFFFFFF"));
        m_type = Type::Colour;
        return;
    }
    else if (attribData == "file")
    {
        m_stringValue = node.attribute("value").as_string();
        m_type = Type::File;
        return;
    }
}
