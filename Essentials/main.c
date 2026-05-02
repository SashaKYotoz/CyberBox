#include "sorts/sorts.h"

int main(void) {
    int array[10] = {1, -2, 11, 4, 864, 6, 7, 85, 9, 20};
    selection_sort(array, ARRAY_LENGTH(array));
    output(array, ARRAY_LENGTH(array));
    return 0;
}