# logic_parser library
add_library(config_parser STATIC config_parser.cpp)
target_link_libraries(
	config_parser
	PUBLIC standard_logic_downscale_tree lexer syntax_parser logic_downscale_grammar
)

# memory_config library
add_library(memory_config STATIC memory_config.cpp)
target_link_libraries(memory_config PUBLIC config_parser)