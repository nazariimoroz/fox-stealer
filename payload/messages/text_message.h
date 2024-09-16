#ifndef TEXT_MESSAGE_H
#define TEXT_MESSAGE_H

#include "message.h"
#include <string>

class text_message_t : public message_t
{
public:
    text_message_t() = default;
    ~text_message_t() override = default;

    std::string text;
};


#endif //TEXT_MESSAGE_H
