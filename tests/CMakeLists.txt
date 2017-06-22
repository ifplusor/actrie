cmake_minimum_required(VERSION 2.8)


add_executable(testa test.c)
target_link_libraries(testa
        PUBLIC actrie)

