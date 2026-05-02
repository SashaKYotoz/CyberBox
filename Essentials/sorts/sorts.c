#include "sorts.h"

#include <stdio.h>

void swap(int *a, int *b) {
    const int tmp = *a;
    *a = *b;
    *b = tmp;
}

//quick sort
int partition(int *array, int low, int high) {
    int pivot = array[low];
    int i = low;
    int j = high;
    while (i <= j) {
        while (array[i] <= pivot && i <= high - 1)
            i++;
        while (array[j] > pivot && j >= low)
            j--;
        if (i < j)
            swap(&array[i], &array[j]);
    }
    swap(&array[low], &array[j]);
    return j;
}

void quick_sort(int *array, int low, int high) {
    if (low < high) {
        int pivot = partition(array, low, high);
        quick_sort(array, low, pivot - 1);
        quick_sort(array, pivot + 1, high);
    }
}

//selection sort
void selection_sort(int *array, int length) {
    int left = 0, right = length - 1;
    while (left <= right) {
        int min = left;
        int max = left;
        for (int i = left + 1; i < right; i++) {
            if (array[i] < array[min])
                min = i;
            if (array[i] > array[max])
                max = i;
        }
        swap(&array[left], &array[min]);
        max = max == left ? min : max;
        swap(&array[right], &array[max]);
        right--;
        left++;
    }
}

//bubble sort
void bubble_sort(int *array, int length) {
    for (int i = 0; i < length; i++) {
        for (int j = 0; j < length - i - 1; j++) {
            if (array[j] > array[j + 1])
                swap(&array[j], &array[j + 1]);
        }
    }
}

//merge sort
void merge(int *array, int left, int mid, int right) {
    int i, j, k;
    int n1 = mid - left + 1;
    int n2 = right - mid;
    int L[n1], R[n2];

    for (i = 0; i < n1; i++)
        L[i] = array[left + i];
    for (j = 0; j < n2; j++)
        R[j] = array[mid + 1 + j];
    i = 0;
    j = 0;
    k = left;
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            array[k] = L[i];
            i++;
        } else {
            array[k] = R[j];
            j++;
        }
        k++;
    }
    while (i < n1) {
        array[k] = L[i];
        i++;
        k++;
    }
    while (j < n2) {
        array[k] = R[j];
        j++;
        k++;
    }
}

void merge_sort(int *array, int left, int right) {
    if (left < right) {
        const int mid = left + (right - left) / 2;
        merge_sort(array, left, mid);
        merge_sort(array, mid + 1, right);

        merge(array, left, mid, right);
    }
}

//output
void output(int *array, int length) {
    for (int i = 0; i < length; i++)
        printf("%d\n", array[i]);
}