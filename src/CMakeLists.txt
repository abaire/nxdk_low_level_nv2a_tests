include(XBEUtils REQUIRED)

find_package(NXDK REQUIRED)

find_package(NXDK_SDL2 REQUIRED)
find_package(Threads REQUIRED)

configure_file(configure.h.in configure.h)

macro(set_opt_compile_and_link_options TARGET_NAME)
    target_compile_options(
            "${TARGET_NAME}"
            PRIVATE
            -O3
            -fno-strict-aliasing
            -Wall
            $<$<COMPILE_LANGUAGE:CXX>:-Wno-builtin-macro-redefined>   # Suppress warning from NXDK undef of __STDC_NO_THREADS__
            -D_USE_MATH_DEFINES
    )
    target_link_options(
            "${TARGET_NAME}"
            PRIVATE
            "/debug:none"
    )
endmacro()

macro(set_compile_and_link_options TARGET_NAME)
    if (CMAKE_BUILD_TYPE MATCHES Debug)
        target_compile_options(
                "${TARGET_NAME}"
                PRIVATE
                -g
                -gdwarf-4
                -O0
                -Wall
                -fstandalone-debug
                $<$<COMPILE_LANGUAGE:CXX>:-Wno-builtin-macro-redefined>   # Suppress warning from NXDK undef of __STDC_NO_THREADS__
                -D_USE_MATH_DEFINES
        )
        target_link_options(
                "${TARGET_NAME}"
                PRIVATE
                "/debug:full"
        )
    else ()
        set_opt_compile_and_link_options("${TARGET_NAME}")
    endif ()
endmacro()

#
add_executable(
        ptimer_alarm_test
        ptimer_alarm_main.cpp
        ${_VERTEX_SHADER_FILES}
)

# Pull debug info out of the binary into a host-side linked binary.
split_debug(ptimer_alarm_test)

set_compile_and_link_options(ptimer_alarm_test)
target_include_directories(
        ptimer_alarm_test
        PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_CURRENT_BINARY_DIR}"
        ${_VERTEX_SHADER_INCLUDE_DIRS}
)

target_link_libraries(
        ptimer_alarm_test
        PRIVATE
        NXDK::NXDK
        NXDK::NXDK_CXX
        NXDK::SDL2
)

add_dependencies(
        ptimer_alarm_test
        ${_VERTEX_SHADER_GEN_TARGET}
)

add_xbe(
        xbe_file "${CMAKE_CURRENT_BINARY_DIR}/ptimer_alarm_test.exe"
        TITLE "PTIMER alarm test"
        RESOURCE_ROOTS
        "${CMAKE_SOURCE_DIR}/resources"
        RESOURCE_DIRS
        "${CMAKE_SOURCE_DIR}/resources"
)
add_xiso(ptimer_alarm_test_xiso xbe_file)
