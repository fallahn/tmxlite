project(tmxlite_parse_test)
SET(PROJECT_NAME tmxlite_parse_test)
cmake_minimum_required(VERSION 2.8)

if(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")
SET(PROJECT_STATIC_RUNTIME FALSE CACHE BOOL "Use statically linked standard/runtime libraries?")
SET(PAUSE_AT_END TRUE CACHE BOOL "Wait for user input after tests have finished running")
#SET(PROJECT_STATIC_TMX FALSE CACHE BOOL "Use statically linked tmxlite library?")

if(CMAKE_COMPILER_IS_GNUCXX OR APPLE)
  if(PROJECT_STATIC_RUNTIME)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++14 -static")
  else()
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++14")
  endif()
endif()

SET(ASSETS_PATH "${CMAKE_CURRENT_SOURCE_DIR}")
configure_file(src/config.h.in "${CMAKE_CURRENT_BINARY_DIR}/config.h" @ONLY)

SET (CMAKE_CXX_FLAGS_DEBUG "-g -D_DEBUG_")
SET (CMAKE_CXX_FLAGS_RELEASE "-O4 -DNDEBUG")

find_package(TMXLITE REQUIRED)

include_directories(${TMXLITE_INCLUDE_DIR})

set(PROJECT_SRC src/main.cpp)

if(WIN32)
  add_executable(${PROJECT_NAME} WIN32 ${PROJECT_SRC})
else()
  add_executable(${PROJECT_NAME} ${PROJECT_SRC})
endif()

target_include_directories(${PROJECT_NAME}
  ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(${PROJECT_NAME}
  ${TMXLITE_LIBRARIES})

#install executable
install(TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION .)

#install game data
install(DIRECTORY assets
  DESTINATION .)
