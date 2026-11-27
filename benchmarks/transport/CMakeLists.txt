find_package(Threads REQUIRED)

include(FetchContent)
FetchContent_Declare(asio
    GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
    GIT_TAG asio-1-30-2
    GIT_SHALLOW TRUE
)
FetchContent_GetProperties(asio)
if(NOT asio_POPULATED)
    FetchContent_Populate(asio)
endif()

add_executable(pluribus_transport_bench asio_bench.cpp)
target_compile_features(pluribus_transport_bench PRIVATE cxx_std_20)
target_compile_definitions(pluribus_transport_bench PRIVATE ASIO_STANDALONE)
target_include_directories(pluribus_transport_bench PRIVATE
    "${asio_SOURCE_DIR}/asio/include"
)
target_link_libraries(pluribus_transport_bench PRIVATE
    pluribusfabric
    Threads::Threads
)
