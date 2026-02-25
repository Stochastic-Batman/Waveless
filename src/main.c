#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compress.h"

void print_usage(const char* program_name) {
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  Encode: %s -c <input.wav> <output.wvl> [threshold_ratio]\n", program_name);
    fprintf(stderr, "  Decode: %s -d <input.wvl> <output.wav>\n", program_name);
    fprintf(stderr, "\nOptions:\n");
    fprintf(stderr, "  threshold_ratio: 0.0 to 1.0 (default: 0.01)\n");
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        print_usage(argv[0]);
        return 1;
    }

    double threshold_ratio = 0.1;

    if (strcmp(argv[1], "-c") == 0) {
        if (argc >= 5) {
            threshold_ratio = atof(argv[4]);
            if (threshold_ratio < 0.0 || threshold_ratio > 1.0) {
                fprintf(stderr, "Error: threshold_ratio must be between 0.0 and 1.0\n");
                return 1;
            }
        }
        
        printf("Compressing: %s -> %s (threshold: %.3f)\n", argv[2], argv[3], threshold_ratio);
        
        if (compress(argv[2], argv[3], threshold_ratio) != 0) {
            fprintf(stderr, "Compression failed\n");
            return 1;
        }
        
        printf("Compression complete!\n");
        
    } else if (strcmp(argv[1], "-d") == 0) {
        printf("Decompressing: %s -> %s\n", argv[2], argv[3]);
        
        if (decompress(argv[2], argv[3]) != 0) {
            fprintf(stderr, "Decompression failed\n");
            return 1;
        }
        
        printf("Decompression complete!\n");
        
    } else {
        fprintf(stderr, "Error: Unknown flag '%s'\n", argv[1]);
        print_usage(argv[0]);
        return 1;
    }

    return 0;
}
