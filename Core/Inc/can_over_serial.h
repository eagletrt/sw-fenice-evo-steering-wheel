#ifndef SERIAL_TO_CAN_H
#define SERIAL_TO_CAN_H

#include "steering_config.h"

#include <stdio.h>
#include <string.h>

#define RCV_SIZ 23

void can_over_serial_init(void);
void can_over_serial_routine(void);

#endif // SERIAL_TO_CAN_H
