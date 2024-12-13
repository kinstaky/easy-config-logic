# include directory
set(PROJECT_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")

# token library
add_library(token STATIC token.cpp)
target_include_directories(token PUBLIC ${PROJECT_INCLUDE_DIR})


# prodution library
add_library(production STATIC production.cpp)
target_link_libraries(production PUBLIC token)


# grammar library
add_library(grammar STATIC grammar.cpp)
target_link_libraries(grammar PUBLIC token production)


# lexer library
add_library(lexer STATIC lexer.cpp)
target_link_libraries(lexer PUBLIC token parse_result)


# syntax parser library
add_library(syntax_parser STATIC syntax_parser.cpp)
target_link_libraries(
	syntax_parser PUBLIC
	token production grammar parse_result
)