project(glwrap-monkey)

include_directories(../../include)

link_libraries(sfml-window sfml-graphics)

set(CMAKE_CXX_FLAGS "-Wall -std=c++0x")

add_executable(glwrap-monkey monkey.cpp)
