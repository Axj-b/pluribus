find_package(Threads REQUIRED)

include(FetchContent)
set(protobuf_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(protobuf_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(protobuf_BUILD_LIBPROTOC ${PLURIBUS_BUILD_CODEC_BENCH} CACHE BOOL "" FORCE)
set(protobuf_BUILD_PROTOC_BINARIES ${PLURIBUS_BUILD_CODEC_BENCH} CACHE BOOL "" FORCE)
FetchContent_Declare(protobuf
    GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
    GIT_TAG v3.21.12
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(protobuf)

add_library(pluribusfabric STATIC
    src/frame.cpp
    src/frame_stream.cpp
    src/handshake_codec.cpp
    src/loopback.cpp
    src/session.cpp
    proto/handshake.pb.cc
)
add_library(pluribus::fabric ALIAS pluribusfabric)

if(WIN32)
    target_sources(pluribusfabric PRIVATE src/tcp_win32.cpp)
    target_link_libraries(pluribusfabric PRIVATE ws2_32)
else()
    target_sources(pluribusfabric PRIVATE src/tcp_posix.cpp)
endif()

target_compile_features(pluribusfabric PUBLIC cxx_std_23)
target_include_directories(pluribusfabric
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/proto
)
target_link_libraries(pluribusfabric
    PUBLIC
        pluribuscore
    PRIVATE
        pluribus_warnings
        Threads::Threads
        protobuf::libprotobuf
)

if(MSVC)
    set_source_files_properties(proto/handshake.pb.cc PROPERTIES COMPILE_OPTIONS "/W0")
else()
    set_source_files_properties(proto/handshake.pb.cc PROPERTIES COMPILE_OPTIONS "-w")
    set_source_files_properties(src/handshake_codec.cpp PROPERTIES COMPILE_OPTIONS "-Wno-shadow")
endif()
