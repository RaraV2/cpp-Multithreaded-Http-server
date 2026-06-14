cmake_minimum_required(VERSION 3.16)

project(
    Multithreaded_http_server_in_cpp
    VERSION 1.0
    DESCRIPTION "Multithreaded HTTP/1.1 Server in C++"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find OpenSSL on the system
find_package(OpenSSL REQUIRED)

# Define the executable and its source files with corrected paths
add_executable(server
    httpserver/server.cc
    internal/header/headers.cc
    internal/request/request.cc
    internal/response/response.cc
    internal/server_services/http_services.cc
)

# Specify include directories so files can find headers easily
target_include_directories(server PRIVATE
    .
    3rd-party-library
)

# Compiler warnings and options
target_compile_options(server PRIVATE
    -Wall
    -Wextra
    -pthread
)

# Link external libraries
target_link_libraries(server PRIVATE
    OpenSSL::SSL
    OpenSSL::Crypto
)
