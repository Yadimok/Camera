//export LIBGL_ALWAYS_SOFTWARE=1 glxgears


#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <GLFW/glfw3.h>

int gCloseWindow = 0;
void glfwCloseWindowCb(GLFWwindow *window);
void glfwKeyCb(GLFWwindow *window, int key, int scancode, int action, int modes);

void flipHorizontalImage(uint8_t *pixels, size_t width, size_t height, int bpp);
void flipVerticalImage(uint8_t *pixels, size_t width, size_t height, int bpp);
void grayScaleImage(uint8_t *pixels, size_t width, size_t height, int bpp);

void printUsage()
{
    std::cout << "\nConvert data from YUYV format to RGB888 image.\n";
    std::cout << "The image size of: width = 1280, heigth = 720 pixels.\n";
    std::cout << "Type of image:\n";
    std::cout << "0 Flip vertical image.\n";
    std::cout << "1 Flip horizontal image.\n";
    std::cout << "2 Gray image.\n" << std::endl;
}

int main(int argc, char *argv[])
{
    if (argc < 3) {
        printUsage();
        std::cout << "Usage: " << argv[0] << " <yuv file> <type>\n";
        std::cout << "Exited...\n\n";
        exit(1);
    }

    int type = atoi(argv[2]);
    if ((type < 0) || (type > 2)) {
        std::cout << "Incorrect type.\n";
        std::cout << "Input a number in the range 0..2.\n";
        std::cout << "Exited...\n\n";
        exit(1);
    }

    const size_t WIDTH_WINDOW       = 1300;
    const size_t HEIGHT_WINDOW      = 740;
    const size_t WIDTH_IMAGE        = 1280;
    const size_t HEIGHT_IMAGE       = 720;
    const size_t BITS_PER_PIXEL_RGB = 3;

    GLFWwindow *pWindow;

    if (!glfwInit()) {
        printf("glfwInit failed\n");
        exit(1);
    }

    pWindow = glfwCreateWindow(WIDTH_WINDOW, HEIGHT_WINDOW, "OpenGL RGB image", NULL, NULL);
    if (pWindow == NULL) {
        printf("glfwCreateWindow failed\n");
        exit(1);
    }

    int window_width = 0, window_height = 0;

    glfwMakeContextCurrent(pWindow);
    glfwSetWindowCloseCallback(pWindow, glfwCloseWindowCb);
    glfwSetKeyCallback(pWindow, glfwKeyCb);

    GLuint text_handle;
    glGenTextures(1, &text_handle);
    glBindTexture(GL_TEXTURE_2D, text_handle);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    int fd = -1, rc = 0, total_size = 0;
    struct stat st;

    fd = open(argv[1], O_RDONLY);
    rc = fstat(fd, &st);
    total_size = st.st_size;

    uint8_t *data_yuv = (uint8_t *)mmap(NULL, total_size, PROT_READ | PROT_EXEC, MAP_SHARED, fd, 0);
    if (data_yuv == MAP_FAILED) {
        perror("Unable to mmap data");
        close(fd);
    }

    int i, j;
    uint8_t u, y0, v, y1;
    uint8_t r, g, b;

    uint8_t *rgb888 = new uint8_t [WIDTH_IMAGE * HEIGHT_IMAGE * BITS_PER_PIXEL_RGB]();

    for (i=0, j=0; i<total_size; i+=4, j+=6)
    {
        y0   = data_yuv[i+0];
        u    = data_yuv[i+1];
        y1   = data_yuv[i+2];
        v    = data_yuv[i+3];

        r = (unsigned char)(y0 + 1.4075 * (v - 128));
        g = (unsigned char)(y0 - 0.3455 * (u - 128) - (0.7169 * (v - 128)));
        b = (unsigned char)(y0 + 1.7790 * (u - 128));

        if (type == 0) {
            rgb888[j+0] = b;
            rgb888[j+1] = g;
            rgb888[j+2] = r;
        } else {
            rgb888[j+0] = r;
            rgb888[j+1] = g;
            rgb888[j+2] = b;
        }

        r = (unsigned char)(y1 + 1.4075 * (v - 128));
        g = (unsigned char)(y1 - 0.3455 * (u - 128) - (0.7169 * (v - 128)));
        b = (unsigned char)(y1 + 1.7790 * (u - 128));

        if (type == 0) {
            rgb888[j+3] = b;
            rgb888[j+4] = g;
            rgb888[j+5] = r;
        } else {
            rgb888[j+3] = r;
            rgb888[j+4] = g;
            rgb888[j+5] = b;
        }
    }

    switch (type)
    {
        case 0:
            flipHorizontalImage(rgb888, WIDTH_IMAGE, HEIGHT_IMAGE, 3);
            break;
        
        case 1:
            flipVerticalImage(rgb888, WIDTH_IMAGE, HEIGHT_IMAGE, 3);
            break;

        case 2:
            grayScaleImage(rgb888, WIDTH_IMAGE, HEIGHT_IMAGE, 3);
            break;
    }

    while (!gCloseWindow)
    {      
        glViewport(0, 0, WIDTH_WINDOW, HEIGHT_WINDOW);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glfwGetFramebufferSize(pWindow, &window_width, &window_height);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, window_width, window_height, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH_IMAGE, HEIGHT_IMAGE, 0, GL_RGB, GL_UNSIGNED_BYTE, rgb888);

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, text_handle);
        glBegin(GL_QUADS);
        glTexCoord2d(0, 0); glVertex2d(10, 10);
        glTexCoord2d(1, 0); glVertex2d(10 + WIDTH_IMAGE, 10);
        glTexCoord2d(1, 1); glVertex2d(10 + WIDTH_IMAGE, 10 + HEIGHT_IMAGE);
        glTexCoord2d(0, 1); glVertex2d(10, 10 + HEIGHT_IMAGE);
        glEnd();
        glDisable(GL_TEXTURE_2D);

        glfwSwapBuffers(pWindow);
        glfwPollEvents();
    }

    glfwDestroyWindow(pWindow);

    delete[] rgb888;

    if (-1 == munmap(data_yuv, total_size)) {
        perror("Unable to munmap data");
    }

    close(fd);

    return 0;
}

void flipHorizontalImage(uint8_t *pixels, size_t width, size_t height, int bpp)
{
    int stride = width * bpp;
    int col, row;

    for (col = 0; col < (stride / 2); col++)
    {
        for (row = 0; row < height; row++)
        {
            int index1 = row * stride + col;
            int index2 = ((row + 1) * stride - 1) - col;

            unsigned char tmp = pixels[index1];
            pixels[index1] = pixels[index2];
            pixels[index2] = tmp;
        }
    }
}
void flipVerticalImage(uint8_t *pixels, size_t width, size_t height, int bpp)
{
    int stride = width * bpp;
    int max_height = height-1;
    int col, row;

    for (row = 0; row < height/2; row++)
    {
        for (col = 0; col < stride; col++)
        {
            int index1 = row * stride + col;
            int index2 = max_height * stride + col;

            unsigned char tmp = pixels[index1];
            pixels[index1] = pixels[index2];
            pixels[index2] = tmp;
        }
        max_height--;
    }
}

void grayScaleImage(uint8_t *pixels, size_t width, size_t height, int bpp)
{
    double r, g, b;
    unsigned char gray;
    unsigned int totalSize = width * height * bpp;
    int data;
    for (data=0; data<totalSize; data +=3)
    {
        r = pixels[data + 0];
        g = pixels[data + 1];
        b = pixels[data + 2];

        r = r * 0.2126;
        g = g * 0.7152;
        b = b * 0.0722;

        gray = (r + g + b + 0.5);

        pixels[data + 0] = gray;
        pixels[data + 1] = gray;
        pixels[data + 2] = gray;
    }
}

void glfwCloseWindowCb(GLFWwindow *window)
{
    gCloseWindow = 1;
}

void glfwKeyCb(GLFWwindow *window, int key, int scancode, int action, int modes)
{
    if ((key == GLFW_KEY_ESCAPE) && (action == GLFW_PRESS))
        gCloseWindow = 1;
}