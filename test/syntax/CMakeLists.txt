# test logic comparer
add_executable(test_logic_comparer test_logic_comparer.cpp)
target_link_libraries(test_logic_comparer PRIVATE gtest_main logic_comparer)


# google test discover
gtest_discover_tests(test_logic_comparer)

# add parser tests
add_subdirectory(parser)