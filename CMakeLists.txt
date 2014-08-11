cmake_minimum_required (VERSION 2.6)
project (comp3320_project)

SET(VERSION_MAJOR 0)
SET(VERSION_MINOR 1)
SET(VERSION_PATCH 0)

# Tell CMake where to find our CMake modules
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

# add_subdirectory (src)

# Compiler settings
SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -g -O0 -Wpedantic -Wextra -Wall -std=c++11" )

# Shared libraries
FIND_PACKAGE(OpenGL REQUIRED)
FIND_PACKAGE(GLEW REQUIRED)
FIND_PACKAGE(GLFW REQUIRED)

FILE(GLOB_RECURSE inc_files
    "**.h"
    )

# FILE(GLOB src_files
#      "src/*.cpp" ,
#      )

SET(exec_resources
    ${inc_files}
    ${src_files}
    )

# Add Executables
set(game_src "src/program.cpp" ${exec_resources})
add_executable (game ${game_src})
SET_TARGET_PROPERTIES(game PROPERTIES LINKER_LANGUAGE CXX)
# target_link_libraries(game ${OPENGL} glfw3 glew png jpeg assimp m boost_system boost_filesystem)
