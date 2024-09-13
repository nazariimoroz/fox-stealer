#include <iostream>

#define _WIN32_WINNT 0x0601
#include <filesystem>
#include <fstream>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http/fields.hpp>

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
