//
// Created by bain on 26.01.21.
//

#ifndef FGET_CRYPTO_H
#define FGET_CRYPTO_H

#include <string>
namespace crypto {
    void derive_secret_pbkdf2(const std::string& password, char* salts, int salt_len, char* output);
    std::string decrypt_b64string(const std::string& encrypted, char* key, char* iv);
    char* decrypt_block(char* data, size_t data_len, char* key, char* iv);
}
#endif //FGET_CRYPTO_H

