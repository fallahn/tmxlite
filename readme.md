tmxlite
-------

[![Github Actions](https://github.com/fallahn/tmxlite/actions/workflows/cmake.yml/badge.svg)](https://github.com/fallahn/tmxlite/actions)

#### Description
A lightweight C++14 parsing library for tmx map files created with the Tiled map editor. Requires no external linking, all dependencies are included. Optionally Zstd support can be enabled by linking the required external library. Fully supports tmx maps up to 1.0 (see [here](https://doc.mapeditor.org/en/stable/reference/tmx-changelog/#tiled-1-0)) with CSV, zlib and base64 compression. Also supports some features of newer map versions (see below). The parser is renderer agnostic, and is cross platform on Windows, linux and macOS. It has also been successfully built for Android too.

As the library contains no specific rendering functions some example projects are included, along with the relevant CMake files. These are meant mostly for guidance and are not 100% optimised, but should get you off on the right foot when using libraries such as SFML, SDL2 or directly with OpenGL. Examples for any specific rendering library are welcome via a pull request.

##### Other Features
As well as full support for maps up to version 1.0, tmxlite also supports these features found in newer versions of the [tmx specification](https://doc.mapeditor.org/en/stable/reference/tmx-changelog/#tiled-1-0):

* Object Templates - Templates (and any associated tile sets) are automatically loaded and parsed if found. Object properties are transparently handled so that objects can be read from an `ObjectGroup` as if they were unique instances. If an `Object` uses a templated tileset then `Object::getTilsetName()` will contain a non-empty string which can be used as a key with `Map::getTemplateTilesets()` to retrieve the associated tileset data.
* Infinite Maps - Maps with the 'infinite' flag set, and saved in either CSV or base64 (compressed and uncompressed) format are supported. A `TileLayer` will return an empty Tile vector in these cases, and tile ID data can be retrieved instead with `TileLayer::getChunks()` which returns a vector of chunk data that makes up the tile layer.
* Parallax layers - the parallax offset property of layers is parsed, as well as each map's parallax origin, if they exist
* Layer tint colours
* Extended tag support such as `class`

By default tmxlite supports zlib compressed maps, however gzip and zstd compression can be enabled at compile time, by linking the relevant external libraries:

###### Zstd compression
Tmxlite supports maps using Zstd compressed tile layers, however Zstd needs to be linked externally. To configure CMake or meson to use Zstd add the definition `USE_ZSTD` and set it to `true`. You may also configure your project manually, and add `-DUSE_ZSTD` to the compiler options. If you are using the `USE_EXTLIBS` option (see below) this is automatically configured for you.

#### Building
Either use the included Visual Studio project file if you are on Windows or the CMake file to generate project files for your compiler of choice. tmxlite can be built as both static or shared libraries, or simply include the source files in your own project. The following options are available for CMake configuration:

 * `TMXLITE_STATIC_LIB` - Set this to true to build a static library, default false
 * `PROJECT_STATIC_RUNTIME` - Statically link the cstd libraries, default false
 * `USE_RTTI` - Enable runtime type information, default true
 * `USE_EXTLIBS` - Use externally linked pugixml, zlib and Zstd libraries, default false
 * `USE_ZSTD` - Use externally linked Zstd library, required for Zstd compressed maps. Default is false, but is overridden if `USE_EXTLIBS` is true

Configuring with meson is also possible, see `meson_options.txt` for details.

#### Quick Start
There is a getting started page on the Github wiki [here](https://github.com/fallahn/tmxlite/wiki/Quick-Start).

#### Examples
The repository contains the following rendering examples:
* [OpenGL Example](https://github.com/fallahn/tmxlite/tree/master/OpenGLExample)
* [SFML 2 Example](https://github.com/fallahn/tmxlite/tree/master/SFMLExample)
* [SFML 3 Example](https://github.com/fallahn/tmxlite/tree/master/SFML3Example)
* [SDL2 Example](https://github.com/fallahn/tmxlite/tree/master/SDL2Example)


Codotaku has published a video on creating a DX12 renderer: https://youtu.be/p1o1N72KQQA 

Examples are generally maintained by the community and offer varying amounts of functionality. All of them will render the included example tile maps at the very least, and some include more complex features such as animated tiles or individual tile flipping.

#### API Documentation
Doxygen generated API documentation can be found online [here](https://codedocs.xyz/fallahn/tmxlite/) or generated with doxygen
using the doxy file in the tmxlite/documentation/ directory.

#### Important information 
tmxlite uses [pugixml](https://pugixml.org/) and [miniz](https://github.com/richgel999/miniz) which are included in the repository, although external zlib and pugixml libraries can be used. Add `-DUSE_EXTLIBS` to your compiler's definitions or when configuring CMake set `USE_EXTLIBS` to TRUE. This will also automatically include Zstd.

***

(c)Matt Marchant & contributors 2016 - 2025
https://trederiagames.com

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
