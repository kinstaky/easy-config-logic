# find pthread library
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# scaler
add_executable(
	scaler
	scaler.cpp
)
target_compile_features(
	scaler
	PRIVATE cxx_std_17
)
target_compile_options(
	scaler
	PRIVATE -Werror -Wall -Wextra
)

# scaler_client
add_executable(
	scaler_client
	scaler_client.cpp scaler_defs.h
)
target_compile_features(
	scaler_client
	PRIVATE cxx_std_17
)
target_compile_options(
	scaler_client
	PRIVATE -Werror -Wall -Wextra
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
	TARGETS scaler scaler_client scaler_server 
	DESTINATION "${CMAKE_BINARY_DIR}/bin"
)