cmake_minimum_required(VERSION 2.8)
project(actrie)


set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

message("buile type: " ${CMAKE_BUILD_TYPE})

if (UNIX)
    message("compile for unix-like OS, include OS X and Cygwin")
    add_compile_options(-fPIC)
    if (NOT CMAKE_BUILD_TYPE)
        message("not defined CMAKE_BUILD_TYPE, use -O2")
        add_compile_options(-O2 -Wall)
    endif ()
    add_compile_options(-g)
elseif (MSVC)
    message("compile with MSVC")
	add_compile_options(/utf-8 /NODEFAULTLIB:library)
else ()
    add_compile_options(-fPIC)
    add_compile_options(-Wall)
    add_compile_options(-g)
endif ()

set(CMAKE_C_STANDARD 99)

# gprof
option(ENABLE_PROFILE "Enable compile with profile information." OFF)
if (ENABLE_PROFILE)
    message("generate profile")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
    set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")
endif ()

if (NOT (DEFINED alib_DIR))
    set(alib_DIR "${PROJECT_SOURCE_DIR}/../alib")
endif ()
add_subdirectory(${alib_DIR} alib)

#add_custom_command(OUTPUT src/lr_table.h
#        python utils/LRAnalyzer.py ARGS --source="utils/productions.syntax" --output="src/lr_table.h"
#        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

set(actrie_HEADER_FILES
#        src/configure.h
#        include/dict.h
#        include/matcher.h
        src/vocab.h
#        src/dict0.h
#        src/actrie.h
#        src/matcher0.h
#        src/acdat.h
#        src/mdimap.h
#        src/disambi.h
#        src/distance.h
        src/tokenizer.h
        src/parser.h
        src/lr_table.h
        src/pattern.h)

set(actrie_SOURCE_FILES
#        src/configure.c
        src/vocab.c
#        src/dict.c
#        src/actrie.c
#        src/acdat.c
#        src/mdimap.c
#        src/disambi.c
#        src/distance.c
#        src/matcher.c
        src/tokenizer.c
        src/parser.c
        src/pattern.c)

add_library(actrie STATIC
        ${actrie_HEADER_FILES} ${actrie_SOURCE_FILES})
target_include_directories(actrie
        PUBLIC include)
target_link_libraries(actrie
        PUBLIC alib)
#        PUBLIC pcre)
target_compile_definitions(actrie
        PUBLIC -DPCRE_STATIC) # we link pcre static lib, this is must in windows

# python interface
option (ENABLE_PYTHON "Enable compile python interface." OFF)
if (ENABLE_PYTHON)
    add_subdirectory(actrie)
endif ()

add_subdirectory(tests)
