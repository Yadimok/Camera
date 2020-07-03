# Camera:

1. <b>TestLibUVCWebCamera</b> - Records a specific frame and saves in `JPEG` format

1.1 Download `libusb` from `https://github.com/libusb/libusb/releases` and install it

1.2 Install `v4l-utils`: `$ sudo apt install v4l-utils`

1.3 Install `libjpeg`: `$ sudo apt install libjpeg-dev`

1.4 Download `libUVC` from `https://github.com/libuvc/libuvc` and install it

1.5 Connect web-camera to PC

1.6 Check web-camera: `$ lsusb`

1.7 Check available formats: `$ v4l2-ctl --list-formats-ext`, for example:

	$ v4l2-ctl --list-formats-ext
	ioctl: VIDIOC_ENUM_FMT
        Type: Video Capture

        [0]: 'YUYV' (YUYV 4:2:2)
                Size: Discrete 1280x720
                        Interval: Discrete 0.100s (10.000 fps)
                Size: Discrete 800x600
                        Interval: Discrete 0.100s (10.000 fps)
                Size: Discrete 640x480
                        Interval: Discrete 0.040s (25.000 fps)
                Size: Discrete 320x240
                        Interval: Discrete 0.033s (30.000 fps)
        [1]: 'MJPG' (Motion-JPEG, compressed)
                Size: Discrete 1280x720
                        Interval: Discrete 0.033s (30.000 fps)
                Size: Discrete 800x600
                        Interval: Discrete 0.033s (30.000 fps)
                Size: Discrete 640x480
                        Interval: Discrete 0.033s (30.000 fps)
                Size: Discrete 320x240
                        Interval: Discrete 0.033s (30.000 fps)

