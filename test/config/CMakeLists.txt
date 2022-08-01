# test logic parser
add_executable(
	test_logic_parser
	test_logic_parser.cpp
)
target_compile_features(
	test_logic_parser
	PRIVATE cxx_std_17
)
target_compile_options(
	test_logic_parser
	PRIVATE -Werror -Wall -Wextra
)
target_link_libraries(
	test_logic_parser
	PRIVATE gtest_main
	PRIVATE logic_parser
)



# test memory config
add_executable(
	test_memory_config
	test_memory_config.cpp
)
target_compile_definitions(
	test_memory_config
	PRIVATE TEST_DATA_DIRECTORY="${CMAKE_CURRENT_BINARY_DIR}/data/"
)
target_compile_features(
	test_memory_config
	PRIVATE cxx_std_17
)
target_compile_options(
	test_memory_config
	PRIVATE -Werror -Wall -Wextra
)
target_link_libraries(
	test_memory_config
	PRIVATE gtest_main
	PRIVATE memory_config
)
file(
	COPY "data/register_config_0_origin.txt" 
	DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/data"
)
file(
	COPY "data/logic_config_1.txt" 
	DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/data"
)
file(
	COPY "data/register_config_1.txt"
	DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/data"
)


# google test discover
include(GoogleTest)
gtest_discover_tests(test_logic_parser)
gtest_discover_tests(test_memory_config)