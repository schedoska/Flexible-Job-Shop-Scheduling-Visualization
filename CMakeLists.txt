cmake_minimum_required(VERSION 3.0)
project(example_project)

file(GLOB_RECURSE sources src/*.cpp src/*.h src/*.hpp)

# add_executable(example ${sources} ${data})
add_executable(example ${sources})

# target_compile_options(example PUBLIC -std=c++1y -Wall -Wfloat-conversion)

# this lets me include files relative to the root source directory with a <> pair
# target_include_directories(example PUBLIC src/main)

target_link_libraries(example PUBLIC
  -lsfml-graphics -lsfml-window -lsfml-system
)