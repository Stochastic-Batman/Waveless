#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "compress.h"
#include "wav.h"
#include "wvl_format.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


// smooths edges to prevent spectral leakage (false frequencies from abrupt boundaries)
void apply_hann_window(Complex* frame, int n) {
    for (int i = 0; i < n; i++) {
        double hann_coefficient = 0.5 * (1 - cos(2 * M_PI * i / (n - 1)));
        frame[i].re *= hann_coefficient;
        frame[i].im *= hann_coefficient;
    }
}


// discard frequencies humans can't hear or that are masked by louder ones
void threshold_frame(Complex* frame, int n, double threshold_ratio) {
    double peak_magnitude = 0.0;
    for (int i = 0; i <= n/2; i++) {
        double magnitude = sqrt(frame[i].re * frame[i].re + frame[i].im * frame[i].im);
        if (magnitude > peak_magnitude)
            peak_magnitude = magnitude;
    }
    
    double threshold = peak_magnitude * threshold_ratio;
    
    for (int i = 0; i <= n/2; i++) {
        double magnitude = sqrt(frame[i].re * frame[i].re + frame[i].im * frame[i].im);
        if (magnitude < threshold) {
            frame[i].re = 0.0;
            frame[i].im = 0.0;
        }
    }
    
    for (int i = 1; i < n/2; i++) {
        frame[n - i].re = frame[i].re;
        frame[n - i].im = -frame[i].im;
    }
}


int compress(const char* wav_path, const char* wvl_path, double threshold_ratio) {
    WavInfo wav_info;
    int16_t* samples = wav_read(wav_path, &wav_info);
    if (!samples) {
        fprintf(stderr, "Failed to read WAV file\n");
        return -1;
    }

    WvlHeader wvl_header;
    wvl_header.magic = WVL_MAGIC;
    wvl_header.num_channels = wav_info.num_channels;
    wvl_header.sample_rate = wav_info.sample_rate;
    wvl_header.frame_size = FRAME_SIZE;
    wvl_header.num_frames = (wav_info.num_samples + FRAME_SIZE - 1) / FRAME_SIZE;

    WvlFrame* frames = calloc(wvl_header.num_frames, sizeof(WvlFrame));
    if (!frames) {
        free(samples);
        return -1;
    }

    for (uint32_t frame_idx = 0; frame_idx < wvl_header.num_frames; frame_idx++) {
        Complex* complex_frame = calloc(FRAME_SIZE, sizeof(Complex));
        
        for (int i = 0; i < FRAME_SIZE; i++) {
            uint32_t sample_idx = frame_idx * FRAME_SIZE + i;
            if (sample_idx < wav_info.num_samples) {
                complex_frame[i].re = samples[sample_idx] / 32768.0;
            }
            complex_frame[i].im = 0.0;
        }

        apply_hann_window(complex_frame, FRAME_SIZE);
        fft(complex_frame, FRAME_SIZE);
        threshold_frame(complex_frame, FRAME_SIZE, threshold_ratio);

        uint16_t num_bins = 0;
        for (int i = 0; i < FRAME_SIZE; i++) {
            if (complex_frame[i].re != 0.0 || complex_frame[i].im != 0.0) {
                num_bins++;
            }
        }

        frames[frame_idx].num_bins = num_bins;
        frames[frame_idx].bins = malloc(num_bins * sizeof(WvlBin));
        
        int bin_idx = 0;
        for (int i = 0; i < FRAME_SIZE; i++) {
            if (complex_frame[i].re != 0.0 || complex_frame[i].im != 0.0) {
                frames[frame_idx].bins[bin_idx].bin_index = i;
                frames[frame_idx].bins[bin_idx].re = (float)complex_frame[i].re;
                frames[frame_idx].bins[bin_idx].im = (float)complex_frame[i].im;
                bin_idx++;
            }
        }

        free(complex_frame);
    }

    int result = wvl_write(wvl_path, &wvl_header, frames);

    wvl_free_frames(frames, wvl_header.num_frames);
    free(samples);

    return result;
}

int decompress(const char* wvl_path, const char* wav_path) {
    WvlHeader wvl_header;
    WvlFrame* frames = NULL;
    
    if (wvl_read(wvl_path, &wvl_header, &frames) != 0) {
        fprintf(stderr, "Failed to read WVL file\n");
        return -1;
    }

    int hop_size = FRAME_SIZE / 2;
    uint32_t total_samples = (wvl_header.num_frames - 1) * hop_size + FRAME_SIZE;
    
    float* output_buffer = calloc(total_samples, sizeof(float));
    if (!output_buffer) {
        wvl_free_frames(frames, wvl_header.num_frames);
        return -1;
    }

    for (uint32_t frame_idx = 0; frame_idx < wvl_header.num_frames; frame_idx++) {
        Complex* complex_frame = calloc(FRAME_SIZE, sizeof(Complex));
        
        for (uint16_t i = 0; i < frames[frame_idx].num_bins; i++) {
            WvlBin bin = frames[frame_idx].bins[i];
            complex_frame[bin.bin_index].re = bin.re;
            complex_frame[bin.bin_index].im = bin.im;
        }

        ifft(complex_frame, FRAME_SIZE);
        apply_hann_window(complex_frame, FRAME_SIZE);

        uint32_t offset = frame_idx * hop_size;
        for (int i = 0; i < FRAME_SIZE; i++) {
            if (offset + i < total_samples) {
                output_buffer[offset + i] += (float)complex_frame[i].re;
            }
        }

        free(complex_frame);
    }

    int16_t* samples = malloc(total_samples * sizeof(int16_t));
    if (!samples) {
        free(output_buffer);
        wvl_free_frames(frames, wvl_header.num_frames);
        return -1;
    }

    for (uint32_t i = 0; i < total_samples; i++) {
        float value = output_buffer[i] * 32768.0f;
        if (value > 32767.0f) value = 32767.0f;
        if (value < -32768.0f) value = -32768.0f;
        samples[i] = (int16_t)value;
    }

    WavInfo wav_info;
    wav_info.num_channels = wvl_header.num_channels;
    wav_info.sample_rate = wvl_header.sample_rate;
    wav_info.bits_per_sample = 16;
    wav_info.num_samples = total_samples;

    int result = wav_write(wav_path, samples, &wav_info);

    free(output_buffer);
    free(samples);
    wvl_free_frames(frames, wvl_header.num_frames);

    return result;
}
