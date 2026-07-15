
add_executable(Horizon Horizon.cpp )

set_target_properties(Horizon PROPERTIES
    DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(Horizon PRIVATE Jogo)
