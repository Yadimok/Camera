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
    pAVFormatCtx = avformat_alloc_context();
    pAVCodec = nullptr;
    pAVInputFormat = nullptr;
    pOptions = nullptr;
    pAVFrame = av_frame_alloc();
    pAVFrameRGB = av_frame_alloc();

    pixelFormat = AV_PIX_FMT_RGB24;

    bRunning = false;

    gotFrameFinished = 0;
    videoStream = -1;

}

GrabVideo::~GrabVideo()
{
    //    av_free_packet(&packet);
    avcodec_close(pAVCodecCtx);
    av_free(pAVFrame);
    av_free(pAVFrameRGB);
    avformat_close_input(&pAVFormatCtx);
}

void GrabVideo::SetRunning(const bool run)
{
    bRunning = run;
}

void GrabVideo::InitVideo(Resolution res)
{
    pAVInputFormat = av_find_input_format("v4l2");

    switch (res) {
    case SIZE_NONE:
        emit SendInfo(tr("Resolution SIZE_NONE"));
        break;
    case SIZE_320_240:
        av_dict_set(&pOptions, "video_size", "320x240", 0);
        break;
    case SIZE_640_480:
        av_dict_set(&pOptions, "video_size", "640x480", 0);
        break;
    case SIZE_800_600:
        av_dict_set(&pOptions, "video_size", "800x600", 0);
        break;
    case SIZE_1280_720:
        av_dict_set(&pOptions, "video_size", "1280x720", 0);
        break;
    default:
        av_dict_set(&pOptions, "video_size", "640x480", 0);
        break;
    }


    if (avformat_open_input(&pAVFormatCtx, fileNameSrc, pAVInputFormat, &pOptions) != 0) {
        emit SendInfo(tr("avformat_open_input failed"));
    }

    if (avformat_find_stream_info(pAVFormatCtx, nullptr) < 0) {
        emit SendInfo(tr("avformat_find_stream_info failed"));
    }

    for (int i=0; pAVFormatCtx->nb_streams; i++) {
        if (pAVFormatCtx->streams[i]->codec->coder_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = i;
            break;
        }
    }

    if (videoStream == -1) {
        emit SendInfo(tr("videoStream failed"));
    }

    pAVCodecCtx = pAVFormatCtx->streams[videoStream]->codec;
    pAVCodec = avcodec_find_decoder(pAVCodecCtx->codec_id);
    if (pAVCodec == nullptr) {
        emit SendInfo(tr("avcodec_find_decoder failed"));
    }

    if (avcodec_open2(pAVCodecCtx, pAVCodec, nullptr) < 0) {
        emit SendInfo(tr("avcodec_open2 failed"));
    }
}

void GrabVideo::GetVideo()
{
    uint8_t *frame_buffer = nullptr;
    int totalBytes = 0;
    struct SwsContext *pSwsCtx = nullptr;

    totalBytes = avpicture_get_size(pixelFormat, pAVCodecCtx->width, pAVCodecCtx->height);
    frame_buffer = (uint8_t *)av_malloc(totalBytes * sizeof (uint8_t));

    avpicture_fill((AVPicture *)pAVFrameRGB, frame_buffer, pixelFormat, pAVCodecCtx->width, pAVCodecCtx->height);
    av_init_packet(&packet);

    while (av_read_frame(pAVFormatCtx, &packet) >= 0)
    {
        if (packet.stream_index == videoStream)
        {
            avcodec_decode_video2(pAVCodecCtx, pAVFrame, &gotFrameFinished, &packet);
            if (gotFrameFinished)
            {
                pSwsCtx = sws_getCachedContext(nullptr, pAVCodecCtx->width, pAVCodecCtx->height, pAVCodecCtx->pix_fmt,
                                               pAVCodecCtx->width, pAVCodecCtx->height, pixelFormat,
                                               SWS_BICUBIC, nullptr, nullptr, nullptr);
                sws_scale(pSwsCtx, ((AVPicture*)pAVFrame)->data, ((AVPicture *)pAVFrame)->linesize, 0, pAVCodecCtx->height,
                          ((AVPicture *)pAVFrameRGB)->data, ((AVPicture *)pAVFrameRGB)->linesize);

                pAVFrameRGB->width = pAVFrame->width;
                pAVFrameRGB->height = pAVFrame->height;

                emit SendFrame(pAVFrameRGB);

                av_free_packet(&packet);
            }
        }

        if (!bRunning)
            break;
    } //while

    sws_freeContext(pSwsCtx);
    av_free(frame_buffer);
}
