# Source files for sad
set(sad_SRCS
    binstr.c
    dprintf.c
    getopt.c
    makeargs.c
    sad.c
    util.cpp
)

# Create the executable
add_executable(sad ${sad_SRCS})

# Set target properties
set_target_properties(sad PROPERTIES
    C_STANDARD 99
    CXX_STANDARD 11
)

# Include directories for the target
target_include_directories(sad PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Platform-specific compiler flags
if(WATCOM)
    # Watcom compiler flags
    target_compile_options(sad PRIVATE
        $<$<COMPILE_LANGUAGE:C>:-bc -bm -ot -6r -zlf>
        $<$<COMPILE_LANGUAGE:CXX>:-bc -bm -ot -6r -zlf -xs>
    )
    target_compile_definitions(sad PRIVATE
        $<$<CONFIG:Release>:NDEBUG>
    )
    # Watcom linker
    set_target_properties(sad PROPERTIES
        LINK_FLAGS "option quiet option caseexact"
    )
elseif(MSVC)
    # MSVC compiler flags
    target_compile_options(sad PRIVATE
        /W3
        $<$<CONFIG:Release>:/O2>
    )
    target_compile_definitions(sad PRIVATE
        _CRT_SECURE_NO_WARNINGS
        $<$<CONFIG:Release>:NDEBUG>
    )
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_C_COMPILER_ID MATCHES "Clang")
    # GCC/Clang compiler flags
    target_compile_options(sad PRIVATE
        -Wall
        -Wextra
        $<$<CONFIG:Release>:-O2>
        $<$<CONFIG:Debug>:-g>
    )
    target_compile_definitions(sad PRIVATE
        $<$<CONFIG:Release>:NDEBUG>
    )
endif()

# Installation
# Uses standard installation directories based on platform
# Override with: cmake -DCMAKE_INSTALL_PREFIX=/custom/path
install(TARGETS sad
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT runtime
)