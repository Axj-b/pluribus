# Configure-time enforcement of the Pluribus component dependency graph.
#
# See docs/spec/09-repository-cmake-structure.md. The allowlist mirrors the
# documented target table; Pluribus-internal edges outside it, cycles and
# unclassified internal targets are fatal errors.

set(_PLURIBUS_COMPONENT_TARGETS
    pluribuscore
    pluribusfabric
    pluribusscheduler
    pluribussim
    pluribusstore
    pluribusmaster
    pluribusnode
    pluribusclient
    plu
    pluribus_test_support
    pluribus_core_tests
    pluribus_fabric_tests
    pluribus_scheduler_tests
    pluribus_fabric_bench
    pluribus_codec_bench
    pluribus_transport_bench)

# "<target>:<semicolon separated allowed Pluribus dependencies>"
set(_PLURIBUS_ALLOWED_ENTRIES
    "pluribuscore@"
    "pluribusfabric@pluribuscore"
    "pluribusscheduler@pluribuscore"
    "pluribussim@pluribuscore"
    "pluribusstore@pluribuscore"
    "pluribusmaster@pluribuscore|pluribusfabric|pluribusstore|pluribusscheduler"
    "pluribusnode@pluribuscore|pluribusfabric|pluribusstore|pluribussim"
    "pluribusclient@pluribuscore|pluribusfabric"
    "plu@pluribusclient"
    "pluribus_test_support@pluribuscore"
    "pluribus_core_tests@pluribuscore|pluribus_test_support"
    "pluribus_fabric_tests@pluribusfabric|pluribuscore|pluribus_test_support"
    "pluribus_scheduler_tests@pluribusscheduler|pluribuscore|pluribus_test_support"
    "pluribus_fabric_bench@pluribusfabric|pluribuscore"
    "pluribus_codec_bench@"
    "pluribus_transport_bench@pluribusfabric|pluribuscore")

function(_pluribus_normalize_target name out)
    set(result "${name}")
    if(TARGET "${result}")
        get_target_property(_alias "${result}" ALIASED_TARGET)
        if(_alias)
            set(result "${_alias}")
        endif()
    endif()
    set(${out} "${result}" PARENT_SCOPE)
endfunction()

function(_pluribus_allowed target out)
    set(result "")
    foreach(entry IN LISTS _PLURIBUS_ALLOWED_ENTRIES)
        string(FIND "${entry}" "@" _pos)
        string(SUBSTRING "${entry}" 0 ${_pos} _name)
        string(SUBSTRING "${entry}" ${_pos} -1 _rest)
        if(_name STREQUAL "${target}")
            string(SUBSTRING "${_rest}" 1 -1 _deps)
            string(REPLACE "|" ";" _deps "${_deps}")
            set(result "${_deps}")
        endif()
    endforeach()
    set(${out} "${result}" PARENT_SCOPE)
endfunction()

function(_pluribus_direct_deps target out)
    set(direct "")
    get_target_property(_link "${target}" LINK_LIBRARIES)
    get_target_property(_ilink "${target}" INTERFACE_LINK_LIBRARIES)
    foreach(item IN LISTS _link _ilink)
        if(NOT item)
            continue()
        endif()
        string(REGEX REPLACE "\\$<BUILD_INTERFACE:([^>]*)>" "\\1" item "${item}")
        string(REGEX REPLACE "\\$<INSTALL_INTERFACE:([^>]*)>" "\\1" item "${item}")
        string(REGEX REPLACE "\\$<LINK_ONLY:([^>]*)>" "\\1" item "${item}")
        if(item MATCHES "\\$<")
            message(FATAL_ERROR "Dependency check cannot evaluate link item '${item}'")
        endif()
        _pluribus_normalize_target("${item}" item)
        if(item IN_LIST _PLURIBUS_COMPONENT_TARGETS)
            list(APPEND direct "${item}")
        endif()
    endforeach()
    list(REMOVE_DUPLICATES direct)
    set(${out} "${direct}" PARENT_SCOPE)
endfunction()

function(pluribus_enforce_dependencies)
    foreach(target IN LISTS _PLURIBUS_COMPONENT_TARGETS)
        if(NOT TARGET "${target}")
            continue()
        endif()

        _pluribus_allowed("${target}" allowed)
        _pluribus_direct_deps("${target}" direct)

        set(closure "")
        set(queue "${direct}")
        while(queue)
            list(GET queue 0 current)
            list(REMOVE_AT queue 0)
            if(current STREQUAL "${target}")
                message(FATAL_ERROR "Pluribus dependency cycle involving ${target}")
            endif()
            if(current IN_LIST closure)
                continue()
            endif()
            list(APPEND closure "${current}")
            if(NOT current IN_LIST allowed)
                message(FATAL_ERROR
                    "Forbidden Pluribus dependency: ${target} -> ${current}")
            endif()
            _pluribus_direct_deps("${current}" next)
            list(APPEND queue ${next})
        endwhile()
    endforeach()

    get_cmake_property(_all BUILDDSYSTEM_TARGETS)
    foreach(target IN LISTS _all)
        if(NOT target MATCHES "^pluribus")
            continue()
        endif()
        _pluribus_normalize_target("${target}" real)
        if(real STREQUAL "pluribus_warnings")
            continue()
        endif()
        if(NOT real IN_LIST _PLURIBUS_COMPONENT_TARGETS)
            message(FATAL_ERROR "Unclassified Pluribus target: ${target}")
        endif()
    endforeach()

    message(STATUS "Pluribus dependency graph check passed")
endfunction()
