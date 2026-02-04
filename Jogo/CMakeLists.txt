
file(GLOB JOGO_SOURCES "*.cpp" "*.h")

add_library(Jogo STATIC ${JOGO_SOURCES})

target_link_libraries(Jogo PUBLIC winmm)

target_include_directories(Jogo PUBLIC .)