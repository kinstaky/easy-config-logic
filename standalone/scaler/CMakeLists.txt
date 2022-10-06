# find pthread library
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# scaler_client
add_executable(
	scaler_client
	scaler_client.cpp scaler_defs.h
)
target_include_directories(
	scaler_client
	PUBLIC ${PROJECT_SOURCE_DIR}/include
)
target_compile_features(
	scaler_client
	PRIVATE cxx_std_17
)
target_compile_options(
	scaler_client
	PRIVATE -Werror -Wall -Wextra
)
target_link_libraries(
	scaler_client	
	PRIVATE Threads::Threads
)

# scaler_server
add_executable(
	scaler_server
	scaler_server.cpp scaler_defs.h
)
target_compile_features(
	scaler_server
	PRIVATE cxx_std_17
)
target_compile_options(
	scaler_server
	PRIVATE -Werror -Wall -Wextra
)
target_link_libraries(
	scaler_server
	PRIVATE Threads::Threads
)

install(
	TARGETS scaler_client scaler_server
	DESTINATION "${CMAKE_BINARY_DIR}/bin"
)