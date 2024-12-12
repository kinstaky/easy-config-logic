# include directory
set(PROJECT_INCLUDE_DIR, "${PROJECT_SOURCE_DIR}/include")

# test lexer
add_executable(test_lexer test_lexer.cpp)
target_link_libraries(test_lexer PRIVATE gtest_main lexer)

# test production
add_executable(test_production test_production.cpp)
target_link_libraries(test_production PRIVATE gtest_main production)

# test grammar
add_executable(test_grammar test_grammar.cpp)
target_link_libraries(
	test_grammar
	PRIVATE gtest_main
	arithmetic_grammar logical_grammar grammar
	production token
)

# test syntax parser
add_executable(test_syntax_parser test_syntax_parser.cpp)
target_link_libraries(
	test_syntax_parser
	PRIVATE gtest_main arithmetic_grammar logical_grammar  syntax_parser
)

# google test discover
gtest_discover_tests(test_lexer)
gtest_discover_tests(test_production)
gtest_discover_tests(test_grammar)
gtest_discover_tests(test_syntax_parser)