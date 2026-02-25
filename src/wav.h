#ifndef WAV_H
#define WAV_H

#include <stdint.h>

typedef struct {
    uint16_t num_channels;
    uint32_t sample_rate;
    uint16_t bits_per_sample;
    uint32_t num_samples;
} WavInfo;

int16_t* wav_read(const char* path, WavInfo* info);
int wav_write(const char* path, const int16_t* samples, const WavInfo* info);

#endif
