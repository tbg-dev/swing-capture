#include <c4a/core/log.h>
#include <c4a/gstreamer/System.h>

#include <gst/gst.h>

static constexpr auto LGR = "c4a::gst::System";

namespace c4a::gst {

//-------------------------------------------------------------------------
System::System(int* argc, char*** argv)
{
    gst_init(argc, argv);
    LOG_DEBUG(LGR, "GStreamer Initialized");
}

//-------------------------------------------------------------------------
System::~System()
{

}

//-------------------------------------------------------------------------
int System::run()
{
    GstElement *pipeline, *source, *sink;
    GstBus *bus;
    GMainLoop *loop;
    GstStateChangeReturn ret;

    pipeline = gst_pipeline_new ("test-pipeline");
    source = gst_element_factory_make ("autovideosrc", "source");
    sink = gst_element_factory_make ("autovideosink", "sink");

    if (!pipeline || !source || !sink) {
        g_printerr ("Not all elements could be created.\n");
        return -1;
    }

    //g_object_set(G_OBJECT (source), "location", "/dev/video0", NULL);

    gst_bin_add_many (GST_BIN (pipeline), source, sink, NULL);
    if (gst_element_link (source, sink) != TRUE) {
        g_printerr ("Elements could not be linked.\n");
        gst_object_unref (pipeline);
        return -1;
    }

    /* Start playing */
    ret = gst_element_set_state (pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_printerr ("Unable to set the pipeline to the playing state.\n");
        gst_object_unref (pipeline);
        return -1;
    }

    /* Wait until error or EOS */
    bus = gst_element_get_bus (pipeline);

    loop = g_main_loop_new(nullptr, FALSE);
    g_main_loop_run(loop);

    /* Free resources */
    g_main_loop_unref(loop);
    gst_object_unref (bus);
    gst_element_set_state (pipeline, GST_STATE_NULL);
    gst_object_unref (pipeline);

    return 0;
}

} // namespace c4a::gst