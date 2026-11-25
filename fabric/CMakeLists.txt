add_library(pluribusfabric STATIC
    src/frame.cpp
)
add_library(pluribus::fabric ALIAS pluribusfabric)

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
)
