
add_executable(Tetris Tetris.cpp)
set_target_properties(Tetris PROPERTIES
    DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)
target_link_libraries(Tetris PRIVATE Jogo)
