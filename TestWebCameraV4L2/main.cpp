#include <iostream>
#include <thread>
#include <memory>

#include "camera.h"

class CameraRunner {
    Camera kCamera;
public:
    void Run() {
        kCamera.InitDevice();
        kCamera.Openx264Encoder();
        kCamera.StartCapture();
    }
};

int main(int argc, char *argv[])
{
    CameraRunner kCameraRunner;
    std::thread tCamera{&CameraRunner::Run, &kCameraRunner};

    if (tCamera.joinable())
        tCamera.join();

    return 0;
}
