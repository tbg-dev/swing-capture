#include <c4a/core/log.h>
#include <c4a/gstreamer/System.h>

int main(int argc, char* argv[])
{
    c4a::core::Logging::setLogLevel(c4a::core::Logging::Level::TRACE);
    c4a::gst::System s(&argc, &argv);
    return s.run();
}