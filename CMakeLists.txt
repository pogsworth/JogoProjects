if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
    message(FATAL_ERROR "In-source builds are not allowed. Please create a build folder (e.g., cmake -B build) and run from there.")
endif()

cmake_minimum_required(VERSION 4.2.1)
project(JogoProjects LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(Jogo)

file(GLOB children RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *)

set(SKIP_LIST "MyCommonLib" "build" ".git" "Jogo")

foreach(child ${children})
    list(FIND SKIP_LIST "${child}" IS_SKIPPED)

    if(NOT IS_SKIPPED EQUAL -1)
        continue()
    endif()

	if(IS_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${child})
		file(GLOB_RECURSE CHILD_SOURCES "${child}/*.cpp" "${child}/*.h")
		
		if(CHILD_SOURCES)
			add_executable(${child} ${CHILD_SOURCES})
			target_link_libraries(${child} PRIVATE Jogo)
			message(STATUS "Found executable: ${child}")
		endif()
	endif()
endforeach()