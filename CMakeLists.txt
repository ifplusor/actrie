cmake_minimum_required(VERSION 2.8)
project(actrie)

set(CMAKE_C_FLAGS -std=gnu99)


add_library(actrie STATIC
        src/dict.c src/actrie.c src/acdat.c)
target_include_directories(actrie
        PUBLIC include)
target_compile_options(actrie
        PRIVATE -O2 -fPIC)


add_executable(testactrie src/tests/test_actrie.c)
add_dependencies(testactrie actrie)
target_include_directories(testactrie
        PRIVATE include)
target_link_libraries(testactrie
        PRIVATE actrie)


add_executable(testacdat src/tests/test_acdat.c)
add_dependencies(testacdat actrie)
target_include_directories(testacdat
        PRIVATE include)
target_link_libraries(testacdat
        PRIVATE actrie)

