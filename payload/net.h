#pragma once

#define _WIN32_WINNT 0x0601
#include <filesystem>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http/fields.hpp>
#include <boost/outcome.hpp>

#include <boost/asio/awaitable.hpp>

#include <boost/asio/experimental/awaitable_operators.hpp>

#include <boost/json.hpp>

#include <iostream>
#include <ranges>
#include <syncstream>
#include <Wincrypt.h>

#include <cryptopp/gcm.h>
#include <cryptopp/aes.h>
#include <cryptopp/filters.h>
#include <cryptopp/base64.h>
#include <cryptopp/hex.h>

#define MY_ENCODING_TYPE  (PKCS_7_ASN_ENCODING | X509_ASN_ENCODING)

#define FSGLUE2(x, y) x##y
#define FSGLUE(x, y) FSGLUE2(x, y)
#define FSUNIQUE_NAME FSGLUE(_fs_unique_name_temporary, __COUNTER__)

#define FS_TRY_EXPECTED_RETURN(VARIABLE, EXPECTED_FUN) BOOST_OUTCOME_TRYA(VARIABLE, EXPECTED_FUN)

#define FS_DEFER2(UNIQUE, FUNC) \
    std::shared_ptr<void> UNIQUE(nullptr, FUNC )
#define FS_DEFER(FUNC) FS_DEFER2(FSUNIQUE_NAME, FUNC)

#ifndef _NDEBUG
#define DLOG(MSG) std::osyncstream(std::cerr) << MSG << std::endl
#else
#define DLOG(MSG) do {} while (0)
#endif

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;
namespace ssl = asio::ssl;
namespace json = boost::json;
namespace fs = std::filesystem;

using namespace asio::experimental::awaitable_operators;

using tcp = asio::ip::tcp;
using udp = asio::ip::udp;

using ssl_socket = ssl::stream<tcp::socket>;
using error_code = boost::system::error_code;

namespace rng
{
    using namespace std::ranges;
    using namespace std::ranges::views;

    template<class T>
    class as_t
    {
    public:
        template<class Self, class Rng>
        friend static T operator|(const Rng& rng, const Self&)
        {
            T to_ret;
            for(auto item : rng)
                to_ret.push_back(item);
            return to_ret;
        }
    };
}

namespace net
{
    class error_t
    {
    public:
        explicit error_t(const std::string& in_msg)
        {
            msg = in_msg;
        }

        [[nodiscard]] std::string str() const { return msg; }

    protected:
        std::string msg;
    };

    inline json::object create_internal_error(const char* message)
    {
        return {
            {"ok", false},
            {"error_code", "500"},
            {"description", message}
        };
    }

    template <class TBegin, class TEnd>
    auto wait(TBegin begin, TEnd end)
    {
        if (begin + 1 == end)
            return std::move(*begin);
        return std::move(*begin) && std::move(wait(begin + 1, end));
    }

    inline std::filesystem::path get_temp_path()
    {
        thread_local fs::path temp_path;
        if (temp_path == "")
        {
            TCHAR appdata_path[MAX_PATH];
            GetTempPath(MAX_PATH, appdata_path);
            temp_path = appdata_path;
        }

        return temp_path;
    }

    inline std::string generate_random_string(std::size_t length)
    {
        static std::string chars(
            "abcdefghijklmnopqrstuvwxyz"
            "ABCDEFGHIJKLMNOPQRSTUVWXYZ");

        std::string to_ret;
        to_ret.reserve(length);
        for (int i = 0; i < length; ++i)
            to_ret += chars[rand() % (chars.size() - 1)];

        return to_ret;
    }

    inline fs::path get_temp_unique_file_path()
    {
        return fs::temp_directory_path()
#ifndef _NDEBUG
            / "test_cookies"
#endif
            / net::generate_random_string(15);
    }

    template <class T>
    using expected = boost::outcome_v2::result<T, error_t>;

    struct cookie_t
    {
        std::string host;
        std::string name;
        std::string path;
        std::string cookie;
        std::uint32_t expiry;

        cookie_t(std::string_view in_host, std::string_view in_name, std::string_view in_path
                 , std::string_view in_cookie, std::uint32_t in_expiry)
        {
            host = in_host;
            name = in_name;
            path = in_path;
            cookie = in_cookie;
            expiry = in_expiry;
        }
    };

    namespace aes_gsm
    {
        inline void handleErrors()
        {
            printf("Some error occured\n");
        }

        inline int encrypt(unsigned char* plaintext, int plaintext_len, unsigned char* aad,
                    int aad_len, unsigned char* key, unsigned char* iv,
                    unsigned char* ciphertext, unsigned char* tag)
        {
            EVP_CIPHER_CTX* ctx;

            int len = 0, ciphertext_len = 0;

            /* Create and initialise the context */
            if (!(ctx = EVP_CIPHER_CTX_new()))
                handleErrors();

            /* Initialise the encryption operation. */
            if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL))
                handleErrors();

            /* Set IV length if default 12 bytes (96 bits) is not appropriate */
            if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, 16, NULL))
                handleErrors();

            /* Initialise key and IV */
            if (1 != EVP_EncryptInit_ex(ctx, NULL, NULL, key, iv)) handleErrors();

            /* Provide any AAD data. This can be called zero or more times as
             * required
             */
            if (1 != EVP_EncryptUpdate(ctx, NULL, &len, aad, aad_len))
                handleErrors();

            /* Provide the message to be encrypted, and obtain the encrypted output.
             * EVP_EncryptUpdate can be called multiple times if necessary
             */
            /* encrypt in block lengths of 16 bytes */
            while (ciphertext_len <= plaintext_len - 16)
            {
                if (1 != EVP_EncryptUpdate(ctx, ciphertext + ciphertext_len, &len, plaintext + ciphertext_len, 16))
                    handleErrors();
                ciphertext_len += len;
            }
            if (1 != EVP_EncryptUpdate(ctx, ciphertext + ciphertext_len, &len, plaintext + ciphertext_len,
                                       plaintext_len - ciphertext_len))
                handleErrors();
            ciphertext_len += len;

            /* Finalise the encryption. Normally ciphertext bytes may be written at
             * this stage, but this does not occur in GCM mode
             */
            if (1 != EVP_EncryptFinal_ex(ctx, ciphertext + ciphertext_len, &len)) handleErrors();
            ciphertext_len += len;

            /* Get the tag */
            if (1 != EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, 16, tag))
                handleErrors();

            /* Clean up */
            EVP_CIPHER_CTX_free(ctx);

            return ciphertext_len;
        }


        inline int decrypt(unsigned char *ciphertext, int ciphertext_len,
                unsigned char *aad, int aad_len,
                unsigned char *tag,
                unsigned char *key,
                unsigned char *iv, int iv_len,
                unsigned char *plaintext)
        {
            EVP_CIPHER_CTX *ctx;
            int len;
            int plaintext_len;
            int ret;

            /* Create and initialise the context */
            if(!(ctx = EVP_CIPHER_CTX_new()))
                handleErrors();

            /* Initialise the decryption operation. */
            if(!EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL))
                handleErrors();

            /* Set IV length. Not necessary if this is 12 bytes (96 bits) */
            if(!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, iv_len, NULL))
                handleErrors();

            /* Initialise key and IV */
            if(!EVP_DecryptInit_ex(ctx, NULL, NULL, key, iv))
                handleErrors();

            /*
             * Provide any AAD data. This can be called zero or more times as
             * required
             */
            if(!EVP_DecryptUpdate(ctx, NULL, &len, aad, aad_len))
                handleErrors();

            /*
             * Provide the message to be decrypted, and obtain the plaintext output.
             * EVP_DecryptUpdate can be called multiple times if necessary
             */
            if(!EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len))
                handleErrors();
            plaintext_len = len;

            /* Set expected tag value. Works in OpenSSL 1.0.1d and later */
            if(!EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, 16, tag))
                handleErrors();

            /*
             * Finalise the decryption. A positive return value indicates success,
             * anything else is a failure - the plaintext is not trustworthy.
             */
            ret = EVP_DecryptFinal_ex(ctx, plaintext + len, &len);

            /* Clean up */
            EVP_CIPHER_CTX_free(ctx);

            if(ret > 0) {
                /* Success */
                plaintext_len += len;
                return plaintext_len;
            } else {
                /* Verify failed */
                return -1;
            }
        }
    }

    namespace win_crypt
    {
        inline expected<std::vector<BYTE>> encrypt(const BYTE* const data, unsigned int len)
        {
            DATA_BLOB DataIn = { len, (BYTE*)data};
            DATA_BLOB DataOut = {0};
            LPWSTR pDescrOut = nullptr;
            if(!CryptUnprotectData(&DataIn,
                &pDescrOut,
                nullptr,
                nullptr,
                nullptr,
                CRYPTPROTECT_AUDIT,
                &DataOut))
            {
#ifndef _NDEBUG
                auto errorMessageID =  ::GetLastError();

                LPSTR messageBuffer = nullptr;

                size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                             NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

                std::string message(messageBuffer, size);

                LocalFree(messageBuffer);

                return net::error_t(
                    std::format("WinCrypt(ERROR): encryption error {}", message));
#else // RELEASE
                return net::error_t("WinCrypt(ERROR): encryption error");
#endif
            }

            std::vector<BYTE> to_ret(DataOut.pbData, DataOut.pbData + DataOut.cbData);

            LocalFree(DataOut.pbData);

            return to_ret;
        }
    }
}
