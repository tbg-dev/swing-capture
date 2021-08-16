#pragma once

#include <c4a/gst/IPipeline.h>
#include <c4a/net/mdns/ILocalService.h>
#include <c4a/net/tcp/IServer.h>
#include <tbg/swingcam/ISwingCamera.h>

#include <thread>

namespace tbg {

class SwingCamera : public ISwingCamera
{
public:
     SwingCamera();
    ~SwingCamera() override = default;

    void run() override;

private:
    // Pipeline and elements
    c4a::gst::IPipelinePtr mPipeline;
    c4a::gst::IElementPtr  mCameraSource  { nullptr };
    c4a::gst::IElementPtr  mCameraTee     { nullptr };

    c4a::gst::IElementPtr  mVideoBranch   { nullptr };
    c4a::gst::IElementPtr  mX264enc       { nullptr };
    c4a::gst::IElementPtr  mRtph264pay    { nullptr };

    c4a::gst::IElementPtr  mImageBranch   { nullptr };
    c4a::gst::IElementPtr  mVideoRate     { nullptr };
    c4a::gst::IElementPtr  mJpegEnc       { nullptr };
    c4a::gst::IElementPtr  mTcpSink       { nullptr };

    c4a::gst::IElementPtr  mRecordBranch  { nullptr };
    c4a::gst::IElementPtr  mRecordQueue   { nullptr };

    c4a::gst::IElementPtr  mDisplayBranch { nullptr };
    c4a::gst::IElementPtr  mLocalDisplay  { nullptr };

    // Video sinks get the full RTP stream over UDP
    struct VideoSink {
        c4a::gst::IElementPtr udpSink;
    };
    c4a::gst::IElementPtr  mVideoSinkTee { nullptr };
    std::vector<VideoSink> mVideoSinks;

    // mDNS Service components
    c4a::net::mdns::ILocalServicePtr  mDnsService  { nullptr };
    c4a::net::tcp::IServerPtr         mTcpServer4;
    c4a::net::tcp::IServerPtr         mTcpServer6;
    std::vector<std::thread>          mThreads;

    bool addVideoSink();

    // Handle TCP message
    bool startServer(const
            c4a::net::tcp::IServerPtr& server,
            const std::string& addr,
            unsigned short port
    );
    bool onTcpReceived(
            const c4a::net::tcp::ISessionPtr& session,
            const c4a::net::tcp::ISession::BufferPtr& buffer
    );
};

} // namespace tbg