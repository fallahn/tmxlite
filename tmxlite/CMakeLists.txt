cmake_minimum_required(VERSION 3.0)
project(tmxlite VERSION 1.3.1)
SET(PROJECT_NAME tmxlite)

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules/")

if(NOT TMXLITE_STATIC_LIB)
  SET(TMXLITE_STATIC_LIB FALSE CACHE BOOL "Should tmxlite be built as a static or shared lib?")
endif()

SET(PROJECT_STATIC_RUNTIME FALSE CACHE BOOL "Use statically linked standard/runtime libraries?")
#option(BUILD_DOC "Build documentation" OFF)

SET(USE_RTTI TRUE CACHE BOOL "Use run time type information?")

if(USE_RTTI)
  if(CMAKE_COMPILER_IS_GNUCXX OR APPLE)
    if(PROJECT_STATIC_RUNTIME)
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -static")
    else()
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
    endif()
  endif()
else()
  if(CMAKE_COMPILER_IS_GNUCXX OR APPLE)
    if(PROJECT_STATIC_RUNTIME)
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fno-rtti -static")
    else()
      SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fno-rtti")
    endif()
  endif()
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(TMXLITE_STATIC_LIB)
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -D_DEBUG_ -DTMXLITE_STATIC")
  SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG -DTMXLITE_STATIC")
  SET(CMAKE_DEBUG_POSTFIX -s-d)
  SET(CMAKE_RELEASE_POSTFIX -s)
else()
  SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -D_DEBUG_")
  SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")
  SET(CMAKE_DEBUG_POSTFIX -d)
endif()

#disable msvc warning
if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
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

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/tmxlite.pc.in ${CMAKE_CURRENT_BINARY_DIR}/tmxlite.pc
               @ONLY)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

include(GNUInstallDirs)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/tmxlite DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
if(TMXLITE_STATIC_LIB)
  install(TARGETS ${PROJECT_NAME} EXPORT tmxlite-targets DESTINATION ${CMAKE_INSTALL_LIBDIR})
else()
  install(TARGETS ${PROJECT_NAME} EXPORT tmxlite-targets 
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

install(
  FILES       ${CMAKE_CURRENT_BINARY_DIR}/tmxlite.pc
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
  COMPONENT   libraries)
