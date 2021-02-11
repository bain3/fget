cmake_minimum_required(VERSION 3.17)
project(fget)

set(CMAKE_CXX_STANDARD 17)

add_executable(fget main.cpp libs/fastpbkdf2.c crypto/crypto.h crypto/crypto.cpp
        connection/connection.h args.h connection/download.cpp
        libs/httplib.cpp connection/upload.cpp connection/connection.cpp)

find_package(OpenSSL REQUIRED)
find_package(CryptoPP REQUIRED)
target_link_libraries(fget OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(fget cryptopp-static)