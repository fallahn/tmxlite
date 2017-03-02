cmake_minimum_required(VERSION 2.8)
project(tmxlite)
SET(PROJECT_NAME tmxlite)


SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/")

if(NOT TMXLITE_STATIC_LIB)
  SET(TMXLITE_STATIC_LIB FALSE CACHE BOOL "Should tmxlite be built as a static or shared lib?")
endif()

SET(PROJECT_STATIC_RUNTIME FALSE CACHE BOOL "Use statically linked standard/runtime libraries?")

if(CMAKE_COMPILER_IS_GNUCXX OR APPLE)
  if(PROJECT_STATIC_RUNTIME)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++14 -static")
  else()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++14")
  endif()
endif()

if(TMXLITE_STATIC_LIB)
  SET(CMAKE_CXX_FLAGS_DEBUG "-g -D_DEBUG_ -DTMXLITE_STATIC")
  SET(CMAKE_CXX_FLAGS_RELEASE "-O4 -DNDEBUG -DTMXLITE_STATIC")
  SET(CMAKE_DEBUG_POSTFIX -s-d)
  SET(CMAKE_RELEASE_POSTFIX -s)
else()
  SET(CMAKE_CXX_FLAGS_DEBUG "-g -D_DEBUG_")
  SET(CMAKE_CXX_FLAGS_RELEASE "-O4 -DNDEBUG")
  SET(CMAKE_DEBUG_POSTFIX -d)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)


#includes the list of source files in the src directory
SET(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
include(${PROJECT_DIR}/CMakeLists.txt)

if(WIN32)
  if(TMXLITE_STATIC_LIB)
    add_library(${PROJECT_NAME} STATIC ${PROJECT_SRC})
  else()
    add_library(${PROJECT_NAME} SHARED ${PROJECT_SRC})
  endif()   
else()
  if(TMXLITE_STATIC_LIB)
    add_library(${PROJECT_NAME} STATIC ${PROJECT_SRC})
  else()
    add_library(${PROJECT_NAME} SHARED ${PROJECT_SRC})
  endif()
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/tmxlite DESTINATION include)
if(TMXLITE_STATIC_LIB)
  install(TARGETS ${PROJECT_NAME} EXPORT tmxlite-targets DESTINATION lib)
else()
  install(TARGETS ${PROJECT_NAME} EXPORT tmxlite-targets 
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin)
endif()
