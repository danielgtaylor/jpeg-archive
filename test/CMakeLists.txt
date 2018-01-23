include_directories (SYSTEM ${MOZJPEG_INCLUDE_DIR})

add_executable(simple-test test.c)
target_link_libraries (simple-test jpegarchive IQA MOZJPEG -lm)

add_test (NAME simple-test COMMAND simple-test)

# Target to re-build and re-test.
add_custom_target (
    check
    COMMAND ${CMAKE_CTEST_COMMAND}
    DEPENDS simple-test
)
