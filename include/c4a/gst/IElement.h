#pragma once

#include <gst/gst.h>

namespace c4a::gst {

// Forward-declare shared pointer
class IElement;
using IElementPtr = std::shared_ptr<IElement>;

class IElement
{
public:
    using OnSignalHandler = std::function<void(IElementPtr,GstPad*)>;

    /** Factory method(s) */
    static IElementPtr create(const char* factoryName, const char* name);

    /** Locate existing element */
    static IElementPtr findElement(const char* name);

    /** Explicit cast to underlying GstElement */
    virtual explicit operator GstElement*() const = 0;

    /** Get element name */
    [[nodiscard]] virtual const char* getName() const = 0;

    /** Element linking */
    virtual IElement& linkSource (IElementPtr src)  = 0;
    virtual IElement& linkSink   (IElementPtr sink) = 0;

    /** Property management */
    template <typename ...V>
    IElement& setProp(const char* prop, V&&... values);

    /** Signal handling */
    virtual IElement& onSignal(const char* signal, OnSignalHandler h) = 0;
};

//-------------------------------------------------------------------------
template <typename ...V>
IElement& IElement::setProp(const char* prop, V&&... values)
{
    g_object_set(
            static_cast<GstElement*>(*this),
            prop,
            std::forward<V>(values)...,
            nullptr
    );
    return *this;
}
    
} // namespace c4a::gst