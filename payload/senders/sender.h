#ifndef SENDER_H
#define SENDER_H
#include <string_view>
#include "net.h"
#include <messages/message.h>

class error_message_t;
class photo_message_t;

class sender_t : public asio::detail::noncopyable
{
public:
    sender_t() = default;
    virtual ~sender_t() = default;

    virtual asio::awaitable<json::object> send_message(const std::unique_ptr<message_t>& message) = 0;
    virtual asio::awaitable<json::object> send_text(std::string_view text) = 0;
    virtual asio::awaitable<json::object> send_photo(const photo_message_t& path) = 0;
    virtual asio::awaitable<json::object> send_error(const error_message_t& error) = 0;
    virtual asio::awaitable<void> send_file(std::string_view path) = 0;
};

#endif //SENDER_H
