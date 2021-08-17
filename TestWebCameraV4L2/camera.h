#ifndef CAMERA_H
#define CAMERA_H

#include <stdio.h>
#include <stdlib.h>
#include <linux/videodev2.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <string.h>

#include <x264.h>
#include <fstream>

#include <opencv4/opencv2/opencv.hpp>
#include <opencv4/opencv2/video.hpp>
#include <opencv4/opencv2/imgproc.hpp>
#include <opencv4/opencv2/core.hpp>


#define DISPLAY_OPENCV  0

struct Buffer {
    uint8_t     *bufferStart;
    size_t      bufferLength;
};

static void xioctl(int fh, int request, void *arg)
{
    int r;

    do {
        r = ioctl(fh, request, arg);
    } while (r == -1 && ((errno == EINTR) || (errno == EAGAIN)));

    if (r == -1) {
        fprintf(stderr, "error %d, %s\\n", errno, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

class Camera
{

    int             m_pts;
    x264_t          *m_x264_handle;
    x264_nal_t      *m_x264_nals;
    x264_param_t    *m_x264_params;
    x264_picture_t  *m_x264_picIn;
    x264_picture_t  *m_x264_picOut;
    int             m_x264_numNals;

    void Setx264Params();
    std::ofstream   m_ofs;

public:
    Camera();
    virtual ~Camera();

    void InitDevice();
    void UnInitDevice();
    void StartCapture();
    void StopCapture();

    void Openx264Encoder();

private:

    enum {
        SIZE_TEXT = 80
    };

    static const char device_name[SIZE_TEXT];

    int                 m_fd;
    int                 m_type, m_r;

    v4l2_capability     m_capability;
    v4l2_format         m_format;
    v4l2_requestbuffers m_rbuffer;
    v4l2_buffer         m_qbuffer;
    v4l2_buffer         m_bufferinfo;

    Buffer              *m_buffers;
    int                 m_nBuffers;

    enum {
        WIDTH           = 1280,
        HEIGHT          = 720,
        FPS             = 10,
        BUF_TIME        = 80,
        TOTAL_FRAMES    = 101
    };

#if DISPLAY_OPENCV
    cv::Mat             m_yuyv;
    cv::Mat             m_rgb;
#endif

    fd_set              m_fds;
    struct timeval      m_tv;
};

#endif // CAMERA_H
