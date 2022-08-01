# include directory
set(PROJECT_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set(PARSER_INCLUDE_DIR "${PROJECT_INCLUDE_DIR}/syntax/parser")

# token library
add_library(
	token STATIC
	token.cpp "${PARSER_INCLUDE_DIR}/token.h"
)
target_include_directories(
	token
	PUBLIC "${PROJECT_INCLUDE_DIR}"
)
target_compile_features(
	token
	PRIVATE cxx_std_17
)
target_compile_options(
	token
	PRIVATE -Werror -Wall -Wextra
)


# prodution library
add_library(
	production STATIC
	production.cpp "${PARSER_INCLUDE_DIR}/production.h"
)
target_include_directories(
	production
	PUBLIC "${PROJECT_INCLUDE_DIR}"
)
target_compile_features(
	production
	PRIVATE cxx_std_17
)
target_compile_options(
	production
	PRIVATE -Werror -Wall -Wextra
)
target_link_libraries(
	production
	PUBLIC token
)



# grammar library
add_library(
	grammar STATIC
	grammar.cpp "${PARSER_INCLUDE_DIR}/grammar.h"
)
target_include_directories(
	grammar
	PUBLIC "${PROJECT_INCLUDE_DIR}"
)
target_compile_features(
	grammar
	PRIVATE cxx_std_17
)
target_compile_options(
	grammar
	PRIVATE -Werror -Wall -Wextra
)
target_link_libraries(
	grammar
	PUBLIC token
	PUBLIC production
)



# lexer library
add_library(
	lexer STATIC
	lexer.cpp "${PARSER_INCLUDE_DIR}/lexer.h"
)
target_include_directories(
	lexer
	PUBLIC "${PROJECT_INCLUDE_DIR}"
)
target_compile_features(
	lexer
	PRIVATE cxx_std_17
)
target_compile_options(
	lexer
	PRIVATE -Werror -Wall -Wextra
)
target_link_libraries(
	lexer
	PUBLIC token
)




# syntax parser library
add_library(
	syntax_parser STATIC
	syntax_parser.cpp "${PARSER_INCLUDE_DIR}/syntax_parser.h"
)
target_include_directories(
	syntax_parser
	PUBLIC "${PROJECT_INCLUDE_DIR}"
)
target_compile_features(
	syntax_parser
	PRIVATE cxx_std_17
)
target_compile_options(
	syntax_parser
	PRIVATE -Werror -Wall -Wextra
)
target_link_libraries(
	syntax_parser
	PUBLIC token
	PUBLIC production
	PUBLIC grammar
)