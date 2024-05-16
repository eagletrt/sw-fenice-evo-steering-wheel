#include "min-heap.h"
#include "primary_network.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST_HEAP_SIZE 10

int8_t min_heap_compare_int(void *a, void *b) {
    int f = *(int *)a;
    int s = *(int *)b;
    if (f < s)
        return -1;
    return f == s ? 0 : 1;
}

int8_t min_heap_compare_indexes(void *a, void *b) {
    int f = primary_id_from_index(*(int *)a);
    int s = primary_id_from_index(*(int *)b);
    if (f < s)
        return -1;
    return f == s ? 0 : 1;
}

int main() {
    MinHeap(int, TEST_HEAP_SIZE) dalbom = min_heap_new(int, TEST_HEAP_SIZE, min_heap_compare_indexes);

    int dalbom2[TEST_HEAP_SIZE] = {
        PRIMARY_STEERING_WHEEL_JMP_TO_BLT_INDEX,
        PRIMARY_HV_FEEDBACK_MISC_VOLTAGE_INDEX,
        PRIMARY_LV_FEEDBACK_INDEX,
        PRIMARY_CONTROL_STATUS_INDEX,
        PRIMARY_ECU_STEER_ACTUATOR_CURRENT_INDEX,
        PRIMARY_NLG5_ACT_I_INDEX,
        PRIMARY_HV_FLASH_CELLBOARD_2_TX_INDEX,
        PRIMARY_HV_FLASH_CELLBOARD_3_RX_INDEX,
        PRIMARY_LV_SET_RADIATOR_SPEED_INDEX,
        PRIMARY_ECU_SET_PTT_STATUS_INDEX,
    };
    char buffer[1000];
    for (size_t i = 0; i < TEST_HEAP_SIZE; i++) {
        primary_message_name_from_id(primary_id_from_index(dalbom2[i]), buffer);
        printf("%d: \t%s\r\n", primary_id_from_index(dalbom2[i]), buffer);
    }

    printf("\r\n");
    for (size_t i = 0; i < TEST_HEAP_SIZE; i++) {
        min_heap_insert(&dalbom, &dalbom2[i]);
    }

    MinHeap(int, TEST_HEAP_SIZE) dalbom3 = min_heap_new(int, TEST_HEAP_SIZE, min_heap_compare_indexes);
    memcpy(&dalbom3, &dalbom, sizeof(MinHeap(int, TEST_HEAP_SIZE)));

    int x = 0;
    while (!min_heap_is_empty(&dalbom)) {
        min_heap_remove(&dalbom, 0, &x);
        primary_message_name_from_id(primary_id_from_index(x), buffer);
        printf("%d: \t%s\r\n", primary_id_from_index(x), buffer);
    }
    printf("\r\n");

    while (!min_heap_is_empty(&dalbom3)) {
        min_heap_remove(&dalbom3, 0, &x);
        primary_message_name_from_id(primary_id_from_index(x), buffer);
        printf("%d: \t%s\r\n", primary_id_from_index(x), buffer);
    }
    printf("\r\n");
    return 0;
}
