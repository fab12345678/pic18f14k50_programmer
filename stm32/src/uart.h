#include <stdint.h>
#include "stm32f30x_conf.h"
#ifndef UART_H
#define UART_H
// initialize everything
int uart_setup();

// blocking send
int uart_send(uint8_t b);

uint8_t uart_size();

// blocking send of a string. Does not send the terminating Null byte
int uart_send_string(char *src);

int uart_send_block(uint8_t *scr, uint32_t len);

//Non Blocking receive byte 
// return -1 if no byte is availble
int uart_recv();
// blocking receive byte
uint8_t uart_recv_b();

int uart_recv_block(uint8_t *buf, uint8_t len);

//Non Blocking receive byte 
// return -1 if no byte is availble, or \n is not found in receive string
int uart_recv_string(char *buf);

#endif