#include <c4a/core/log.h>
#include <gsl/gsl>
#include <gst/gst.h>

#include "Pipeline.h"

static constexpr auto LGR = "c4a::gst::Element";

namespace c4a::gst {

std::unordered_map<std::string, IElementPtr> mElementMap;

//-------------------------------------------------------------------------
struct SignalData
{
public:
    SignalData(const char* s, Element* e)
        : signal(s), element(e)
    { }

    std::string signal;
    Element*    element;
};

//-------------------------------------------------------------------------
gboolean handle_signal(GstElement* e, GstPad* p, void* d)
{
    SignalData* data    = static_cast<SignalData*>(d);
    std::string signal  = data->signal;
    Element*    element = data->element;
    return (element == nullptr) || element->handleSignal(signal, e, p);
}

//-------------------------------------------------------------------------
Element::Element(const char* factoryName, const char* name)
    : mElement(gst_element_factory_make (factoryName, name))
{
}

//-------------------------------------------------------------------------
Element::~Element()
{
    mElementMap.erase(std::string(gst_element_get_name(mElement)));
}

//-------------------------------------------------------------------------
const char* Element::getName() const
{
    return gst_element_get_name(mElement);
}

//-------------------------------------------------------------------------
IElement& Element::linkSource(IElementPtr src)
{
    if (!gst_element_link(static_cast<GstElement*>(*src.get()), mElement)){
        throw std::runtime_error(
                "Couldn't link element " +
                std::string(src->getName()) +
                " to " +
                getName());
    }
    return *this;
}

//-------------------------------------------------------------------------
IElement& Element::linkSink(IElementPtr sink)
{
    if (!gst_element_link(mElement, static_cast<GstElement*>(*sink.get()))) {
        throw std::runtime_error(
                "Couldn't link element " +
                std::string(sink->getName()) +
                " to " +
                getName());
    }
    return *this;
}

//-------------------------------------------------------------------------
IElement& Element::onSignal(const char* signal, IElement::OnSignalHandler handler)
{
    auto handlers = mSignalHandlers.try_emplace(signal, 0);
    handlers.first->second.push_back(handler);
    if (handlers.second) {
        LOG_TRACE(LGR, "Subscribing to signal: " + std::string(signal));
        g_signal_connect(
                mElement,
                signal,
                G_CALLBACK(handle_signal),
                new SignalData(signal, this) // TODO: This leaks memory..
        );
    }
    return *this;
}

//-------------------------------------------------------------------------
bool Element::handleSignal(std::string& signal, GstElement* e, GstPad* p)
{
    LOG_TRACE(LGR, "handleSignal: " + std::string(getName()));
    auto element = Element::findElement(gst_element_get_name(e));
    auto handlers = mSignalHandlers.find(signal);
    if (handlers != mSignalHandlers.end()) {
        for (auto& h : handlers->second) {
            h(element, p);
        }
    }
    return true;
}

//-----------------------------------------------------------------------------
// Static/Factory methods
//-----------------------------------------------------------------------------
IElementPtr IElement::create(const char* factoryName, const char* name)
{
    auto e = std::make_shared<Element>(factoryName, name);
    mElementMap[e->getName()] = e->shared_ptr();
    return e;
}

//-------------------------------------------------------------------------
IElementPtr IElement::findElement(const char* name)
{
    std::string key(name);
    return mElementMap.count(key) ? mElementMap.at(key) : nullptr;
}

} // namespace c4a::gst