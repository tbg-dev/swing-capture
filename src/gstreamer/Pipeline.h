#pragma once

#include <gst/gst.h>
#include <c4a/gst/IPipeline.h>

#include "Bus.h"
#include "Element.h"
#include "Init.h"

namespace c4a::gst {

class Pipeline : public IPipeline
{
public:
    explicit Pipeline(const char* name);
    ~Pipeline() override;

    explicit operator GstElement*() const { return mPipeline; }

    [[nodiscard]] const char* getName() const override;
    [[nodiscard]] const IBus& getBus () const override { return mBus; }
    [[nodiscard]]       IBus& getBus ()       override { return mBus; }

    IPipeline& add        (IElementPtr element) override;
    IPipeline& addAndLink (IElementPtr element) override;
    IPipeline& setLinkTo  (IElementPtr element) override;
    IPipeline& play       () override;
    IPipeline& pause      () override;
    void       waitForEnd () override;

private:
    Init        mInit      { };
    GstElement* mPipeline  {nullptr};
    Bus         mBus;
    IElementPtr mLastAdded {nullptr};
};

} // namespace c4a::gst