#include "camera.h"

const char Camera::device_name[SIZE_TEXT] = "/dev/video0";

Camera::Camera() : m_fd(-1), m_type(0),
    m_x264_handle(nullptr),
    m_x264_nals(nullptr),
    m_x264_params(nullptr),
    m_x264_picIn(nullptr),
    m_x264_picOut(nullptr)
{
    m_fd = open(device_name, O_RDWR | O_NONBLOCK, 0);
    if (-1 == m_fd) {
        std::cerr << "Cannot open device: " << device_name << std::endl;
        exit(EXIT_FAILURE);
    }

    memset(&m_format, 0, sizeof (m_format));
    memset(&m_rbuffer, 0, sizeof (m_rbuffer));
    memset(&m_qbuffer, 0, sizeof (m_qbuffer));
    memset(&m_bufferinfo, 0, sizeof (m_bufferinfo));


    m_pts           = 0;
    m_x264_numNals  = 0;
    m_x264_params   = (x264_param_t *)malloc(sizeof (x264_param_t));
    m_x264_picIn    = (x264_picture_t *)malloc(sizeof(x264_picture_t));
    m_x264_picOut   = (x264_picture_t *)malloc(sizeof (x264_picture_t));

}

Camera::~Camera()
{
    UnInitDevice();

    close(m_fd);
    m_fd = -1;

    if (m_ofs.is_open()) {
        m_ofs.close();
    }

    x264_picture_clean(m_x264_picIn);

    if (m_x264_handle) {
//        x264_encoder_close(m_x264_handle);
        m_x264_handle = nullptr;
    }

    free(m_x264_picIn);
    free(m_x264_picOut);
    free(m_x264_params);
}

void Camera::InitDevice()
{
    m_format.type                 = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    m_format.fmt.pix.width        = WIDTH;
    m_format.fmt.pix.height       = HEIGHT;
    m_format.fmt.pix.pixelformat  = V4L2_PIX_FMT_YUYV;
    xioctl(m_fd, VIDIOC_S_FMT, &m_format);

    m_rbuffer.count               = 2;
    m_rbuffer.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    m_rbuffer.memory              = V4L2_MEMORY_MMAP;
    xioctl(m_fd, VIDIOC_REQBUFS, &m_rbuffer);

    m_buffers = (Buffer *)calloc(m_rbuffer.count, sizeof (*m_buffers));
    if (!m_buffers) {
        std::cerr << "calloc: Out of memory" << std::endl;
        exit(EXIT_FAILURE);
    }

    for (m_nBuffers=0; m_nBuffers<m_rbuffer.count; m_nBuffers++) {
        m_qbuffer.type            = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        m_qbuffer.memory          = V4L2_MEMORY_MMAP;
        m_qbuffer.index           = m_nBuffers;
        xioctl(m_fd, VIDIOC_QUERYBUF, &m_qbuffer);

        m_buffers[m_nBuffers].bufferLength = m_qbuffer.length;
        m_buffers[m_nBuffers].bufferStart = (uint8_t *)mmap(NULL, m_qbuffer.length, PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, m_qbuffer.m.offset);
        if (m_buffers[m_nBuffers].bufferStart == MAP_FAILED) {
            std::cerr << "mmap() failed" << std::endl;
            exit(EXIT_FAILURE);
        }
        memset(m_buffers[m_nBuffers].bufferStart, 0, m_qbuffer.length);
    }
}

void Camera::UnInitDevice()
{
    for (int idx=0; idx<m_nBuffers; idx++) {
        munmap(m_buffers[idx].bufferStart, m_buffers[idx].bufferLength);
    }

    free(m_buffers);
}

void Camera::doProcessing()
{
    m_futProcess = std::async(std::launch::async, &Camera::StartCapture, this);
}

void Camera::StartCapture()
{
    int length_frame = 0;

    for (int i=0; i<m_nBuffers; ++i) {
        memset(&m_bufferinfo, 0x00, sizeof(m_bufferinfo));
        m_bufferinfo.type         = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        m_bufferinfo.memory       = V4L2_MEMORY_MMAP;
        m_bufferinfo.index        = i;
        xioctl(m_fd, VIDIOC_QBUF, &m_bufferinfo);
    }

    m_type                        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    xioctl(m_fd, VIDIOC_STREAMON, &m_type);

#if DISPLAY_OPENCV
    m_yuyv    = cv::Mat(m_format.fmt.pix.height, m_format.fmt.pix.width, CV_8UC2);
    m_rgb     = cv::Mat(m_format.fmt.pix.height, m_format.fmt.pix.width, CV_8UC3);
    cv::namedWindow("Live stream", cv::WINDOW_AUTOSIZE);
#endif

    while (m_pts < TOTAL_FRAMES) {
        do {
            FD_ZERO(&m_fds);
            FD_SET(m_fd, &m_fds);

            m_tv.tv_sec   = 2;
            m_tv.tv_usec  = 0;

            m_r = select(m_fd + 1, &m_fds, nullptr, nullptr, &m_tv);
        } while ((m_r == -1 && (errno = EINTR)));

        if (m_r == -1) {
            std::cerr << "select() failed" << std::endl;
            errno;
            break;
        }


        m_bufferinfo.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        m_bufferinfo.memory = V4L2_MEMORY_MMAP;
        xioctl(m_fd, VIDIOC_DQBUF, &m_bufferinfo);

        m_x264_picIn->img.i_plane       = 1;
        m_x264_picIn->img.i_stride[0]   = WIDTH * 2;
        m_x264_picIn->img.plane[0]      = m_buffers[m_bufferinfo.index].bufferStart;
        m_x264_picIn->i_pts             = m_pts;

        //write to file
        length_frame = x264_encoder_encode(m_x264_handle, &m_x264_nals, &m_x264_numNals, m_x264_picIn, m_x264_picOut);
        if (length_frame > 0) {
            for (int idx=0; idx<m_x264_numNals; idx++)
                m_ofs.write((char *)m_x264_nals[idx].p_payload, m_x264_nals[idx].i_payload);
        }

        m_pts++;

#if DISPLAY_OPENCV
        memcpy(m_yuyv.data, m_buffers[m_bufferinfo.index].bufferStart, m_bufferinfo.bytesused);
        cv::cvtColor(m_yuyv, m_rgb, cv::COLOR_YUV2BGR_YUYV);
        cv::imshow("Live stream", m_rgb);
        if (cv::waitKey(10) == 27)
            break;
#endif

        xioctl(m_fd, VIDIOC_QBUF, &m_bufferinfo);
    }

    while (1) {
        length_frame = x264_encoder_encode(m_x264_handle, &m_x264_nals, &m_x264_numNals, nullptr, m_x264_picOut);
        if (length_frame == 0)
            break;

        for (int idx=0; idx<m_x264_numNals; idx++)
            m_ofs.write((char *)m_x264_nals[idx].p_payload, m_x264_nals[idx].i_payload);
    }

    m_ofs.flush();

    x264_encoder_close(m_x264_handle);

    StopCapture();
}

void Camera::StopCapture()
{
    m_type                      = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    xioctl(m_fd, VIDIOC_STREAMOFF, &m_type);
}

void Camera::Setx264Params()
{
    x264_param_default_preset(m_x264_params, x264_preset_names[4], x264_tune_names[6]);
    m_x264_params->i_width        = WIDTH;
    m_x264_params->i_height       = HEIGHT;
    m_x264_params->i_csp          = X264_CSP_YUYV;
    m_x264_params->i_log_level    = X264_LOG_DEBUG;
    m_x264_params->i_threads      = X264_THREADS_AUTO;
    m_x264_params->i_lookahead_threads = X264_SYNC_LOOKAHEAD_AUTO;
    m_x264_params->i_fps_den      = 1;
    m_x264_params->i_fps_num      = FPS;
    m_x264_params->i_timebase_den = FPS;
    m_x264_params->i_timebase_num = 1;
    m_x264_params->i_frame_total  = 0;

    x264_param_apply_profile(m_x264_params, x264_profile_names[4]);
}

void Camera::Openx264Encoder()
{
    char buftime[BUF_TIME];
    time_t curtime = time(nullptr);
    struct tm tm;
    tm = *localtime(&curtime);
    strftime(buftime, sizeof(buftime), "%d-%m-%Y.x264", &tm);

    std::cout << "File name: " << buftime << std::endl;

    m_ofs.open(buftime, std::ios::out | std::ios::binary);
    if (!m_ofs.is_open()) {
        std::cerr << "Error opening file: " << buftime << std::endl;
        exit(EXIT_FAILURE);
    }

    Setx264Params();

    m_x264_handle = x264_encoder_open_160(m_x264_params);
    x264_picture_alloc(m_x264_picIn,
                       X264_CSP_YUYV,
                       WIDTH,
                       HEIGHT);
    x264_picture_init(m_x264_picOut);

    //write header
    int pi_nal = 0, header_size = 0;
    int ret = x264_encoder_headers(m_x264_handle, &m_x264_nals, &pi_nal);
    if (ret < 0) {
        std::cerr << "Encoder header failed." << std::endl;
        exit(EXIT_FAILURE);
    }

    for (int idx=0; idx<pi_nal; idx++)
        header_size += m_x264_nals[idx].i_payload;

    m_ofs.write((char *)m_x264_nals[0].p_payload, header_size);
    m_ofs.flush();
}
