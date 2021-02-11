//
// Created by bain on 11.02.21.
//

#include "connection.h"

httplib::Client *
connection::get_valid_client(const std::string &scheme, const std::string &host, const bool &insecure) {

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
                    return nullptr;
                }
            } else {
                std::cerr << "Server does not support HTTPS. Use the --insecure option if"
                             " you acknowledge that HTTP is unsecure, but still want to connect."
                          << std::endl;
                return nullptr;
            }
        } else {
            std::cerr << "Could not connect to " << host << std::endl;
        }
    }
    return cli;
}