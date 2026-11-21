# Writes OUTPUT_FILE from INPUT_FILE with GIT_REVISION and GIT_DIRTY describing
# the source tree at SOURCE_DIR. Run in script mode (cmake -P) at build time.
#
# GIT_DIRTY is "true" if a tracked file or the checked out commit of a submodule
# differs from HEAD. Untracked files and untracked content inside submodules
# are ignored. If git is unavailable, GIT_REVISION is "unknown" and GIT_DIRTY is
# "true".

set(GIT_REVISION "unknown")
set(GIT_DIRTY "true")

if (GIT_EXECUTABLE)
    execute_process(
            COMMAND "${GIT_EXECUTABLE}" rev-parse HEAD
            WORKING_DIRECTORY "${SOURCE_DIR}"
            RESULT_VARIABLE _rev_result
            OUTPUT_VARIABLE _rev
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
    )
    if (_rev_result EQUAL 0)
        set(GIT_REVISION "${_rev}")
        execute_process(
                COMMAND "${GIT_EXECUTABLE}" status --porcelain
                --untracked-files=no --ignore-submodules=untracked
                WORKING_DIRECTORY "${SOURCE_DIR}"
                RESULT_VARIABLE _status_result
                OUTPUT_VARIABLE _status
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET
        )
        if (_status_result EQUAL 0 AND _status STREQUAL "")
            set(GIT_DIRTY "false")
        endif ()
    endif ()
endif ()

configure_file("${INPUT_FILE}" "${OUTPUT_FILE}" @ONLY)
