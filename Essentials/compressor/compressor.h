#ifndef ESSENTIALS_COMPRESSOR_H
#define ESSENTIALS_COMPRESSOR_H
#define MAX_SYMBOLS 256
#define MAX_CODE_LENGTH 64

typedef struct {
    unsigned char ch;
    int freq;
    char code[MAX_CODE_LENGTH];
} Symbol;

typedef struct {
    FILE *file;
    unsigned char buffer;
    int bitCount;
} BitWriter;

BitWriter *createBitWriter(const char *filename);
void writeBit(BitWriter *bw, int bit);
void writeBits(BitWriter *bw, const char *bits);
void flushBitWriter(BitWriter *bw);
int compareFreq(const void *a, const void *b);
void buildCodes(Symbol *symbols, int start, int end);
long fileSize(const char *filename);
void compressSHF(const char *inputFile, const char *outputFile);

#endif //ESSENTIALS_COMPRESSOR_H