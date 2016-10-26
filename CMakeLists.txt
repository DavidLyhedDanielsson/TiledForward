cmake_minimum_required(VERSION 3.6)
project(opengl)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
add_definitions(-DGL_ERROR_CALLBACK -DUSE_LOGGER)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin)

file(GLOB SOURCE_FILES *.h *.hpp *.cpp)
add_executable(opengl ${SOURCE_FILES} glShader.cpp glShader.h glShaderProgram.cpp glShaderProgram.h glPixelShader.cpp glPixelShader.h glVertexShader.cpp glVertexShader.h)

target_link_libraries(opengl GLEW GL X11)