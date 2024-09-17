#include "telegram_sender.h"

#include <filesystem>

#include "messages/photo_message.h"
#include "messages/text_message.h"

telegram_sender_t::telegram_sender_t(std::string_view bot_token, std::string_view chat_id)
    : ssl_context(ssl::context::sslv23_client)
    , TELEGRAM_QUERY(HOST, PORT)
    , BOT_TOKEN(bot_token)
    , CHAT_ID(chat_id)
{}

asio::awaitable<json::object> telegram_sender_t::send_message(const std::unique_ptr<message_t>& message)
{
    try
    {
        if(const auto text_message = dynamic_cast<text_message_t*>(message.get()))
        {
            co_return co_await send_text(text_message->text);
        }

        if(const auto photo_message = dynamic_cast<photo_message_t*>(message.get()))
        {
            co_return co_await send_photo(*photo_message);
        }

        co_return net::create_internal_error("bad message");
    } catch (std::exception& e)
    {
        co_return net::create_internal_error(e.what());
    }
}

asio::awaitable<json::object> telegram_sender_t::send_text(std::string_view text)
{
    try
    {
        auto io_context = co_await asio::this_coro::executor;

        ssl_socket socket(io_context, ssl_context);
        co_await create_connection(socket);

        json::object req_body;
        req_body["chat_id"] = CHAT_ID;
        req_body["text"] = text;

        http::request<http::string_body> req;
        req.method(http::verb::post);
        req.version(11);
        req.set(http::field::host, HOST);
        req.set(http::field::content_type, "application/json");
        req.set(http::field::accept, "application/json");
        req.set(http::field::connection, "close");
        req.target("/bot" + BOT_TOKEN + "/sendMessage");
        req.body() = json::serialize(req_body);
        req.prepare_payload();

        co_await http::async_write(socket, req, asio::deferred);

        beast::flat_buffer buffer;
        http::response<http::dynamic_body> res;
        res.set(http::field::content_type, "application/json");
        co_await http::async_read(socket, buffer, res, asio::deferred);

        const auto body_string = beast::buffers_to_string(res.body().data());
        co_return json::parse(body_string).as_object();

    } catch (const std::exception& e)
    {
        co_return net::create_internal_error(e.what());
    }
}

asio::awaitable<json::object> telegram_sender_t::send_photo(const photo_message_t& message)
{
    try
    {
        auto io_context = co_await asio::this_coro::executor;

        ssl_socket socket(io_context, ssl_context);
        co_await create_connection(socket);

        http::request<http::string_body> req;
        req.method(http::verb::post);
        req.version(11);
        req.set(http::field::host, HOST);
        req.set(http::field::content_type, "multipart/form-data; boundary=boundary");
        req.set(http::field::accept, "application/json");
        req.set(http::field::connection, "close");
        req.target("/bot" + BOT_TOKEN + "/sendPhoto");

        // Construct the multipart form data
        std::ostringstream oss;
        oss << "--boundary\r\n";
        oss << "Content-Disposition: form-data; name=\"chat_id\"\r\n";
        oss << "Content-Type: text/plain\r\n\r\n";
        oss << CHAT_ID << "\r\n";
        oss << "--boundary\r\n";
        oss << "Content-Disposition: form-data; name=\"photo\"; filename=\"" << std::filesystem::path(message.path).filename().string() << "\"\r\n";
        oss << "Content-Type: image/" << std::filesystem::path(message.path).filename().extension().string().substr(1) << "\r\n\r\n";

        oss << message.data << "\r\n";
        oss << "--boundary--\r\n";

        req.body() = oss.str();
        req.content_length(req.body().size());

        co_await http::async_write(socket, req, asio::deferred);

        beast::flat_buffer buffer;
        http::response<http::dynamic_body> res;
        res.set(http::field::content_type, "application/json");
        co_await http::async_read(socket, buffer, res, asio::deferred);

        const auto body_string = beast::buffers_to_string(res.body().data());
        co_return json::parse(body_string).as_object();

    } catch (const std::exception& e)
    {
        co_return net::create_internal_error(e.what());
    }
}

asio::awaitable<void> telegram_sender_t::send_file(std::string_view path)
{
    co_return;
}

asio::awaitable<void> telegram_sender_t::create_connection(ssl_socket& socket)
{
    auto io_context = co_await asio::this_coro::executor;

    tcp::resolver resolver(io_context);
    const auto it = co_await resolver.async_resolve(HOST, PORT, asio::deferred);
    co_await asio::async_connect(socket.lowest_layer(), it, asio::deferred);

    socket.handshake(ssl::stream_base::handshake_type::client);
}
