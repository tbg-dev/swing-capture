#include <c4a/core/log.h>
#include <tbg/swingcam/ISwingCamera.h>

static constexpr auto LGR = "main";

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    c4a::core::Logging::initConsoleLogging();
    c4a::core::Logging::setLogLevel(c4a::core::Logging::Level::TRACE);

    // Create the pipeline
    auto swingcam = tbg::ISwingCamera::create();
    swingcam->run();
    return 0;
}