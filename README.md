# WaveLess
Lossy Audio Compressor (FFT)

Minimalist, dependency-free (standard C libraries only) side project.  The goal of this project is to create an encoder/decoder from scratch. By transforming raw audio into the frequency domain, discarding inaudible or low-energy data, and packing what's left into a custom binary format, I wanted to understand exactly how lossy compression works under the hood.

## Project Structure

```text
waveless/
├── data/                 # Input .wav and output .wvl files
├── Makefile
├── README.md
└── src/
    ├── compress.h/c      # Thresholding & psychoacoustic logic
    ├── fft.h/c           # Complex math & Cooley-Tukey FFT/IFFT implementation
    ├── main.c            # CLI & pipeline logic
    ├── wav.h/c           # WAV header parsing & PCM read/write operations
    └── wvl_format.h/c    # Custom .wvl serialization
```

## Usage

### Prerequisites

You only need a standard C compiler (like `gcc` or `clang`) and `make`.

### Building

Clone the repository and run `make`:

```bash
git clone https://github.com/Stochastic-Batman/WaveLess.git
cd WaveLess
make
```

### Running

Compress a standard `.wav` file into the custom `.wvl` format:

```bash
./WaveLess -c data/input.wav data/compressed.wvl
```

Decompress the `.wvl` back to `.wav` to hear the results:

```bash
./WaveLess -d data/compressed.wvl data/reconstructed.wav
```
