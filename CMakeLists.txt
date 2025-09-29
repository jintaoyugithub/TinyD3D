cmake_minimum_required(VERSION 4.1)

set(SAMPLES_BUILD_OPTIONS "" CACHE STRING "List of samples to build")

if (NOT SAMPLES_BUILD_OPTIONS)
    message (FATAL_ERROR "At least select one sample to build!")
endif()

foreach(sample IN LISTS SAMPLES_BUILD_OPTIONS)
endforeach()

project(
    TinyD3D
    VERSION 1.0.0
    LANGUAGES C CXX
)

add_subdirectory(samples)