//
// Created by bain on 02.02.21.
//

#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif

#include <string>
#include "download.h"
#include "libs/httplib.h"

#define escape "["


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
        std::cout << (std::string) escape + "31m" + "Invalid url." + escape + "0m\n";
        return 1;
    }
    std::cout << "Connecting to " << host << " using " << scheme << ", id is " << id << ", key: " << key << std::endl;

    if (scheme == "http")
        std::cout << escape << "34m"
                  << "Provided protocol is HTTP, trying HTTPS anyways."
                  << escape << "0m" << std::endl;

    auto* cli = new httplib::Client(("https://" + host).data());
    auto res = cli->Options("/");
    if (res.error() != 0) {
        if (scheme == "http") {
            if (insecure) {
                delete cli;
                std::cout << escape << "34m" << "Using unprotected HTTP!" << escape << "0m" << std::endl;
                cli = new httplib::Client(("http://"+host).data());
            } else {
                std::cout << escape << "31m" << "Server does not support HTTPS. Use the --insecure flag if"
                                                " you acknowledge that HTTP is unsecure, but still want to connect."
                          << escape << "0m" << std::endl;
            }
        }
    }
    cli->enable_server_certificate_verification(true);
    std::cout << "Server was successfully tested" << std::endl;
//    std::cout << "Getting website" << std::endl;
//    auto res = cli.Get("/", [](uint64_t len, uint64_t total) {
//                           std::cout << len << " / " << total << std::endl;
//                           return true;
//                       }
//    );
//    if (res.error()) {
//        if (res.error()==10)
//            std::cout << (std::string)escape+"31m"+"An error occurred while connecting to the server."+escape+"0m\n";
//    }
//    std::cout << res->body << std::endl;
    delete cli;
    return 0;
}
