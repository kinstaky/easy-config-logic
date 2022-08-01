# include directory
set(PROJECT_INCLUDE_DIR, "${PROJECT_SOURCE_DIR}/include")


# test lexer
add_executable(
	test_lexer
	test_lexer.cpp
)
target_compile_features(
	test_lexer
	PRIVATE cxx_std_17
)
target_compile_options(
	test_lexer
	PRIVATE -Werror -Wall -Wextra
)
target_link_libraries(
	test_lexer
	PRIVATE gtest_main
	PRIVATE lexer
)


# test production
add_executable(
	test_production
	test_production.cpp
)
target_compile_features(
	test_production
	PRIVATE cxx_std_17
)
target_compile_options(
	test_production
	PRIVATE -Werror -Wall -Wextra
)
target_link_libraries(
	test_production
	PRIVATE gtest_main
	PRIVATE production
)


# test grammar
add_executable(
	test_grammar
	test_grammar.cpp
)
target_compile_features(
	test_grammar
	PRIVATE cxx_std_17
)
target_compile_options(
	test_grammar
	PRIVATE -Werror -Wall -Wextra
)
target_link_libraries(
	test_grammar
	PRIVATE gtest_main
	PRIVATE arithmetic_grammar
	PRIVATE logical_grammar 
	PRIVATE grammar 
	PRIVATE production
	PRIVATE token
)



# test syntax parser
add_executable(
	test_syntax_parser
	test_syntax_parser.cpp
)
target_compile_features(
	test_syntax_parser
	PRIVATE cxx_std_17
)
target_compile_options(
	test_syntax_parser
	PRIVATE -Werror -Wall -Wextra
)
target_link_libraries(
	test_syntax_parser
	PRIVATE gtest_main
	PRIVATE arithmetic_grammar
	PRIVATE logical_grammar 
	PRIVATE syntax_parser
)



# google test discover

include(GoogleTest)
gtest_discover_tests(test_lexer)
gtest_discover_tests(test_production)
gtest_discover_tests(test_grammar)
gtest_discover_tests(test_syntax_parser)

