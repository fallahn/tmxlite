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

// based on fragment shader at 
// http://www.the2dgame.com/index?page=articles&ref=ART4

#ifndef SHADER_INL_
#define SHADER_INL_

#include <string>

static const std::string vertexShader = R"(
    #version 130
    
    in vec3 a_position;
    in vec2 a_texCoord;
    
    uniform mat4 u_projectionMatrix;
    
    out vec2 v_texCoord;
    
    void main()
    {
        gl_Position = u_projectionMatrix * vec4(a_position, 1.0);
        //gl_Position = vec4(a_position, 1.0);
        
        v_texCoord = a_texCoord;
    })";
    
/*
static const std::string fragmentShader = R"(
    #version 130
    
    in vec2 v_texCoord;
    
    uniform sampler2D u_tileMap;
    uniform usampler2D u_lookupMap;
    
    out vec4 colour;
    
    void main()
    {
        colour = texture(u_lookupMap, v_texCoord);
    })";*/
    
static const std::string fragmentShader = R"(
    #version 130
    #define FLIP_HORIZONTAL 8u
    #define FLIP_VERTICAL 4u
    #define FLIP_DIAGONAL 2u

    in vec2 v_texCoord;

    uniform usampler2D u_lookupMap;
    uniform sampler2D u_tileMap;

    uniform vec2 u_tileSize = vec2(64.0);
    uniform vec2 u_tilesetCount = vec2(6.0, 7.0);
    uniform vec2 u_tilesetScale = vec2(1.0);

    uniform float u_opacity = 1.0;

    out vec4 colour;
    /*fixes rounding imprecision on AMD cards*/
    const float epsilon = 0.000005;

    void main()
    {
        uvec2 values = texture(u_lookupMap, v_texCoord).rg;
        if(values.r > 0u)
        {
            float index = float(values.r) - 1.0;
            vec2 position = vec2(mod(index + epsilon, u_tilesetCount.x), floor((index / u_tilesetCount.x) + epsilon)) / u_tilesetCount;
            vec2 offsetCoord = (v_texCoord * (textureSize(u_lookupMap, 0) * u_tilesetScale)) / u_tileSize;
            
            vec2 texelSize = vec2(1.0) / textureSize(u_lookupMap, 0);
            vec2 offset = mod(v_texCoord, texelSize);
            vec2 ratio = offset / texelSize;
            offset = ratio * (1.0 / u_tileSize);
            offset *= u_tileSize / u_tilesetCount;

            if(values.g != 0u)
            {
                vec2 tileSize = vec2(1.0) / u_tilesetCount;
                if((values.g & FLIP_DIAGONAL) != 0u)
                {
                    float temp = offset.x;
                    offset.x = offset.y;
                    offset.y = temp;
                    temp = tileSize.x / tileSize.y;
                    offset.x *= temp;
                    offset.y /= temp;
                    offset.x = tileSize.x - offset.x;
                    offset.y = tileSize.y - offset.y;
                }
                if((values.g & FLIP_VERTICAL) != 0u)
                {
                    offset.y = tileSize.y - offset.y;
                }
                if((values.g & FLIP_HORIZONTAL) != 0u)
                {
                    offset.x = tileSize.x - offset.x;
                }
            }
            colour = texture(u_tileMap, position + offset);
            colour.a = min(colour.a, u_opacity);
        }
        else
        {
             colour = vec4(0.0);
        }
    })";

#endif //SHADER_INL_
