//
// Created by bain on 02.02.21.
//

#include <filesystem>
#include "connection.h"
#include "../libs/json.hpp"
#include "../crypto/crypto.h"



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

int connection::download(std::string path, const std::string &path_to, const bool &insecure, const bool &overwrite) {

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
        std::cerr << "Invalid url. " << path << std::endl;
        return 1;
    }

    auto* cli = get_valid_client(scheme, host, insecure);
    if (cli == nullptr) return 1;

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
    char salts[32];
    json["salt"].get_to(salts);
    std::string filename_enc;
    json["filename"].get_to(filename_enc);

    // derive key and iv
    char derived_secret[96];
    crypto::derive_secret_pbkdf2(key, salts, sizeof(salts), derived_secret);

    std::string filename = crypto::decrypt_b64string(filename_enc, derived_secret, derived_secret + 64);
    if (filename.empty()) {
        // Assume that no filename means bad decryption
        std::cerr << "Could not decrypt filename (key: " << key << "). Aborting download." << std::endl;
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
    if (std::filesystem::is_directory(path_)) path_/=filename; // add filename if none was provided
    if (std::filesystem::exists(path_) && !overwrite) {
        std::cerr << "File " << path_.filename() << " already exists" << std::endl;
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
                std::memcpy(current_iv, block_dec, 32);
                delete block_dec;
                curr_block_length = 0;
            }
        }
        return true;
        },
    // Progress tracker
    [&filename, &progress](uint64_t len, uint64_t total) {
        if (progress+10 <= (double)len/total*100) {
            progress = (double)len/total*100;
            std::cout << "["<< std::setfill(' ') << std::setw(3) << progress << "%] Downloading and decrypting \"" << filename << "\"\r";
            std::flush(std::cout);
        }
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
