#include <c4a/core/log.h>
#include <fmt/printf.h>
#include <gsl/gsl>
#include <gst/gst.h>

#include "Pipeline.h"

static constexpr auto LGR = "c4a::gst::Bus";

namespace c4a::gst {

//-------------------------------------------------------------------------
gboolean handle_message([[maybe_unused]] GstBus* i, GstMessage* m, gpointer b)
{
    auto bus = static_cast<Bus*>(b);
    return (bus == nullptr) || bus->handleMessage(m);
}

//-------------------------------------------------------------------------
Bus::Bus(GstBus* bus)
    : mBus(bus)
{
    // Add the watch
    gst_bus_add_watch(mBus, handle_message, this);
}

//-------------------------------------------------------------------------
Bus::~Bus()
{
    gst_bus_remove_watch(mBus);
    gst_object_unref(mBus);
}

//-------------------------------------------------------------------------
rxcpp::observable<GstMessage> Bus::onMessage()
{
    return rxcpp::observable<>::create<GstMessage>(
        [this](const rxcpp::subscriber<GstMessage>& s) {
            mSubscribers.push_back(s);
        }
    );
}

//-------------------------------------------------------------------------
bool Bus::handleMessage(GstMessage* message)
{
    if (message == nullptr) {
        LOG_ERROR(LGR, "Received null message on bus");
        return false;
    }

    auto messageType = GST_MESSAGE_TYPE(message);

    // Convey standard error/warn/info logs to logger
    GError* err = nullptr;
    gchar* debugInfo = nullptr;
    switch (messageType) {
        case GST_MESSAGE_ERROR:
            gst_message_parse_error(message, &err, &debugInfo);
            LOG_ERROR(LGR, fmt::sprintf(
                    "Error received from element %s: %s",
                    GST_OBJECT_NAME(message->src),
                    err->message
            ));
            LOG_ERROR(LGR, fmt::sprintf(
                    "Debugging information: %s",
                    debugInfo ? debugInfo : "none"
            ));

            break;

        case GST_MESSAGE_WARNING:
            gst_message_parse_warning(message, &err, &debugInfo);
            LOG_WARN(LGR, fmt::sprintf(
                    "Warning received from element %s: %s",
                    GST_OBJECT_NAME(message->src),
                    err->message
            ));
            LOG_WARN(LGR, fmt::sprintf(
                    "Debugging information: %s",
                    debugInfo ? debugInfo : "none"
            ));
            break;

        case GST_MESSAGE_INFO:
            gst_message_parse_info(message, &err, &debugInfo);
            LOG_INFO(LGR, fmt::sprintf(
                    "Info received from element %s: %s",
                    GST_OBJECT_NAME(message->src),
                    err->message
            ));
            LOG_INFO(LGR, fmt::sprintf(
                    "Debugging information: %s",
                    debugInfo ? debugInfo : "none"
            ));
            break;

        default:
            break;
    }

    if (err != nullptr) {
        g_clear_error(&err);
    }
    if (debugInfo != nullptr) {
        g_free(debugInfo);
    }

    // Notify subscribers
    for (auto&& s : mSubscribers) {
        try {
            if (s.is_subscribed()) {
                s.on_next(*message);
                if (messageType == GST_MESSAGE_EOS) {
                    s.on_completed();
                }
            }
        } catch (...) {
            s.on_error(std::current_exception());
        }
    }

    // Remove non-subscribers
    auto is = mSubscribers.begin();
    for (; is != mSubscribers.end(); ) {
        if (!is->is_subscribed()) {
            is = mSubscribers.erase(is);
        } else {
            ++is;
        }
    }

    return true;
}

} // namespace c4a::gst