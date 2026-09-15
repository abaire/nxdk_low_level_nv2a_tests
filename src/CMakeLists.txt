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
        ptimer_tests/test_alarm_target_zero.cpp
        ptimer_tests/test_alarm_target_zero.h
        ptimer_tests/test_alarm_unaligned_target.cpp
        ptimer_tests/test_alarm_unaligned_target.h
        ptimer_tests/test_alarm_unprogrammed.cpp
        ptimer_tests/test_alarm_unprogrammed.h
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

# m2mf_test
create_test_xiso(
        m2mf_test
        "NV2A Class 0x39 M2MF test"
        m2mf_main.cpp
        pbkit_util.cpp
        pbkit_util.h
        logger.cpp
        logger.h
        test_suite.h
        m2mf_tests/m2mf_test_common.cpp
        m2mf_tests/m2mf_test_common.h
        m2mf_tests/test_m2mf_linear_copy.cpp
        m2mf_tests/test_m2mf_linear_copy.h
        m2mf_tests/test_m2mf_linear_copy.cpp
        m2mf_tests/test_m2mf_linear_copy.h
        m2mf_tests/test_m2mf_min_transfer.cpp
        m2mf_tests/test_m2mf_min_transfer.h
        m2mf_tests/test_m2mf_max_line_count.cpp
        m2mf_tests/test_m2mf_max_line_count.h
        m2mf_tests/test_m2mf_large_pitch.cpp
        m2mf_tests/test_m2mf_large_pitch.h
        m2mf_tests/test_m2mf_zero_size.cpp
        m2mf_tests/test_m2mf_zero_size.h
        m2mf_tests/test_m2mf_alignment_phases.cpp
        m2mf_tests/test_m2mf_alignment_phases.h
        m2mf_tests/test_m2mf_alignment_odd_sizes.cpp
        m2mf_tests/test_m2mf_alignment_odd_sizes.h
        m2mf_tests/test_m2mf_pitched_symmetric.cpp
        m2mf_tests/test_m2mf_pitched_symmetric.h
        m2mf_tests/test_m2mf_pitched_asymmetric.cpp
        m2mf_tests/test_m2mf_pitched_asymmetric.h
        m2mf_tests/test_m2mf_pitched_packing.cpp
        m2mf_tests/test_m2mf_pitched_packing.h
        m2mf_tests/test_m2mf_sysmem_to_vram.cpp
        m2mf_tests/test_m2mf_sysmem_to_vram.h
        m2mf_tests/test_m2mf_vram_to_sysmem.cpp
        m2mf_tests/test_m2mf_vram_to_sysmem.h
        m2mf_tests/test_m2mf_vram_to_vram.cpp
        m2mf_tests/test_m2mf_vram_to_vram.h
        m2mf_tests/test_m2mf_framebuffer.cpp
        m2mf_tests/test_m2mf_framebuffer.h
        m2mf_tests/test_m2mf_batching_independent.cpp
        m2mf_tests/test_m2mf_batching_independent.h
        m2mf_tests/test_m2mf_batching_causal_chain.cpp
        m2mf_tests/test_m2mf_batching_causal_chain.h
        m2mf_tests/test_m2mf_notifier_record.cpp
        m2mf_tests/test_m2mf_notifier_record.h
        m2mf_tests/test_m2mf_notifier_switching.cpp
        m2mf_tests/test_m2mf_notifier_switching.h
)

target_link_libraries(
        m2mf_test
        PRIVATE
        pbkitplusplus
)
