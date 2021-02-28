//
// Created by bain on 26.01.21.
//

#ifndef FGET_CRYPTO_H
#define FGET_CRYPTO_H

#include <string>
namespace crypto {
    void derive_secret_pbkdf2(const std::string& password, const char* salts, int salt_len, char* output);
    std::string encrypt_b64string(const std::string &decrypted, char *key, char *iv);
    std::string decrypt_b64string(const std::string& encrypted, char* key, char* iv);
    std::string b64encode(const std::string& string);
    char* decrypt_block(char* data, size_t data_len, char* key, char* iv);
    char* encrypt_block(char* data, size_t data_len, char* key, char* iv);
    char * random(const int &number);

    std::string generate_key(const int &strength);
}
#endif //FGET_CRYPTO_H

