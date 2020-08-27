find_package(Boost)

add_library(pipeline STATIC
        src/csv.cpp
        src/definitions.cpp
        src/http.cpp
        src/looper.cpp
        src/node.cpp
        src/platform.cpp
        src/protocol.cpp
        src/synchronizer.cpp
        src/worker.cpp
        )
target_include_directories(pipeline
        PUBLIC
        include
        PRIVATE
        ${Boost_INCLUDE_DIRS}
        )
target_link_libraries(pipeline
        PUBLIC
        protocol
        )
