cmake_minimum_required(VERSION 3.20)

project(supervisor VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_C_STANDARD 11)

add_executable(supervisor
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/app.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/net.cpp"
)

target_include_directories(supervisor PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_library(intercept SHARED intercept.cpp)
target_link_libraries(intercept PRIVATE minhook) # or path to minhook.lib

set_target_properties(intercept PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)
