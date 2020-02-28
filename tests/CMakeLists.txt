cmake_minimum_required(VERSION 2.8)

link_libraries(actrie)

add_executable(test_trie test_trie.c)
add_executable(test_avl test_avl.c)
add_executable(test_parser test_parser.c)
add_executable(test_matcher test_matcher.c)
