
#include "connection.h"
#include <filesystem>
#include <iostream>
#include "../crypto/crypto.h"
#include "../libs/json.hpp"

int
connection::upload(const std::string &path_str, std::string path_to, const int &strength, const bool &insecure) {

    // parse file path
    std::filesystem::path path(path_str);
    if (!std::filesystem::exists(path) || !std::filesystem::is_regular_file(path)) {
        std::cerr << path.string() << " does not point to a valid file." << std::endl;
        return 1;
    }

    // parse host url or domain
    if (path_to == ".") path_to = "f.bain.cz";
    std::string scheme;
    std::string host;
    int scheme_end = path_to.find_first_of("://");
    if (scheme_end != std::string::npos) {
        scheme = lower_string(path_to.substr(0, scheme_end));
    } else {
        scheme_end = -3;
        scheme = "https";
    }
    host = path_to.substr(scheme_end+3);
    if (host[host.length()-1] == '/') host.erase(host.length()-1);

//    auto* cli = get_valid_client(scheme, host, insecure);
//    if (cli == nullptr) return 1;

    // generate key
    std::string key = crypto::generate_key(strength);

    int* salt = crypto::generate_random_ints(2);
    char derived_key[96];
    crypto::derive_secret_pbkdf2(key, reinterpret_cast<char*>(salt), sizeof(salt), derived_key);

    // create metadata
    std::string enc_filename = crypto::encrypt_b64string(path.filename().string(), derived_key, derived_key+64);
    std::cout << path.filename().string() << std::endl;
    std::cout << enc_filename << std::endl;

    nlohmann::json json;
    json["filename"] = enc_filename;
    json["salts"] = std::vector<int>(salt, salt+2);
    std::cout << json.dump() << std::endl;

    return 0;
}
