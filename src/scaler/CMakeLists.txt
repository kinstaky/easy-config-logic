add_library(scaler_service scaler_service.cpp)
target_include_directories(
	scaler_service PUBLIC
	"${PROJECT_SOURCE_DIR}/include"
)
target_link_libraries(
	scaler_service PUBLIC ecl_grpc_proto
)