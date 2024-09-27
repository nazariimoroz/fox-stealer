#pragma once

#define _WIN32_WINNT 0x0601
#include <filesystem>
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
#define FSUNIQUE_NAME FSGLUE(_fs_unique_name_temporary, __COUNTER__)

#define FS_TRY_EXPECTED_RETURN(VARIABLE, EXPECTED_FUN) BOOST_OUTCOME_TRYA(VARIABLE, EXPECTED_FUN)

#define FS_DEFER2(UNIQUE, FUNC) \
    std::shared_ptr<void> UNIQUE(nullptr, FUNC )
#define FS_DEFER(FUNC) FS_DEFER2(FSUNIQUE_NAME, FUNC)

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
namespace fs = std::filesystem;

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

    inline std::filesystem::path get_temp_path()
    {
        thread_local fs::path temp_path;
        if(temp_path == "")
        {
            TCHAR appdata_path[MAX_PATH];
            GetTempPath(MAX_PATH, appdata_path);
            temp_path = appdata_path;
        }

        return temp_path;
    }

    inline std::string generate_random_string(std::size_t length)
    {
        static std::string chars(
        "abcdefghijklmnopqrstuvwxyz"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ");

        std::string to_ret;
        to_ret.reserve(length);
        for(int i = 0; i < length; ++i)
            to_ret += chars[rand() % (chars.size() - 1)];

        return to_ret;
    }

    inline fs::path get_temp_unique_file_path()
    {
        return fs::temp_directory_path()
#ifndef _NDEBUG
                / "test_cookies"
#endif
                / net::generate_random_string(15);
    }

    template<class T>
    using expected = boost::outcome_v2::result<T, error_t>;

    struct cookie_t
    {
        std::string host;
        std::string name;
        std::string path;
        std::string cookie;
        std::uint32_t expiry;

        cookie_t(std::string_view in_host, std::string_view in_name, std::string_view in_path
            , std::string_view in_cookie, std::uint32_t in_expiry)
        {
            host = in_host;
            name = in_name;
            path = in_path;
            cookie = in_cookie;
            expiry = in_expiry;
        }
    };
}
