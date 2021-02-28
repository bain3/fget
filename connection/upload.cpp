
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

    // generate key & salt
    std::string key = crypto::generate_key(strength);
    char* salt = crypto::random(32);
    char derived_key[96];
    crypto::derive_secret_pbkdf2(key, salt, 32, derived_key);

    // dump metadata into json and set headers
    nlohmann::json json;
    json["filename"] = crypto::encrypt_b64string(path.filename().string(), derived_key, derived_key+64);
    json["salt"] = std::vector<char>(salt, salt+32);
    httplib::Headers headers = {
            {"X-Metadata", crypto::b64encode(json.dump())},
            {"User-Agent", "fget"}
    };

    // prepare block and file
    size_t size_data = std::filesystem::file_size(path);
    size_t size_out = size_data+(int)(std::ceil((float)size_data/(BLOCK_LENGTH-48)))*48;
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Could not open file." << std::endl;
        return 1;
    }
    char plaintext_block[BLOCK_LENGTH-16];
    char* cipher_block;
    char current_iv[32];
    std::memcpy(current_iv, derived_key+32, 32);
    size_t cipher_block_offset;
    size_t cipher_block_remaining = 0;

    // get a valid client
    auto* cli = get_valid_client(scheme, host, insecure);
    if (cli == nullptr) return 1;

    // setup for tracking progress
    size_t done = 0;
    int progress = -10;

    // encrypt and upload the file
    auto res = cli->Post("/new", headers, size_out,
    [&](size_t offset, size_t length, httplib::DataSink &sink) {
        while (length) {

            // progress tracking
            if (progress+10 <= (double)done/size_out*100) {
                progress = (double)done/size_out*100;
                std::cout << "["<< std::setfill(' ') << std::setw(3) << progress << "%] Encrypting and uploading \""
                << path.filename().string() << "\"\r";
                std::flush(std::cout);
            }

            // check if we have enough cipher data
            if (cipher_block_remaining >= length) {
                // send data
                sink.write(cipher_block+offset-cipher_block_offset, length);
                done += length;
                cipher_block_remaining -= length;
                length = 0;
            } else {
                // we do not have enough data, create a new block
                // send reminder of current block, if any
                if (cipher_block_remaining!=0) {
                    sink.write(cipher_block + offset - cipher_block_offset, cipher_block_remaining);
                    done += cipher_block_remaining;
                    offset += cipher_block_remaining;
                    length -= cipher_block_remaining;
                }

                // get plaintext data from file
                if (!file.good()) {
                    std::cerr << "File stream gone bad. Something happened with the file being uploaded. Aborting..."
                              << std::endl;
                    return false;
                }
                size_t cur_block_len = file.tellg();
                file.read(plaintext_block + 32, BLOCK_LENGTH - 48);
                cur_block_len = (file.tellg() == -1 ? size_data : (size_t)file.tellg())-cur_block_len+32;

                // generate and copy new salts into the plaintext block
                char* new_iv = crypto::random(32);
                std::memcpy(plaintext_block, new_iv, 32);

                // encrypt the block
                delete[] cipher_block;
                cipher_block = crypto::encrypt_block(plaintext_block, cur_block_len, derived_key, current_iv);

                cipher_block_offset = (size_t)(offset/BLOCK_LENGTH)*BLOCK_LENGTH;
                cipher_block_remaining = cur_block_len+16;

                // set new iv (byte order problem again)
                std::memcpy(current_iv, new_iv, 32);
                delete new_iv;
            }
        }
        return true;
    },"application/octet-stream");

    if (res.error() || res->status != 200) {
        std::cerr << "Failed to upload the file." << std::endl;
        std::cerr << res->body << std::endl;
        std::cerr << host << std::endl;
        return 1;
    }

    // progress tracking
    std::cout << "[100%] Encrypting and uploading \"" << path.filename().string() << "\"" << std::endl;

    // parse received json and print the final url, and revocation token
    nlohmann::json response_json = nlohmann::json::parse(res->body);
    std::cout << scheme << "://" << host << "/" << response_json["uuid"].get<std::string>() << "#" << key << std::endl;
    std::cout << "Revocation token: " << response_json["revocation_token"].get<std::string>() << std::endl;
    return 0;
}
