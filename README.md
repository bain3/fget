# fget - Native client for f.bain(-like websites)
Since f.bain cannot serve files directly, utilities like wget do not work.
This native client is a command line utility allowing headless computers to
download and upload content to f.bain(-like websites).

## Compilation
This project uses cmake. A few libraries need to be linked to
successfully compile:
 - Cryptopp ([how to compile using cmake](https://cryptopp.com/wiki/CMake))
 - OpenSSL

Both libraries should be available on vcpkg if you're using windows.

## Usage
```
Usage: fget [options] path [path_to]
  --help          Shows this help message
  --insecure      Allows connections through HTTP, not recommended

Download mode:
  path            URL
  path_to         Save path (default: decrypted filename)
  -o, --overwrite Overwrite files
Upload mode:
  path            Path the the file to upload
  path_to         Host which will be used (default: f.bain.cz)
  -s, --strength  How long the encryption key should be

Native client for f.bain-like websites. Downloads and uploads encrypted files.
```