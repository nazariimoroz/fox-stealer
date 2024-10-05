#ifndef FILE_MESSAGE_H
#define FILE_MESSAGE_H

#include <fstream>

#include "net.h"
#include "message.h"

class file_message_t : public message_t
{
public:
    file_message_t() = default;
    ~file_message_t() override = default;

    inline void init_with_path(std::string_view in_path)
    {
        std::ifstream fin(in_path);
        if(!fin.is_open())
            DLOG(std::format("FileMessage(ERROR): cant open {}", in_path));
        data.assign((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());
        fin.close();
    }

    inline void move_to_data_and_set_path(std::string&& in_data, std::string_view in_path)
    {
        path = in_path;
        data = std::move(in_data);
    }

    std::string path;

    // zero terminator IS NOT AN END of photo data
    // be carefull in use
    std::string data;
};

#endif //FILE_MESSAGE_H
