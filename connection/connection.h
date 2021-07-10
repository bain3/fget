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

    int upload(const std::string &path_str, const std::string& host_url, const int &strength, const bool &insecure);

    /**
     * Tests which protocol the client can use. Returns a valid client
     * or nullptr if the server does not support HTTPS and
     * insecure options is not enabled.
     */
    httplib::Client* get_valid_client(const std::string& scheme, const std::string& host, const bool& insecure);

    int get_max_file_size(httplib::Client* client);
    int get_max_file_size(const std::string& host_url, bool insecure);

    int change_max_file_size(const std::string &host_url, bool insecure, const std::string &new_val,
                             const std::string &token);

    inline std::string lower_string(std::string s) {
        for (char & i : s) {
            if (i >= 'A' && i <= 'Z') i += 32;
        }
        return s;
    }

    inline void get_host_scheme(const std::string& host_url, std::string& host, std::string& scheme) {
        // parse host url or domain
        int scheme_end = host_url.find_first_of("://");
        if (scheme_end != std::string::npos) {
            scheme = lower_string(host_url.substr(0, scheme_end));
        } else {
            scheme_end = -3;
            scheme = "https";
        }
        host = host_url.substr(scheme_end + 3);
        if (host[host.length()-1] == '/') host.erase(host.length()-1);
    }
}

#endif //FGET_CONNECTION_H
