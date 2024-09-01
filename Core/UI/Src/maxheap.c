#include "maxheap.h"

int parent_index(int i) {
    return (i - 1) / 2;
}

int left_child_index(int i) {
    return 2 * i + 1;
}

int right_child_index(int i) {
    return 2 * i + 2;
}

void swap(int A[], int a, int b) {
    int tmp = A[a];
    A[a]    = A[b];
    A[b]    = tmp;
}

void max_heap_restore(int heaptree[], int i, int dim, int cmp(int index)) {
    int max = i;
    int l   = left_child_index(i);
    int r   = right_child_index(i);
    if (l <= dim && cmp(heaptree[l]) > cmp(heaptree[max])) {
        max = l;
    }
    if (r <= dim && cmp(heaptree[r]) > cmp(heaptree[max])) {
        max = r;
    }
    if (i != max) {
        swap(heaptree, i, max);
        max_heap_restore(heaptree, max, dim, cmp);
    }
}

void heap_build(int heaptree[], int ARRAYSIZE, int cmp(int index)) {
    for (int i = ARRAYSIZE / 2; i >= 0; --i) {
        max_heap_restore(heaptree, i, ARRAYSIZE, cmp);
    }
}

void heap_sort(int array[], int ARRAYSIZE, int cmp(int index)) {
    heap_build(array, ARRAYSIZE, cmp);
    for (int i = ARRAYSIZE; i >= 2; --i) {
        swap(array, 0, i - 1);
        max_heap_restore(array, 0, i - 2, cmp);
    }
}
