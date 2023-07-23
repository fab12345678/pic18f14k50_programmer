#include "uart.h"
#include <stdint.h>
#include <string.h>
#include "stm32f30x_conf.h"

#define UART_USED USART2

uint8_t uart_recv_buf[256];
static uint8_t uart_recv_buf_start= 0;
static uint8_t uart_recv_buf_end = 0;
// initialize everything
int uart_setup(){
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    memset(&USART_InitStructure, 0, sizeof(USART_InitStructure));
    memset(&NVIC_InitStructure, 0, sizeof(USART_InitStructure));
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  if(UART_USED == USART2){
    // Set up Uart pins
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_7);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_7);
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
  }

  if(UART_USED == USART3){
    // Set up Uart pins
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_7);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_7);
    NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
  }
    // Set up Uart
    USART_InitStructure.USART_BaudRate = 115200; 
    USART_InitStructure.USART_Mode=USART_Mode_Tx | USART_Mode_Rx;  
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_HardwareFlowControl=USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_Init(UART_USED, &USART_InitStructure);
    USART_Cmd(UART_USED, ENABLE);
    // Configure Interrupt Receive
    USART_ITConfig(UART_USED, USART_IT_RXNE, ENABLE);
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_InitStructure);
    uart_recv_buf_start= 0;
    uart_recv_buf_end = 0;


  return 0;
}

// blocking send
int uart_send(uint8_t b){
  USART_SendData(UART_USED, b);
  while(USART_GetFlagStatus(UART_USED, USART_FLAG_TXE)!=SET);
  return 0;
}

uint8_t uart_size(){
  return uart_recv_buf_end - uart_recv_buf_start;
}

// blocking send of a string. Does not send the terminating Null byte
int uart_send_string(char *src){
  while(*src != 0){
    USART_SendData(UART_USED, *src);
    while(USART_GetFlagStatus(UART_USED, USART_FLAG_TXE)!=SET){};
    src ++;
  }
  return 0;
}

int uart_send_block(uint8_t *scr, uint32_t len){
  for(int i = 0; i < len; i++){
    USART_SendData(UART_USED, scr[i]);
    while(USART_GetFlagStatus(UART_USED, USART_FLAG_TXE)!=SET);
  }
  return 0;
}

//Non Blocking receive byte 
// return -1 if no byte is availble
int uart_recv(){
  if(uart_recv_buf_end == uart_recv_buf_start) return -1;
  int d = uart_recv_buf[uart_recv_buf_start];
  uart_recv_buf_start ++;
  return d;
}
// blocking byte receive, waits until a byte is available
uint8_t uart_recv_b(){
//  int r;
//  while((r = uart_recv()) == -1) ;
//  return r;
  while(uart_recv_buf_end == uart_recv_buf_start);
  uint8_t d = uart_recv_buf[uart_recv_buf_start];
  uart_recv_buf_start ++;
  return d;
}



int uart_recv_block(uint8_t *buf, uint8_t len){
  // wait until the correct number of bytes is available
  while(uart_recv_buf_end - uart_recv_buf_start < len);
  // copy into buf
  for(int i = 0; i < len; i++){
    buf[i] = uart_recv();
  }
  return 0;
}

//Non Blocking receive byte 
// return -1 if no byte is availble, or \n is not found in receive string
int uart_recv_string(char *buf){
  if(uart_recv_buf_end == uart_recv_buf_start) return -1;
  // copy in buf until \n is reached
  // return -1 if no byte if ound
  int len = 0;
  while(1){
    if(uart_recv_buf_start == uart_recv_buf_end){ // no \n found
      uart_recv_buf_start -= len;
      return -1;
    }
    char c = uart_recv_buf[uart_recv_buf_start];
    uart_recv_buf_start ++;
    buf[len] = c;
    if(c == '\n'){ // \n found
      buf[len + 1] = 0;
      return 0;
    }
    len ++;
  }


}

void IRQHandler(){
  // RXNE interrupt
  if(USART_GetITStatus(UART_USED, USART_IT_RXNE) == SET){
    int recvd = USART_ReceiveData(UART_USED);
    //uart_send(recvd);
    uart_recv_buf[uart_recv_buf_end] = recvd;
    uart_recv_buf_end += 1;
    uart_recv_buf_end &= 0xFF;
  }
}
void USART3_IRQHandler(){
  IRQHandler();
}
void USART2_IRQHandler(){
  IRQHandler();
}