#include <c4a/core/log.h>
#include <gst/gst.h>

#include "Init.h"

static constexpr auto LGR = "c4a::gst::Init";

namespace c4a::gst {

//-------------------------------------------------------------------------
Init::Init(int* argc, char*** argv)
{
    static bool init = false;
    if (!init) {
        init = true;
        gst_init(argc, argv);
        LOG_DEBUG(LGR, "GStreamer Initialized");
    }
}

} // namespace c4a::gst