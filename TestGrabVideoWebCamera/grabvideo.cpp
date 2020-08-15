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

#include "grabvideo.h"

GrabVideo::GrabVideo(QObject *parent) : QObject(parent)
{
    avdevice_register_all();
    avcodec_register_all();

    pAVCodecCtx = nullptr;
    pAVCodec = nullptr;
    pAVInputFormat = nullptr;
    pOptions = nullptr;
    pSwsCtx = nullptr;

    bRunning = false;

    gotFrameFinished = 0;
    videoStream = -1;

    pAVFormatCtx = avformat_alloc_context();
}

GrabVideo::~GrabVideo()
{
    if (pAVFormatCtx != nullptr) {
        avformat_close_input(&pAVFormatCtx);
        avformat_free_context(pAVFormatCtx);
    }
}

void GrabVideo::InitVideo()
{
    pAVFrame = av_frame_alloc();
    pAVFrameRGB = av_frame_alloc();

    pAVInputFormat = av_find_input_format("video4linux2");
    if (pAVInputFormat == nullptr) {
        emit signalSendInfo(tr("av_find_input_format failed"));
        return;
    }

    av_dict_set(&pOptions, "video_size", "1280x720", 0);

    if (avformat_open_input(&pAVFormatCtx, fileNameSrc, pAVInputFormat, &pOptions) != 0) {
        emit signalSendInfo(tr("avformat_open_input failed"));
        //        avformat_close_input(&pAVFormatCtx);
        return;
    }

    if (avformat_find_stream_info(pAVFormatCtx, nullptr) < 0) {
        emit signalSendInfo(tr("avformat_find_stream_info failed"));
        return;
    }

    for (int i=0; pAVFormatCtx->nb_streams; i++) {
        if (pAVFormatCtx->streams[i]->codec->coder_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
            break;
        }
    }

    if (videoStream == -1) {
        emit signalSendInfo(tr("videoStream failed"));
        return;
    }

    pAVCodecCtx = pAVFormatCtx->streams[videoStream]->codec;
    pAVCodec = avcodec_find_decoder(pAVCodecCtx->codec_id);
    if (pAVCodec == nullptr) {
        emit signalSendInfo(tr("avcodec_find_decoder failed"));
        return;
    }

    if (avcodec_open2(pAVCodecCtx, pAVCodec, nullptr) < 0) {
        emit signalSendInfo(tr("avcodec_open2 failed"));
        return;
    }

    bRunning = true;
    pixelFormat = AV_PIX_FMT_RGB24;

    pSwsCtx = sws_getContext(pAVCodecCtx->width, pAVCodecCtx->height, pAVCodecCtx->pix_fmt,
                             pAVCodecCtx->width, pAVCodecCtx->height, pixelFormat,
                             SWS_BICUBIC, nullptr, nullptr, nullptr);
}

void GrabVideo::OpenCamera()
{
    uint8_t *frame_buffer = nullptr;
    int totalBytes = 0;

    AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));

    totalBytes = avpicture_get_size(pixelFormat, pAVCodecCtx->width, pAVCodecCtx->height);
    frame_buffer = (uint8_t *)av_malloc(totalBytes * sizeof (uint8_t));

    avpicture_fill((AVPicture *)pAVFrameRGB, frame_buffer, pixelFormat, pAVCodecCtx->width, pAVCodecCtx->height);

    while (bRunning)
    {
        if (av_read_frame(pAVFormatCtx, packet) >= 0)
        {
            if (packet->stream_index == videoStream)
            {
                avcodec_decode_video2(pAVCodecCtx, pAVFrame, &gotFrameFinished, packet);
                if (gotFrameFinished)
                {
                    if (pSwsCtx)
                    {
                        sws_scale(pSwsCtx, ((AVPicture*)pAVFrame)->data, ((AVPicture *)pAVFrame)->linesize, 0, pAVCodecCtx->height,
                                  ((AVPicture *)pAVFrameRGB)->data, ((AVPicture *)pAVFrameRGB)->linesize);

                        QImage img(pAVFrameRGB->data[0], pAVCodecCtx->width, pAVCodecCtx->height, QImage::Format_RGB888);
                        signalSendImage(img);
                    }
                }
            }
            av_packet_unref(packet);
        }
    } //while

    av_free(frame_buffer);
}

void GrabVideo::CloseCamera()
{
    bRunning = false;

    if (pSwsCtx != nullptr)
        sws_freeContext(pSwsCtx);

    if (pAVFrame != nullptr)
        av_free(pAVFrame);

    if (pAVFrameRGB != nullptr)
        av_free(pAVFrameRGB);

    avcodec_close(pAVCodecCtx);
}
