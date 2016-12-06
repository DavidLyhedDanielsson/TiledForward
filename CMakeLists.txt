cmake_minimum_required(VERSION 3.6)
project(opengl)

set(CMAKE_CXX_STANDARD 11)

set(DEBUG_BUILD TRUE)

if(DEBUG_BUILD)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
    add_definitions(-DNDEBUG)
endif()
add_definitions(-DGL_ERROR_CALLBACK -DUSE_LOGGER)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${CMAKE_CURRENT_LIST_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  ${CMAKE_CURRENT_LIST_DIR}/bin)

file(GLOB SOURCE_FILES *.h *.hpp *.cpp)
add_executable(opengl ${SOURCE_FILES})

target_link_libraries(opengl GLEW GL X11 pthread stdc++fs dl IL)

if(DEBUG_BUILD)
    set(PNG_LIBRARY_NAME png_d)
else()
    set(PNG_LIBRARY_NAME png)
endif()

project(${PNG_LIBRARY_NAME})

add_library(${PNG_LIBRARY_NAME} SHARED pngTexture.cpp)