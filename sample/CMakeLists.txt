project(glwrap-sample)

include_directories(../include)

link_libraries(sfml-window sfml-graphics)

set(CMAKE_CXX_FLAGS "-Wall -std=c++0x")

add_executable(glwrap-sample sample.cpp)
