#ifndef TELEGRAM_SENDER_H
#define TELEGRAM_SENDER_H
#include "sender.h"

class file_message_t;

class telegram_sender_t : public sender_t
{
public:
    telegram_sender_t(std::string_view bot_token, std::string_view chat_id);
    ~telegram_sender_t() override = default;

    asio::awaitable<json::object> send_message(const std::unique_ptr<message_t>& message) override;
    asio::awaitable<json::object> send_text(const text_message_t& text) override;
    asio::awaitable<json::object> send_photo(const photo_message_t& message) override;
    asio::awaitable<json::object> send_error(const error_message_t& error) override;
    asio::awaitable<json::object> send_file(const file_message_t& message) override;

protected:
    ssl::context ssl_context;

    const char* HOST = "api.telegram.org";
    const char* PORT = "443";
    const asio::ip::tcp::resolver::query TELEGRAM_QUERY;

    const std::string BOT_TOKEN;
    const std::string CHAT_ID;

protected:
    asio::awaitable<void> create_connection(ssl_socket& socket);
};


#endif //TELEGRAM_SENDER_H
