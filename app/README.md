# swing-capture
Swing Capture app for Teebox Golf.  This application uses the available camera
to stream video of the swing to subscribed TBG services.  Given input from a 
Swing Detector it can also initiate recording of the swing in the highest 
available framerate. 

RTP Streaming with H264:
# gst-launch-1.0 autovideosrc device=/dev/videoX ! x264enc tune=zerolatency bitrate=500 speed-preset=superfast ! rtph264pay config-interval=10 pt=96 ! udpsink host=127.0.0.1 port=1234
# gst-launch-1.0 udpsrc uri=udp://127.0.0.1:1234 ! application/x-rtp,media=video,clock-rate=90000,encoding-name=H264 ! rtph264depay ! h264parse ! avdec_h264 ! videoconvert ! autovideosink

Using rtpbin
# gst-launch-1.0 rtpbin name=rtpbin \
#   autovideosrc device=/dev/videoX ! x264enc ! rtph264pay ! rtpbin.send_rtp_sink_0 \
#     rtpbin.send_rtp_src_0 ! udpsink host=127.0.0.1 port=1234                            \
#     rtpbin.send_rtcp_src_0 ! udpsink host=127.0.0.1 port=5001 sync=false async=false
