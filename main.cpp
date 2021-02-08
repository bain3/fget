
#include "args.h"
#include <iostream>
#include "download.h"

std::string lower_string(std::string s) {
    for (char & i : s) {
        if (i >= 'A' && i <= 'Z') i += 32;
    }
    return s;
}

int main(int argc, char** argv) {
    Args::Parser parser(argc, argv);
    bool insecure;
    bool overwrite;
    std::string path;
    std::string path_to;
    try {
        bool help = parser.get_arg<bool>({.long_name="help"}, false);
        if (help) {
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
    } catch (Args::Exceptions::ArgumentParsingError& e) {
        std::cout << e.what() << std::endl;
        std::exit(0);
    }
    if (lower_string(path.substr(0, 4)) == "http") return download(path, path_to, insecure, overwrite);
    else std::cout << "upload mode not yet implemented" << std::endl;
    return 0;
}
