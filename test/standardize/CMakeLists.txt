# test logic comparer
add_executable(test_standard_logic_tree test_standard_logic_tree.cpp)
target_link_libraries(
	test_standard_logic_tree
	PRIVATE gtest_main
	standard_logic_tree lexer logical_grammar syntax_parser logic_comparer
)

# test logic downscale tree
add_executable(
	test_standard_logic_downscale_tree
	test_standard_logic_downscale_tree.cpp
)
target_link_libraries(
	test_standard_logic_downscale_tree
	PRIVATE gtest_main
	standard_logic_downscale_tree lexer logic_downscale_grammar syntax_parser
)


# google test discover
gtest_discover_tests(test_standard_logic_tree)
gtest_discover_tests(test_standard_logic_downscale_tree)
