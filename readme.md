tmxlite
-------

[![Build Status](https://img.shields.io/travis/fallahn/tmxlite.svg?branch=master&label=*nix)](https://travis-ci.org/fallahn/tmxlite)
[![Build status](https://ci.appveyor.com/api/projects/status/qhhh1geu47uoi2lj/branch/master?svg=true)](https://ci.appveyor.com/project/fallahn/tmxlite/branch/master)

#### Description
A lightweight C++14 parsing library for tmx map files created with the Tiled map editor. Requires no external linking, all dependencies are included. Supports tmx maps up to 1.0 (see [here](https://doc.mapeditor.org/en/stable/reference/tmx-changelog/#tiled-1-0)) with CSV, zlib and base64 compression. The parser is renderer agnostic, and is cross platform on Windows, linux and OS X. It has also been successfully built for Android too.

As the library contains no specific rendering functions some example projects are included, along with the relevant CMake files. These are meant mostly for guidance and are not 100% optimised, but should get you off on the right foot when using libraries such as SFML or SDL2/OpenGL. Examples for any specific rendering library are welcome via a pull request.



#### Building
Either use the included Visual Studio project file if you are on Windows or the CMake file to generate project files for your compiler of choice. tmxlite can be built as both static or shared libraries, or simply include the source files in your own project.

#### Quick Start
There is a getting started page on the Github wiki [here](https://github.com/fallahn/tmxlite/wiki/Quick-Start).

#### Examples
Check out the following examples:
* [OpenGL Example](https://github.com/fallahn/tmxlite/tree/master/OpenGLExample)
* [SFML Example](https://github.com/fallahn/tmxlite/tree/master/SFMLExample)

#### API Documentation
Doxygen generated API documentation can be found online [here](https://codedocs.xyz/fallahn/tmxlite/) or generated with doxygen
using the doxy file in the tmxlite/documentation/ directory.

#### Important information 
tmxlite uses [pugixml](https://pugixml.org/) and [miniz](https://github.com/richgel999/miniz) which are included in the repository.

***

Matt Marchant & contributors 2016 - 2019
http://trederia.blogspot.com

tmxlite - Zlib license.

This software is provided 'as-is', without any express or
implied warranty. In no event will the authors be held
liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute
it freely, subject to the following restrictions:

The origin of this software must not be misrepresented;
you must not claim that you wrote the original software.
If you use this software in a product, an acknowledgment
in the product documentation would be appreciated but
is not required.

Altered source versions must be plainly marked as such,
and must not be misrepresented as being the original software.

This notice may not be removed or altered from any
source distribution.
***

A big thanks goes to all who have contributed to tmxlite via the github community.
