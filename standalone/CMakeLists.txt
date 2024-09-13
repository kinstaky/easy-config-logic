# sytnax tree
add_executable(syntax_tree syntax_tree.cpp)
target_link_libraries(
	syntax_tree
	PRIVATE lexer syntax_parser logical_grammar logic_downscale_grammar
)

# compare
add_executable(compare compare_logical_expression.cpp)
target_link_libraries(compare PRIVATE logic_comparer)

# standardize
add_executable(standardize standardize.cpp)
target_link_libraries(
	standardize
	PRIVATE
	standard_logic_downscale_tree logic_downscale_grammar syntax_parser lexer
)

# convert
add_executable(convert convert.cpp)
target_link_libraries(convert PRIVATE memory_config config_parser)

# config
add_executable(config config.cpp)
target_link_libraries(config PRIVATE memory_config config_parser)

# logic test
add_executable(logic_test logic_test.cpp)
target_link_libraries(logic_test PRIVATE memory_config config_parser)

# scaler server
add_executable(server server.cpp)
target_link_libraries(server PRIVATE service)


install(
	TARGETS syntax_tree compare standardize convert config logic_test
	DESTINATION "${CMAKE_BINARY_DIR}/bin"
)