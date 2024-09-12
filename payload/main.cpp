#include <iostream>

#define _WIN32_WINNT 0x0601
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;


using ssl_socket = asio::ssl::stream<asio::ip::tcp::socket>;


int main() try
{
    asio::io_context io_context;
    asio::ssl::context ssl_context(asio::ssl::context::sslv23_client);
    ssl_socket socket(io_context, ssl_context);

    asio::ip::tcp::resolver resolver(io_context);
    auto it = resolver.resolve({"api.telegram.org", "443"});
    asio::connect(socket.lowest_layer(), it);

    socket.handshake(asio::ssl::stream_base::handshake_type::client);

    std::stringstream req;
    req << "GET " << "/bot6920020229:AAFsRJR5WUZcWk5StJxYdZPTQdBXB6vvLt0/sendMessage";
    req << "?chat_id=515352778";
    req << "&text=HelloWorld";
    req << " HTTP/1.1\r\n";
    req << "Host: " << "api.telegram.org" << "\r\n";
    req << "Accept: */*\r\n";
    req << "Connection: close\r\n\r\n";

    asio::write(socket, asio::buffer(req.str()));

    std::string response;
    boost::system::error_code ec;

    do {
        char buf[1024];
        size_t bytes_transferred = socket.read_some(asio::buffer(buf), ec);
        if (!ec) response.append(buf, buf + bytes_transferred);
    } while (!ec);

    // print and exit
    std::cout << "Response received: '" << response << "'\n";

    return 0;
} catch (std::exception& ex)
{
    std::cerr << ex.what() << "\n";
}
