tmxlite
-------

A lightweight C++14 parsing library for tmx map files created with the Tiled
map editor. Requires no external linking, all dependencies are included.
Supports tmx maps up to 0.18 with CSV, zlib and base64 compression. The parser
is renderer agnostic, and is cross platform on Windows, linux and OS X. It 
has also been successfully built for Android too.

As the library contains no specific rendering functions some example projects
are included, along with the relevant CMake files. These are meant
mostly for guidance and are not 100% optimised, but should get you off on the right
foot when using libraries such as SFML or SDL2/OpenGL. Examples for any specific
rendering library are welcome via a pull request.

##### Building
Either use the included Visual Studio project file if you are on Windows
or the CMake file to generate project files for your compiler of choice. tmxlite
can be built as both static or shared libraries, or simply include the source
files in your own project.

/*********************************************************************  
Matt Marchant 2016 - 2017  
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

A big thanks goes to all who have contributed to tmxlite via the github community.  

tmxlite uses [pugixml](http://pugixml.org/) and [miniz](https://github.com/richgel999/miniz)