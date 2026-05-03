# Proto file
get_filename_component(ecl_proto "ecl.proto" ABSOLUTE)
get_filename_component(ecl_proto_path "${ecl_proto}" PATH)

# Generated sources
set(ecl_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/ecl.pb.cc")
set(ecl_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/ecl.pb.h")
set(ecl_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/ecl.grpc.pb.cc")
set(ecl_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/ecl.grpc.pb.h")
add_custom_command(
        OUTPUT "${ecl_proto_srcs}" "${ecl_proto_hdrs}"
                "${ecl_grpc_srcs}" "${ecl_grpc_hdrs}"
    COMMAND ${_PROTOBUF_PROTOC}
        ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${ecl_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${ecl_proto}"
    DEPENDS "${ecl_proto}"
)

# ecl_grpc_proto
add_library(
        ecl_grpc_proto
        ${ecl_grpc_srcs} ${ecl_grpc_hdrs} ${ecl_proto_srcs} ${ecl_proto_hdrs}
)
target_include_directories(ecl_grpc_proto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(
        ecl_grpc_proto
        PUBLIC ${_REFLECTION} ${_GRPC_GRPCPP} ${_PROTOBUF_LIBPROTOBUF}
)