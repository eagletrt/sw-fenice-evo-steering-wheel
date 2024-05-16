#include "cansniffer.h"

int8_t min_heap_compare_indexes(void *a, void *b) {
    int f = primary_id_from_index(*(int *)a);
    int s = primary_id_from_index(*(int *)b);
    if (f < s)
        return -1;
    return f == s ? 0 : 1;
}
