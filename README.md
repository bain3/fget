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

If on linux, you can build fget with `build.sh`. It will install cryptopp and libssl 
globally, so its best to run it in a docker container. The output is either
a `.deb` file, if the system is ubuntu/debian (exe is in the fake-root folder),
or an executable located in the project root. The only dependency for systems is
libc greater or equal to the one used by the building os, and libssl (openssl).
The build script currently supports arch based and debian based systems.

## Usage
```
Usage: fget [options] path [path_to]
  --help          Shows this help message
  --insecure      Allows connections through HTTP, not recommended
  -h, --host      Select a host (default: f.bain.cz)
  --show-max-fs   Shows the file size upload limit
  --edit-max-fs   Edits the file size upload limit with a token
    Example: fget -h f.bain.cz --edit-max-fs <token> 5G
    Size magnitudes are: B K M G T

Download mode:
  path            URL
  path_to         Save path (default: decrypted filename)
  -o, --overwrite Overwrite files
Upload mode:
  path            Path the the file to upload
  -s, --strength  Encryption key length
  
Examples:
  $ fget sup3rsecret.doc   
      uploads sup3rsecret.doc to f.bain, outputs a link and a revocation token
  $ fget https://f.bain.cz/XXXXX#aaaaaaaaaaaa
      downloads a file from f.bain
  $ fget https://f.bain.cz/XXXXX#aaaaaaaaaaaa secret.doc
      downloads a file from f.bain, saves as "secret.doc"

Native client for f.bain-like websites. Downloads and uploads encrypted files.
```
