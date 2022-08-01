# include directory
set(PROJECT_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set(STANDARDIZE_INCLUDE_DIR "${PROJECT_INCLUDE_DIR}/standardize")

# standard_logic_node library
add_library(
	standard_logic_node STATIC
	standard_logic_node.cpp "${STANDARDIZE_INCLUDE_DIR}/standard_logic_node.h"
)
target_include_directories(
	standard_logic_node
	PUBLIC "${PROJECT_INCLUDE_DIR}"
)
target_compile_features(
	standard_logic_node
	PRIVATE cxx_std_17
)
target_compile_options(
	standard_logic_node
	PRIVATE -Werror -Wall -Wextra
)
target_link_libraries(
	standard_logic_node
	PUBLIC token 
)


# standard_logic_tree library
add_library(
	standard_logic_tree STATIC
	standard_logic_tree.cpp "${STANDARDIZE_INCLUDE_DIR}/standard_logic_tree.h"
)
target_include_directories(
	standard_logic_tree
	PUBLIC "${PROJECT_INCLUDE_DIR}"
)
target_compile_features(
	standard_logic_tree
	PRIVATE cxx_std_17
)
target_compile_options(
	standard_logic_tree
	PRIVATE -Werror -Wall -Wextra
)
target_link_libraries(
	standard_logic_tree
	PUBLIC standard_logic_node production token
)