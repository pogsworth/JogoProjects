
add_executable(MMDC
    mmdc.cpp

    2600/2600.cpp
    2600/2600.h
    2600/tia.cpp
    2600/tia.h
    2600/vcs.h

    2600/6502/6502.cpp
    2600/6502/6502.h
)

set_target_properties(MMDC PROPERTIES
    DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
)

target_include_directories(MMDC PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/2600
    ${CMAKE_CURRENT_SOURCE_DIR}/2600/6502
)

target_link_libraries(MMDC PRIVATE Jogo)
