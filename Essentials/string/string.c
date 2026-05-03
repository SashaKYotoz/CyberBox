#include "string.h"

// c_ <- custom

int c_strlen(const char *str) {
    int counter;

    for (counter = 0; str[counter]; counter++) {}
    return counter;
}

char *c_strcpy(char *destination, char *source) {
    char *res;

    for (res = destination; (*destination++ = *source++);) {}
    return res;
}

char *c_strcat(char *destination, char *source) {
    char *res;

    for (res = destination; *destination++;) {}
    for (--destination; (*destination++ = *source++);) {}
    return res;
}

int c_strcmp(const char *left, const char *right) {
    for (; *left && *left == *right; left++, right++) {}
    return *left - *right;
}