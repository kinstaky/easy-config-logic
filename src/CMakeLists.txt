# add syntax libraries
add_subdirectory(syntax)

# add standardize libraries
add_subdirectory(standardize)

# add config libraries
add_subdirectory(config)

# i2c library
add_library(i2c STATIC i2c.cpp)
target_include_directories(i2c PUBLIC ${PROJECT_SOURCE_DIR}/include)

# service library
add_library(service service.cpp)
target_include_directories(
	service PUBLIC
	"${PROJECT_SOURCE_DIR}/include"
)
target_link_libraries(
	service PUBLIC ecl_grpc_proto config_parser memory_config
)