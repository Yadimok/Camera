/*
*
* Copyright (C) Yadimok2020
*
* TestLibUVCWebCamera is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; either
* version 2 of the License, or (at your option) any later version.
*
* TestLibUVCWebCamera is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Library General Public License for more details.
*
* You should have received a copy of the GNU Library General Public License
* along with TestLibUVCWebCamera. If not, write to
* the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
* Boston, MA 02110-1301, USA.
**/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern "C" {
    #include <libuvc/libuvc.h>
    #include <jpeglib.h>
}

#define WIDTH			1280
#define HEIGHT			720
#define FPS				30
#define SERIAL_NUM		"SN0001"
#define VENDOR_ID 		0x09da
#define PRODUCT_ID 		0x2690
#define NUMBER_OF_FRAME	10
#define FILE_NAME		"Test.jpg"
#define QUALITY 		100

void cb(uvc_frame_t *frame, void *ptr)
{
    uvc_frame_t *rgb;
    uvc_error_t ret;

    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;

    static int frameCounter = 0;

    rgb = uvc_allocate_frame(frame->width * frame->height * 3);
    if (!rgb)
    {
        printf("uvc_allocate_frame failed\n");
        return;
    }

    ret = uvc_mjpeg2rgb(frame, rgb);
    if (ret)
    {
        printf("uvc_mjpeg2rgb failed\n");
        uvc_free_frame(rgb);
        return;
    }

    if (frameCounter == NUMBER_OF_FRAME)
    {
        unsigned char *image_data;
        JSAMPROW row_pointer[1];

        /* Step 1: allocate and initialize JPEG compression object */
        cinfo.err = jpeg_std_error(&jerr);
        jpeg_create_compress(&cinfo);

        image_data = (unsigned char *)malloc(rgb->width * rgb->height * 3);
        memcpy(image_data, rgb->data, (rgb->width * rgb->height * 3));

        /* Step 2: Setting the parameters of the output file here */
        FILE *outfile = fopen(FILE_NAME, "wb");
        if (!outfile)
        {
            printf("Error opening output jpeg file %s\n!", FILE_NAME);
            return;
        }

        jpeg_stdio_dest(&cinfo, outfile);

        /* Step 3: set parameters for compression */
        cinfo.image_width       = rgb->width;
        cinfo.image_height      = rgb->height;
        cinfo.input_components  = 3;
        cinfo.in_color_space    = JCS_RGB;

        jpeg_set_defaults(&cinfo);
        cinfo.num_components    = 3;
        cinfo.dct_method        = JDCT_ISLOW;
        jpeg_set_quality(&cinfo, QUALITY, TRUE);

        /* Step 4: Start compressor */
        jpeg_start_compress(&cinfo, TRUE);

        /* Step 5: while (scan lines remain to be written) */
        while(cinfo.next_scanline < cinfo.image_height)
        {
            row_pointer[0] = &image_data[cinfo.next_scanline * cinfo.image_width * cinfo.num_components];
            jpeg_write_scanlines(&cinfo, row_pointer, 1);
        }

        /* Step 6: Finish compression */
        jpeg_finish_compress(&cinfo);

        /* Step 7: release JPEG compression object */
        jpeg_destroy_compress(&cinfo);

        fclose(outfile);

        free(image_data);
    }
    frameCounter++;

    uvc_free_frame(rgb);
}

int main(int argc, char *argv[])
{
    uvc_context_t *ctx;
    uvc_device_t *dev;
    uvc_device_descriptor_t *descriptor;
    uvc_device_handle_t *devh;
    uvc_stream_ctrl_t ctrl;
    uvc_error_t res;

    res = uvc_init(&ctx, NULL);
    if (res < 0)
    {
        uvc_perror(res, "uvc_init");
        return res;
    }
    printf("UVC initialized\n");

    res = uvc_find_device(ctx, &dev, VENDOR_ID, PRODUCT_ID, SERIAL_NUM);
    if (res < 0)
    {
        uvc_perror(res, "uvc_find_device");
    } else
    {
        printf("Device found\n");
        res = uvc_open(dev, &devh);
        if (res < 0)
        {
            uvc_perror(res, "uvc_open");
        } else
        {
            printf("Device opened\n");

            res = uvc_get_device_descriptor(dev, &descriptor);
            if (res < 0)
            {
                uvc_perror(res, "uvc_get_device_descriptor");
            }

            printf("%0x4:%x, %0x4\n", descriptor->idVendor, descriptor->idProduct, descriptor->bcdUVC);
            printf("Serial number: %s\n", descriptor->serialNumber);
            printf("Manufacturer: %s\n", descriptor->manufacturer);
            printf("Product: %s\n", descriptor->product);

            res = uvc_get_stream_ctrl_format_size(devh, &ctrl, UVC_FRAME_FORMAT_MJPEG, WIDTH, HEIGHT, FPS);
            if (res < 0)
            {
                uvc_perror(res, "uvc_get_stream_ctrl_format_size"); /* device doesn't provide a matching stream */
            } else
            {
                res = uvc_start_streaming(devh, &ctrl, cb, (void *)12345, 0);
                if (res < 0)
                {
                    uvc_perror(res, "uvc_start_streaming");
                } else
                {
                    printf("Streaming...\n");
                    uvc_set_ae_mode(devh, 1); /* e.g., turn on auto exposure */
                    sleep(1); /* stream for 1 seconds */
                    uvc_stop_streaming(devh);
                    printf("Done streaming\n");
                }
            }
            uvc_close(devh);
            printf("Device closed\n");

            uvc_free_device_descriptor(descriptor);
            printf("Descriptor free\n");
        }
        uvc_unref_device(dev);
    }
    uvc_exit(ctx);
    printf("UVC exited\n");

    return 0;
}
