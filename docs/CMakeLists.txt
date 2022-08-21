set(DOXYGEN_EXTRACT_ALL YES)
set(DOXYGEN_GENERATE_XML YES)

doxygen_add_docs(
	docs

	syntax/parser/token.h
	syntax/parser/lexer.h
	syntax/parser/production.h
	syntax/parser/grammar.h
	syntax/parser/syntax_parser.h
	syntax/logical_grammar.h
	syntax/logic_comparer.h

	standardize/standard_logic_node.h
	stdnaardize/stadnard_logic_tree.h

	config/logic_parser.h
	config/memory_config.h
	
	WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}/include"
)