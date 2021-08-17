#include <iostream>
#include <memory>

#include "camera.h"

int main(int argc, char *argv[])
{
    std::unique_ptr<Camera> pCamera(new Camera());
    pCamera->InitDevice();
    pCamera->Openx264Encoder();
    pCamera->StartCapture();


    return 0;
}
