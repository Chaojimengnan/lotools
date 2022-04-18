cmake_minimum_required(VERSION 3.5)

set(VCPKG_MANIFEST_DIR ${CMAKE_SOURCE_DIR}/vcpkg)

project(lotools)

if (NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    message(STATUS "lotools: No standard set, automatically set to C++17")
endif()

if(CMAKE_CXX_STANDARD LESS 17)
    message(FATAL_ERROR "lotools: The library requires at least C++17!")
endif()

option(LO_TOOLS_BUILD_TEST "Build tests" OFF)


add_executable(${PROJECT_NAME} src/main.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE include)


# if(LO_TOOLS_BUILD_TEST)
#     message(STATUS "lo-tools: Generating tests")
#     enable_testing()
#     add_subdirectory(tests)
# endif()