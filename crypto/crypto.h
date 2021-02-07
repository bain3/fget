//
// Created by bain on 26.01.21.
//

#ifndef FGET_CRYPTO_H
#define FGET_CRYPTO_H

#include <string>

void derive_secret_pbkdf2(const std::string& password, const char* salts, int salt_len, char* output);
std::string decrypt_string(const std::string& encrypted, char* key, char* iv);
#endif //FGET_CRYPTO_H

