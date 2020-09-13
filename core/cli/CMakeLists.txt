add_executable(tu_replayer
        tu_replayer.cpp
        )
target_link_libraries(tu_replayer
        magnetics
        nlohmann_json::nlohmann_json
        )
