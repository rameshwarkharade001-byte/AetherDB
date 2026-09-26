cmake_minimum_required(VERSION 3.16)
project(AetherDB VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -Wextra -Werror")

include_directories(include)

add_executable(aether_bench src/main.cpp)
target_link_libraries(aether_bench PRIVATE pthread)

add_executable(aether_tests tests/test_engine.cpp)
target_link_libraries(aether_tests PRIVATE pthread)
