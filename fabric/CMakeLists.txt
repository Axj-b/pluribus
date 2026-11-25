find_package(Threads REQUIRED)

add_library(pluribusfabric STATIC
    src/frame.cpp
    src/frame_stream.cpp
    src/loopback.cpp
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
)
target_link_libraries(pluribusfabric
    PUBLIC
        pluribuscore
    PRIVATE
        pluribus_warnings
        Threads::Threads
)
