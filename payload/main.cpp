#include <iostream>

#define _WIN32_WINNT 0x0601
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>

#include <boost/json.hpp>

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;
namespace ssl = asio::ssl;
namespace json = boost::json;

using tcp = asio::ip::tcp;
using udp = asio::ip::udp;

using ssl_socket = ssl::stream<tcp::socket>;
using error_code = boost::system::error_code;

http::request<http::string_body> prepare_request(const json::object& obj)
{
    http::request<http::string_body> req;
    req.method(http::verb::post);
    req.set(http::field::host, "api.telegram.org");
    req.set(http::field::content_type, "application/json");
    req.set(http::field::accept, "application/json");
    req.set(http::field::connection, "close");
    req.target("/bot6920020229:AAFsRJR5WUZcWk5StJxYdZPTQdBXB6vvLt0/sendMessage");
    req.body() = json::serialize(obj);
    req.prepare_payload();

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
    req_body["text"] = "test";

    auto req = prepare_request(req_body);

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
