cmake_minimum_required(VERSION 2.8)
project(actrie)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

message("buile type: " ${CMAKE_BUILD_TYPE})

if(UNIX)
  message("compile for unix-like OS, include OS X and Cygwin")
  add_compile_options(-Wall -fPIC -fno-strict-aliasing)
  if(NOT CMAKE_BUILD_TYPE)
    message("not defined CMAKE_BUILD_TYPE, use -O2")
    add_compile_options(-O2)
  endif()
elseif(MSVC)
  message("compile with MSVC")
  add_compile_options(/utf-8 /NODEFAULTLIB:library)
else()
  add_compile_options(-Wall -fPIC -fno-strict-aliasing)
endif()

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

option(IMPORT_ALIB "Enable import alib library." ON)
if(NOT (DEFINED alib_DIR))
  set(alib_DIR "${PROJECT_SOURCE_DIR}/deps/alib")
endif()
if(IMPORT_ALIB)
  add_subdirectory(${alib_DIR} alib)
else()
  include_directories(${alib_DIR}/include)
  link_directories(${alib_DIR}/lib)
endif()

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
    include/matcher.h
    include/utf8helper.h
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
    src/matcher.c
    src/utf8helper.c)

add_library(actrie STATIC ${actrie_HEADER_FILES} ${actrie_SOURCE_FILES})
target_include_directories(actrie PUBLIC include)
target_link_libraries(actrie PUBLIC alib)

# python interface
option(ENABLE_PYTHON "Enable compile python interface." OFF)
if(ENABLE_PYTHON)
  add_subdirectory(actrie)
endif()

add_subdirectory(tests)
