#pragma once

#include <vector>

#include <gst/gst.h>
#include <rxcpp/rx.hpp>

#include <c4a/core/memory.h>
#include <c4a/gst/IElement.h>

#include "Init.h"

namespace c4a::gst {

class Element
        : public IElement
        , public core::enable_shared_from_this<IElement>
{
public:
    explicit Element(const char* factoryName, const char* name);
    ~Element() override;

    explicit operator GstElement*() const override { return mElement; }

    [[nodiscard]] const char* getName() const override;

    IElement& linkSource (IElementPtr src ) override;
    IElement& linkSink   (IElementPtr sink) override;

    IElement& onSignal(
            const char* signal,
            IElement::OnSignalHandler handler
    ) override;

private:
    Init        mInit    { };
    GstElement* mElement {nullptr};

    // C-style callback will delegate to class method
    friend gboolean handle_signal(GstElement*, GstPad*, gpointer);
    bool handleSignal(std::string& signal, GstElement* element, GstPad* pad);

    std::unordered_map<std::string, std::vector<OnSignalHandler>>
        mSignalHandlers;
};
    
} // namespace c4a::gst