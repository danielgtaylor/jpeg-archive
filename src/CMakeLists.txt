include (ExternalProject)

ExternalProject_Add (
    IAQ

    SOURCE_DIR "${CMAKE_SOURCE_DIR}/src/iqa"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND make RELEASE=1
    BUILD_IN_SOURCE 1

    INSTALL_COMMAND ""
)
