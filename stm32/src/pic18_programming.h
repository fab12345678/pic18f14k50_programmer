
#include <stdint.h>
#ifndef pic18_programming_H
#define pic18_programming_H


#define INSTR_CORE (0)
#define INSTR_SHIFT_TABLAT (2)
#define INSTR_TABLE_RD (8)
#define INSTR_TABLE_RD_POST_INC (9)
#define INSTR_TABLE_RD_POST_DEC (10)
#define INSTR_TABLE_RD_PRE_INC (11)
#define INSTR_TABLE_WR (12)
#define INSTR_TABLE_WR_POST_INC (13)
#define INSTR_TABLE_WR_SP_POST_INC (14)
#define INSTR_TABLE_WR_SP (15)


#define MOVLW 0x0E
#define MOVWF 0x6E
#define NOP 0x00


int8_t VPP_on();
int8_t VPP_off();
int8_t VDD_on();
int8_t VDD_off();
int8_t bulk_erase();
int8_t config_byte_program(uint8_t byte, uint32_t addr);
int8_t pgm_flash(uint8_t *data, uint32_t addr, uint32_t len);
int8_t pgm_read(uint8_t *data, uint32_t addr, uint32_t len);
uint8_t PGD_pinMode(uint8_t mode);
uint8_t pgm_init();
uint32_t spi_write_pgm_delay(uint8_t cmd, uint16_t payload);
uint32_t spi_write(uint8_t cmd, uint16_t payload);
uint8_t spi_read(uint8_t cmd);

#endif
