//
// Created by bain on 26.01.21.
//
#include "crypto.h"
#include <cryptopp/sha.h>
#include <cryptopp/pwdbased.h>
#include <cryptopp/hex.h>
#include <cryptopp/base64.h>
#include <cryptopp/gcm.h>
#include <cryptopp/aes.h>
#include <cryptopp/osrng.h>
#include <iostream>

char base73[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ$-_.+!*'(),";

namespace crypto {
    void derive_secret_pbkdf2(const std::string &password, const char *salts, int salt_len, char *output) {
        auto *salt_bytes = new CryptoPP::byte[salt_len];

        // Problem with byte order, ints are read the wrong way. Wasn't fun to resolve.
        for (int i = 0; i < salt_len; i++) {
            salt_bytes[i / 4 * 4 + i % 4] = salts[i / 4 * 4 + 3 - (i % 4)];
        }
        CryptoPP::byte derived[96];
        CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA256> pbkdf;
        CryptoPP::byte unused = 0;
        pbkdf.DeriveKey(derived, sizeof(derived), unused, (CryptoPP::byte *) password.data(), password.size(),
                        salt_bytes, salt_len, 50000, 0.0f);
        std::string result;
        CryptoPP::HexEncoder encoder(new CryptoPP::StringSink(result));

        encoder.Put(derived, sizeof(derived));
        encoder.MessageEnd();
        std::memcpy(output, derived, 96);
    }

    std::string decrypt_b64string(const std::string &encrypted, char *key, char *iv) {
        std::string output;
        try {
            // CryptoPP's pipe system is quite cool
            CryptoPP::GCM<CryptoPP::AES>::Decryption d;
            d.SetKeyWithIV(reinterpret_cast<const CryptoPP::byte *>(key), 32,
                           reinterpret_cast<const CryptoPP::byte *>(iv), 32);
            CryptoPP::AuthenticatedDecryptionFilter df(d, new CryptoPP::StringSink(output));
            // b64 string -> b64 decoder -> DecryptionFilter(through redirector) -> string(sink)
            CryptoPP::StringSource ss(encrypted, true,
                                      new CryptoPP::Base64Decoder(new CryptoPP::Redirector(df)));
            if (df.GetLastResult()) { // confirm the message digest was correct
                return output;
            }
        }
        catch (CryptoPP::Exception &e) {
            return "";
        }
        return "";
    }

    std::string encrypt_b64string(const std::string &decrypted, char *key, char *iv) {
        std::string output;
        try {
            CryptoPP::GCM<CryptoPP::AES>::Encryption e;
            e.SetKeyWithIV(reinterpret_cast<const CryptoPP::byte *>(key), 32,
                           reinterpret_cast<const CryptoPP::byte *>(iv), 32);
            CryptoPP::AuthenticatedEncryptionFilter ef(e, new CryptoPP::Base64Encoder(
                    new CryptoPP::StringSink(output)));
            // plaintext -> ecryption filter -> base64 encoder -> cipher string
            CryptoPP::StringSource ss(decrypted, true,
                                      new CryptoPP::Redirector(ef));
            return output;
        }
        catch (CryptoPP::Exception &e) {
            return "";
        }
    }

    char* decrypt_block(char* data, size_t data_len, char* key, char* iv) {
        auto* output = new CryptoPP::byte[data_len];
        try {
            CryptoPP::GCM<CryptoPP::AES>::Decryption d;
            d.SetKeyWithIV(reinterpret_cast<const CryptoPP::byte *>(key), 32,
                           reinterpret_cast<const CryptoPP::byte *>(iv), 32);
            CryptoPP::AuthenticatedDecryptionFilter df(d, new CryptoPP::ArraySink(output, data_len));
            CryptoPP::ArraySource ss((CryptoPP::byte*)data, data_len, true,
                                     new CryptoPP::Redirector(df));
            if (df.GetLastResult()) { // confirm the message digest was correct
                return (char*)output;
            }
        }
        catch (CryptoPP::Exception &e) {
            return nullptr;
        }
        return nullptr;
    }

    std::string generate_key(const int &strength) {
        auto* random = new CryptoPP::byte[strength*8];
        CryptoPP::AutoSeededRandomPool rng;
        rng.GenerateBlock(random, strength*8);
        std::string output;
        do {
            rng.GenerateBlock(random, strength*8);
            output = "";
            for (int i = 0; i < strength; i++) {
                char c = base73[random[i] % 73];
                output.push_back(c);
            }
        } while (output[strength-1] == '.' || output[strength-1] == ',' || output[strength-1] == ')');

        return output;
    }

    int* generate_random_ints(const int &number) {
        auto* random = new CryptoPP::byte[number*32];
        CryptoPP::AutoSeededRandomPool rng;
        rng.GenerateBlock(random, number*32);
        return (int*)random;
    }

    std::string b64encode(const std::string& string) {
        std::string output;
        CryptoPP::StringSink sink(output);
        CryptoPP::StringSource ss(string, true, new CryptoPP::Base64Encoder(&sink));
        return output;
    }
}

