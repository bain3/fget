cmake_minimum_required(VERSION 3.16)
project(fget)

set(CMAKE_CXX_STANDARD 17)
set(JSON_BuildTests OFF CACHE INTERNAL "")

include(FetchContent)

FetchContent_Declare(
        httplib
        GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
        GIT_TAG v0.8.7
)
FetchContent_MakeAvailable(httplib)

FetchContent_Declare(
        json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.9.1
)
FetchContent_MakeAvailable(json)

add_executable(fget main.cpp crypto/crypto.h crypto/crypto.cpp
        connection/connection.h args.h connection/download.cpp
        connection/upload.cpp connection/connection.cpp connection/maxfs.cpp)

find_package(OpenSSL REQUIRED)
find_package(CryptoPP REQUIRED)
target_link_libraries(fget OpenSSL::SSL OpenSSL::Crypto httplib::httplib nlohmann_json::nlohmann_json)
target_link_libraries(fget cryptopp-static)