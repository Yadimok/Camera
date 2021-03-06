/*
*
* Copyright (C) Yadimok2020
*
* TestGrabVideoWebCamera is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* TestGrabVideoWebCamera is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Library General Public License
* along with TestGrabVideoWebCamera. If not, write to
* the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA 02110-1301, USA.
**/


#ifndef GRABVIDEO_H
#define GRABVIDEO_H

#include <QObject>
#include <QImage>

extern "C" {
#include <libavdevice/avdevice.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
}

const char fileNameSrc[] = "/dev/video0";

class GrabVideo : public QObject
{
    Q_OBJECT


public:
    explicit GrabVideo(QObject *parent = nullptr);
    ~GrabVideo();

    void InitVideo();
    void OpenCamera();
    void CloseCamera();

signals:
    void signalSendInfo(QString message);
    void signalSendImage(QImage image);

public slots:

private:
    AVCodecContext *pAVCodecCtx;
    AVFormatContext *pAVFormatCtx;
    AVCodec *pAVCodec;
    AVInputFormat *pAVInputFormat;
    AVFrame *pAVFrame;
    AVFrame *pAVFrameRGB;
    AVDictionary *pOptions;
    AVPixelFormat pixelFormat;
    struct SwsContext *pSwsCtx;

    int gotFrameFinished;
    int videoStream;

    std::atomic<bool> bRunning;

};

#endif // GRABVIDEO_H
