add_library(pluribuscore STATIC
    src/error.cpp
    src/execution.cpp
    src/ids.cpp
    src/names.cpp
    src/node.cpp
    src/paths.cpp
    src/resources.cpp
)
add_library(pluribus::core ALIAS pluribuscore)

target_compile_features(pluribuscore PUBLIC cxx_std_23)
target_include_directories(pluribuscore
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
target_link_libraries(pluribuscore PRIVATE pluribus_warnings)
