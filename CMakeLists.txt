cmake_minimum_required(VERSION 2.8)
project(actrie)


IF (WIN32)
	MESSAGE("compile for win32")
ELSEIF (UNIX)
	MESSAGE("compile for unix-like OS, include OS X and Cygwin")
	SET(MY_COMPILE_OPTIONS -O2 -fPIC)
ENDIF ()


set(CMAKE_C_STANDARD 99)


add_library(actrie STATIC
        src/dict.c src/actrie.c src/acdat.c)
target_include_directories(actrie
        PUBLIC include)
IF (MY_COMPILE_OPTIONS)
target_compile_options(actrie
        PRIVATE ${MY_COMPILE_OPTIONS})
ENDIF ()


add_subdirectory(tests)
