#find_package(Protobuf)
#protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${CMAKE_CURRENT_SOURCE_DIR}/indoors/pipeline/protocol/message.proto)
#add_library(protocol STATIC ${PROTO_SRCS} ${PROTO_HDRS})
#target_include_directories(protocol PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
#target_link_libraries(protocol PUBLIC libprotobuf)

add_library(protocol STATIC
        src/indoors/pipeline/protocol/event.pb.cc
        )
target_include_directories(protocol
        PUBLIC
        src
        )
target_link_libraries(protocol
        PUBLIC
        protobuf::libprotobuf
        )
