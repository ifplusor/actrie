cmake_minimum_required(VERSION 3.7)
project(actrie)


add_library(actrie SHARED)
target_include_directories(actrie
        PUBLIC include)
target_sources(actrie
        PRIVATE src/common.c src/actrie.c src/acdat.c)
target_compile_options(actrie
        PRIVATE -O2)
target_compile_definitions(actrie
        PRIVATE BINSCH)


add_executable(testactrie testActrie.c)
add_dependencies(testactrie actrie)
target_include_directories(testactrie
        PRIVATE include)
target_link_libraries(testactrie
        PRIVATE -L${PROJECT_BINARY_DIR}
        PRIVATE libactrie.so)


add_executable(testacdat testAcdat.c)
add_dependencies(testacdat actrie)
target_include_directories(testacdat
        PRIVATE include)
target_link_libraries(testacdat
        PRIVATE -L${PROJECT_BINARY_DIR}
        PRIVATE libactrie.so)


