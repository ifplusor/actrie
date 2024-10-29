cmake_minimum_required(VERSION 3.5)
project(actrie)

set(CMAKE_EXECUTABLE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
if(MSVC)
  set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/lib)
endif()

message("Generated with config types: ${CMAKE_CONFIGURATION_TYPES}")
message("Buile type: ${CMAKE_BUILD_TYPE}")

set(CMAKE_C_STANDARD 11)

# gprof
option(ENABLE_PROFILE "Enable compile with profile information." OFF)
if(ENABLE_PROFILE)
  message("generate profile")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")
endif()

# alib
add_subdirectory(${PROJECT_SOURCE_DIR}/deps/alib alib)

option(GENERATE_LRTABLE "Generate LR table from productions." OFF)
if(GENERATE_LRTABLE)
  add_custom_command(
    OUTPUT src/lr_table.h
           python
           utils/LRAnalyzer.py
           ARGS
           --source="utils/productions.syntax"
           --output="src/parser/lr_table.h"
           --priority="utils/priority.syntax"
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endif()

set(actrie_HEADER_FILES
    include/actrie/config.h
    include/actrie/matcher.h
    include/actrie/utf8ctx.h
    include/actrie/utf8helper.h
    src/vocab.h
    src/pattern.h
    src/parser/lr_reduce.h
    src/parser/lr_table.h
    src/parser/tokenizer.h
    src/parser/parser.h
    src/reglet/context.h
    src/reglet/engine.h
    src/reglet/expr/expr0.h
    src/reglet/expr/pass.h
    src/reglet/expr/text.h
    src/reglet/expr/ambi.h
    src/reglet/expr/anto.h
    src/reglet/expr/dist.h
    src/reglet/expr/expr.h
    src/trie/actrie.h
    src/trie/acdat.h)

set(actrie_SOURCE_FILES
    src/vocab.c
    src/pattern.c
    src/parser/tokenizer.c
    src/parser/parser.c
    src/reglet/engine.c
    src/reglet/expr/pass.c
    src/reglet/expr/text.c
    src/reglet/expr/ambi.c
    src/reglet/expr/anto.c
    src/reglet/expr/dist.c
    src/trie/actrie.c
    src/trie/acdat.c
    src/config.c
    src/matcher.c
    src/utf8ctx.c
    src/utf8helper.c)

add_library(actrie STATIC ${actrie_HEADER_FILES} ${actrie_SOURCE_FILES})
target_include_directories(actrie PUBLIC include)
target_link_libraries(actrie PUBLIC alib)

if(MSVC)
  target_compile_options(actrie PRIVATE /utf-8)
else()
  target_compile_options(actrie PRIVATE -Wall -fPIC -fno-strict-aliasing)
endif()

add_subdirectory(tests)
