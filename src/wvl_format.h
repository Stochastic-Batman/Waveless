#ifndef WVL_FORMAT_H
#define WVL_FORMAT_H

#include <stdint.h>
#include "fft.h"
#include "wav.h"

#define WVL_MAGIC 0x574C5653  // "WLVS"
#define FRAME_SIZE 1024

typedef struct {
    uint32_t magic;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t num_frames;
    uint32_t frame_size;
} WvlHeader;

// Each surviving bin: its index + packed re/im as floats
typedef struct {
    uint16_t bin_index;
    float    re;
    float    im;
} WvlBin;

// One RLE-compressed frame
typedef struct {
    uint16_t  num_bins;  // non-zero bins surviving threshold
    WvlBin*   bins;
} WvlFrame;

int wvl_write(const char* path, const WvlHeader* header, WvlFrame* frames);
int wvl_read(const char* path, WvlHeader* header, WvlFrame** frames);
void wvl_free_frames(WvlFrame* frames, uint32_t num_frames);

#endif
