#ifndef UVCFRAMECAPTURE_H
#define UVCFRAMECAPTURE_H

#include <QObject>
#include <QImage>
#include <QMutex>

extern "C" {
    #include <libuvc/libuvc.h>
}

#define WIDTH			1280
#define HEIGHT			720
#define FPS				30
#define SERIAL_NUM		"SN0001"
#define VENDOR_ID 		0x09da
#define PRODUCT_ID 		0x2690


class UVCFrameCapture : public QObject
{
    Q_OBJECT
public:
    explicit UVCFrameCapture(QObject *parent = nullptr);
    ~UVCFrameCapture();

    int OpenDevice();

    void StartStream();
    void StopStream();

signals:
    void signalSendFrame(QImage image);

private:
    uvc_context_t *ctx;
    uvc_device_t *dev;
    uvc_device_descriptor_t *descriptor;
    uvc_device_handle_t *devh;
    uvc_stream_ctrl_t ctrl;
    uvc_error_t res;

    QMutex mMutex;

    bool bOpen;
    bool bStreaming;

    static void cb(uvc_frame_t *frame, void *ptr);
    void cb_handler(QImage image);

    int Init();
    void Close();
};

#endif // UVCFRAMECAPTURE_H
