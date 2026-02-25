CC = gcc
CFLAGS = -Wall -Wextra -O2 -lm
SRC = src/main.c src/compress.c src/fft.c src/wav.c src/wvl_format.c
TARGET = WaveLess

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(SRC) -o $(TARGET) $(CFLAGS)

clean:
	rm -f $(TARGET)
