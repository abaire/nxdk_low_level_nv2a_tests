include(XBEUtils REQUIRED)

find_package(NXDK REQUIRED)
find_package(NXDK_SDL2 REQUIRED)
find_package(Threads REQUIRED)

configure_file(configure.h.in configure.h)

# ---------------------------------------------------------------------------
# Compiler Options Macros
# ---------------------------------------------------------------------------

macro(set_opt_compile_and_link_options TARGET_NAME)
    target_compile_options(
            "${TARGET_NAME}"
            PRIVATE
            -O3
            -fno-strict-aliasing
            -Wall
            $<$<COMPILE_LANGUAGE:CXX>:-Wno-builtin-macro-redefined>
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
                $<$<COMPILE_LANGUAGE:CXX>:-Wno-builtin-macro-redefined>
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

# ---------------------------------------------------------------------------
# Test Artifact Generation Macro
# Usage: create_nxdk_test_artifact(target_name "XBE Title" source1.cpp source2.cpp ...)
# ---------------------------------------------------------------------------

macro(create_test_xiso TARGET_NAME XBE_TITLE)
    add_executable(
            ${TARGET_NAME}
            ${ARGN}
            ${_VERTEX_SHADER_FILES}
    )

    split_debug(${TARGET_NAME})

    set_compile_and_link_options(${TARGET_NAME})

    target_include_directories(
            ${TARGET_NAME}
            PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}"
            "${CMAKE_CURRENT_BINARY_DIR}"
            ${_VERTEX_SHADER_INCLUDE_DIRS}
            "${CMAKE_SOURCE_DIR}/third_party"
    )

    target_link_libraries(
            ${TARGET_NAME}
            PRIVATE
            NXDK::NXDK
            NXDK::NXDK_CXX
            NXDK::SDL2
            PRIVATE
            printf
    )

    if (DEFINED _VERTEX_SHADER_GEN_TARGET)
        add_dependencies(
                ${TARGET_NAME}
                ${_VERTEX_SHADER_GEN_TARGET}
        )
    endif ()

    set(XBE_TARGET "${TARGET_NAME}_xiso")

    add_xbe(
            ${XBE_TARGET} "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.exe"
            TITLE "${XBE_TITLE}"
            RESOURCE_ROOTS
            "${CMAKE_SOURCE_DIR}/resources"
            RESOURCE_DIRS
            "${CMAKE_SOURCE_DIR}/resources"
    )

    add_xiso(${TARGET_NAME}_xiso ${XBE_TARGET})

endmacro()


# ---------------------------------------------------------------------------
# Target Definitions
# ---------------------------------------------------------------------------

# ptimer_alarm_test
create_test_xiso(
        ptimer_alarm_test
        "PTIMER alarm test"
        ptimer_alarm_main.cpp
        pbkit_util.cpp
        pbkit_util.h
        logger.cpp
        logger.h
        test_suite.h
        ptimer_tests/ptimer_test_common.cpp
        ptimer_tests/ptimer_test_common.h
        ptimer_tests/test_alarm_rearm.cpp
        ptimer_tests/test_alarm_rearm.h
        ptimer_tests/test_alarm_rollover_and_periodicity.cpp
        ptimer_tests/test_alarm_rollover_and_periodicity.h
        ptimer_tests/test_alarm_set_in_the_past.cpp
        ptimer_tests/test_alarm_set_in_the_past.h
        ptimer_tests/test_alarm_unaligned_target.cpp
        ptimer_tests/test_alarm_unaligned_target.h
        ptimer_tests/test_clock_scaling_ratios.cpp
        ptimer_tests/test_clock_scaling_ratios.h
        ptimer_tests/test_interrupt_acknowledge.cpp
        ptimer_tests/test_interrupt_acknowledge.h
        ptimer_tests/test_interrupt_masking.cpp
        ptimer_tests/test_interrupt_masking.h
        ptimer_tests/test_alarm_flag_is_set_while_masked.cpp
        ptimer_tests/test_alarm_flag_is_set_while_masked.h
        ptimer_tests/test_alarm_flag_is_set_while_masked_reset_time.cpp
        ptimer_tests/test_alarm_flag_is_set_while_masked_reset_time.h
        ptimer_tests/test_register_bitmasks.cpp
        ptimer_tests/test_register_bitmasks.h
        ptimer_tests/test_time_register_read_write.cpp
        ptimer_tests/test_time_register_read_write.h
        ptimer_tests/test_time_register_rollover.cpp
        ptimer_tests/test_time_register_rollover.h
        ptimer_tests/test_zero_divisor_clock.cpp
        ptimer_tests/test_zero_divisor_clock.h
)

# pfifo_cache1_test
create_test_xiso(
        pfifo_cache1_test
        "PFIFO CACHE1 test"
        pfifo_cache1_main.cpp
        pbkit_util.cpp
        pbkit_util.h
)

# clock_test
create_test_xiso(
        clock_test
        "Clock test"
        clock_main.cpp
        pbkit_util.cpp
        pbkit_util.h
)
