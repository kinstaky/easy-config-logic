# include directory
set(PROJECT_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set(SYNTAX_INCLUDE_DIR "${PROJECT_INCLUDE_DIR}/syntax")

# logical grammar library
add_library(
	logical_grammar STATIC
	logical_grammar.cpp "${SYNTAX_INCLUDE_DIR}/logical_grammar.h"
)
target_include_directories(
	logical_grammar
	PUBLIC "${PROJECT_INCLUDE_DIR}"
)
target_compile_features(
	logical_grammar
	PRIVATE cxx_std_17
)
target_compile_options(
	logical_grammar
	PRIVATE -Werror -Wall -Wextra
)
target_link_libraries(
	logical_grammar
	PUBLIC token
	PUBLIC production
	PUBLIC grammar
)

# logic-downscale grammar library
add_library(
	logic_downscale_grammar STATIC
	logic_downscale_grammar.cpp "${SYNTAX_INCLUDE_DIR}/logic_downscale_grammar.h"
)
target_include_directories(
	logic_downscale_grammar
	PUBLIC "${PROJECT_INCLUDE_DIR}"
)
target_compile_features(
	logic_downscale_grammar
	PRIVATE cxx_std_17
)
target_compile_options(
	logic_downscale_grammar
	PRIVATE -Werror -Wall -Wextra
)
target_link_libraries(
	logic_downscale_grammar
	PUBLIC token
	PUBLIC production
	PUBLIC grammar
)

# arithmetic grammar library
add_library(
	arithmetic_grammar STATIC
	arithmetic_grammar.cpp "${SYNTAX_INCLUDE_DIR}/arithmetic_grammar.h"
)
target_include_directories(
	arithmetic_grammar
	PUBLIC "${PROJECT_INCLUDE_DIR}"
)
target_compile_features(
	arithmetic_grammar
	PRIVATE cxx_std_17
)
target_compile_options(
	arithmetic_grammar
	PRIVATE -Werror -Wall -Wextra
)
target_link_libraries(
	arithmetic_grammar
	PUBLIC token
	PUBLIC production
	PUBLIC grammar
)

# logical compare library
add_library(
	logic_comparer STATIC
	logic_comparer.cpp "${SYNTAX_INCLUDE_DIR}/logic_comparer.h"
)
target_include_directories(
	logic_comparer
	PUBLIC "${PROJECT_INCLUDE_DIR}"
)
target_compile_features(
	logic_comparer
	PRIVATE cxx_std_17
)
target_compile_options(
	logic_comparer
	PRIVATE -Werror -Wall -Wextra
)
target_link_libraries(
	logic_comparer
	PUBLIC logical_grammar
	PUBLIC lexer
	PUBLIC syntax_parser
)


# add parser library
add_subdirectory(parser)

