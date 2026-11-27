find_package(Threads REQUIRED)

add_library(pluribus_test_support STATIC
    support/test_main.cpp
)
target_compile_features(pluribus_test_support PUBLIC cxx_std_23)
target_include_directories(pluribus_test_support PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/support>
)
target_link_libraries(pluribus_test_support
    PUBLIC pluribuscore
    PRIVATE pluribus_warnings
)

add_executable(pluribus_core_tests unit/core_tests.cpp)
target_link_libraries(pluribus_core_tests PRIVATE
    pluribuscore
    pluribus_test_support
    pluribus_warnings
)
add_test(NAME core.unit COMMAND pluribus_core_tests)

add_executable(pluribus_fabric_tests
    unit/fabric_tests.cpp
    unit/fabric_transport_tests.cpp
    unit/fabric_session_tests.cpp
    unit/fabric_control_tests.cpp
)
target_link_libraries(pluribus_fabric_tests PRIVATE
    pluribusfabric
    pluribus_test_support
    pluribus_warnings
    Threads::Threads
)
add_test(NAME fabric.unit COMMAND pluribus_fabric_tests)

add_executable(pluribus_fabric_bench bench/fabric_transport_bench.cpp)
target_link_libraries(pluribus_fabric_bench PRIVATE
    pluribusfabric
    pluribus_warnings
    Threads::Threads
)

if(WIN32)
    add_test(NAME deps.static
        COMMAND pwsh -NoProfile -File "${CMAKE_SOURCE_DIR}/scripts/check-deps.ps1")
endif()

add_executable(pluribus_scheduler_tests unit/scheduler_tests.cpp)
target_link_libraries(pluribus_scheduler_tests PRIVATE
    pluribusscheduler
    pluribus_test_support
    pluribus_warnings
)
add_test(NAME scheduler.unit COMMAND pluribus_scheduler_tests)
