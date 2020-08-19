include(FetchContent)

FetchContent_Declare(
        protobuf
        GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
        GIT_TAG        v3.6.1
        SOURCE_SUBDIR  cmake
)

FetchContent_GetProperties(protobuf)
if (NOT protobuf_POPULATED)
    FetchContent_Populate(protobuf)
    set(protobuf_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    add_subdirectory(${protobuf_SOURCE_DIR}/cmake ${protobuf_BINARY_DIR} EXCLUDE_FROM_ALL)
endif ()

# TODO build and install protoc
