
#include "args.h"
#include <iostream>
#include "connection/connection.h"

int main(int argc, char** argv) {
    Args::Parser parser(argc, argv);
    bool insecure;
    bool overwrite;
    std::string path;
    std::string path_to;
    int strength;
    try {
        bool help = parser.get_arg<bool>({.long_name="help"}, false);
        if (help || argc == 1) {
            std::cout
            << "Usage: fget [options] path [path_to]" << std::endl
            << " path            Download if URL, upload if local" << std::endl
            << " path_to         Download path" << std::endl
            << " --help          Shows this help message" << std::endl
            << " -o, --overwrite Overwrite files" << std::endl
            << " --insecure      Allows connections through HTTP, not recommended" << std::endl
            << std::endl
            << "Native client for f.bain-like websites. Downloads and uploads encrypted files." << std::endl;

            return 0;
        }
        insecure = parser.get_arg<bool>({.long_name="insecure"}, false);
        overwrite = parser.get_arg<bool>({.long_name="overwrite", .short_name='o'}, false);
        path = parser.get_arg<std::string>({.position=1, .long_name="path"});
        path_to = parser.get_arg<std::string>({.position=2, .long_name="path_to"}, ".");
        strength = parser.get_arg<int>({.long_name="strength", .short_name='s'}, 12);
    } catch (Args::Exceptions::ArgumentParsingError& e) {
        std::cout << e.what() << std::endl;
        std::exit(0);
    }
    if (connection::lower_string(path.substr(0, 4)) == "http")
        return connection::download(path, path_to, insecure, overwrite);
    else
        return connection::upload(path, path_to, strength, insecure);
}
