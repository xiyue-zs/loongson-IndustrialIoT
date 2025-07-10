#pragma once
#include <string>
#include <vector>
#include <linux/videodev2.h>

class USBCamera {
public:
    USBCamera(const std::string& device = "/dev/video0");
    ~USBCamera();
    
    bool init();
    bool capture_frame(std::vector<uint8_t>& buffer);
    void close();

private:
    int fd_;
    std::string device_;
    v4l2_format format_{};
    bool init_success_ = false;
    
    bool set_format();
    bool init_mmap();
};