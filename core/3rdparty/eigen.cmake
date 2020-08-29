include(FetchContent)

FetchContent_Declare(
        eigen
        URL https://gitlab.com/libeigen/eigen/-/archive/3.3.7/eigen-3.3.7.tar.bz2
)

FetchContent_GetProperties(eigen)
if (NOT eigen_POPULATED)
    FetchContent_Populate(eigen)
    set(BUILD_TESTING OFF CACHE BOOL "" FORCE)
    set(EIGEN_LEAVE_TEST_IN_ALL_TARGET OFF CACHE BOOL "" FORCE)
    add_subdirectory(${eigen_SOURCE_DIR} ${eigen_BINARY_DIR} EXCLUDE_FROM_ALL)
endif ()
