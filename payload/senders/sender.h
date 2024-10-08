#ifndef SENDER_H
#define SENDER_H
#include <string_view>
#include "net.h"
#include <messages/message.h>

class file_message_t;
class text_message_t;
class error_message_t;
class photo_message_t;

class sender_t : public asio::detail::noncopyable
{
public:
    sender_t() = default;
    virtual ~sender_t() = default;

    virtual asio::awaitable<json::object> send_message(const std::unique_ptr<message_t>& message) = 0;
    virtual asio::awaitable<json::object> send_text(const text_message_t& text) = 0;
    virtual asio::awaitable<json::object> send_photo(const photo_message_t& message) = 0;
    virtual asio::awaitable<json::object> send_error(const error_message_t& error) = 0;
    virtual asio::awaitable<json::object> send_file(const file_message_t& message) = 0;
};

#endif //SENDER_H
