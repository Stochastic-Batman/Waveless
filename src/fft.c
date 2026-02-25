#include <math.h>
#include "fft.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


Complex complex_add(Complex a, Complex b) {
    return (Complex){a.re + b.re, a.im + b.im};
}

Complex complex_sub(Complex a, Complex b) {
    return (Complex){a.re - b.re, a.im - b.im};
}

Complex complex_mul(Complex a, Complex b) {
    return (Complex){a.re * b.re - a.im * b.im, a.re * b.im + a.im * b.re};
}

// Using iterative Radix-2 FFT instead of recursion
void fft(Complex* x, int n) {
    // Bit-Reversal Permutation
    // Reorder the input array to match the recursive even/odd decomposition
    int j = 0;
    for (int i = 0; i < n; i++) {
        if (i < j) {
            Complex tmp = x[i];
            x[i] = x[j];
            x[j] = tmp;
        }

        int m = n >> 1;
        while (m >= 1 && j >= m) {
            j -= m;
            m >>= 1;
        }
        j += m;
    }

    // The Butterfly Stages
    // Loop through stages (blocks of size 2, 4, 8... up to n)
    for (int len = 2; len <= n; len <<= 1) {
        double angle = -2.0 * M_PI / len;
        Complex w_step = {cos(angle), sin(angle)};  // e^(-i * 2 * PI / len)

        for (int i = 0; i < n; i += len) {
            Complex w = {1.0, 0.0};  // W^0 = 1;
            int half = len >> 1;
            
            // Combine pairs within the block (Butterfly)
            for (int k = 0; k < half; k++) {
                int even_idx = i + k;
                int odd_idx = i + k + half;

                Complex wok = complex_mul(w, x[odd_idx]);
                Complex ek = x[even_idx];

                // Butterfly: 
                // X[k] = E[k] + W * O[k]
                // X[k + N/2] = E[k] - W * O[k]
                x[even_idx] = complex_add(ek, wok);
                x[odd_idx] = complex_sub(ek, wok);

                w = complex_mul(w, w_step);
            }
        }
    }
}

void ifft(Complex* x, int n) {
    // Bit-Reversal (Same as above, but I was lazy, so I copy-pasted instead of factoring it out)
    int j = 0;
    for (int i = 0; i < n; i++) {
        if (i < j) {
            Complex tmp = x[i];
            x[i] = x[j];
            x[j] = tmp;
        }
        int m = n >> 1;
        while (m >= 1 && j >= m) {
            j -= m;
            m >>= 1;
        }
        j += m;
    }

    // Butterfly Stages
    for (int len = 2; len <= n; len <<= 1) {
        double angle = 2.0 * M_PI / len;  // yes, no negative sign
        Complex w_step = {cos(angle), sin(angle)};

        for (int i = 0; i < n; i += len) {
            Complex w = {1.0, 0.0};
            int half = len >> 1;
            
            for (int k = 0; k < half; k++) {
                int even_idx = i + k;
                int odd_idx = i + k + half;

                Complex wok = complex_mul(w, x[odd_idx]);
                Complex ek = x[even_idx];

                x[even_idx] = complex_add(ek, wok);
                x[odd_idx] = complex_sub(ek, wok);

                w = complex_mul(w, w_step);
            }
        }
    }

    // Scaling
    for (int i = 0; i < n; i++) {
        x[i].re /= n;
        x[i].im /= n;
    }
}

