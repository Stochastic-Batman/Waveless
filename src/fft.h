#ifndef FFT_H
#define FFT_H

typedef struct {
    double re;
    double im;
} Complex;

Complex complex_add(Complex a, Complex b);
Complex complex_sub(Complex a, Complex b);
Complex complex_mul(Complex a, Complex b);

void fft(Complex* x, int n);
void ifft(Complex* x, int n);

#endif
