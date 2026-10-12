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

include(FetchContent)

FetchContent_Declare(
    minhook
    GIT_REPOSITORY https://github.com/tsudakageyu/minhook.git
    GIT_TAG        8af6b4acae5a9388fd742b56fa79ece89d96f823
)

FetchContent_MakeAvailable(minhook)

if(TARGET minhook AND NOT TARGET minhook::minhook)
    add_library(minhook::minhook ALIAS minhook)
endif()

FetchContent_Declare(
    augment
    GIT_REPOSITORY https://github.com/Linifadomra/Augment.git
    GIT_TAG        79c3561e9fa79cf13cdd781bf3c21d047bd1426e
)

FetchContent_MakeAvailable(augment)

add_library(intercept SHARED ${CMAKE_CURRENT_SOURCE_DIR}/src/intercept.cpp)
target_link_libraries(intercept PRIVATE augment ws2_32 dbghelp minhook::minhook)

set_target_properties(intercept PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
