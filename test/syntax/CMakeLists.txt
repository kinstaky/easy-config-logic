# test logic comparer
add_executable(
	test_logic_comparer
	test_logic_comparer.cpp
)
target_compile_features(
	test_logic_comparer
	PRIVATE cxx_std_17
)
target_compile_options(
	test_logic_comparer
	PRIVATE -Werror -Wall -Wextra
)
target_link_libraries(
	test_logic_comparer
	PRIVATE gtest_main
	PRIVATE logic_comparer
)


# google test discover
include(GoogleTest)
gtest_discover_tests(test_logic_comparer)

# add parser tests
add_subdirectory(parser)