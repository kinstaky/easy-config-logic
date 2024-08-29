# add syntax libraries
add_subdirectory(syntax)

# add standardize libraries
add_subdirectory(standardize)

# add config libraries
add_subdirectory(config)

# i2c library
add_library(i2c STATIC i2c.cpp)
