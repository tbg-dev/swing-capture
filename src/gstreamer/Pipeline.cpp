#include <c4a/core/log.h>
#include <gsl/gsl>
#include <gst/gst.h>

#include "Pipeline.h"

static constexpr auto LGR = "c4a::gst::Pipeline";

namespace c4a::gst {

//-------------------------------------------------------------------------
Pipeline::Pipeline(const char* name)
    : mPipeline(gsl::not_null(gst_pipeline_new(name)))
    , mBus(gst_element_get_bus(mPipeline))
{
    LOG_TRACE(LGR, "Pipeline created: " +
        std::string(gst_element_get_name(mPipeline)));
}

//-------------------------------------------------------------------------
Pipeline::~Pipeline()
{
    gst_element_set_state(mPipeline, GST_STATE_NULL);
    gst_object_unref(mPipeline);
    LOG_TRACE(LGR, "Pipeline destroyed: " +
        std::string(gst_element_get_name(mPipeline)));
}

//-------------------------------------------------------------------------
const char* Pipeline::getName() const
{
    return gst_element_get_name(mPipeline);
}

//-------------------------------------------------------------------------
IPipeline& Pipeline::add(IElementPtr element)
{
    // Store last added element for easy linking
    setLinkTo(element);

    // Add the element
    gst_bin_add(GST_BIN (mPipeline), static_cast<GstElement*>(*mLastAdded));

    return *this;
}

//-------------------------------------------------------------------------
IPipeline& Pipeline::addAndLink(IElementPtr element)
{
    // Cache the last added element
    auto src = mLastAdded;
    if (src == nullptr) {
        throw std::runtime_error(
            "No source element to link to " +
            std::string(element->getName()));
    }

    // Add the new element
    add(element);

    // Link source
    element->linkSource(src);

    return *this;
}

//-------------------------------------------------------------------------
IPipeline& Pipeline::setLinkTo(IElementPtr element)
{
    // Store last added element for easy linking
    mLastAdded = element;
    return *this;
}

//-------------------------------------------------------------------------
IPipeline& Pipeline::play()
{
    // Start playing
    auto ret = gst_element_set_state(mPipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        throw std::runtime_error(
                "Unable to set the pipeline to the playing state");
    }
    return *this;
}

//-------------------------------------------------------------------------
IPipeline& Pipeline::pause()
{
    // Start playing
    auto ret = gst_element_set_state(mPipeline, GST_STATE_PAUSED);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        throw std::runtime_error(
                "Unable to set the pipeline to the paused state");
    }
    return *this;
}

//-------------------------------------------------------------------------
void Pipeline::waitForEnd()
{
    GMainLoop* loop = g_main_loop_new(nullptr, FALSE);
    g_main_loop_run(loop);
    g_main_loop_unref(loop);
}

//-----------------------------------------------------------------------------
// Factory methods
//-----------------------------------------------------------------------------
IPipelinePtr IPipeline::create(const char* name)
{
    return std::make_shared<Pipeline>(name);
}

} // namespace c4a::gst