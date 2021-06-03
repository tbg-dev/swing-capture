from build.python.conanfile import c4aConanFile

class tbgSwingCapture(c4aConanFile):
    name = "swing_capture"
    version = "0.0.1"
    url = "https://github.com/tbg-dev/swing-capture"
    description = "Swing Capture app for Teebox Golf."
    requires = (
        "c4a_core/0.0.1@tbg-dev/stable",
        "gstreamer/1.18.4@tbg-dev/stable"
    )
