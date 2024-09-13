#ifndef SENDER_H
#define SENDER_H
#include <string_view>
#include "net.h"

class sender_t : public asio::detail::noncopyable
{
public:
    sender_t() = default;
    virtual ~sender_t() = default;

    virtual asio::awaitable<json::object> send_message(std::string_view message) = 0;
    virtual asio::awaitable<void> send_photo(std::string_view path) = 0;
    virtual asio::awaitable<void> send_file(std::string_view path) = 0;
};

#endif //SENDER_H
