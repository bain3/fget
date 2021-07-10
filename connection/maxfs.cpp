#include "connection.h"
#include <nlohmann/json.hpp>

int connection::get_max_file_size(httplib::Client *client) {
    auto resp = client->Get("/max-filesize");

    if (resp.error() != httplib::Error::Success || resp->status != 200)
        return -1;

    nlohmann::json resp_json = nlohmann::json::parse(resp->body);
    return resp_json["max"].get<int>();
}

int connection::get_max_file_size(const std::string &host_url, bool insecure) {
    // parse host url or domain
    std::string scheme;
    std::string host;
    get_host_scheme(host_url, host, scheme);

    httplib::Client *c = get_valid_client(scheme, host, insecure);
    int max = get_max_file_size(c);

    return max;
}

int connection::change_max_file_size(const std::string &host_url, bool insecure, const std::string &new_val,
                                     const std::string &token) {
    // parse host url or domain
    std::string scheme;
    std::string host;
    get_host_scheme(host_url, host, scheme);
    httplib::Client *client = get_valid_client(scheme, host, insecure);

    httplib::Headers headers = {{"Authorization", token}};
    auto resp = client->Post(("/max-filesize/" + new_val).data(), headers, "", "application/json");
    if (resp.error() != httplib::Error::Success) {
        std::cerr << "Error while contacting host." << std::endl;
        return 1;
    }

    if (resp->status == 401) {
        std::cerr << "Error while changing limit. Invalid token." << std::endl;
        return 1;
    } else if (resp->status == 200) {
        std::cout << "Successfully changed limit to " << new_val << std::endl;
        return 0;
    } else {
        std::cerr << "Unexpected error code (" << resp->status << ")" << std::endl;
        return 1;
    }
}
