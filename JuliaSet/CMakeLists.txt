
add_executable(JuliaSet JuliaSet.cpp)
set_target_properties(JuliaSet PROPERTIES
    DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)
target_link_libraries(JuliaSet PRIVATE Jogo)
