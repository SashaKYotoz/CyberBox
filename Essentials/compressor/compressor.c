#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compressor.h"

#define MAX_SYMBOLS 256
#define MAX_CODE_LENGTH 64


BitWriter *createBitWriter(const char *filename) {
    BitWriter *bw = malloc(sizeof(BitWriter));
    bw->file = fopen(filename, "wb");
    bw->buffer = 0;
    bw->bitCount = 0;
    return bw;
}

void writeBit(BitWriter *bw, int bit) {
    bw->buffer = (bw->buffer << 1) | (bit & 1);
    bw->bitCount++;
    if (bw->bitCount == 8) {
        fputc(bw->buffer, bw->file);
        bw->bitCount = 0;
        bw->buffer = 0;
    }
}

void writeBits(BitWriter *bw, const char *bits) {
    for (int i = 0; bits[i]; ++i)
        writeBit(bw, bits[i] - '0');
}

void flushBitWriter(BitWriter *bw) {
    if (bw->bitCount > 0) {
        bw->buffer <<= (8 - bw->bitCount);
        fputc(bw->buffer, bw->file);
    }
    fclose(bw->file);
    free(bw);
}

int compareFreq(const void *a, const void *b) {
    return ((Symbol *) b)->freq - ((Symbol *) a)->freq;
}

void buildCodes(Symbol *symbols, int start, int end) {
    if (start >= end) return;

    int total = 0;
    for (int i = start; i <= end; ++i)
        total += symbols[i].freq;

    int sum = 0, split = start;
    for (int i = start; i <= end; ++i) {
        sum += symbols[i].freq;
        if (sum >= total / 2) {
            split = i;
            break;
        }
    }

    for (int i = start; i <= split; ++i)
        strcat(symbols[i].code, "0");
    for (int i = split + 1; i <= end; ++i)
        strcat(symbols[i].code, "1");

    buildCodes(symbols, start, split);
    buildCodes(symbols, split + 1, end);
}

long fileSize(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) return 0;
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fclose(file);
    return size;
}

void calculate_compression_stats(const char *input, const char *compressed) {
    long original = fileSize(input);
    long compressedSize = fileSize(compressed);
    if (original > 0 && compressedSize > 0) {
        double ratio = 100.0 * (1.0 - (double)compressedSize / original);
        printf("Original size:   %ld bytes\n", original);
        printf("Compressed size: %ld bytes\n", compressedSize);
        printf("Compression:     %.2f%%\n", ratio);
    }
}

void compressSHF(const char *inputFile, const char *outputFile) {
    FILE *in = fopen(inputFile, "rb");
    if (!in) {
        perror("Couldn't open input file");
        return;
    }

    int freq[MAX_SYMBOLS] = {0};
    int totalBytes = 0;
    int c;

    while ((c = fgetc(in)) != EOF) {
        freq[c]++;
        totalBytes++;
    }

    Symbol symbols[MAX_SYMBOLS];
    int count = 0;
    for (int i = 0; i < MAX_SYMBOLS; i++) {
        if (freq[i] > 0) {
            symbols[count].ch = (unsigned char)i;
            symbols[count].freq = freq[i];
            symbols[count].code[0] = '\0';
            count++;
        }
    }

    qsort(symbols, count, sizeof(Symbol), compareFreq);
    buildCodes(symbols, 0, count - 1);
    rewind(in);

    BitWriter *bw = createBitWriter(outputFile);

    fwrite(&count, sizeof(int), 1, bw->file);
    for (int i = 0; i < count; ++i) {
        fputc(symbols[i].ch, bw->file);
        unsigned char len = strlen(symbols[i].code);
        fputc(len, bw->file);
        fwrite(symbols[i].code, 1, len, bw->file);
    }

    fwrite(&totalBytes, sizeof(int), 1, bw->file);

    while ((c = fgetc(in)) != EOF) {
        for (int i = 0; i < count; ++i) {
            if (symbols[i].ch == (unsigned char)c) {
                writeBits(bw, symbols[i].code);
                break;
            }
        }
    }

    flushBitWriter(bw);
    fclose(in);
}

int main() {
    const char *input = "chimeric_darkness.json";
    const char *compressed = "output.txt";

    printf("Compressing %s to %s\n", input, compressed);
    compressSHF(input, compressed);
    calculate_compression_stats(input, compressed);

    return 0;
}