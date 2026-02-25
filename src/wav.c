#include <stdlib.h>
#include <stdio.h>
#include "wav.h"


/* __attribute__((packed)) prevents the compiler from adding padding between members, 
   ensuring the struct is exactly 44 bytes to match the WAV file format. */
typedef struct __attribute__((packed)) {
    char     chunk_id[4];
    uint32_t chunk_size;
    char     format[4];
    char     subchunk1_id[4];
    uint32_t subchunk1_size;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    char     subchunk2_id[4];
    uint32_t subchunk2_size;
} WavHeader;  // based on https://en.wikipedia.org/wiki/WAV#WAV_file_header


int16_t* wav_read(const char* path, WavInfo* info) {
    FILE* fptr = fopen(path, "rb");
    if (fptr == NULL) {
        perror("Error opening WAV file");
        exit(1);
    }

    WavHeader header;
    if (fread(&header, sizeof(WavHeader), 1, fptr) < 1) {
        fclose(fptr);
        return NULL;
    }

    info->num_channels = header.num_channels;
    info->sample_rate = header.sample_rate;
    info->bits_per_sample = header.bits_per_sample;
    info->num_samples = header.subchunk2_size / (header.num_channels * (header.bits_per_sample / 8));

    int16_t* samples = malloc(header.subchunk2_size);
    if (samples) {
        fread(samples, 1, header.subchunk2_size, fptr);
    }

    fclose(fptr);
    return samples;
}

int wav_write(const char* path, const int16_t* samples, const WavInfo* info) {
    FILE* fptr = fopen(path, "wb");
    if (fptr == NULL) {
        perror("Error opening file for writing");
        return -1;
    }

    uint32_t data_size = info->num_samples * info->num_channels * (info->bits_per_sample / 8);
    
    WavHeader header;

    // RIFF Header
    header.chunk_id[0] = 'R'; header.chunk_id[1] = 'I'; header.chunk_id[2] = 'F'; header.chunk_id[3] = 'F';
    header.chunk_size = 36 + data_size;
    header.format[0] = 'W'; header.format[1] = 'A'; header.format[2] = 'V'; header.format[3] = 'E';

    // fmt Subchunk
    header.subchunk1_id[0] = 'f'; header.subchunk1_id[1] = 'm'; header.subchunk1_id[2] = 't'; header.subchunk1_id[3] = ' ';
    header.subchunk1_size = 16;
    header.audio_format = 1; // PCM
    header.num_channels = info->num_channels;
    header.sample_rate = info->sample_rate;
    header.bits_per_sample = info->bits_per_sample;
    header.byte_rate = info->sample_rate * info->num_channels * (info->bits_per_sample / 8);
    header.block_align = info->num_channels * (info->bits_per_sample / 8);

    // data Subchunk
    header.subchunk2_id[0] = 'd'; header.subchunk2_id[1] = 'a'; header.subchunk2_id[2] = 't'; header.subchunk2_id[3] = 'a';
    header.subchunk2_size = data_size;

    // Write header and data
    fwrite(&header, sizeof(WavHeader), 1, fptr);
    fwrite(samples, 1, data_size, fptr);

    fclose(fptr);
    return 0;
}
