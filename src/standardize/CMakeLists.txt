# include directory
set(PROJECT_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/include")
set(STANDARDIZE_INCLUDE_DIR "${PROJECT_INCLUDE_DIR}/standardize")

# standard_logic_node library
add_library(
	standard_logic_node STATIC standard_logic_node.cpp
)
target_include_directories(
	standard_logic_node PUBLIC ${PROJECT_INCLUDE_DIR}
)
target_link_libraries(
	standard_logic_node PUBLIC token
)


# standard_logic_tree library
add_library(
	standard_logic_tree STATIC standard_logic_tree.cpp
)
target_link_libraries(
	standard_logic_tree PUBLIC standard_logic_node production token
)


# standard_logic_downscale_tree library
add_library(
	standard_logic_downscale_tree STATIC standard_logic_downscale_tree.cpp
)
target_link_libraries(
	standard_logic_downscale_tree
	PUBLIC standard_logic_node production token
)