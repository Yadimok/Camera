//gcc -Wall -pedantic web_camera_v4l2_example.c -o web_camera_v4l2_example

#include <stdio.h>
#include <stdlib.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/v4l2-common.h>
#include <linux/v4l2-controls.h>
#include <linux/videodev2.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <string.h>

#define WIDTH   1280
#define HEIGHT  720

int main(int argc, char *argv[])
{
    int fd;

    fd = open("/dev/video0", O_RDWR);
    if (fd < 0) {
        printf("open failed\n");
        exit(EXIT_FAILURE);
    }

    struct v4l2_capability capability;
    if (ioctl(fd, VIDIOC_QUERYCAP, &capability) < 0) {
        printf("ioctl VIDIOC_QUERYCAP failed\n");
        exit(EXIT_FAILURE);
    }

    printf("driver : %s\n",capability.driver);
    printf("card : %s\n",capability.card);
    printf("bus_info : %s\n",capability.bus_info);
    printf("version : %d.%d.%d\n",
           ((capability.version >> 16) & 0xFF),
           ((capability.version >> 8) & 0xFF),
           (capability.version & 0xFF));
    printf("capabilities: 0x%08x\n", capability.capabilities);
    printf("device capabilities: 0x%08x\n", capability.device_caps);

    struct v4l2_format format;
    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.fmt.pix.width = WIDTH;
    format.fmt.pix.height = HEIGHT;
    format.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    if (ioctl(fd, VIDIOC_S_FMT, &format) < 0) {
        printf("ioctl VIDIOC_S_FMT failed\n");
        exit(EXIT_FAILURE);
    }

    struct v4l2_requestbuffers rbuffer;
    rbuffer.count = 1;
    rbuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    rbuffer.memory = V4L2_MEMORY_MMAP;
    if (ioctl(fd, VIDIOC_REQBUFS, &rbuffer) < 0) {
        printf("ioctl VIDIOC_REQBUFS failed\n");
        exit(EXIT_FAILURE);
    }

    struct v4l2_buffer qbuffer;
    qbuffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    qbuffer.memory = V4L2_MEMORY_MMAP;
    qbuffer.index = 0;
    if (ioctl(fd, VIDIOC_QUERYBUF, &qbuffer) < 0) {
        printf("ioctl VIDIOC_QUERYBUF failed\n");
        exit(EXIT_FAILURE);
    }

    void *buffer = mmap(NULL, qbuffer.length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, qbuffer.m.offset);
    if (buffer == MAP_FAILED) {
        printf("mmap failed\n");
        exit(EXIT_FAILURE);
    }

    memset(buffer, 0, qbuffer.length);

    struct v4l2_buffer bufferinfo;
    memset(&bufferinfo, 0, sizeof (bufferinfo));
    bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    bufferinfo.memory = V4L2_MEMORY_MMAP;
    bufferinfo.index = 0;

    int type = bufferinfo.type;
    if (ioctl(fd, VIDIOC_STREAMON, &type) < 0) {
        printf("ioctl VIDIOC_STREAMON failed\n");
        exit(EXIT_FAILURE);
    }

    if (ioctl(fd, VIDIOC_QBUF, &bufferinfo) < 0) {
        printf("ioctl VIDIOC_QBUF failed\n");
        exit(EXIT_FAILURE);
    }

    if (ioctl(fd, VIDIOC_DQBUF, &bufferinfo) < 0) {
        printf("ioctl VIDIOC_DQBUF failed\n");
        exit(EXIT_FAILURE);
    }

    FILE *pFile;
    pFile = fopen("test.jpeg", "wb");
    fwrite(buffer, bufferinfo.bytesused, 1, pFile);

    (void)fclose(pFile);


    if (ioctl(fd, VIDIOC_STREAMOFF, &type) < 0) {
        printf("ioctl VIDIOC_STREAMOFF failed\n");
        exit(EXIT_FAILURE);
    }

    (void)munmap(buffer, qbuffer.length);

    (void)close(fd);

    return EXIT_SUCCESS;
}
