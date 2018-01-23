include (ExternalProject)
include(ProcessorCount)

ProcessorCount(NJOBS)

find_program (MAKE_PROGRAM NAMES make mingw32-make nmake)
find_program (AUTORECONF_PROGRAM NAMES autoreconf)

ExternalProject_Add (
    IQA_external
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/iqa"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND "${MAKE_PROGRAM}" -j${NJOBS} RELEASE=1
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

set (mozjpeg_git_repo "https://github.com/mozilla/mozjpeg.git")
set (mozjpeg_git_tag "v3.3.1")
if (WIN32)
    ExternalProject_Add (
        MOZJPEG_external
        GIT_REPOSITORY ${mozjpeg_git_repo}
        GIT_TAG ${mozjpeg_git_tag}
        UPDATE_COMMAND ""
        CMAKE_ARGS
            -DWITH_JPEG8=1
            -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
            -DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}
        BUILD_BYPRODUCTS "<BINARY_DIR>/libjpeg.a"
        INSTALL_COMMAND ""
    )
else ()
    ExternalProject_Add (
        MOZJPEG_external
        GIT_REPOSITORY ${mozjpeg_git_repo}
        GIT_TAG ${mozjpeg_git_tag}
        UPDATE_COMMAND ""
        CONFIGURE_COMMAND ${AUTORECONF_PROGRAM} -ivf
        COMMAND ./configure --with-jpeg8
        BUILD_COMMAND ${MAKE_PROGRAM} -j${NJOBS}
        BUILD_BYPRODUCTS "<BINARY_DIR>/.libs/libjpeg.a"
        BUILD_IN_SOURCE 1
        INSTALL_COMMAND ""
    )
endif ()

ExternalProject_Get_Property (MOZJPEG_external source_dir)
ExternalProject_Get_Property (MOZJPEG_external binary_dir)

set (MOZJPEG_INCLUDE_DIR ${source_dir} ${binary_dir})

if (WIN32)
    set (MOZJPEG_LIB ${binary_dir}/libjpeg.a)
else ()
    set (MOZJPEG_LIB ${binary_dir}/.libs/libjpeg.a)
endif ()

add_library (MOZJPEG STATIC IMPORTED GLOBAL)
set_property (TARGET MOZJPEG PROPERTY IMPORTED_LOCATION ${MOZJPEG_LIB})
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
