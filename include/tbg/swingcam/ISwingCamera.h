#pragma once

namespace tbg {

// Forward-declare shared pointer
class ISwingCamera;
using ISwingCameraPtr = std::shared_ptr<ISwingCamera>;

/** Interface definition for the swing camera */
class ISwingCamera
{
public:
    /** Factory method(s) */
    static ISwingCameraPtr create();

    /** Virtual destructor */
    virtual ~ISwingCamera() = default;

    /** Run the camera */
    virtual void run() = 0;

    /** Update the pipeline */
    virtual void update() = 0;

    /** Wait for the pipeline to complete */
    virtual void wait() = 0;

    /** Trigger impact (recording) */
    virtual void impact() = 0;
};

} // namespace tbg