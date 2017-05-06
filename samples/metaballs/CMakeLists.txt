cmake_minimum_required(VERSION 2.6)

set(project_name glwrap-metaballs)
project(${project_name})

# why do I need this?!!
set(CMAKE_MODULE_PATH /usr/share/apps/cmake/modules)

include_directories(../../include)

find_package(SFML 2.0 COMPONENTS system window graphics REQUIRED)
include_directories(${SFML_INCLUDE_DIR})

set(CMAKE_CXX_FLAGS "-Wall -std=c++0x")

add_executable(${project_name} metaballs.cpp)

target_link_libraries(${project_name} ${SFML_LIBRARIES} GLEW glut GL)
