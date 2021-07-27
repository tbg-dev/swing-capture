#include <c4a/core/log.h>
#include <c4a/gst/IPipeline.h>

static constexpr auto LGR = "main";

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    c4a::core::Logging::initConsoleLogging();
    c4a::core::Logging::setLogLevel(c4a::core::Logging::Level::TRACE);

    // Create the pipeline
    auto pipeline = c4a::gst::IPipeline::create("capture");

    // Create the camera source
    auto cameraSource = c4a::gst::IElement::create("autovideosrc", "cameraSource");

    // Create the tee
    auto tee = c4a::gst::IElement::create("tee", "tee");
    auto branch1 = c4a::gst::IElement::create("queue", "branch1");
    auto branch2 = c4a::gst::IElement::create("queue", "branch2");

    // Create the RTP streaming elements
    auto x264enc = c4a::gst::IElement::create("x264enc", "x264enc");
    auto rtph264pay = c4a::gst::IElement::create("rtph264pay", "rtph264pay");
    auto udpsink = c4a::gst::IElement::create("udpsink", "udpsink");

    // Create the video display elements
    auto recordingFilter = c4a::gst::IElement::create("videobalance", "recordingFilter");
    auto localDisplay = c4a::gst::IElement::create("autovideosink", "localDisplay");

    // Testing signal connect
    tee->onSignal("pad-added", [] (const c4a::gst::IElementPtr& e, GstPad* p) {
        LOG_DEBUG(LGR, "PAD ADDED! " + std::string(e->getName()));
    });

    // Configure RTP elements
    x264enc
        ->setProp("speed-preset", 2) // superfast
         .setProp("bitrate", 500)
         .setProp("tune", 0x04);     // zero-latency
    rtph264pay
        ->setProp("config-interval", 10)
         .setProp("pt", 96);
    udpsink
        ->setProp("host", "127.0.0.1")
         .setProp("port", 1234);

    // Observe bus messages
    pipeline->getBus().onMessage().subscribe(
            [] (GstMessage message) {
                LOG_TRACE(LGR,
                          "Received pipeline message: " +
                          std::string(GST_MESSAGE_TYPE_NAME(&message))
                );
            }
    );

    // Build and run the pipeline
    pipeline
        ->add(cameraSource)
         .addAndLink(tee)
         .addAndLink(branch1)
         .addAndLink(x264enc)
         .addAndLink(rtph264pay)
         .addAndLink(udpsink)
         .setLinkTo(tee)
         .addAndLink(branch2)
         .addAndLink(recordingFilter)
         .addAndLink(localDisplay)
         .play();

    // TODO: Do this when recording
    //recordingFilter->setProp("hue", 0.2);

    // Wait for end
    pipeline->waitForEnd();

    return 0;
}