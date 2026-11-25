add_library(pluribus_warnings INTERFACE)
add_library(pluribus::warnings ALIAS pluribus_warnings)

if(MSVC)
    target_compile_options(pluribus_warnings INTERFACE
        /W4
        /permissive-
        /Zc:__cplusplus
    )
else()
    target_compile_options(pluribus_warnings INTERFACE
        -Wall
        -Wextra
        -Wpedantic
        -Wshadow
    )
endif()

if(PLURIBUS_WARNINGS_AS_ERRORS)
    if(MSVC)
        target_compile_options(pluribus_warnings INTERFACE /WX)
    else()
        target_compile_options(pluribus_warnings INTERFACE -Werror)
    endif()
endif()
