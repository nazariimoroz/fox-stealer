//
// Copyright 2024 Nazarii Moroz
//

#include "win_stealer.h"

#include "net.h"

std::string pc_t::str() const
{
    std::stringstream ss;
    ss << "PC Name: " << name << "\n";
    ss << "OS Name: " << os << "\n";
    ss << "Total Memory: " << total_memory << "\n";
    return ss.str();
}

std::future<std::string> win_stealer_t::steal()
{
    return std::async(std::launch::async, [this]() -> std::string
    {
        static int a = 5;
        a += 5;
        std::this_thread::sleep_for(std::chrono::seconds(a));

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

        const auto to_ret = pc.str();
        DLOG(to_ret);
        return to_ret;
    });
}
