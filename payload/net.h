//
// Copyright 2024 Nazarii Moroz
//

#ifndef NET_H
#define NET_H

#define _WIN32_WINNT 0x0601
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http/fields.hpp>

#include <boost/asio/awaitable.hpp>

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

#endif //NET_H
