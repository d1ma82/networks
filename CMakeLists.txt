cmake_minimum_required(VERSION 3.12)

set(CMAKE_BUILD_TYPE Debug)

project(tcp)

file(GLOB SERVER_SRC 
"./server.cpp"
)

file(GLOB CLIENT_SRC
"./client.cpp"
)
 
add_executable(server ${SERVER_SRC})
set_target_properties(server PROPERTIES RUNTIME_OUTPUT_DIRECTORY "../bin")
target_compile_features(server PRIVATE cxx_std_17)

add_executable(client ${CLIENT_SRC})
set_target_properties(client PROPERTIES RUNTIME_OUTPUT_DIRECTORY "../bin")
target_compile_features(client PRIVATE cxx_std_17)

