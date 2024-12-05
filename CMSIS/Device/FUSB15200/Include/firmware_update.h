#ifndef FIRMWARE_UPDATE_H_
#define FIRMWARE_UPDATE_H_

#include "fusb15xxx.h"

uint8_t firmware_boot_check(void);
void    firmware_update(void);

#endif /* FIRMWARE_UPDATE_H_ */
