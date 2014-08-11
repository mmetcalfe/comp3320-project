CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(comp3320_project)

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
FIND_PACKAGE(GLM REQUIRED)

# Include directories
INCLUDE_DIRECTORIES(
    "src/utility"
    "src/NUGL"
    )

#FILE(GLOB_RECURSE inc_files
#    "util/**.h"
#    )

FILE(GLOB src_files
      "src/**.cpp" ,
    )

# Add Executables
SET(game_src ${inc_files} ${src_files})
ADD_EXECUTABLE (game ${game_src})
SET_TARGET_PROPERTIES(game PROPERTIES LINKER_LANGUAGE CXX)
TARGET_LINK_LIBRARIES(game
    ${OPENGL_gl_LIBRARY}
    ${GLEW_LIBRARY}
    ${GLFW_LIBRARIES}
#    png jpeg assimp m boost_system boost_filesystem
    )
