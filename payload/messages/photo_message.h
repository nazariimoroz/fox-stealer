#ifndef PHOTO_MESSAGE_H
#define PHOTO_MESSAGE_H

#include "message.h"

#include <string>

class photo_message_t : public message_t
{
public:
    photo_message_t() = default;
    ~photo_message_t() override = default;

    inline void init_with_path(std::string_view in_path)
    {/*TODO*/}

    inline void move_to_data_and_set_path(std::string&& in_data, std::string_view in_path)
    {
        path = in_path;
        data = std::move(in_data);
    }

    std::string path;
    std::string data;
};

#endif //PHOTO_MESSAGE_H
