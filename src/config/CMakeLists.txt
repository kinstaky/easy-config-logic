# include directory
set(PROJECT_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set(CONFIG_INCLUDE_DIR "${PROJECT_INCLUDE_DIR}/config")

# logic_parser library
add_library(
	logic_parser STATIC
	logic_parser.cpp "${CONFIG_INCLUDE_DIR}/logic_parser.h"
)
target_include_directories(
	logic_parser
	PUBLIC "${PROJECT_INCLUDE_DIR}"
)
target_compile_features(
	logic_parser
	PRIVATE cxx_std_17
)
target_compile_options(
	logic_parser
	PRIVATE -Werror -Wall -Wextra
)
target_link_libraries(
	logic_parser
	PUBLIC standard_logic_tree lexer syntax_parser logical_grammar
)


# memory_config library
add_library(
	memory_config STATIC
	memory_config.cpp "${CONFIG_INCLUDE_DIR}/memory_config.h"
)
target_include_directories(
	memory_config
	PUBLIC "${PROJECT_INCLUDE_DIR}"
)
target_compile_features(
	memory_config
	PRIVATE cxx_std_17
)
target_compile_options(
	memory_config
	PRIVATE -Werror -Wall -Wextra
)
target_link_libraries(
	memory_config
	PUBLIC logic_parser
)