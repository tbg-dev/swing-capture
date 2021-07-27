#pragma once

#include <gst/gst.h>
#include <rxcpp/rx.hpp>
#include <vector>

#include <c4a/gst/IBus.h>

namespace c4a::gst {

class Pipeline;

class Bus : public IBus
{
public:
    rxcpp::observable<GstMessage> onMessage() override;

private:
    // Should always be created & owned by a pipeline
    friend class Pipeline;
    explicit Bus(GstBus* bus);
    virtual ~Bus();

    // C-style callback will delegate to class method
    friend gboolean handle_message(GstBus*, GstMessage*, gpointer);
    bool handleMessage(GstMessage* message);

    // Provide cast operator to native GstBus
    explicit operator GstBus*() const { return mBus; }
    GstBus* mBus { nullptr };

    // Store all subscribers to the bus
    std::vector<rxcpp::subscriber<GstMessage>> mSubscribers;
};
    
} // namespace c4a::gst