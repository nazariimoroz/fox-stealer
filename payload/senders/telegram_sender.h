#ifndef TELEGRAM_SENDER_H
#define TELEGRAM_SENDER_H
#include "sender.h"

class telegram_sender_t : public sender_t
{
public:
    explicit telegram_sender_t(std::string_view bot_token, std::string_view chat_id);
    ~telegram_sender_t() override = default;

    asio::awaitable<json::object> send_message(std::string_view message) override;
    asio::awaitable<void> send_photo(std::string_view path) override;
    asio::awaitable<void> send_file(std::string_view path) override;

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
