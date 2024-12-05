#ifndef FLASH_CONTROL_H_
#define FLASH_CONTROL_H_

#include "fusb15xxx.h"

void    flash_init(void);
uint8_t flash_erase(uint32_t start_address, uint32_t end_address);
uint8_t flash_program(uint8_t *data, uint32_t address, uint32_t len);

void    nvr_init(void);
uint8_t nvr_erase(uint32_t start_address);
uint8_t nvr_program(uint32_t *data, uint32_t address, uint32_t len);

#endif /* FLASH_CONTROL_H_ */
