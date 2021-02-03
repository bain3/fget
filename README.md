# fget - Native client for f.bain(-like websites)
Since f.bain cannot serve files directly utilities like wget do not work.
This native client is a command line utility allowing headless computers to
download and upload content to f.bain(-like websites).

## Compilation
This project uses cmake. A few libraries need to be linked to
successfully compile:
 - Cryptopp ([how to compile using cmake](https://cryptopp.com/wiki/CMake))
 - OpenSSL

Both libraries should be available on vcpkg if you're using windows.