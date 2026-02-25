#ifndef COMPRESS_H
#define COMPRESS_H

#include "fft.h"

void apply_hann_window(Complex* frame, int n);
void threshold_frame(Complex* frame, int n, double threshold_ratio);
int compress(const char* wav_path, const char* wvl_path, double threshold_ratio);
int decompress(const char* wvl_path, const char* wav_path);

#endif
