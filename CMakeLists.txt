cmake_minimum_required(VERSION 2.8)
project(actrie)


set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

message("buile type: " ${CMAKE_BUILD_TYPE})

IF (WIN32)
    MESSAGE("compile for win32")
ELSEIF (UNIX)
    MESSAGE("compile for unix-like OS, include OS X and Cygwin")
    add_compile_options(-fPIC)
    IF (NOT CMAKE_BUILD_TYPE)
        message("not defined CMAKE_BUILD_TYPE, use -O2")
        add_compile_options(-O2)
    ENDIF ()
ENDIF ()


set(CMAKE_C_FLAGS -std=c99)


set(HEADER_FILES
        include/common.h
        include/dict.h
        include/matcher.h
        src/utf8.h
        src/stream.h
        src/dynabuf.h
        src/vocab.h
        src/dict0.h
        src/actrie.h
        src/avl.h
        src/matcher0.h
        src/acdat.h
#        src/disambi.h
        src/distance.h)

set(SOURCE_FILES
        src/common.c
        src/utf8.c
        src/stream.c
        src/dynabuf.c
        src/vocab.c
        src/dict.c
        src/avl.c
        src/actrie.c
        src/acdat.c
#        src/disambi.c
        src/distance.c
        src/matcher.c)

add_library(actrie STATIC
        ${HEADER_FILES} ${SOURCE_FILES})
target_include_directories(actrie
        PUBLIC include)
target_compile_definitions(actrie
        PUBLIC "-DAVL_USER_INDEX=1")


add_subdirectory(tests)
