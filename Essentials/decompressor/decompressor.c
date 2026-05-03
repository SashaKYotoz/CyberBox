#include <stdio.h>
#include <stdlib.h>
#include "../compressor/compressor.h"
#include "../string/string.h"

long fileSize(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) return 0;
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fclose(file);
    return size;
}

void decompressSHF(const char *inputFile, const char *outputFile) {
    FILE *in = fopen(inputFile, "rb");
    if (!in) {
        perror("Couldn't open compressed input file");
        return;
    }

    int count;
    if (fread(&count, sizeof(int), 1, in) != 1) {
        fprintf(stderr, "Error reading symbol count. File might be empty or corrupted.\n");
        fclose(in);
        return;
    }

    Symbol symbols[MAX_SYMBOLS];

    for (int i = 0; i < count; ++i) {
        symbols[i].ch = (unsigned char)fgetc(in);

        unsigned char len = (unsigned char)fgetc(in);
        fread(symbols[i].code, 1, len, in);
        symbols[i].code[len] = '\0';
    }

    int totalBytes;
    if (fread(&totalBytes, sizeof(int), 1, in) != 1) {
        fprintf(stderr, "Error reading total bytes.\n");
        fclose(in);
        return;
    }

    FILE *out = fopen(outputFile, "wb");
    if (!out) {
        perror("Couldn't open output file for decompression");
        fclose(in);
        return;
    }

    unsigned char buffer = 0;
    int bitCount = 0;
    int bytesWritten = 0;

    char currentCode[MAX_CODE_LENGTH + 1] = {0};
    int codeLen = 0;

    while (bytesWritten < totalBytes) {
        if (bitCount == 0) {
            int c = fgetc(in);
            if (c == EOF) {
                fprintf(stderr, "\nWarning: Reached EOF before totalBytes were decompressed.\n");
                break;
            }
            buffer = (unsigned char)c;
            bitCount = 8;
        }

        int bit = buffer >> 7 & 1;
        buffer <<= 1;
        bitCount--;
        currentCode[codeLen++] = bit ? '1' : '0';
        currentCode[codeLen] = '\0';

        for (int i = 0; i < count; ++i) {
            if (c_strcmp(currentCode, symbols[i].code) == 0) {
                fputc(symbols[i].ch, out);
                bytesWritten++;

                codeLen = 0;
                currentCode[0] = '\0';
                break;
            }
        }
    }

    fclose(in);
    fclose(out);
}

int main() {
    const char *compressed = "output.txt";
    const char *decompressed = "restored.txt";

    printf("Decompressing %s to %s\n", compressed, decompressed);
    decompressSHF(compressed, decompressed);

    const long decompressedSize = fileSize(decompressed);
    if(decompressedSize > 0)
        printf("Success! Restored file size: %ld bytes\n", decompressedSize);
    else
        printf("Decompression failed or output is empty.\n");

    return 0;
}