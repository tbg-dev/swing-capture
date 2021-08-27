#include <c4a/core/log.h>
#include <opencv2/opencv.hpp>
#include <tbg/swingcam/ISwingCamera.h>

#include <chrono>
#include <thread>

static constexpr auto LGR = "main";

int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[])
{
    c4a::core::Logging::initConsoleLogging();
    c4a::core::Logging::setLogLevel(c4a::core::Logging::Level::TRACE);

    // Create the pipeline
    auto swingcam = tbg::ISwingCamera::create();
    swingcam->run();

    // TODO: Wait for input
    for (int i = 0; i < 30 * 2; ++i) {
        swingcam->update();
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 / 30));
    }
    swingcam->impact();
    swingcam->wait();
    return 0;
}