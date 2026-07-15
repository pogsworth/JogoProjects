if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds are not allowed. Please create a build folder (e.g., cmake -B build) and run from there.")
endif()

cmake_minimum_required(VERSION 3.31)
project(JogoProjects LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(WORKSPACE_ROOT="${CMAKE_SOURCE_DIR}/")

add_subdirectory(Jogo)
add_subdirectory(Horizon)
add_subdirectory(JuliaSet)
add_subdirectory(MMDC)
add_subdirectory(Tetris)
add_subdirectory(UIExample)
