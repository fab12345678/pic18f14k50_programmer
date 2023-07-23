
#define STM32F334x8

#include "stm32f30x_conf.h"
#include "uart.h"
#include "pic18_programming.h"
#include "delay.h"


#define RESPONSECODE_OK ';'
#define RESPONSECODE_UNKNOWN_TYPE '?'
#define RESPONSECODE_CHECKSUM_WRONG '!'
#define RESPONSECODE_TIMEOUT '*'
#define RESPONSECODE_PROGRAMMING_FAILED '.'

uint32_t lin_addr_offset = 0;
void hexstr(uint8_t i, uint8_t *c);

uint8_t handle_hex_line(uint8_t type, uint16_t adr, uint8_t len, uint8_t *data);

void flash_check();
void handleInput();

int clear = 1; // pic18 needs to be cleared

int main(void){

  uart_setup();
  pgm_init();
  delay_ms(1000); // prevents bulk erase if short pulse on supply

  if(0){
    delay_us(100);
    flash_check();
    delay_us(100);
    VPP_off();
    return 0;
  }
  while(1){
    if(uart_size() > 0){
      if(clear == 1){
        VPP_on();
        VDD_on();
        delay_us(400);
        handleInput();
        clear = 0;
      } else {
        handleInput();

      }
    }
  }
  return 0;
}

void handleInput(){
  uint8_t len = uart_recv_b();
  uint16_t addr = uart_recv_b();
  addr = (addr << 8) | uart_recv_b();
  uint8_t type = uart_recv_b();
  uint8_t data[len];
  uint8_t sum = 0;

  if(type == 0 && clear == 1) bulk_erase();

  for(int i = 0; i < len; i++){
    data[i]= uart_recv_b();
    sum += data[i];
  }
  uint8_t checksum = uart_recv_b();
  sum += 0xFF & (len + (0xFF & (addr >> 8)) + (0xFF & addr) + type + checksum);
  if(sum == 0){
    // handle hex line
    uint8_t b = handle_hex_line(type, addr, len, data); 
    uart_send(b);
    return;
  } else {
    uart_send(RESPONSECODE_CHECKSUM_WRONG);
    return;
  }
}

                                     

void flash_check(){
  uint8_t data[20] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16, 17, 18,19,20};
  uint8_t buf[4] = {0x12, 0x44, ' ', 0};
  //pgm_flash(data, 0x2, 20);
  delay_us(100);
  // pgm_flash(data, 0x3FFA, 6);
  // delay_us(100);
  //delay_us(100);
  pgm_read(data, 0x300000, 16);
  for(int i = 0; i < 16; i++){
    hexstr(data[i], buf);
    uart_send_string((char *) buf);
  }
  uart_send('\n');
  pgm_read(data, 0x3FF0, 16);
  for(int i = 0; i < 16; i++){
    hexstr(data[i], buf);
    uart_send_string((char *) buf);
  }
  uart_send('\n');


}

uint8_t handle_hex_line(uint8_t type, uint16_t addr, uint8_t len, uint8_t *data){
  switch (type){
    case 0: // write to flash
      // add the differnce between pgm_flash and config flash
      if((lin_addr_offset<<16) + addr >= 0x300000){
        for(int i = 0; i<len;i +=1){
          config_byte_program(data[i], (lin_addr_offset <<8) + addr + i);
        }
      } else {
        pgm_flash(data, (lin_addr_offset<<16) + addr, len);
        // int s1 = 0, s2 = 0;
        // for(int i = 0; i < len; i++) s1 += data[i];
        // pgm_read(data, (lin_addr_offset<<16) + addr, len);
        // for(int i = 0; i < len; i++) s2 += data[i];
        // if(s1 != s2) return RESPONSECODE_PROGRAMMING_FAILED;
      }
    break;
    case 1: // end of file
      VPP_off();
      VDD_off();
      delay_ms(1000);
      VDD_on();
      lin_addr_offset = 0;
      clear = 1;
      // read and check
      //flash_check();
    break;
    //case 2: // extended segment address
    //case 3: // start segment address record
    case 4: // extended linear address
      if(len != 2) return RESPONSECODE_UNKNOWN_TYPE;
      lin_addr_offset = (((uint32_t) data[1])<<8) | data[0];
    break;
    case 5: // added by me, verification
      {
        // Something does not work, so skip configuration bits for now
        if(((lin_addr_offset<<16) + addr) > 0x3FFF) return RESPONSECODE_OK;
        uint8_t d_len = data[0];
        uint8_t checksum = data[1];
        uint8_t data[d_len];
        for(int i = 0; i < d_len; i++) data[i] = 0x11;
        uint8_t sum = 0;
        pgm_read(data, (lin_addr_offset<<16) + addr, d_len);
        for(int i = 0; i < d_len; i++) sum += data[i];
        if(sum != checksum){
          return RESPONSECODE_PROGRAMMING_FAILED;
        }
      }
    break;
    default:
      return RESPONSECODE_UNKNOWN_TYPE;
  }
  return RESPONSECODE_OK;
}
/*
-- bulk erase 













*/

















void hexstr(uint8_t i, uint8_t *c){
  uint8_t a = i / 16;
  uint8_t b = i % 16;
  if(a < 10) c[0] = a + '0';
  else c[0] = 'a' + a - 10;
  if(b < 10) c[1] = b + '0';
  else c[1] = 'a' + b - 10;
}
