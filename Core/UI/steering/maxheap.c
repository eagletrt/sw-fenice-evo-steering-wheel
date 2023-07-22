#include "maxheap.h"

/*
#define TEST_HEAPTREE_SIZE 10
int TEST_HEAPTREE[TEST_HEAPTREE_SIZE] = {249, 185, 381, 366, 184, 203, 72, 72,
280};

int test() {
  for (int i = 0; i < TEST_HEAPTREE_SIZE; ++i) {
    printf("%d ", TEST_HEAPTREE[i]);
  }
  printf("\n");

  heap_sort(TEST_HEAPTREE, TEST_HEAPTREE_SIZE);

  for (int i = 0; i < TEST_HEAPTREE_SIZE; ++i) {
    printf("%d ", TEST_HEAPTREE[i]);
  }
  printf("\n");
  return 0;
}
*/

int parent_index(int i) { return (i - 1) / 2; }

int left_child_index(int i) { return 2 * i + 1; }

int right_child_index(int i) { return 2 * i + 2; }

void swap(int A[], int a, int b) {
  int tmp = A[a];
  A[a] = A[b];
  A[b] = tmp;
}

void max_heap_restore(int heaptree[], int i, int dim) {
  int max = i;
  int l = left_child_index(i);
  int r = right_child_index(i);
  if (l <= dim && heaptree[l] > heaptree[max]) {
    max = l;
  }
  if (r <= dim && heaptree[r] > heaptree[max]) {
    max = r;
  }
  if (i != max) {
    swap(heaptree, i, max);
    max_heap_restore(heaptree, max, dim);
  }
}

void heap_build(int heaptree[], int ARRAYSIZE) {
  for (int i = ARRAYSIZE / 2; i >= 0; --i) {
    max_heap_restore(heaptree, i, ARRAYSIZE);
  }
}

void heap_sort(int array[], int ARRAYSIZE) {
  heap_build(array, ARRAYSIZE);
  for (int i = ARRAYSIZE; i >= 2; --i) {
    swap(array, 0, i - 1);
    max_heap_restore(array, 0, i - 2);
  }
}
