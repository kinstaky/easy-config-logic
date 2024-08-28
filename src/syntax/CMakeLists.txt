# include directory
set(PROJECT_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")


# logical grammar library
add_library(logical_grammar STATIC logical_grammar.cpp)
target_link_libraries(logical_grammar PUBLIC token production grammar)


# logic-downscale grammar library
add_library(logic_downscale_grammar STATIC logic_downscale_grammar.cpp)
target_link_libraries(
	logic_downscale_grammar
	PUBLIC token production grammar
)


# arithmetic grammar library
add_library(arithmetic_grammar STATIC arithmetic_grammar.cpp)
target_link_libraries(
	arithmetic_grammar
	PUBLIC token production grammar
)

# logical compare library
add_library(logic_comparer STATIC logic_comparer.cpp)
target_link_libraries(
	logic_comparer
	PUBLIC logical_grammar lexer syntax_parser
)


# add parser library
add_subdirectory(parser)

