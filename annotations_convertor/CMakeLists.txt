cmake_minimum_required(VERSION 3.1.0)

project(AnnotationsConverter)

set(CMAKE_CXX_STANDARD 17)

find_package(Boost REQUIRED system)

add_executable(${PROJECT_NAME}
   AnnotationsConverter.cpp
   AnnotationsConverter.hpp
   main.cpp)

target_link_libraries(${PROJECT_NAME}
   stdc++fs)


