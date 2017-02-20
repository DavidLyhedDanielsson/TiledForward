cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_STANDARD 14)

set(BINARY_DIR ${CMAKE_CURRENT_LIST_DIR}/bin)
set(CONTENT_DIR ${CMAKE_CURRENT_LIST_DIR}/bin/content)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BINARY_DIR})

add_custom_target(
        glslCompile ALL
        glslangValidator
        ${CONTENT_DIR}/forward.vert
        ${CONTENT_DIR}/forward.frag
        ${CONTENT_DIR}/instance.vert
        ${CONTENT_DIR}/instance.frag
        ${CONTENT_DIR}/zPrepass.vert
        ${CONTENT_DIR}/tiles.comp
)

project(opengl)

set(DEBUG_BUILD TRUE)

if(DEBUG_BUILD)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -g")
    add_definitions(-DNDEBUG)
endif()
add_definitions(-DGL_ERROR_CALLBACK -DUSE_LOGGER -DGL_VERTEX_NO_WARNING -DGL_INDEX_NO_WARNING)

include_directories(/usr/include/freetype2/)

file(GLOB SOURCE_FILES *.h *.hpp *.cpp *.c console/*)
add_executable(opengl ${SOURCE_FILES})

target_link_libraries(opengl GLEW GL X11 pthread stdc++fs dl IL freetype assimp)
add_dependencies(opengl glslCompile)

if(DEBUG_BUILD)
    set(PNG_LIBRARY_NAME png_d)
else()
    set(PNG_LIBRARY_NAME png)
endif()

project(${PNG_LIBRARY_NAME})

add_library(${PNG_LIBRARY_NAME} SHARED pngTexture.cpp)
