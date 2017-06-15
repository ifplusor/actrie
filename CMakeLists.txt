cmake_minimum_required(VERSION 2.8)
project(actrie)

set(CMAKE_C_STANDARD 99)


add_library(actrie STATIC
        src/dict.c src/actrie.c src/acdat.c)
target_include_directories(actrie
        PUBLIC include)
target_compile_options(actrie
        PRIVATE -O2 -fPIC)


add_subdirectory(tests)
