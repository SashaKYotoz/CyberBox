#ifndef ESSENTIALS_SORTS_H
#define ESSENTIALS_SORTS_H
#define ARRAY_LENGTH(arr) (sizeof(arr) / sizeof((arr)[0]))

void swap(int *a, int *b);
void output(int *array, int length);
void quick_sort(int *array, int low, int high);
void selection_sort(int *array, int length);
void bubble_sort(int *array, int length);
void merge_sort(int *array, int left, int right);
#endif //ESSENTIALS_SORTS_H