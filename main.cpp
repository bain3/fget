
#include "args.h"
#include <iostream>
#include "connection/connection.h"

int main(int argc, char** argv) {
    Args::Parser parser(argc, argv);
    bool insecure, overwrite, show_max_fs, edit_max_fs;
    std::string path;
    std::string path_to;
    std::string host;
    int strength;
    try {
        bool help = parser.get_arg<bool>({.long_name="help"}, false);
        if (help || argc == 1) {
            std::cout
            << "Usage: fget [options] path [path_to]" << std::endl
            << "  --help          Shows this help message" << std::endl
            << "  --insecure      Allows connections through HTTP, not recommended" << std::endl
            << "  -h, --host      Select a host (default: f.bain.cz)" << std::endl
            << "  --show-max-fs   Shows the file size upload limit" << std::endl
            << "  --edit-max-fs   Edits the file size upload limit with a token" << std::endl
            << "    Example: fget -h f.bain.cz --edit-max-fs <token> 5G" << std::endl
            << "    Size magnitudes are: B K M G T" << std::endl
            << std::endl
            << "Download mode:" << std::endl
            << "  path            URL" << std::endl
            << "  path_to         Save path (default: decrypted filename)" << std::endl
            << "  -o, --overwrite Overwrite files" << std::endl
            << "Upload mode:" << std::endl
            << "  path            Path the the file to upload" << std::endl
            << "  -s, --strength  Encryption key length" << std::endl
            << std::endl
            << "Native client for f.bain-like websites. Downloads and uploads encrypted files." << std::endl;

            return 0;
        }
        insecure = parser.get_arg<bool>({.long_name="insecure"}, false);
        show_max_fs = parser.get_arg<bool>({.long_name="show-max-fs"}, false);
        edit_max_fs = parser.get_arg<bool>({.long_name="edit-max-fs"}, false);
        overwrite = parser.get_arg<bool>({.long_name="overwrite", .short_name='o'}, false);
        strength = parser.get_arg<int>({.long_name="strength", .short_name='s'}, 12);
        host = parser.get_arg<std::string>({.long_name="host", .short_name='h'}, "f.bain.cz");
        if (!show_max_fs) {
            path = parser.get_arg<std::string>({.position=1, .long_name="path"});
            path_to = parser.get_arg<std::string>({.position=2, .long_name="path_to"}, ".");
        }
    } catch (Args::Exceptions::ArgumentParsingError& e) {
        std::cout << e.what() << std::endl;
        std::exit(1);
    }

    if (show_max_fs) {
        int max_fs = connection::get_max_file_size(host, insecure);
        char magnitudes[] = "KMGT";
        int magnitude = -1;
        while (max_fs > 1000 && magnitude < 3) {
            magnitude++;
            max_fs /= 1000;
        }
        std::string out = std::to_string(max_fs);
        if (magnitude >= 0) out += magnitudes[magnitude];
        out += 'B';
        std::cout << out << std::endl;
        return 0;
    } else if (edit_max_fs) {
        return connection::change_max_file_size(host, insecure, path_to, path);
    }

    if (connection::lower_string(path.substr(0, 4)) == "http")
        return connection::download(path, path_to, insecure, overwrite);
    else
        return connection::upload(path, host, strength, insecure);
}
