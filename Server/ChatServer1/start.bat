@echo off
set PROTOC_PATH=C:\Users\YuMou\grpc\grpc\visualpro\third_party\protobuf\Debug\protoc.exe
set GRPC_PLUGIN_PATH=C:\Users\YuMou\grpc\grpc\visualpro\Debug\grpc_cpp_plugin.exe
set PROTO_FILE=message.proto

echo Generating Grpc code...
%PROTOC_PATH% -I="." --grpc_out="." --plugin=protoc-gen-grpc="%GRPC_PLUGIN_PATH%" "%PROTO_FILE%"

echo Generating c++ code...
%PROTOC_PATH% --cpp_out="." "%PROTO_FILE%"

echo Done.