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
    

static const std::string fragmentShader = R"(
    #version 130
    
    in vec2 v_texCoord;
    
    uniform sampler2D u_tileMap;
    uniform usampler2D u_lookupMap;
    
    out vec4 colour;
    
    void main()
    {
        colour = vec4(1.0, 0.0, 0.0, 1.0);
    })";

#endif //SHADER_INL_
