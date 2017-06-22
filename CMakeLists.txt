cmake_minimum_required(VERSION 2.8)
project(actrie)


set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)


IF (WIN32)
    MESSAGE("compile for win32")
ELSEIF (UNIX)
    MESSAGE("compile for unix-like OS, include OS X and Cygwin")
    add_compile_options(-O2 -fPIC)
ENDIF ()


set(CMAKE_C_STANDARD 99)


add_library(actrie STATIC
        src/common.c src/dict.c src/actrie.c src/acdat.c src/matcher.c)
target_include_directories(actrie
        PUBLIC include)


add_executable(testb src/distance.c)
target_link_libraries(testb
        PUBLIC actrie)


add_subdirectory(tests)
