# Waveless
Lossy Audio Compressor (FFT)

Minimalist, dependency-free (standard C libraries only) side project.  The goal of this project is to create an encoder/decoder from scratch. By transforming raw audio into the frequency domain, discarding inaudible or low-energy data, and packing what's left into a custom binary format, I wanted to understand exactly how lossy compression works under the hood.

## How It Works

Waveless operates in a multi-step pipeline, compressing the audio footprint while minimizing "robotic" artifacts upon playback.

1. **WAV Parsing:** Reads raw 16-bit PCM data from a standard `.wav` file, stripping the 44-byte RIFF header.
2. **Framing & Windowing:** Chunks the audio into discrete blocks (e.g., 1024 samples) and applies a windowing function (a Hann window) to prevent spectral leakage and edge clicking.
3. **Forward FFT:** Uses a custom Cooley-Tukey Radix-2 Fast Fourier Transform to convert the time-domain audio frame into the frequency domain.
4. **Psychoacoustic Thresholding:** Evaluates the magnitude of each frequency bin. Bins representing frequencies above human hearing thresholds, or bins with energies falling below a dynamic percentage of the frame's peak magnitude, are zeroed out.
5. **Packing to `.wvl`:** The surviving data is serialized into my custom `.wvl` binary format, using Run-Length Encoding (RLE) to efficiently store the massive blocks of zeroes.
6. **Reconstruction via IFFT:** To play the audio back, Waveless unpacks the `.wvl` file, runs an Inverse FFT, and stitches the audio frames back together using the **Overlap-Add (OLA)** method to ensure continuous, artifact-free waveforms.

## Project Structure

```text
waveless/
├── data/               # Input .wav and output .wvl files
├── Makefile
├── README.md
└── src/
    ├── compress.h/c    # Thresholding & psychoacoustic logic
    ├── fft.h/c         # Complex math & Cooley-Tukey FFT/IFFT implementation
    ├── main.c          # CLI & pipeline logic
    ├── wav.h/c         # WAV header parsing & PCM read/write operations
    └── wvl_format.h    # Custom .wvl serialization
```

## Usage

### Prerequisites

You only need a standard C compiler (like `gcc` or `clang`) and `make`.

### Building

Clone the repository and run `make`:

```bash
git clone https://github.com/Stochastic-Batman/waveless.git
cd waveless
make
```

### Running

Encode a standard `.wav` file into the custom `.wvl` format:

```bash
./waveless -e data/input.wav data/compressed.wvl
```

Decode the `.wvl` back to `.wav` to hear the results:

```bash
./waveless -d data/compressed.wvl data/reconstructed.wav
```

### Debug Mode

Pass the `-c` flag to export the magnitude arrays of specific frames as `.csv` files. This is useful for visualizing the frequency bins and seeing exactly what the thresholding algorithm is keeping versus discarding!
