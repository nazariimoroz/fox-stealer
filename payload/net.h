#pragma once

#define _WIN32_WINNT 0x0601
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http/fields.hpp>

#include <boost/asio/awaitable.hpp>

#include <boost/asio/experimental/awaitable_operators.hpp>

#include <boost/json.hpp>

#include <iostream>
#include <syncstream>

#ifndef _NDEBUG
#define DLOG(MSG) std::osyncstream(std::cerr) << MSG << std::endl
#else
#define DLOG(MSG) do {} while (0)
#endif

namespace asio = boost::asio;
namespace beast = boost::beast;
namespace http = beast::http;
namespace ssl = asio::ssl;
namespace json = boost::json;

using namespace asio::experimental::awaitable_operators;

using tcp = asio::ip::tcp;
using udp = asio::ip::udp;

using ssl_socket = ssl::stream<tcp::socket>;
using error_code = boost::system::error_code;

namespace net
{
    inline json::object create_internal_error(const char* message)
    {
        return {
            {"ok", false},
            {"error_code", "500"},
            {"description", message}
        };
    }
}
