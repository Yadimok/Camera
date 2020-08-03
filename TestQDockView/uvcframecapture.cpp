#include "uvcframecapture.h"
#include <QDebug>

UVCFrameCapture::UVCFrameCapture(QObject *parent) : QObject(parent)
{
    ctx = nullptr;
    dev = nullptr;
    descriptor = nullptr;
    devh = nullptr;
    res = UVC_SUCCESS;

    bOpen = false;
    bStreaming = false;
}

UVCFrameCapture::~UVCFrameCapture()
{

}

int UVCFrameCapture::Init()
{
    if (bOpen) {
//        qDebug() << "libUVC already open";
        return 0;
    }

    res = uvc_init(&ctx, nullptr);
    if (res < 0) {
//        uvc_perror(res, "uvc_init");
        bOpen = false;
        return res;
    }

    bOpen = true;
//    qDebug() << "Init()";
    return 0;
}

void UVCFrameCapture::Close()
{
    if (bOpen) {

        StopStream();

        if (devh != nullptr) {
            uvc_close(devh);
//            qDebug() << "Device closed";
        }

        if (descriptor != nullptr)
            uvc_free_device_descriptor(descriptor);

        uvc_unref_device(dev);
        uvc_exit(ctx);

        bOpen = false;
//        qDebug() << "Close()";
    }
}

int UVCFrameCapture::OpenDevice()
{
    if (bOpen) {
        (void) Close();
    }

    (void)Init();

    res = uvc_find_device(ctx, &dev, VENDOR_ID, PRODUCT_ID, SERIAL_NUM);
    if (res < 0) {
//        uvc_perror(res, "uvc_find_device");
        return res;
    } else {
//        qDebug() << "Device found";
        res = uvc_open(dev, &devh);
        if (res < 0) {
            uvc_perror(res, "uvc_open");
            return res;
        } else {
//            qDebug() << "Device opened";
            res = uvc_get_device_descriptor(dev, &descriptor);
            if (res < 0) {
//                uvc_perror(res, "uvc_get_device_descriptor");
                return res;
            }

//            qDebug("%0x4:%x, %0x4", descriptor->idVendor, descriptor->idProduct, descriptor->bcdUVC);
//            qDebug("Serial number: %s", descriptor->serialNumber);
//            qDebug("Manufacturer: %s", descriptor->manufacturer);
//            qDebug("Product: %s", descriptor->product);

        }
    }
    return 0;
}

void UVCFrameCapture::StartStream()
{
    if (!bOpen) {
        bStreaming = false;
        return;
    }

    if (devh) {
        res = uvc_get_stream_ctrl_format_size(devh, &ctrl, UVC_FRAME_FORMAT_MJPEG, WIDTH, HEIGHT, FPS);
        if (res < 0) {
//            uvc_perror(res, "uvc_get_stream_ctrl_format_size");
        } else {
            res = uvc_start_streaming(devh, &ctrl, cb, this, 0);
            if (res < 0) {
//                uvc_perror(res, "uvc_start_streaming");
            } else {
                bStreaming = true;

//                qDebug() << "Streaming...";
//                uvc_set_ae_mode(devh, 1);
            }
        }
    }
}

void UVCFrameCapture::StopStream()
{
    if (bStreaming) {
        bStreaming = false;
        uvc_stop_streaming(devh);
//        qDebug() << "Stop streaming";
    }
}

void UVCFrameCapture::cb(uvc_frame_t *frame, void *ptr)
{
    uvc_frame_t *rgb;
    uvc_error_t ret;

    rgb = uvc_allocate_frame(frame->width * frame->height * 3);
    if (!rgb)
    {
//        qDebug("uvc_allocate_frame failed");
        return;
    }

    ret = uvc_mjpeg2rgb(frame, rgb);
    if (ret)
    {
//        qDebug("uvc_mjpeg2rgb failed");
        uvc_free_frame(rgb);
        return;
    }

    QImage image = QImage((unsigned char *)rgb->data, rgb->width, rgb->height, QImage::Format_RGB888);

    UVCFrameCapture *capture = reinterpret_cast<UVCFrameCapture *>(ptr);
    capture->cb_handler(image);

    uvc_free_frame(rgb);
}

void UVCFrameCapture::cb_handler(QImage image)
{
    QMutexLocker locker(&mMutex);
    emit signalSendFrame(image);
}

