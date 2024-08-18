# logic_parser library
add_library(logic_parser STATIC logic_parser.cpp)
target_link_libraries(
	logic_parser
	PUBLIC standard_logic_tree lexer syntax_parser logical_grammar
)

# memory_config library
add_library(memory_config STATIC memory_config.cpp)
target_link_libraries(memory_config PUBLIC logic_parser)