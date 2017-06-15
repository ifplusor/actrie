cmake_minimum_required(VERSION 2.8)


add_executable(testactrie test_actrie.c)
target_link_libraries(testactrie
        PRIVATE actrie)


add_executable(testacdat test_acdat.c)
target_link_libraries(testacdat
        PRIVATE actrie)

