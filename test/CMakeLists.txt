# fetch googletest
FetchContent_Declare(
	googletest
	URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

# syntax test
add_subdirectory(syntax)

# standaardize test
add_subdirectory(standardize)

# config test
add_subdirectory(config)