cmake_minimum_required(VERSION 3.0)
project(fjssp_visualization)

file(GLOB_RECURSE sources src/*.cpp src/*.h src/*.hpp)

add_executable(fjssp_visualization ${sources})

# target_compile_options(example PUBLIC -std=c++1y -Wall -Wfloat-conversion)
# target_include_directories(example PUBLIC src/main)

target_link_libraries(fjssp_visualization PUBLIC
  -lsfml-graphics -lsfml-window -lsfml-system
)