
add_executable(UIExample UIExample.cpp)
set_target_properties(UIExample PROPERTIES
    DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)
target_link_libraries(UIExample PRIVATE Jogo)
