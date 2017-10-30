if (NOT (DEFINED PythonVersion))
    set(PythonVersion 3.6)
endif ()


set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

message("build for python")

set(HEADER_FILES
        )

set(SOURCE_FILES
        src/wrap.c)

add_library(match SHARED
        ${HEADER_FILES} ${SOURCE_FILES})
set_target_properties(match PROPERTIES PREFIX "")
target_link_libraries(match
        PRIVATE actrie)
target_include_directories(match
        PRIVATE /usr/include/python2.7
        PRIVATE /usr/lib/python2.7/config)
target_compile_options(match
        PRIVATE -fPIC)

