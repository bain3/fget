//
// Created by bain on 29.01.21.
//

#ifndef FGET_CONNECTION_H
#define FGET_CONNECTION_H

#include <string>
#include <httplib.h>

#define esc "["

namespace connection {
    const int BLOCK_LENGTH = 5242928;
    int download(std::string path, const std::string &path_to, const bool &insecure, const bool &overwrite);

    int upload(const std::string &path_str, std::string path_to, const int &strength, const bool &insecure);

    /*
     * Tests which protocol the client can use. Returns a valid client
     * or nullptr if the server does not support HTTPS and
     * insecure options is not enabled.
     */
    httplib::Client* get_valid_client(const std::string& scheme, const std::string& host, const bool& insecure);

    inline std::string lower_string(std::string s) {
        for (char & i : s) {
            if (i >= 'A' && i <= 'Z') i += 32;
        }
        return s;
    }
}

#endif //FGET_CONNECTION_H
