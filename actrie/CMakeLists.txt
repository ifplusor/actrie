cmake_minimum_required(VERSION 2.8)

if(NOT (DEFINED PythonVersion))
  set(PythonVersion 3)
endif()

find_package(PythonLibs ${PythonVersion} EXACT REQUIRED)
if(NOT ${PYTHON_FOUND})
  message(FATAL_ERROR "not found python lib!")
endif()
message(STATUS "PYTHON_INCLUDE_DIR: ${PYTHON_INCLUDE_DIR}")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

message("build for python")

set(HEADER_FILES)

set(SOURCE_FILES src/wrap.c)

add_library(_actrie SHARED ${HEADER_FILES} ${SOURCE_FILES})
set_target_properties(_actrie PROPERTIES PREFIX "")
target_link_libraries(_actrie PRIVATE actrie)
target_include_directories(_actrie PRIVATE ${PYTHON_INCLUDE_DIR})
if(UNIX)
  target_compile_options(_actrie PRIVATE -fPIC)
endif()
