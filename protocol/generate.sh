protoc --proto_path=. --cpp_out=../core/protocol/src indoors/pipeline/protocol/event.proto
protoc --proto_path=. --js_out=import_style=commonjs,binary:../javascript/src indoors/pipeline/protocol/event.proto
protoc --proto_path=. --python_out=../python indoors/pipeline/protocol/event.proto
