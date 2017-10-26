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
        add_compile_options(-O2 -Wall)
    ENDIF ()
ENDIF ()

set(CMAKE_C_STANDARD 99)

if (GenProfile)
    MESSAGE("generate profile")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
    SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")
endif ()

if (NOT (DEFINED ALIB_DIR))
    set(ALIB_DIR "${PROJECT_SOURCE_DIR}/../alib")
endif()
add_subdirectory(${ALIB_DIR} alib)

set(ACTRIE_HEADER_FILES
        src/configure.h
        include/dict.h
        include/matcher.h
        src/vocab.h
        src/dict0.h
        src/actrie.h
        src/matcher0.h
        src/acdat.h
        src/mdimap.h
        src/disambi.h
        src/distance.h)

set(ACTRIE_SOURCE_FILES
        src/configure.c
        src/vocab.c
        src/dict.c
        src/actrie.c
        src/acdat.c
        src/mdimap.c
        src/disambi.c
        src/distance.c
        src/matcher.c)

add_library(actrie STATIC
        ${ACTRIE_HEADER_FILES} ${ACTRIE_SOURCE_FILES})
target_include_directories(actrie
        PUBLIC include)
target_link_libraries(actrie
        PUBLIC alib)

add_subdirectory(tests)

set(ALIB_SOURCE_FILES
        ${ALIB_DIR}/src/amem.c
        ${ALIB_DIR}/src/aobj.c
        ${ALIB_DIR}/src/astr.c
        ${ALIB_DIR}/src/dynastr.c
        ${ALIB_DIR}/src/acom.c
        ${ALIB_DIR}/src/list.c
        ${ALIB_DIR}/src/dlnk.c
        ${ALIB_DIR}/src/dynapool.c
        ${ALIB_DIR}/src/avl.c
        ${ALIB_DIR}/src/stream.c
        ${ALIB_DIR}/src/dynabuf.c
        ${ALIB_DIR}/src/utf8.c)

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
add_executable(test_profile
        ${ALIB_SOURCE_FILES}
        ${ACTRIE_SOURCE_FILES}
        tests/test_dist.c)
target_include_directories(test_profile
        PRIVATE ${ALIB_DIR}/include
        PRIVATE include)
