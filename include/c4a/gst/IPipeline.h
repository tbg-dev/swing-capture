#pragma once

#include <gst/gst.h>

#include <c4a/gst/IBus.h>
#include <c4a/gst/IElement.h>

namespace c4a::gst {

// Forward-declare shared pointer
class IPipeline;
using IPipelinePtr = std::shared_ptr<IPipeline>;

/** Interface definition for a GStreamer Pipeline. */
class IPipeline
{
public:
    /** Factory method(s) */
    static IPipelinePtr create(const char* name);

    /** Virtual destructor */
    virtual ~IPipeline() = default;

    /** Pipeline accessors */
    virtual const char* getName() const = 0;
    virtual const IBus& getBus () const = 0;
    virtual IBus&       getBus ()       = 0;

    /** Fluent interface for constructing pipeline & managing state */
    virtual IPipeline& add        (IElementPtr element) = 0;
    virtual IPipeline& addAndLink (IElementPtr element) = 0;
    virtual IPipeline& setLinkTo  (IElementPtr element) = 0;
    virtual IPipeline& play       () = 0;
    virtual IPipeline& pause      () = 0;
    virtual void       waitForEnd () = 0;
};

} // namespace c4a::gst