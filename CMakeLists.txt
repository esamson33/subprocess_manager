cmake_minimum_required(VERSION 3.15)
project(SubprocessManager LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}")

find_package(Boost REQUIRED)
find_package(spdlog REQUIRED)
find_package(yaml-cpp REQUIRED)

#enable_testing()

add_subdirectory(src)
#add_subdirectory(test)