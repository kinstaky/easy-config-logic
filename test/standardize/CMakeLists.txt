# test logic comparer
add_executable(
	test_standard_logic_tree
	test_standard_logic_tree.cpp
)
target_compile_features(
	test_standard_logic_tree
	PRIVATE cxx_std_17
)
target_compile_options(
	test_standard_logic_tree
	PRIVATE -Werror -Wall -Wextra
)
target_link_libraries(
	test_standard_logic_tree
	PRIVATE gtest_main
	PRIVATE standard_logic_tree lexer logical_grammar syntax_parser logic_comparer
)


# google test discover
include(GoogleTest)
gtest_discover_tests(test_standard_logic_tree)
