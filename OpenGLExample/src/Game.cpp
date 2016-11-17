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

#include <Game.hpp>
#include <GLExtensions.hpp>
#include <GLCheck.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <tmxlite/Map.hpp>

#include <cassert>
#include <array>

namespace
{
    bool running = true;
    const float timePerFrame = 1.f / 60.f;
    
#include <Shader.inl>
}

Game::Game()
  : m_shader(0)
{
    
}

Game::~Game()
{    
    if(m_shader)
    {
        glCheck(glDeleteProgram(m_shader));
    }
}

//public
void Game::run(SDL_Window* window)
{
    assert(window);
    
    if(ogl_LoadFunctions() == ogl_LOAD_FAILED)
    {
        std::cout << "Failed to load OpenGL functions" << std::endl;
        return;
    }
    loadMap();
        
    Uint32 lastTime = SDL_GetTicks();
    float elapsed = static_cast<float>(lastTime) / 1000.f;
    while(running)
    {
        while(elapsed > timePerFrame)
        {
            doEvents();
            update(timePerFrame);
            elapsed -= timePerFrame;
        }
        
        lastTime = SDL_GetTicks() - lastTime;
        elapsed += static_cast<float>(lastTime) / 1000.f;
        draw(window);
    }
}

//private
void Game::doEvents()
{
    SDL_Event evt;
    while(SDL_PollEvent(&evt))
    {
        if(evt.type == SDL_QUIT) running = false;
        
        if(evt.type == SDL_KEYUP)
        {
            switch(evt.key.keysym.sym)
            {
            default:break;
            case SDLK_ESCAPE: running = false; break;
            /*case SDLK_r: 
                glCheck(glClearColor(1.f, 0.f, 0.f, 1.f));
                break;
            case SDLK_g: 
                glCheck(glClearColor(0.f, 1.f, 0.f, 1.f));
                break;
            case SDLK_b: 
                glCheck(glClearColor(0.f, 0.f, 1.f, 1.f));
                break;                                */
            }
        }
    }
}

void Game::update(float dt)
{

}

void Game::draw(SDL_Window* window)
{
    glCheck(glClear(GL_COLOR_BUFFER_BIT));
    glCheck(glUseProgram(m_shader));
    
    for(const auto& layer : m_mapLayers)
    {
        layer->draw();
    }

    SDL_GL_SwapWindow(window);
}


void Game::loadMap()
{
    //create shared resources, shader and tileset textures
    initGLStuff();
    
    tmx::Map map;
    map.load("assets/demo.tmx");
    
    //TODO pass reference to shader and textures to each layer
    m_mapLayers.emplace_back(std::make_unique<MapLayer>(map, 0));
}


void Game::initGLStuff()
{
    m_projectionMatrix = glm::ortho(0.f, 800.f, 600.f, 0.f, -0.1f, 100.f);
    
    loadShader();
    glCheck(glUseProgram(m_shader));
    glCheck(glUniformMatrix4fv(glGetUniformLocation(m_shader, "u_projectionMatrix"), 1, GL_FALSE, &m_projectionMatrix[0][0]));
}

void Game::loadShader()
{
    GLuint vertID = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragID = glCreateShader(GL_FRAGMENT_SHADER);
    
    //compile / check vert shader
    auto srcPtr = vertexShader.c_str();
    glCheck(glShaderSource(vertID, 1, &srcPtr, nullptr));
    glCheck(glCompileShader(vertID));
    
    GLint result = GL_FALSE;
    int resultLength = 0;
    
    glCheck(glGetShaderiv(vertID, GL_COMPILE_STATUS, &result));
    glCheck(glGetShaderiv(vertID, GL_INFO_LOG_LENGTH, &resultLength));
    if(result == GL_FALSE)
    {
        std::string str;
        str.resize(resultLength + 1);
        glCheck(glGetShaderInfoLog(vertID, resultLength, nullptr, &str[0]));
        std::cout <<"Failed Compiling Vertex Shader, status: " << result << std::endl;
        std::cout << str << std::endl;
    }
    else
    {
        std::cout << "Compiled Vertex Shader" << std::endl;
    }
    
    //compile / check frag shader
    srcPtr = fragmentShader.c_str();
    glCheck(glShaderSource(fragID, 1, &srcPtr, nullptr));
    glCheck(glCompileShader(fragID));
    
    result = GL_FALSE;
    resultLength = 0;
    
    glCheck(glGetShaderiv(fragID, GL_COMPILE_STATUS, &result));
    glCheck(glGetShaderiv(fragID, GL_INFO_LOG_LENGTH, &resultLength));
    if(result == GL_FALSE)
    {
        std::string str;
        str.resize(resultLength + 1);
        glCheck(glGetShaderInfoLog(fragID, resultLength, nullptr, &str[0]));
        std::cout <<"Failed Compiling Fragment Shader, status: " << result << std::endl;
        std::cout << str << std::endl;
    }
    else
    {
        std::cout << "Compiled Fragment Shader" << std::endl;
    }
    
    //link and assign vert attribs
    m_shader = glCreateProgram();
    glCheck(glAttachShader(m_shader, vertID));
    glCheck(glAttachShader(m_shader, fragID));
    
    glCheck(glBindAttribLocation(m_shader, 0, "a_position"));
    glCheck(glBindAttribLocation(m_shader, 1, "a_texCoord"));
    
    glCheck(glLinkProgram(m_shader));
    
    result = GL_FALSE;
    resultLength = 0;
    glCheck(glGetProgramiv(m_shader, GL_LINK_STATUS, &result));
    glCheck(glGetProgramiv(m_shader, GL_INFO_LOG_LENGTH, &resultLength));
    if(result == GL_FALSE)
    {
        std::string str;
        str.resize(resultLength + 1);
        glCheck(glGetProgramInfoLog(m_shader, resultLength, nullptr, &str[0]));
        std::cout << "Failed to Link Shader Program, status: " << result << std::endl;
        std::cout << str << std::endl;
    }
    else
    {
        std::cout << "Linked Shader Successfully" << std::endl;
    }
    
    glCheck(glDetachShader(m_shader, vertID));
    glCheck(glDetachShader(m_shader, fragID));
    
    glCheck(glDeleteShader(vertID));
    glCheck(glDeleteShader(fragID));
}














