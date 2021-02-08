//
// Created by bain on 02.02.21.
//

#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
#define CPPHTTPLIB_OPENSSL_SUPPORT
#endif

#include <string>
#include <filesystem>
#include "download.h"
#include "libs/httplib.h"
#include "libs/json.hpp"
#include "crypto/crypto.h"

#define esc "["

const int BLOCK_LENGTH = 5242928;

std::string urlDecode(std::string str){
    std::string ret;
    char ch;
    int i, ii, len = str.length();

    for (i=0; i < len; i++){
        if(str[i] == '%'){
            sscanf(str.substr(i + 1, 2).c_str(), "%x", &ii);
            ch = static_cast<char>(ii);
            ret += ch;
            i = i + 2;
        } else {
            ret += str[i];
        }
    }
    return ret;
}
int download(std::string path, const std::string &path_to, const bool &insecure, const bool &overwrite) {

    // Parse url
    path = urlDecode(path);
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
    auto meta = cli->Get(("/" + id + "/meta").data());
    if (meta->status != 200) {
        if (meta->status == 404) {
            std::cerr << "No file with the provided ID was found on the server." << std::endl;
        } else {
            std::cerr << "Server returned error code " << meta->status << std::endl;
        }
        return 1;
    }

    // Parse json
    nlohmann::json json = nlohmann::json::parse(meta->body);
    int salts_int[2];
    json["salt"].get_to(salts_int);
    std::string filename_enc;
    json["filename"].get_to(filename_enc);

    // derive key and iv
    auto *salts = static_cast<char *>(static_cast<void *>(salts_int));
    char derived_secret[96];
    crypto::derive_secret_pbkdf2(key, salts, sizeof(salts), derived_secret);

    std::string filename = crypto::decrypt_b64string(filename_enc, derived_secret, derived_secret + 64);
    if (filename.empty()) {
        // Assume that no filename means bad decryption
        std::cerr << "Could not decrypt filename. Aborting download." << std::endl;
        return 1;
    }

    // allocate 5mb of memory for a block, set the iv
    char* block = new char[BLOCK_LENGTH];
    size_t curr_block_length = 0;
    size_t total = 0;
    char current_iv[32];
    std::memcpy(current_iv, derived_secret+32, 32);

    // open output file
    std::ofstream outputfile;
    std::filesystem::path path_(path_to);
    if (!path_.has_filename() || std::filesystem::is_directory(path_)) path_/=filename; // add filename if none was provided
    if (std::filesystem::exists(path_) && !overwrite) {
        std::cerr << "File with the name " << path_.filename() << " already exists" << std::endl;
        return 1;
    }
    outputfile.open(path_);

    int progress = -10;
    // request data and decrypt
    auto content_resp = cli->Get(("/"+id+"/raw").data(),
    // Content receiver
    [&](const char *data, size_t data_length) {
        size_t data_offset = 0;
        while (data_offset < data_length) {
            size_t to_copy = std::min(BLOCK_LENGTH-curr_block_length, data_length-data_offset);
            std::memcpy(block+curr_block_length, data+data_offset, to_copy);
            curr_block_length += to_copy;
            data_offset += to_copy;
            total += to_copy;
            // If the curr_block_length is bigger then abort. Something went horribly wrong.
            if (curr_block_length > BLOCK_LENGTH) {
                std::cerr << "current block length is bigger than max, abort" << std::endl;
                return false;
            }
            if (curr_block_length == BLOCK_LENGTH) {
                char* block_dec = crypto::decrypt_block(block, curr_block_length, derived_secret, current_iv);
                if (block_dec == nullptr) {
                    std::cerr << "Decryption error" << std::endl;
                    return false;
                }
                outputfile.write(block_dec+32, curr_block_length-48);
                outputfile.flush();
                // epic byte order problems again
                for (int i = 0; i < 32; i++) {
                    current_iv[i / 4 * 4 + i % 4] = block_dec[i / 4 * 4 + 3 - (i % 4)];
                }
                delete block_dec;
                curr_block_length = 0;
            }
        }
        return true;
        },
    // Progress tracker
    [&filename, &progress](uint64_t len, uint64_t total) {
        if (progress+10 > len/total*100) return true;
        progress = len/total*100;
        std::cout << "["<< std::setfill(' ') << std::setw(3) << progress << "%] Downloading and decrypting \"" << filename << "\"" << std::endl;
        return true;
    });
    std::cout << std::endl;
    if (curr_block_length != 0) {
        char* block_dec = crypto::decrypt_block(block, curr_block_length, derived_secret, current_iv);
        if (block_dec == nullptr) {
            std::cerr << "Decryption error" << std::endl;
            return false;
        }
        outputfile.write(block_dec+32, curr_block_length-48);
        outputfile.flush();
        delete block_dec;
        curr_block_length = 0;
    }
    outputfile.close();
    std::cout << "File saved as " << path_.filename() << std::endl;
    delete cli;
    return 0;
}
