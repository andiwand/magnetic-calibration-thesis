include(GoogleTest REQUIRED)

enable_testing()
add_executable(magnetics-test
        csv_test.cpp
        moving_average_test.cpp
        orientation_filter_test.cpp
        protocol_test.cpp
        recorder_player_test.cpp
        series_buffer_test.cpp
        synchronizer_test.cpp
        )
target_link_libraries(magnetics-test
        PRIVATE
        gtest_main
        gtest
        pipeline
        magnetics
        GL
        GLU
        glut
        )
target_include_directories(magnetics-test
        PRIVATE
        ../pipeline/src
        ../magnetics/src
        )
gtest_add_tests(TARGET magnetics-test)
