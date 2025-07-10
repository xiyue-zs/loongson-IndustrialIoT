#include "camera_driver.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <cstring>
#include <stdexcept>
#include <iostream>

USBCamera::USBCamera(const std::string& device) : device_(device) {}

USBCamera::~USBCamera() { close(); }

bool USBCamera::init() {
    fd_ = open(device_.c_str(), O_RDWR);
    if (fd_ < 0) {
        perror("Camera open failed");
        return false;
    }
    
    if (!set_format()) return false;
    if (!init_mmap()) return false;
    
    init_success_ = true;
    return true;
}

bool USBCamera::set_format() {
    memset(&format_, 0, sizeof(format_));
    format_.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format_.fmt.pix.width = 640;
    format_.fmt.pix.height = 480;
    format_.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    format_.fmt.pix.field = V4L2_FIELD_ANY;
    
    if (ioctl(fd_, VIDIOC_S_FMT, &format_) < 0) {
        perror("Set format failed");
        return false;
    }
    return true;
}

bool USBCamera::init_mmap() {
    v4l2_requestbuffers req{};
    req.count = 4;
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    req.memory = V4L2_MEMORY_MMAP;

    if (ioctl(fd_, VIDIOC_REQBUFS, &req) < 0) {
        perror("Request buffers failed");
        return false;
    }
    return true;
}

bool USBCamera::capture_frame(std::vector<uint8_t>& buffer) {
    if (!init_success_) return false;

    v4l2_buffer buf{};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;

    if (ioctl(fd_, VIDIOC_QBUF, &buf) < 0) {
        perror("Queue buffer failed");
        return false;
    }

    if (ioctl(fd_, VIDIOC_STREAMON, &buf.type) < 0) {
        perror("Stream on failed");
        return false;
    }

    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(fd_, &fds);
    timeval tv{5, 0}; // 5s timeout

    if (select(fd_ + 1, &fds, NULL, NULL, &tv) <= 0) {
        perror("Capture timeout");
        return false;
    }

    if (ioctl(fd_, VIDIOC_DQBUF, &buf) < 0) {
        perror("Dequeue buffer failed");
        return false;
    }

    buffer.resize(buf.bytesused);
    memcpy(buffer.data(), (uint8_t*)buf.m.offset, buf.bytesused);

    ioctl(fd_, VIDIOC_STREAMOFF, &buf.type);
    return true;
}

void USBCamera::close() {
    if (fd_ >= 0) ::close(fd_);
    fd_ = -1;
}