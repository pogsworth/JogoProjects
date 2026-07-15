
add_executable(MMDC
    # Core Application
    mmdc.cpp

    # 2600 Core Files
    2600/2600.cpp
    2600/2600.h
    2600/tia.cpp
    2600/tia.h
    2600/vcs.h

    # 6502 CPU Submodule
    2600/6502/6502.cpp
    2600/6502/6502.h
)

# Tells VS to use the physical folder structure for the Solution Explorer tree
set_target_properties(MMDC PROPERTIES USE_FOLDERS ON)

# Crucial step: Tell CMake where to look for headers!
target_include_directories(MMDC PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/2600
    ${CMAKE_CURRENT_SOURCE_DIR}/2600/6502
)

target_link_libraries(MMDC PRIVATE Jogo)
