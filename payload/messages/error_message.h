#ifndef ERROR_MESSAGE_H
#define ERROR_MESSAGE_H

#include "message.h"

#include <string>
#include "net.h"

#define FS_TRY_EXPECTED_OR_RETURN_ERROR_MESSAGE_1(UNAME, VARIABLE, EXPECTED_FUN)    \
    auto UNAME = EXPECTED_FUN;                                                      \
    if(UNAME.has_error())                                                           \
    {                                                                               \
        std::move(completion_handler)(                                              \
            std::make_unique<error_message_t>(UNAME.as_failure().error()));         \
        return;                                                                     \
    }                                                                               \
    VARIABLE = UNAME.assume_value()

#define FS_TRY_EXPECTED_OR_RETURN_ERROR_MESSAGE(VARIABLE, EXPECTED_FUN)\
    FS_TRY_EXPECTED_OR_RETURN_ERROR_MESSAGE_1(FSUNIQUE_NAME, VARIABLE, EXPECTED_FUN)

class error_message_t : public message_t
{
public:
    explicit error_message_t(const std::string& in_error)
    {
        error = in_error;
    }
    explicit error_message_t(const net::error_t& in_error)
    {
        error = in_error.str();
    }
    error_message_t(const error_message_t& em) = default;
    error_message_t& operator=(const error_message_t& em) = default;
    error_message_t(error_message_t&& em) = default;
    error_message_t& operator=(error_message_t&& em) = default;
    ~error_message_t() override = default;

    std::string error;
};

#endif //ERROR_MESSAGE_H
