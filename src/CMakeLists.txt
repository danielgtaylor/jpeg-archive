include (ExternalProject)

find_program (MAKE_PROGRAM NAMES make mingw32-make nmake)

ExternalProject_Add (
    IQA_external
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/iqa"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND "${MAKE_PROGRAM}" RELEASE=1
    BUILD_BYPRODUCTS "<SOURCE_DIR>/build/release/libiqa.a"
    BUILD_IN_SOURCE 1
    INSTALL_COMMAND ""
)

ExternalProject_Get_Property (IQA_external binary_dir)

add_library (IQA STATIC IMPORTED GLOBAL)
set_property (TARGET IQA PROPERTY IMPORTED_LOCATION ${binary_dir}/build/release/libiqa.a)
add_dependencies (IQA IQA_external)

# Since this is an in-source build, add it to the cleanup.
set_property (DIRECTORY PROPERTY ADDITIONAL_MAKE_CLEAN_FILES ${binary_dir}/build/)

ExternalProject_Add (
    MOZJPEG_external
    GIT_REPOSITORY "https://github.com/mozilla/mozjpeg.git"
    GIT_TAG "v3.3.1"
    UPDATE_DISCONNECTED 1
    CMAKE_ARGS
        -DWITH_JPEG8=1
        -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
        -DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}
    BUILD_BYPRODUCTS "<BINARY_DIR>/libjpeg.a"
    INSTALL_COMMAND ""
)

ExternalProject_Get_Property (MOZJPEG_external source_dir)
ExternalProject_Get_Property (MOZJPEG_external binary_dir)

set (MOZJPEG_INCLUDE_DIR ${source_dir} ${binary_dir})

add_library (MOZJPEG STATIC IMPORTED GLOBAL)
set_property (TARGET MOZJPEG PROPERTY IMPORTED_LOCATION ${binary_dir}/libjpeg.a)
add_dependencies (MOZJPEG MOZJPEG_external)

include_directories (SYSTEM ${MOZJPEG_INCLUDE_DIR})

add_library (
    jpegarchive STATIC
    commander.c commander.h
    edit.c edit.h
    hash.c hash.h
    smallfry.c smallfry.h
    util.c util.h
)
add_dependencies (jpegarchive MOZJPEG)

set (MOZJPEG_INCLUDE_DIR ${MOZJPEG_INCLUDE_DIR} PARENT_SCOPE)
