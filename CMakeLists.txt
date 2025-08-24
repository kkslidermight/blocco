cmake_minimum_required(VERSION 3.25)
project(blocco VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(${CMAKE_SOURCE_DIR}/cmake/PreventInSourceBuilds.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/CompilerWarnings.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/FetchSDL3.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/VulkanHelpers.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/Options.cmake)

add_subdirectory(shaders)
add_subdirectory(src)
add_subdirectory(tests)
