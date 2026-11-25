add_library(pluribusscheduler STATIC
    src/scheduler.cpp
)
add_library(pluribus::scheduler ALIAS pluribusscheduler)

target_compile_features(pluribusscheduler PUBLIC cxx_std_23)
target_include_directories(pluribusscheduler
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
target_link_libraries(pluribusscheduler
    PUBLIC
        pluribuscore
    PRIVATE
        pluribus_warnings
)
