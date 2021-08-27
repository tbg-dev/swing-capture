#include <c4a/core/asio.h>
#include <c4a/core/log.h>
#include <c4a/net/ip.h>

#include "SwingCamera.h"

static constexpr auto LGR = "tbg::SwingCamera";

namespace tbg {

//-------------------------------------------------------------------------
SwingCamera::SwingCamera()
    : mPipeline(c4a::gst::IPipeline::create("SwingCamera"))
    , mTcpServer4(c4a::net::tcp::IServer::create())
    , mTcpServer6(c4a::net::tcp::IServer::create())
{
    // Start the TCP servers (on the same port)
    if (!startServer(mTcpServer4, c4a::net::ip::TCP_ANY_V4, 0)) {
        LOG_ERROR(LGR, "Couldn't start TCPv4 server for mDNS service");
        return;
    }
    if (!startServer(
            mTcpServer6, c4a::net::ip::TCP_ANY_V6,
            mTcpServer4->getEndpoint().port()
    )) {
        LOG_WARN(LGR, "Couldn't start TCPv6 server for mDNS service");
        mTcpServer6 = nullptr;
    }

    // Create the camera source / tee
    #ifdef defined(OS_LINUX)
    mCameraSource = c4a::gst::IElement::create("autovideosrc", "cameraSource");
    #else
    mCameraSource = c4a::gst::IElement::create("autovideosrc", "cameraSource");
    #endif
    mCameraTee = c4a::gst::IElement::create("tee", "teeCamera");

    // Create the RTP streaming elements
    mVideoBranch = c4a::gst::IElement::create("queue", "branchVideo");
    mX264enc = c4a::gst::IElement::create("x264enc", "x264enc");
    mRtph264pay = c4a::gst::IElement::create("rtph264pay", "rtph264pay");
    mVideoSinkTee = c4a::gst::IElement::create("tee", "teeVideoSink");
    mVideoSinkTee->setProp("allow-not-linked", TRUE);

    // Create the record path
    mRecordBranch = c4a::gst::IElement::create("queue", "branchRecord");
    // TODO: Attempting to create a 2s delay, but this doesn't work...
    mRecordBranch->setProp("leaky", 2); // leak downstream (drop old buffers)
    mRecordBranch->setProp("max-size-buffers", 480);
    mRecordBranch->setProp("max-size-bytes", 100 * 1024 * 1024);
    mRecordBranch->setProp("max-size-time", 2e9);
    mRecordTee = c4a::gst::IElement::create("tee", "teeRecord");
    mRecordTee->setProp("allow-not-linked", TRUE);

    // Create the file sink
    mFileBranch = c4a::gst::IElement::create("queue", "branchFile");
    mFileConvert = c4a::gst::IElement::create("videoconvert", "fileconvert");
    mFileMux = c4a::gst::IElement::create("avimux", "filemux");
    mFileSink = c4a::gst::IElement::create("filesink", "filesink");
    mFileSink->setProp("location", "shot.avi");

    // Create the jpg image stream
    mImageBranch = c4a::gst::IElement::create("queue", "branchImage");
    mVideoRate = c4a::gst::IElement::create("videorate", "videorate");
    mJpegEnc = c4a::gst::IElement::create("jpegenc", "jpegenc");
    mTcpSink = c4a::gst::IElement::create("tcpclientsink", "tcpsink");

    // Create the video display elements
    mLocalDisplay = c4a::gst::IElement::create("autovideosink", "localDisplay");
    if (mLocalDisplay != nullptr) {
        mDisplayBranch = c4a::gst::IElement::create("queue", "branchDisplay");
    }

    // TODO: Do caps need to get freed/unref?

    GstCaps* fps30 = gst_caps_new_simple(
            "video/x-raw",
            "framerate", GST_TYPE_FRACTION, 30, 1,
            nullptr
    );

    // Configure RTP elements
    mX264enc
        ->setProp("speed-preset", 2) // superfast
         .setProp("bitrate", 500)
         .setProp("tune", 0x04);     // zero-latency
    mRtph264pay
        ->setProp("config-interval", 10)
         .setProp("pt", 96);

    // Configure TCP elements
    mTcpSink
        ->setProp("host", "127.0.0.1")
         .setProp("port", 4321); // TODO: choose available port


    // Build the pipeline
    mPipeline
        ->add(mCameraSource)
         .addAndLink(mCameraTee)
         // RTP Video
         .addAndLink(mVideoBranch)
         .addAndLink(mX264enc, fps30)
         .addAndLink(mRtph264pay)
         .addAndLink(mVideoSinkTee)
         // Record Video
         .setLinkTo(mCameraTee)
         .addAndLink(mRecordBranch);

    // Local display
    if (mLocalDisplay != nullptr) {
        mPipeline
            ->setLinkTo(mCameraTee)
             .addAndLink(mDisplayBranch)
             .addAndLink(mLocalDisplay, fps30);
    }

    LOG_TRACE(LGR, "Pipeline created: " + std::string(mPipeline->getName()));
}

//-----------------------------------------------------------------------------
void SwingCamera::run()
{
    // Start the pipeline
    mPipeline->play();

    // Start the mDNS service
    mDnsService = c4a::net::mdns::ILocalService::create(
            c4a::net::mdns::ILocalService::Type::TCP,
            "teeboxgolf", "SwingCamera",
            mTcpServer4->getEndpoint().port()
    );
    mDnsService->addTxt("angle", "front"); // TODO: More properties here
    mDnsService->start();
}

//-----------------------------------------------------------------------------
void SwingCamera::update()
{
    mPipeline->runIteration();
}

//-----------------------------------------------------------------------------
void SwingCamera::wait()
{
    mPipeline->waitForEnd();
}

//-----------------------------------------------------------------------------
void SwingCamera::impact()
{
    LOG_TRACE(LGR, "Swing impact triggered");

    GstCaps* fps30 = gst_caps_new_simple(
            "video/x-raw",
            "framerate", GST_TYPE_FRACTION, 30, 1,
            nullptr
    );

    GstCaps* fps5 = gst_caps_new_simple(
            "video/x-raw",
            "framerate", GST_TYPE_FRACTION, 5, 1,
            nullptr
    );

    LOG_TRACE(LGR, "impact 1");

    // Attach record branch
    mPipeline->pause()
        .setLinkTo(mRecordBranch)
        .addAndLink(mRecordTee)
        // Attach file sink
        .setLinkTo(mRecordTee)
        .addAndLink(mFileBranch)
        .addAndLink(mFileConvert, fps30)
        .addAndLink(mFileMux)
        .addAndLink(mFileSink);

    LOG_TRACE(LGR, "impact 2");

    // TODO: attach image sequence clients
    // Attach image sequence
    mPipeline->setLinkTo(mRecordTee)
        .addAndLink(mImageBranch)
        .addAndLink(mVideoRate)
        .addAndLink(mJpegEnc, fps5)
        .addAndLink(mTcpSink);

    LOG_TRACE(LGR, "impact 3");

    // Resume the pipeline
    mPipeline->play();

    LOG_TRACE(LGR, "impact 4");

    // Schedule EOS
    auto ioctx = c4a::core::asio::IContext::create();
    ioctx->runAsync();

    auto timer = c4a::core::asio::ITimer::createFromNow(
            ioctx,
            std::chrono::seconds(5));

    timer->onExpire([this, timer] (const boost::system::error_code& ec) {
        LOG_TRACE(LGR, "Timer expired");
        mRecordTee->unlinkSource(mRecordBranch);
        mRecordTee->getPad("sink")->sendEvent(gst_event_new_eos());
    });
}

//-----------------------------------------------------------------------------
bool SwingCamera::addVideoSink()
{
    /*c4a::gst::IElement::create("queue", "branch1")
    mVideoSinkTee->linkSink()
    udpsink
    ->setProp("host", "127.0.0.1")
    .setProp("port", 1234);*/
    return true;
}

//-----------------------------------------------------------------------------
bool SwingCamera::startServer(
        const c4a::net::tcp::IServerPtr& server,
        const std::string& addr,
        unsigned short port
) {
    if (server == nullptr) {
        LOG_ERROR(LGR, "Unable to create TCP server");
        return false;
    }

    // Bind the server - do this on the main thread so we get the port
    if (!server->bind(c4a::net::ip::makeTcpEndpoint(addr, port))) {
        LOG_ERROR(LGR, "Error binding TCP server to " + addr);
        return false;
    }

    // Register TCP handler
    server->registerHandler(
            [this]
            (const c4a::net::tcp::ISessionPtr& session) {
                // Register our listener and start reading
                session->onReceive(
                        [this] (auto&& p1, auto&& p2) {
                            return onTcpReceived(
                                    std::forward<decltype(p1)>(p1),
                                    std::forward<decltype(p2)>(p2));
                        }
                )->doRead();
            }
    );

    // Start listening on a thread
    mThreads.emplace_back([server] {
        server->listen();
    });

    return true;
}

//-----------------------------------------------------------------------------
bool SwingCamera::onTcpReceived(
        const c4a::net::tcp::ISessionPtr& session,
        const c4a::net::tcp::ISession::BufferPtr& buffer
) {
    if (buffer == nullptr) {
        return false;
    }

    auto message = std::string(buffer->data(), buffer->size());
    LOG_DEBUG(LGR, "Received: " + message);

    // TODO: Respond as needed

    session->doClose();
    return false;
}

//-----------------------------------------------------------------------------
// Factory methods
//-----------------------------------------------------------------------------
ISwingCameraPtr ISwingCamera::create()
{
    return std::make_shared<SwingCamera>();
}

} // namespace tbg