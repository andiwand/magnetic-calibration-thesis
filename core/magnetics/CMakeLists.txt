add_library(magnetics STATIC
        src/compass.cpp
        src/hard_iron.cpp
        src/live_demo_filter.cpp
        src/live_demo_task.cpp
        src/magnetics_filter.cpp
        src/moving_average.cpp
        src/orientation_filter.cpp
        src/player_task.cpp
        src/recorder_task.cpp
        src/task.cpp
        src/total_rotation.cpp
        src/web_server.cpp
        )
target_include_directories(magnetics
        PUBLIC
        include
        PRIVATE
        ${Boost_INCLUDE_DIRS}
        )
target_link_libraries(magnetics
        PUBLIC
        pipeline
        eigen
        )
