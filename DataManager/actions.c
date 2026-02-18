#include <stdio.h>
#include <stdlib.h>

long find_payload_offset(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) return -1;

    unsigned char magic[8];
    if (fread(magic, 1, 8, file) < 8) {
        fclose(file);
        return -1;
    }

    int is_jpg = magic[0] == 0xFF && magic[1] == 0xD8;
    int is_png = magic[0] == 0x89 && magic[1] == 0x50 && magic[2] == 0x4E && magic[3] == 0x47;

    if (!is_jpg && !is_png) {
        fclose(file);
        return -1;
    }

    long offset = -1;
    fseek(file, 0, SEEK_SET);

    if (is_jpg) {
        int prev = 0, curr = 0;
        while ((curr = fgetc(file)) != EOF) {
            if (prev == 0xFF && curr == 0xD9) {
                offset = ftell(file);
            }
            prev = curr;
        }
    } else if (is_png) {
        unsigned char iend_marker[] = {0x49, 0x45, 0x4E, 0x44, 0xAE, 0x42, 0x60, 0x82};
        int match_index = 0;
        int curr = 0;

        while ((curr = fgetc(file)) != EOF) {
            if (curr == iend_marker[match_index]) {
                match_index++;
                if (match_index == 8) {
                    offset = ftell(file);
                    break;
                }
            } else {
                if (curr == iend_marker[0]) {
                    match_index = 1;
                } else {
                    match_index = 0;
                }
            }
        }
    }

    fclose(file);
    return offset;
}

unsigned char* read_hidden_data(const char *filename, long offset, long *size) {
    if (offset <= 0) return NULL;

    FILE *file = fopen(filename, "rb");
    if (!file) return NULL;

    fseek(file, 0, SEEK_END);
    *size = ftell(file) - offset;

    if (*size <= 0) {
        fclose(file);
        return NULL;
    }

    fseek(file, offset, SEEK_SET);

    unsigned char *buffer = malloc(*size);
    if (buffer) {
        fread(buffer, 1, *size, file);
    }

    fclose(file);
    return buffer;
}

void modify_hidden_data(const char *filename, long offset, unsigned char *new_data, long new_size) {
    if (offset <= 0) return;

    FILE *file = fopen(filename, "rb");
    if (!file) return;

    unsigned char *base_image = malloc(offset);
    if (base_image) {
        fread(base_image, 1, offset, file);
    }
    fclose(file);

    file = fopen(filename, "wb");
    if (!file) {
        if (base_image) free(base_image);
        return;
    }

    if (base_image) {
        fwrite(base_image, 1, offset, file);
        free(base_image);
    }

    if (new_data && new_size > 0) {
        fwrite(new_data, 1, new_size, file);
    }

    fclose(file);
}