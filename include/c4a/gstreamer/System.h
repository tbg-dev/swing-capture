#pragma once

#include <gst/gst.h>

namespace c4a::gst {

class System
{
public:
    System() : System(nullptr, nullptr) { }
    System(int* argc, char*** argv);
    virtual ~System();

    int run();
};
    
} // namespace c4a::gst