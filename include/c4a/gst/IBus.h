#pragma once

#include <gst/gst.h>
#include <rxcpp/rx.hpp>

namespace c4a::gst {

class IBus
{
public:
    /** Observable stream of bus messages */
    virtual rxcpp::observable<GstMessage> onMessage() = 0;
};
    
} // namespace c4a::gst