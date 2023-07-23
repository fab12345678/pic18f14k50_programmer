#include "pic18_programming.h"
#include "stm32f30x_conf.h"
#include "delay.h"

#define PGD 5
#define PGC 6
#define VPP 9
#define VDD 8

#define PGD_write(val) (GPIOA->ODR = (GPIOA->ODR & ~((1^(val))<<PGD)) | ((val) << PGD))
#define PGC_write(val) (GPIOA->ODR = (GPIOA->ODR & ~((1^(val))<<PGC)) | ((val) << PGC))
#define VPP_write(val) (GPIOB->ODR = (GPIOB->ODR & ~((1^(val))<<VPP)) | ((val) << VPP))
#define VDD_write(val) (GPIOA->ODR = (GPIOA->ODR & ~((1^(val))<<VDD)) | ((val) << VDD))

#define CLK_DELAY 2 
int8_t VDD_on(){
  VDD_write(1);
  return 0;
}
int8_t VDD_off(){
  VDD_write(0);
  return 0;
}

int8_t VPP_on(){
  VPP_write(1);
  return 0;
}
int8_t VPP_off(){
  VPP_write(0);
  return 0;
}
int8_t bulk_erase(){
  // move 3c to tblptru
  spi_write(INSTR_CORE, (MOVLW<<8) | 0x3C);
  spi_write(INSTR_CORE, (MOVWF<<8) | 0xF8);
  // move 00 to tblptrh
  spi_write(INSTR_CORE, (MOVLW<<8) | 0x00);
  spi_write(INSTR_CORE, (MOVWF<<8) | 0xF7);
  // move 00 to tblptrh
  spi_write(INSTR_CORE, (MOVLW<<8) | 0x05);
  spi_write(INSTR_CORE, (MOVWF<<8) | 0xF6);
  spi_write(INSTR_TABLE_WR, 0x0F0F); // why twice

  // move 3c to tblptru
  spi_write(INSTR_CORE, (MOVLW<<8) | 0x3C);
  spi_write(INSTR_CORE, (MOVWF<<8) | 0xF8);
  // move 00 to tblptrh
  spi_write(INSTR_CORE, (MOVLW<<8) | 0x00);
  spi_write(INSTR_CORE, (MOVWF<<8) | 0xF7);
  // move 00 to tblptrh
  spi_write(INSTR_CORE, (MOVLW<<8) | 0x04);
  spi_write(INSTR_CORE, (MOVWF<<8) | 0xF6);
  spi_write(INSTR_TABLE_WR, 0x8F8F); // why twice
  spi_write(INSTR_CORE, NOP);
  spi_write(INSTR_CORE, NOP);
  // wait until rease completes ~ 10 ms

  return 0;
}
int8_t config_byte_program(uint8_t byte, uint32_t addr){
  uint8_t addru = (addr >> 16) & 0xFF;
  uint8_t addrh = (addr >> 8) & 0xFF;
  uint8_t addrl = (addr) & 0xFF;
  // 1) enable access to flash 
  spi_write(INSTR_CORE, (0x8EA6)); // BSF EECON1, EEPGD
  spi_write(INSTR_CORE, (0x9CA6)); // BCF EECON1, CFGS
  spi_write(INSTR_CORE, (0x84A6)); // BSF EECON1, WREN

  // move addru to tblptru
  spi_write(INSTR_CORE, (MOVLW<<8) | addru);
  spi_write(INSTR_CORE, (MOVWF<<8) | 0xF8);
  // move addrh to tblptrh
  spi_write(INSTR_CORE, (MOVLW<<8) | addrh);
  spi_write(INSTR_CORE, (MOVWF<<8) | 0xF7);
  // move addrl to tblptrh
  spi_write(INSTR_CORE, (MOVLW<<8) | addrl);
  spi_write(INSTR_CORE, (MOVWF<<8) | 0xF6);
  
  spi_write(INSTR_TABLE_WR_SP, ((uint32_t) byte << 8) | byte);
  delay_us(100);
  spi_write_pgm_delay(INSTR_CORE, 0x0000);
  delay_us(100);
  return 0;
}
int8_t pgm_flash(uint8_t *data, uint32_t addr, uint32_t len){
  // 1) enable access to flash 
  spi_write(INSTR_CORE, (0x8EA6)); // BSF EECON1, EEPGD
  spi_write(INSTR_CORE, (0x9CA6)); // BCF EECON1, CFGS
  spi_write(INSTR_CORE, (0x84A6)); // BSF EECON1, WREN
  uint32_t len_rem = len;
  uint8_t set_tblptr = 1;
  while(len_rem > 0){

    if(set_tblptr){
      uint8_t addru = (addr >> 16) & 0xFF;
      uint8_t addrh = (addr >> 8) & 0xFF;
      uint8_t addrl = (addr) & 0xFF;
      // 2) point to row and write
      // move addru to tblptru
      spi_write(INSTR_CORE, (MOVLW<<8) | addru);
      spi_write(INSTR_CORE, (MOVWF<<8) | 0xF8);
      // move addrh to tblptrh
      spi_write(INSTR_CORE, (MOVLW<<8) | addrh);
      spi_write(INSTR_CORE, (MOVWF<<8) | 0xF7);
      // move addrl to tblptrh
      spi_write(INSTR_CORE, (MOVLW<<8) | addrl);
      spi_write(INSTR_CORE, (MOVWF<<8) | 0xF6);
      set_tblptr = 0;
    }
    if(len_rem == 1){
      spi_write(INSTR_TABLE_WR_SP, 0xFF00 | data[0]);
      delay_us(100);
      spi_write_pgm_delay(INSTR_CORE, 0x0000);
      delay_us(100);
      len_rem -= 1;
      data += 1;
      addr += 1;
    } else if(len_rem == 2){ // these are the last 2 bytes, start programming
      spi_write(INSTR_TABLE_WR_SP, (((uint32_t) data[1])<<8) | data[0]);
      delay_us(100);
      spi_write_pgm_delay(INSTR_CORE, 0x0000);
      delay_us(100);
      len_rem -= 2;
      data += 2;
      addr += 2;
    } else if((addr+1) % 16 == 0) { // this is the last address of the block
      spi_write(INSTR_TABLE_WR_SP, 0xFF00 | data[0]);
      delay_us(100);
      spi_write_pgm_delay(INSTR_CORE, 0x0000);
      delay_us(100);
      set_tblptr = 1;
      len_rem -= 1;
      data += 1;
      addr += 1;
    } else if((addr+2) % 16 == 0) { // this is the last address of the block
      spi_write(INSTR_TABLE_WR_SP, (((uint32_t) data[1])<<8) | data[0]);
      delay_us(100);
      spi_write_pgm_delay(INSTR_CORE, 0x0000);
      delay_us(100);
      set_tblptr = 1;
      len_rem -= 2;
      data += 2;
      addr += 2;
    } else { // nothing special, we can write to mem
      spi_write(INSTR_TABLE_WR_POST_INC, ((uint32_t) data[1]<<8) | data[0]);
      len_rem -= 2;
      data += 2;
      addr += 2;
    }
  }
  return 0;
}

uint8_t PGD_pinMode(uint8_t mode){
  GPIO_InitTypeDef GPIO_InitStructure; 
  GPIO_InitStructure.GPIO_Mode = mode;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  return 0; 
}

uint8_t pgm_init(){
  // initializes software spi pins
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  GPIO_InitTypeDef GPIO_InitStructure; 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_8;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  GPIOA->ODR &= ~(1<< PGC | 1<<PGD| 1<<VDD);  // all pints low
  GPIOB->ODR &= ~(1<<VPP);  // all pints low
  // GPIOB->ODR |= (1<<VPP);  // all pints low
  // GPIOB->ODR &= ~(1<<VPP);  // all pints low
  return 0;
}

uint32_t spi_write_pgm_delay(uint8_t cmd, uint16_t payload){
  // first 3 iterations
  for(int i = 0; i < 3; i++){
    PGD_write((cmd>>i) & 1);
    PGC_write(1);
    delay_us(CLK_DELAY);
    PGC_write(0);
    delay_us(CLK_DELAY);
  }
  // 4th iteration
  PGD_write((cmd>>3) & 1);
  PGC_write(1);
  // hold pgc high now
  delay_ms(5); // PA9
  PGC_write(0);
  delay_us(CLK_DELAY);
  delay_us(100); // PA10
  for(int i = 0; i < 16; i++){
    PGD_write((payload>>i) & 1);
    PGC_write(1);
    delay_us(CLK_DELAY);
    PGC_write(0);
    delay_us(CLK_DELAY);
  }
  return 0;
}


uint32_t spi_write(uint8_t cmd, uint16_t payload){
  for(int i = 0; i < 4; i++){
    PGD_write((cmd>>i) & 1);
    PGC_write(1);
    delay_us(CLK_DELAY);
    PGC_write(0);
    delay_us(CLK_DELAY);
  }
  PGD_write(0);
  delay_us(1); // PA5, at least 40 ns
  for(int i = 0; i < 16; i++){
    PGD_write((payload>>i) & 1);
    PGC_write(1);
    delay_us(CLK_DELAY);
    PGC_write(0);
    delay_us(CLK_DELAY);
  }
  return 0;
}

int8_t pgm_read(uint8_t *data, uint32_t addr, uint32_t len){
  uint8_t addru = (addr >> 16) & 0xFF;
  uint8_t addrh = (addr >> 8) & 0xFF;
  uint8_t addrl = (addr) & 0xFF;
  // move addru to tblptru
  spi_write(INSTR_CORE, (MOVLW<<8) | addru);
  spi_write(INSTR_CORE, (MOVWF<<8) | 0xF8);
  // move addrh to tblptrh
  spi_write(INSTR_CORE, (MOVLW<<8) | addrh);
  spi_write(INSTR_CORE, (MOVWF<<8) | 0xF7);
  // move addrl to tblptrh
  spi_write(INSTR_CORE, (MOVLW<<8) | addrl);
  spi_write(INSTR_CORE, (MOVWF<<8) | 0xF6);
  delay_us(CLK_DELAY * 4);
  for(int i = 0; i < len; i++){
    data[i] = spi_read(INSTR_TABLE_RD_POST_INC);
    delay_us(CLK_DELAY * 4);
  }
  return 0;
}
uint8_t spi_read(uint8_t cmd){
  for(int i = 0; i < 4; i++){
    uint8_t d = (cmd >>i) & 1;
    PGD_write(d);
    PGC_write(1);
    delay_us(CLK_DELAY);
    PGC_write(0);
    delay_us(CLK_DELAY);
  }
  PGD_write(0);
  delay_us(10); // PA5, at least 40 ns
  for(int i = 0; i < 8; i++){
    PGC_write(1);
    delay_us(CLK_DELAY);
    PGC_write(0);
    delay_us(CLK_DELAY);
  }
  delay_us(1); // P6: 20 ns delay
  //configure PGD as input
  uint8_t res = 0;
  PGD_pinMode(GPIO_Mode_IN);
  for(int i = 0; i < 8; i++){
    PGC_write(1);
    delay_us(CLK_DELAY);
    res |= ((1&(GPIOA->IDR >> PGD))<<i);
    PGC_write(0);
    // read here
    delay_us(CLK_DELAY);
  } 
  PGD_pinMode(GPIO_Mode_OUT);

  return res;
}

