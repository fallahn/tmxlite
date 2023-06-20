/*********************************************************************
Matt Marchant 2016 - 2023
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

#ifndef USE_EXTLIBS
#include "miniz.h"
#else
#include <zlib.h>
#endif
#include <tmxlite/FreeFuncs.hpp>
#include <tmxlite/Types.hpp>
#include <tmxlite/detail/Log.hpp>

#include <cstring>
#include <fstream>

bool tmx::decompress(const char* source, std::vector<unsigned char>& dest, std::size_t inSize, std::size_t expectedSize)
{
    if (!source)
    {
        LOG("Input string is empty, decompression failed.", Logger::Type::Error);
        return false;
    }

//#ifdef USE_EXTLIBS


//#else
    int currentSize = static_cast<int>(expectedSize);
    std::vector<unsigned char> byteArray(expectedSize / sizeof(unsigned char));
    z_stream stream;
    stream.zalloc = Z_NULL;
    stream.zfree = Z_NULL;
    stream.opaque = Z_NULL;
    stream.next_in = (Bytef*)source;
    stream.avail_in = static_cast<unsigned int>(inSize);
    stream.next_out = (Bytef*)byteArray.data();
    stream.avail_out = static_cast<unsigned int>(expectedSize);

    //we'd prefer to use inflateInit2 but it appears 
    //to be incorrect in miniz. This is fine for zlib
    //compressed data, but gzip compressed streams
    //will fail to inflate.
#ifdef USE_EXTLIBS
    if (inflateInit2(&stream, 15 + 32) != Z_OK)
#else
    if (inflateInit(&stream) != Z_OK)
#endif
    {
        LOG("inflate init failed", Logger::Type::Error);
        return false;
    }

    int result = 0;
    do
    {
        result = inflate(&stream, Z_SYNC_FLUSH);

        switch (result)
        {
        default: break;
        case Z_NEED_DICT:
        case Z_STREAM_ERROR:
            result = Z_DATA_ERROR;
        case Z_DATA_ERROR:
            Logger::log("If using gzip or zstd compression try using zlib instead", Logger::Type::Info);
        case Z_MEM_ERROR:
            inflateEnd(&stream);
            Logger::log("inflate() returned " +  std::to_string(result), Logger::Type::Error);
            return false;
        }

        if (result != Z_STREAM_END)
        {
            int oldSize = currentSize;
            currentSize *= 2;
            std::vector<unsigned char> newArray(currentSize / sizeof(unsigned char));
            std::memcpy(newArray.data(), byteArray.data(), currentSize / 2);
            byteArray = std::move(newArray);

            stream.next_out = (Bytef*)(byteArray.data() + oldSize);
            stream.avail_out = oldSize;

        }
    } while (result != Z_STREAM_END);

    if (stream.avail_in != 0)
    {
        LOG("stream.avail_in is 0", Logger::Type::Error);
        LOG("zlib decompression failed.", Logger::Type::Error);
        return false;
    }

    const int outSize = currentSize - stream.avail_out;
    inflateEnd(&stream);

    std::vector<unsigned char> newArray(outSize / sizeof(unsigned char));
    std::memcpy(newArray.data(), byteArray.data(), outSize);
    byteArray = std::move(newArray);

    //copy bytes to vector
    dest.insert(dest.begin(), byteArray.begin(), byteArray.end());
//#endif
    return true;
}

std::ostream& operator << (std::ostream& os, const tmx::Colour& c)
{
    os << "RGBA: " << (int)c.r << ", " << (int)c.g << ", " << (int)c.b << ", " << (int)c.a;
    return os;
}

void tmx::splitStringInto(const std::string& s, char sep, std::vector<std::string>* out)
{
    std::stringstream ss(s);
    std::string tmp;
    while (std::getline(ss, tmp, sep)) {
        out->push_back(std::move(tmp));
    }
}

void tmx::joinStringInto(const std::vector<std::string>& parts, char sep, std::string* out)
{
    size_t reservation = 0;
    for (const std::string& part : parts)
    {
        reservation += part.size() + 1;
    }

    out->reserve(out->size() + reservation);
    for (auto i = parts.begin(); i != parts.end(); ++i)
    {
        *out += *i;
        if (i + 1 != parts.end()) {
            out->push_back(sep);
        }
    }
}

#ifdef _WIN32
TMXLITE_CONSTINIT bool tmx::enableWindowsPathHandling = true;
#else
TMXLITE_CONSTINIT bool tmx::enableWindowsPathHandling = false;
#endif

bool tmx::isAbsoluteFilePath(const std::string& path, std::string* prefix)
{
    if (path.empty())
    {
        return false;
    }
    if (path.front() == '/')
    {
        if (prefix)
        {
            *prefix = "/";
        }
        return true;
    }
    if (enableWindowsPathHandling)
    {
        if (path.front() == '\\')
        {
            if (prefix)
            {
                *prefix = "\\";
            }
            return true;
        }
        if (path.size() >= 3 && path[1] == ':' && (path[2] == '\\' || path[2] == '/'))
        {
            if (prefix)
            {
                prefix->assign(path, 0, 3);
            }
            return true;
        }
    }
    return false;
}

std::string tmx::resolveFilePath(std::string path, std::string workingDir)
{
    std::replace(path.begin(), path.end(), '\\', '/');
    std::replace(workingDir.begin(), workingDir.end(), '\\', '/');

    if (path.empty())
    {
        return workingDir;
    }

    std::string absolutePathPrefix;
    bool pathIsAbsolute = isAbsoluteFilePath(path, &absolutePathPrefix);
    bool workingDirIsAbsolute = isAbsoluteFilePath(workingDir, pathIsAbsolute ? nullptr : &absolutePathPrefix);

    std::vector<std::string> parts;
    parts.reserve(std::count(workingDir.begin(), workingDir.end(), '/') + std::count(path.begin(), path.end(), '/') + 1);
    if (!workingDir.empty() && !pathIsAbsolute)
    {
        splitStringInto(workingDir, '/', &parts);
        if (workingDirIsAbsolute)
        {
            parts.erase(parts.begin());
        }
    }
    splitStringInto(path, '/', &parts);
    if (pathIsAbsolute)
    {
        parts.erase(parts.begin());
    }

    // Drop all empty components and all "."
    parts.erase(
        std::remove_if(parts.begin(), parts.end(), [](const std::string& part) {
            return part.empty() || part == ".";
        }),
        parts.end()
    );

    // After the above cleanups, we can cancel out all "X/.." where X != ".."
    size_t i = 1;
    while (i < parts.size())
    {
        if (parts[i] == ".." && parts[i - 1] != "..")
        {
            parts.erase(parts.begin() + i - 1, parts.begin() + i + 1);
            if (i > 1)
            {
                --i;
            }
        }
        else
        {
            ++i;
        }
    }

    std::string result;
    result.reserve(path.size() + workingDir.size() + 3);
    result += absolutePathPrefix;
    joinStringInto(parts, '/', &result);
    return result;
}

bool tmx::readFileIntoString(const std::string& path, std::string* out)
{
    std::ifstream stream(path);
    if (!stream.is_open())
    {
        return false;
    }
    out->append((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    return !stream.bad();
}
