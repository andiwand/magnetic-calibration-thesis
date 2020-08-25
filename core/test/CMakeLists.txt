include(GoogleTest REQUIRED)

enable_testing()
add_executable(magnetics-test
        buffer_test.cpp
        csv_test.cpp
        http_test.cpp
        magneto_sphere_test.cpp
        moving_average_test.cpp
        opengl_test.cpp
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
