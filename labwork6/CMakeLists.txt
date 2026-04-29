cmake_minimum_required(VERSION 3.20)
project(wayhome)
set(CMAKE_CXX_STANDARD 17)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

include(FetchContent)

FetchContent_Declare(
    cpr
    GIT_REPOSITORY https://github.com/libcpr/cpr.git
    GIT_TAG        1.11.1
)

FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.2
)

FetchContent_MakeAvailable(cpr json)

add_executable(WayHome main.cpp races.cpp json_utils.cpp)

target_link_libraries(WayHome
    PRIVATE
    cpr::cpr
    nlohmann_json::nlohmann_json
)