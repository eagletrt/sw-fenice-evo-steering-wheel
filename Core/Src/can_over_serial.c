#include "can_over_serial.h"

#if CAN_OVER_SERIAL_ENABLED == 1

uint8_t rcv_data[RCV_SIZ];
uint8_t msg_buf[BUFSIZ];
uint32_t msg_buf_size = 0;
extern bool serial_received;

void message_parser(uint8_t *msg, size_t msg_siz);
void activate_usart_it(uint8_t *dataptr, uint32_t data_size);

void can_over_serial_init(void) {
  activate_usart_it(rcv_data, RCV_SIZ);
}

void can_over_serial_routine(void) {
  if (serial_received) {
    for (size_t ic = 0; ic < RCV_SIZ; ic++) {
      msg_buf[msg_buf_size] = rcv_data[ic];
      msg_buf_size++;
      if (rcv_data[ic] == '$') {
        message_parser(msg_buf, msg_buf_size);
        msg_buf_size = 0;
        break;
      }
    }
    memset(rcv_data, 0, RCV_SIZ);
    serial_received = false;
    activate_usart_it(rcv_data, RCV_SIZ);
  }
}

#endif // CAN_OVER_SERIAL_ENABLED
