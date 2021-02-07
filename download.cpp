//
// Created by bain on 02.02.21.
//

#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif

#include <string>
#include "download.h"
#include "libs/httplib.h"
#include "libs/json.hpp"
#include "crypto/crypto.h"

#define esc "["


int download(const std::string &path, const std::string &path_to, const bool &insecure) {

    // Parse url
    std::string scheme;
    std::string host;
    std::string id;
    std::string key;
    int scheme_end = path.find_first_of("://");
    if (scheme_end != std::string::npos) {
        scheme = path.substr(0, scheme_end);
        int host_end = path.find_last_of('/');
        if (host_end != std::string::npos) {
            host = path.substr(scheme_end + 3, host_end - scheme_end - 3);
            int id_end = path.find_first_of('#');
            if (id_end != std::string::npos) {
                id = path.substr(host_end + 1, id_end - host_end - 1);
                key = path.substr(id_end + 1);
            }
        }
    }

    if (scheme.empty() || host.empty() || id.empty() || key.empty()) {
        std::cerr << "Invalid url." << std::endl;
        return 1;
    }
//    std::cout << "Connecting to " << host << " using " << scheme << ", id is " << id << ", key: " << key << std::endl;

    if (scheme == "http")
        std::cout << esc << "34m"
                  << "Provided protocol is HTTP, trying HTTPS anyways."
                  << esc << "0m" << std::endl;

    auto *cli = new httplib::Client(("https://" + host).data());
    cli->enable_server_certificate_verification(true);

    // Check if server supports HTTPS, use HTTP only with the --insecure option
    auto res = cli->Options("/");
    if (res.error() != 0) {
        if (scheme == "http") {
            if (insecure) {
                // The user has provided the --insecure option. Warn the user and check if the server responds on HTTP.
                delete cli;
                std::cout << esc << "34m" << "Using unprotected HTTP!" << esc << "0m" << std::endl;
                cli = new httplib::Client(("http://" + host).data());
                auto reshttp = cli->Options("/");
                if (reshttp.error()) {
                    std::cerr << "Could not connect to " << host << " over HTTP." << std::endl;
                    return 1;
                }
            } else {
                std::cerr << "Server does not support HTTPS. Use the --insecure option if"
                             " you acknowledge that HTTP is unsecure, but still want to connect."
                          << std::endl;
                return 1;
            }
        } else {
            std::cerr << "Could not connect to " << host << std::endl;
        }
    }

    // Get meta about the file. Check if it exists and handle any errors.
    auto meta = cli->Get(("/"+id+"/meta").data());
    if (meta->status != 200) {
        if (meta->status == 404) {
            std::cerr << "No file with the provided ID was found on the server." << std::endl;
        } else {
            std::cerr << "Server returned error code " << meta->status << std::endl;
        }
    }

    // Parse json
    nlohmann::json json = nlohmann::json::parse(meta->body);
    int salts_int[2]; json["salt"].get_to(salts_int);
    std::string filename_enc; json["filename"].get_to(filename_enc);

    // derive key and iv
    auto* salts = static_cast<char*>(static_cast<void *>(salts_int));
    char derived_secret[96];
    derive_secret_pbkdf2(key, salts, sizeof(salts), derived_secret);
    long long sum = 0;
    for (char i : derived_secret) sum+= i;
    std::cout << sum << std::endl;
    // TODO: Check if filename was successfully decrypted
    std::string filename = decrypt_string(filename_enc, derived_secret, derived_secret+64);

    std::cout << "Downloading \"" << filename << "\"...";
//    std::cout << "Getting website" << std::endl;
//    auto res = cli.Get("/", [](uint64_t len, uint64_t total) {
//                           std::cout << len << " / " << total << std::endl;
//                           return true;
//                       }
//    );
//    if (res.error()) {
//        if (res.error()==10)
//            std::cout << (std::string)esc+"31m"+"An error occurred while connecting to the server."+esc+"0m\n";
//    }
//    std::cout << res->body << std::endl;
    delete cli;
    return 0;
}
