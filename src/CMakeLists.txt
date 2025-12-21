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
    )

    target_link_libraries(
            ${TARGET_NAME}
            PRIVATE
            NXDK::NXDK
            NXDK::NXDK_CXX
            NXDK::SDL2
    )

    if (DEFINED _VERTEX_SHADER_GEN_TARGET)
        add_dependencies(
                ${TARGET_NAME}
                ${_VERTEX_SHADER_GEN_TARGET}
        )
    endif ()

    set(XBE_TARGET "${TARGET_NAME}_xbe")

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
)
