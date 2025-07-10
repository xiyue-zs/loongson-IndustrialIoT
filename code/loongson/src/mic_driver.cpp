#include "mic_driver.hpp"
#include <iostream>

INMP441_Mic::INMP441_Mic(const char* device) {
    if (snd_pcm_open(&pcm_handle_, device, SND_PCM_STREAM_CAPTURE, 0) < 0) {
        std::cerr << "MIC: Open error" << std::endl;
        pcm_handle_ = nullptr;
    }
}

INMP441_Mic::~INMP441_Mic() { close(); }

bool INMP441_Mic::init() {
    if (!pcm_handle_) return false;
    
    snd_pcm_hw_params_t* params;
    snd_pcm_hw_params_alloca(&params);
    
    if (snd_pcm_hw_params_any(pcm_handle_, params) < 0) {
        std::cerr << "MIC: Init error" << std::endl;
        return false;
    }
    
    snd_pcm_hw_params_set_access(pcm_handle_, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    snd_pcm_hw_params_set_format(pcm_handle_, params, SND_PCM_FORMAT_S16_LE);
    snd_pcm_hw_params_set_channels(pcm_handle_, params, 1);
    snd_pcm_hw_params_set_rate_near(pcm_handle_, params, &sample_rate_, 0);
    snd_pcm_hw_params_set_period_size_near(pcm_handle_, params, &period_size_, 0);
    
    if (snd_pcm_hw_params(pcm_handle_, params) < 0) {
        std::cerr << "MIC: Set params failed" << std::endl;
        return false;
    }
    return true;
}

bool INMP441_Mic::capture_audio(std::vector<int16_t>& buffer, size_t samples) {
    if (!pcm_handle_) return false;
    
    buffer.resize(samples);
    snd_pcm_sframes_t frames = snd_pcm_readi(pcm_handle_, buffer.data(), samples);
    
    if (frames < 0) {
        std::cerr << "MIC: Read error: " << snd_strerror(frames) << std::endl;
        return false;
    }
    return true;
}

void INMP441_Mic::close() {
    if (pcm_handle_) {
        snd_pcm_close(pcm_handle_);
        pcm_handle_ = nullptr;
    }
}