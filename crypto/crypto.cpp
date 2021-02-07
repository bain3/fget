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

void derive_secret_pbkdf2(const std::string& password, const char* salts, int salt_len, char* output) {

    auto* salt_bytes = new CryptoPP::byte[salt_len];

    // Problem with byte order, ints are read the wrong way. Wasn't fun to resolve.
    for (int i = 0; i < salt_len; i++) {
        salt_bytes[i/4*4+i%4] = salts[i/4*4+3-(i%4)];
    }
    CryptoPP::byte derived[96];
    CryptoPP::PKCS5_PBKDF2_HMAC<CryptoPP::SHA256> pbkdf;
    CryptoPP::byte unused = 0;
    pbkdf.DeriveKey(derived, sizeof(derived), unused, (CryptoPP::byte*)password.data(), password.size(),
                    salt_bytes, salt_len, 50000, 0.0f);
    std::string result;
    CryptoPP::HexEncoder encoder(new CryptoPP::StringSink(result));

    encoder.Put(derived, sizeof(derived));
    encoder.MessageEnd();
    std::memcpy(output, derived, 96);
}

std::string decrypt_string(const std::string& encrypted, char* key, char* iv) {

    CryptoPP::Base64Decoder decoder;
    decoder.Put( (CryptoPP::byte*)encrypted.data(), encrypted.size() );
    decoder.MessageEnd();

    CryptoPP::word64 size = decoder.MaxRetrievable();
    auto* decoded = new CryptoPP::byte[size];
    if(size && size <= SIZE_MAX)
    {
        decoder.Get(decoded, size);
    }

    std::string output;
    try
    {
        CryptoPP::GCM< CryptoPP::AES >::Decryption d;
        d.SetKeyWithIV(reinterpret_cast<const CryptoPP::byte *>(key), 32, reinterpret_cast<const CryptoPP::byte *>(iv), 32);

        CryptoPP::AuthenticatedDecryptionFilter df( d,
                                          new CryptoPP::StringSink( output )
        ); // AuthenticatedDecryptionFilter

        // The StringSource dtor will be called immediately
        //  after construction below. This will cause the
        //  destruction of objects it owns. To stop the
        //  behavior so we can get the decoding result from
        //  the DecryptionFilter, we must use a redirector
        //  or manually Put(...) into the filter without
        //  using a StringSource.
        df.Put(decoded, size);
        df.MessageEnd();


        // If the object does not throw, here's the only
        //  opportunity to check the data's integrity
        if(df.GetLastResult()) {
            return output;
        }
    }
    catch( CryptoPP::Exception& e )
    {
        return "";
    }
    return std::string();
}