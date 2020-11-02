add_executable(tu_replayer
        tu_replayer.cpp
        )
target_link_libraries(tu_replayer
        magnetics
        nlohmann_json::nlohmann_json
        )

add_executable(recording_meta
        recording_meta.cpp
        )
target_link_libraries(recording_meta
        magnetics
        )

add_executable(copy_recording
        copy_recording.cpp
        )
target_link_libraries(copy_recording
        magnetics
        )

add_executable(evaluation
        evaluation.cpp
        )
target_link_libraries(evaluation
        magnetics
        )
