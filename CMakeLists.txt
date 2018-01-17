cmake_minimum_required (VERSION 3.0)
project (jpeg-archive)

enable_testing ()

include (ExternalProject)

add_subdirectory (src)
add_subdirectory (test)

include_directories (SYSTEM ${MOZJPEG_INCLUDE_DIR})

add_executable (jpeg-compare jpeg-compare.c)
target_link_libraries (jpeg-compare jpegarchive IQA MOZJPEG -lm)

add_executable (jpeg-hash jpeg-hash.c)
target_link_libraries (jpeg-hash jpegarchive IQA MOZJPEG -lm)

add_executable (jpeg-recompress jpeg-recompress.c)
target_link_libraries (jpeg-recompress jpegarchive IQA MOZJPEG -lm)

install (
    TARGETS jpeg-compare jpeg-hash jpeg-recompress
    DESTINATION bin
)
