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
#include <OpenGL.hpp>
#include <GLCheck.hpp>

#include <glm/gtc/matrix_transform.hpp>

#include <tmxlite/Map.hpp>

#include <SDL2/SDL_image.h>

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
    for(const auto& t : m_tileTextures)
    {
        glCheck(glDeleteTextures(1, &t));
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
    tmx::Map map;
    map.load("assets/demo.tmx");    
    
    //create shared resources, shader and tileset textures
    initGLStuff(map);

    //create a drawable object for each tile layer
    const auto& layers = map.getLayers();
    for(auto i = 0u; i < layers.size(); ++i)
    {
        if(layers[i]->getType() == tmx::Layer::Type::Tile)
        {
            m_mapLayers.emplace_back(std::make_unique<MapLayer>(map, i, m_tileTextures));
        }
    }
}


void Game::initGLStuff(const tmx::Map& map)
{
    m_projectionMatrix = glm::ortho(0.f, 800.f, 600.f, 0.f, -0.1f, 100.f);
    
    loadShader();
    glCheck(glUseProgram(m_shader));
    glCheck(glUniformMatrix4fv(glGetUniformLocation(m_shader, "u_projectionMatrix"), 1, GL_FALSE, &m_projectionMatrix[0][0]));
    
    //we'll make sure the current tile texture is active in 0, 
    //and lookup texture is active in 1 in MapLayer::draw()
    glCheck(glUniform1i(glGetUniformLocation(m_shader, "u_tileMap"), 0));
    glCheck(glUniform1i(glGetUniformLocation(m_shader, "u_lookupMap"), 1));
    
    const auto& tilesets = map.getTilesets();
    for(const auto& ts : tilesets)
    {
        loadTexture(ts.getImagePath());
    }    
    
    
    glCheck(glClearColor(0.6f, 0.8f, 0.92f, 1.f));
    glCheck(glEnable(GL_BLEND));
    glCheck(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
    glCheck(glBlendEquation(GL_FUNC_ADD));
}

void Game::loadShader()
{
    GLuint vertID = glCreateShader(GL_VERTEX_SHADER);
    
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
        
        glCheck(glDeleteShader(vertID));
        return; //m_shader is still 0 so we won't spam the console with gl errors
    }
    else
    {
        std::cout << "Compiled Vertex Shader" << std::endl;
    }
    
    //compile / check frag shader
    GLuint fragID = glCreateShader(GL_FRAGMENT_SHADER);    
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
        
        glCheck(glDeleteShader(vertID));
        glCheck(glDeleteShader(fragID));
        return;
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
        
        glCheck(glDetachShader(m_shader, vertID));
        glCheck(glDetachShader(m_shader, fragID));
        
        glCheck(glDeleteShader(vertID));
        glCheck(glDeleteShader(fragID));
        glCheck(glDeleteProgram(m_shader));
        m_shader = 0;
        return;
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

void Game::loadTexture(const std::string& path)
{    
    auto img = IMG_Load(path.c_str());
    if(img)
    {
        //TODO we should also check for tileset transparency here
        // and process it if found
        SDL_LockSurface(img);
        
        m_tileTextures.emplace_back(0);
        auto& texture = m_tileTextures.back();
                        
        GLint format = (img->format->BitsPerPixel == 32) ? GL_RGBA : GL_RGB;
                
        glCheck(glGenTextures(1, &texture));
        glCheck(glBindTexture(GL_TEXTURE_2D, texture));
        glCheck(glTexImage2D(GL_TEXTURE_2D, 0, format, img->w, img->h, 0, format, GL_UNSIGNED_BYTE, img->pixels));

        glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
        glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
        glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        glCheck(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        
        SDL_UnlockSurface(img);
        SDL_FreeSurface(img);
        
        std::cout << "Loaded " << path << std::endl;
    }
    else
    {
        std::cout << "Failed to load " << path << std::endl;
    }
}












