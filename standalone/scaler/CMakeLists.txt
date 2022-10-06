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

# scaler_http_server
add_executable(
	scaler_http_server
	scaler_http_server.cpp "${PROJECT_SOURCE_DIR}/include/external/json.hpp" "${PROJECT_SOURCE_DIR}/include/external/httplib.h"
)
target_include_directories(
	scaler_http_server
	PUBLIC "${PROJECT_SOURCE_DIR}/include"
)
target_compile_features(
	scaler_http_server
	PRIVATE cxx_std_17
)
target_compile_options(
	scaler_http_server
	PRIVATE -Werror -Wall -Wextra
)
target_link_libraries(
	scaler_http_server	
	PRIVATE Threads::Threads
)

install(
	TARGETS scaler scaler_client scaler_server scaler_http_server
	DESTINATION "${CMAKE_BINARY_DIR}/bin"
)