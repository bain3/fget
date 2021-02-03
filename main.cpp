
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
    std::string path;
    std::string path_to;
    try {
        insecure = parser.get_arg<bool>({.long_name="insecure"}, false);
        path = parser.get_arg<std::string>({.position=1, .long_name="path"});
        path_to = parser.get_arg<std::string>({.position=2, .long_name="path2"}, ".");
    } catch (Args::Exceptions::ArgumentParsingError& e) {
        std::cout << e.what() << std::endl;
        std::exit(0);
    }
    if (insecure) std::cout << "INSECURE" << std::endl;
    if (lower_string(path.substr(0, 4)) == "http") return download(path, path_to, insecure);
    else std::cout << "upload mode" << std::endl;
    return 0;
}
