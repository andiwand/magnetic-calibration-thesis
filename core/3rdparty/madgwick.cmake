include(FetchContent)

FetchContent_Declare(
        madgwick
        URL http://www.x-io.co.uk/res/sw/madgwick_algorithm_c.zip
)

FetchContent_GetProperties(madgwick)
if (NOT madgwick_POPULATED)
    FetchContent_Populate(madgwick)

    add_library(madgwick STATIC
            ${madgwick_SOURCE_DIR}/MadgwickAHRS/MadgwickAHRS.c
            ${madgwick_SOURCE_DIR}/MahonyAHRS/MahonyAHRS.c
            )
    target_include_directories(madgwick PUBLIC ${madgwick_SOURCE_DIR})
endif ()
