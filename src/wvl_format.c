#include <stdio.h>
#include <stdlib.h>
#include "wvl_format.h"

int wvl_write(const char* path, const WvlHeader* header, WvlFrame* frames) {
    FILE* fp = fopen(path, "wb");
    if (!fp) {
        perror("Error opening file for writing");
        return -1;
    }

    fwrite(header, sizeof(WvlHeader), 1, fp);

    for (uint32_t i = 0; i < header->num_frames; i++) {
        fwrite(&frames[i].num_bins, sizeof(uint16_t), 1, fp);
        fwrite(frames[i].bins, sizeof(WvlBin), frames[i].num_bins, fp);
    }

    fclose(fp);
    return 0;
}

int wvl_read(const char* path, WvlHeader* header, WvlFrame** frames) {
    FILE* fp = fopen(path, "rb");
    if (!fp) {
        perror("Error opening file for reading");
        return -1;
    }

    if (fread(header, sizeof(WvlHeader), 1, fp) != 1) {
        fclose(fp);
        return -1;
    }

    if (header->magic != WVL_MAGIC) {
        fprintf(stderr, "Invalid WVL file magic number\n");
        fclose(fp);
        return -1;
    }

    *frames = malloc(header->num_frames * sizeof(WvlFrame));
    if (!*frames) {
        fclose(fp);
        return -1;
    }

    for (uint32_t i = 0; i < header->num_frames; i++) {
        if (fread(&(*frames)[i].num_bins, sizeof(uint16_t), 1, fp) != 1) {
            wvl_free_frames(*frames, i);
            fclose(fp);
            return -1;
        }

        (*frames)[i].bins = malloc((*frames)[i].num_bins * sizeof(WvlBin));
        if (!(*frames)[i].bins) {
            wvl_free_frames(*frames, i);
            fclose(fp);
            return -1;
        }

        if (fread((*frames)[i].bins, sizeof(WvlBin), (*frames)[i].num_bins, fp) != (*frames)[i].num_bins) {
            free((*frames)[i].bins);
            wvl_free_frames(*frames, i);
            fclose(fp);
            return -1;
        }
    }

    fclose(fp);
    return 0;
}

void wvl_free_frames(WvlFrame* frames, uint32_t num_frames) {
    if (!frames) return;
    
    for (uint32_t i = 0; i < num_frames; i++) {
        free(frames[i].bins);
    }
    free(frames);
}
