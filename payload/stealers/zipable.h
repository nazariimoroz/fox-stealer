#ifndef BROWSER_STEALER_H
#define BROWSER_STEALER_H
#include "base_stealer.h"

class zipable_t
{
public:
    zipable_t() = default;
    virtual ~zipable_t() = default;

    // is_global  -> will save to in_save_path/"Browser Name"/.
    // !is_global -> will save to in_save_path/.
    virtual void set_save_folder(const fs::path& in_save_path, bool is_global = true) = 0;
};

#endif //BROWSER_STEALER_H
