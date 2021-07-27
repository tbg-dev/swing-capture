#pragma once

namespace c4a::gst {

class Init
{
public:
    Init() : Init(nullptr, nullptr) { }
    Init(int* argc, char*** argv);
    virtual ~Init() = default;
};
    
} // namespace c4a::gst