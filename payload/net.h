#pragma once

#define _WIN32_WINNT 0x0601
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http/fields.hpp>
#include <boost/outcome.hpp>

#include <boost/asio/awaitable.hpp>

#include <boost/asio/experimental/awaitable_operators.hpp>

#include <boost/json.hpp>

#include <iostream>
#include <syncstream>

#define FSGLUE2(x, y) x##y
#define FSGLUE(x, y) FSGLUE2(x, y)
#define FSUNIQUE_NAME FSGLUE(_outcome_try_unique_name_temporary, __COUNTER__)

#define FS_TRY_EXPECTED_RETURN(VARIABLE, EXPECTED_FUN) BOOST_OUTCOME_TRYA(VARIABLE, EXPECTED_FUN)

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
    class error_t
    {
    public:
        explicit error_t(const std::string& in_msg)
        {
            msg = in_msg;
        }

        [[nodiscard]] std::string str() const { return msg; }

    protected:
        std::string msg;
    };

    inline json::object create_internal_error(const char* message)
    {
        return {
            {"ok", false},
            {"error_code", "500"},
            {"description", message}
        };
    }

    template<class TBegin, class TEnd>
    auto wait(TBegin begin, TEnd end)
    {
        if(begin + 1 == end)
            return std::move(*begin);
        return std::move(*begin) && std::move(wait(begin + 1, end));
    }

    template<class T>
    using expected = boost::outcome_v2::result<T, error_t>;
}
