cmake_minimum_required(VERSION 2.8)


add_executable(testactrie test_actrie.c)
target_link_libraries(testactrie
        PRIVATE actrie)


add_executable(testa test.c)
target_link_libraries(testa
        PUBLIC actrie)
target_compile_options(testa
        PRIVATE -O2)

