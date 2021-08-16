from build.python.conanfile import c4aConanFile

class tbgSwingCapture(c4aConanFile):
    name = "swing-capture"
    version = "0.0.1"
    url = "https://github.com/tbg-dev/swing-capture"
    description = "Swing Capture app for Teebox Golf."
    requires = (
        "c4a-core/0.0.1@tbg-dev/stable"
    )
