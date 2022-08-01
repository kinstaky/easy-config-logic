# compare
add_executable(
	compare
	compare_logical_expression.cpp
)
target_compile_features(
	compare
	PRIVATE cxx_std_17
)
target_compile_options(
	compare
	PRIVATE -Werror -Wall -Wextra
)
target_link_libraries(
	compare
	PRIVATE logic_comparer
)

# standardize
add_executable(
	standardize
	standard_logic_adapter.cpp
)
target_compile_features(
	standardize
	PRIVATE cxx_std_17
)
target_compile_options(
	standardize
	PRIVATE -Werror -Wall -Wextra
)
target_link_libraries(
	standardize
	PRIVATE standard_logic_tree logical_grammar syntax_parser lexer
)


# convert
add_executable(
	convert
	convert.cpp
)
target_compile_features(
	convert
	PRIVATE cxx_std_17
)
target_compile_options(
	convert
	PRIVATE -Werror -Wall -Wextra
)
target_link_libraries(
	convert
	PRIVATE memory_config logic_parser
)

# config
add_executable(
	config
	config.cpp
)
target_compile_features(
	config
	PRIVATE cxx_std_17
)
target_compile_options(
	config
	PRIVATE -Werror -Wall -Wextra
)
target_link_libraries(
	config
	PRIVATE memory_config logic_parser
)

# logic test
add_executable(
	logic_test
	logic_test.cpp
)
target_compile_features(
	logic_test
	PRIVATE cxx_std_17
)
target_compile_options(
	logic_test
	PRIVATE -Werror -Wall -Wextra
)
target_link_libraries(
	logic_test
	PRIVATE memory_config logic_parser
)

