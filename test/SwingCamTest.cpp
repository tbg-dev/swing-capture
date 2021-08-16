//#include <gtest/gtest.h>

#include <c4a/core/log.h>
#include <gtest/gtest.h>
#include <tbg/swingcam/ISwingCamera.h>

static constexpr auto LGR = "GstreamerTest";

//-----------------------------------------------------------------------------
class GstreamerTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        c4a::core::Logging::setLogLevel(c4a::core::Logging::Level::TRACE);
    }
};

//-----------------------------------------------------------------------------
TEST_F(GstreamerTest, HelloWorld)
{
    auto swingcam = tbg::ISwingCamera::create();
    swingcam->run();
}