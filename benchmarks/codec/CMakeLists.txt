include(FetchContent)

# Third-party projects build faster and more reliably with their supported
# standard; the benchmark target itself requests C++20 below.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Cap'n Proto requires compiler-specific extensions for its generated code.
set(CMAKE_CXX_EXTENSIONS ON)

set(FLATBUFFERS_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(FLATBUFFERS_BUILD_FLATC ON CACHE BOOL "" FORCE)
set(FLATBUFFERS_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(flatbuffers
    GIT_REPOSITORY https://github.com/google/flatbuffers.git
    GIT_TAG v24.3.25
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(flatbuffers)

set(protobuf_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(protobuf_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(protobuf
    GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
    GIT_TAG v3.21.12
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(protobuf)

set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(CAPNP_BUILD_TOOLS ON CACHE BOOL "" FORCE)
FetchContent_Declare(capnproto
    GIT_REPOSITORY https://github.com/capnproto/capnproto.git
    GIT_TAG v1.0.2
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(capnproto)

include("${capnproto_SOURCE_DIR}/c++/cmake/CapnProtoMacros.cmake")

set(PROTO_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(PROTO_GENERATED_CC "${PROTO_GENERATED_DIR}/control.pb.cc")
set(PROTO_GENERATED_H "${PROTO_GENERATED_DIR}/control.pb.h")
add_custom_command(
    OUTPUT "${PROTO_GENERATED_CC}" "${PROTO_GENERATED_H}"
    COMMAND $<TARGET_FILE:protoc>
        "--proto_path=${CMAKE_CURRENT_SOURCE_DIR}"
        "--cpp_out=${PROTO_GENERATED_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/control.proto"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/control.proto" protoc
    COMMENT "Generating protobuf control schema"
    VERBATIM
)

set(FLATBUFFERS_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(FLATBUFFERS_COMMON_HEADER
    "${FLATBUFFERS_GENERATED_DIR}/common_generated.h")
set(FLATBUFFERS_SUBMIT_HEADER
    "${FLATBUFFERS_GENERATED_DIR}/submit_generated.h")
set(FLATBUFFERS_NODE_HEADER
    "${FLATBUFFERS_GENERATED_DIR}/node_descriptor_generated.h")
add_custom_command(
    OUTPUT
        "${FLATBUFFERS_COMMON_HEADER}"
        "${FLATBUFFERS_SUBMIT_HEADER}"
        "${FLATBUFFERS_NODE_HEADER}"
    COMMAND $<TARGET_FILE:flatc>
        --cpp
        -o "${FLATBUFFERS_GENERATED_DIR}"
        -I "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/common.fbs"
        "${CMAKE_CURRENT_SOURCE_DIR}/submit.fbs"
        "${CMAKE_CURRENT_SOURCE_DIR}/node_descriptor.fbs"
    DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/submit.fbs"
        "${CMAKE_CURRENT_SOURCE_DIR}/node_descriptor.fbs"
        "${CMAKE_CURRENT_SOURCE_DIR}/common.fbs"
        flatc
    COMMENT "Generating FlatBuffers control schemas"
    VERBATIM
)

capnp_generate_cpp(CAPNP_SOURCES CAPNP_HEADERS control.capnp)

add_executable(pluribus_codec_bench
    benchmark.cpp
    "${PROTO_GENERATED_CC}"
    "${FLATBUFFERS_COMMON_HEADER}"
    "${FLATBUFFERS_SUBMIT_HEADER}"
    "${FLATBUFFERS_NODE_HEADER}"
    ${CAPNP_SOURCES}
)

target_compile_features(pluribus_codec_bench PRIVATE cxx_std_20)
target_include_directories(pluribus_codec_bench PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${PROTO_GENERATED_DIR}"
    "${FLATBUFFERS_GENERATED_DIR}"
    "${capnproto_SOURCE_DIR}/c++/src"
    "${capnproto_BINARY_DIR}/c++/src"
)
target_link_libraries(pluribus_codec_bench PRIVATE
    flatbuffers
    protobuf::libprotobuf
    capnp
    kj
)
