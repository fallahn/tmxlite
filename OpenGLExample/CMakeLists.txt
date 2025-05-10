cmake_minimum_required(VERSION 3.5)
project(OpenGL_example)
SET(PROJECT_NAME OpenGL_example)

if(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
SET(PROJECT_STATIC_RUNTIME FALSE CACHE BOOL "Use statically linked standard/runtime libraries?")
#SET(PROJECT_STATIC_TMX FALSE CACHE BOOL "Use statically linked tmxlite library?")

if(CMAKE_COMPILER_IS_GNUCXX OR APPLE)
  if(PROJECT_STATIC_RUNTIME)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++14 -static")
  else()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++14")
  endif()
endif()

SET (CMAKE_CXX_FLAGS_DEBUG "-g -D_DEBUG_")
SET (CMAKE_CXX_FLAGS_RELEASE "-O4 -DNDEBUG")

find_package(TMXLITE REQUIRED)
find_package(SDL2 REQUIRED)
find_package(SDL2_Image REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLM REQUIRED)

include_directories(
  ${TMXLITE_INCLUDE_DIR}
  ${SDL2_INCLUDE_DIR}
  ${SDL2_IMAGE_INCLUDE_DIR}
  ${OPENGL_INCLUDE_DIR}
  ${GLM_INCLUDE_DIR}
  src)

SET(PROJECT_DIR ${CMAKE_SOURCE_DIR}/src)
include(${PROJECT_DIR}/CMakeLists.txt)

if(WIN32)
  add_executable(${PROJECT_NAME} WIN32 ${PROJECT_SRC})
else()
  add_executable(${PROJECT_NAME} ${PROJECT_SRC})
endif()

target_link_libraries(${PROJECT_NAME}
  ${TMXLITE_LIBRARIES}
  ${SDL2_LIBRARY}
  ${SDL2_IMAGE_LIBRARIES}
  ${OPENGL_LIBRARIES})

#install executable
install(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION .)

#install game data
install(DIRECTORY assets
  DESTINATION .)