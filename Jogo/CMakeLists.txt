
file(GLOB JOGO_SOURCES "*.cpp" "*.h")

add_library(Jogo STATIC ${JOGO_SOURCES})

target_include_directories(Jogo PUBLIC .)