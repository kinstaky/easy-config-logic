# add syntax libraries
add_subdirectory(syntax)

# add standardize libraries
add_subdirectory(standardize)

# add config libraries
add_subdirectory(config)

# add scaler libraries
add_subdirectory(scaler)

# i2c library
add_library(i2c STATIC i2c.cpp)
target_include_directories(i2c PUBLIC ${PROJECT_SOURCE_DIR}/include)
