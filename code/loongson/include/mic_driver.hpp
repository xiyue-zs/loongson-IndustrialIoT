#pragma once
#include <alsa/asoundlib.h>
#include <vector>

class INMP441_Mic {
public:
    INMP441_Mic(const char* device = "default");
    ~INMP441_Mic();
    
    bool init();
    bool capture_audio(std::vector<int16_t>& buffer, size_t samples);
    void close();

private:
    snd_pcm_t* pcm_handle_;
    unsigned int sample_rate_ = 44100;
    snd_pcm_uframes_t period_size_ = 1024;
};