cmake_minimum_required(VERSION 3.6)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)

project(obj)

add_library(obj STATIC libobj.cpp libobj.h)

project(opengl)

set(DEBUG_BUILD TRUE)

if(DEBUG_BUILD)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
    add_definitions(-DNDEBUG)
endif()
add_definitions(-DGL_ERROR_CALLBACK -DUSE_LOGGER)

include_directories(/usr/include/freetype2/)

file(GLOB SOURCE_FILES *.h *.hpp *.cpp console/*)
add_executable(opengl ${SOURCE_FILES})

target_link_libraries(opengl GLEW GL X11 pthread stdc++fs dl IL freetype ${CMAKE_CURRENT_LIST_DIR}/bin/libobj.a)

if(DEBUG_BUILD)
    set(PNG_LIBRARY_NAME png_d)
else()
    set(PNG_LIBRARY_NAME png)
endif()

project(${PNG_LIBRARY_NAME})

add_library(${PNG_LIBRARY_NAME} SHARED pngTexture.cpp)