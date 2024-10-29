cmake_minimum_required(VERSION 3.5)

if(MSVC)
  add_compile_options(/utf-8)
else()
  add_compile_options(-Wall -fPIC -fno-strict-aliasing)
endif()

link_libraries(actrie)

add_executable(test_trie test_trie.c)
add_executable(test_avl test_avl.c)
add_executable(test_parser test_parser.c)
add_executable(test_matcher test_matcher.c)
