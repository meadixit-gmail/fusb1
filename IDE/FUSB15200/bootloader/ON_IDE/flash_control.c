#include "flash_control.h"

#define FLASH_KEY 0xABCD1234
#define NVR_KEY   0xCDEF5678

#define FLASH_PROGRAM     0x1
#define FLASH_ERASE       0x2
#define FLASH_PROT        0x4
#define FLASH_SECTOR_SIZE 0x200

void flash_init(void)
{
    Flash_Control->FLASH_KEY_MAIN = FLASH_KEY;
}

uint8_t flash_erase(uint32_t start_address, uint32_t end_address)
{
    for (; start_address < end_address;)
    {
        /* Erase */
        Flash_Control->FLASH_ADDR = start_address;
        Flash_Control->FLASH_CTL  = FLASH_ERASE;
        while (Flash_Control->FLASH_INT_STS == 0)
        {
        }
        if (Flash_Control->FLASH_INT_STS & FLASH_PROT)
        {
            Flash_Control->FLASH_INT_STS &= FLASH_PROT;
            return 1;
        }
        else
        {
            Flash_Control->FLASH_INT_STS &= FLASH_ERASE;
        }

        start_address += FLASH_SECTOR_SIZE;
    }

    return 0;
}

uint8_t flash_program(uint8_t *data, uint32_t address, uint32_t len)
{
    uint16_t i;

    for (i = 0; i < len; i += 4)
    {
        /* Program */
        Flash_Control->FLASH_ADDR = address;
        Flash_Control->FLASH_DATA =
            data[i] | (data[i + 1] << 8) | (data[i + 2] << 16) | (data[i + 3] << 24);
        Flash_Control->FLASH_CTL = FLASH_PROGRAM;
        while (Flash_Control->FLASH_INT_STS == 0)
        {
        }
        if (Flash_Control->FLASH_INT_STS & FLASH_PROT)
        {
            Flash_Control->FLASH_INT_STS &= FLASH_PROT;
            return 1;
        }
        else
        {
            Flash_Control->FLASH_INT_STS &= FLASH_PROGRAM;
        }

        address += 4;
    }

    return 0;
}

void nvr_init(void)
{
    Flash_Control->FLASH_KEY_NVR = NVR_KEY;
}

uint8_t nvr_erase(uint32_t start_address)
{
    /* Erase */
    Flash_Control->FLASH_ADDR    = start_address;
    Flash_Control->FLASH_NVR_CTL = FLASH_ERASE;
    while (Flash_Control)
    {
    }
    if (Flash_Control->FLASH_INT_STS & FLASH_PROT)
    {
        Flash_Control->FLASH_INT_STS &= FLASH_PROT;
        return 1;
    }
    else
    {
        Flash_Control->FLASH_INT_STS &= FLASH_ERASE;
    }

    return 0;
}

uint8_t nvr_program(uint32_t *data, uint32_t address, uint32_t len)
{
    uint16_t i;

    for (i = 0; i < len; i++)
    {
        /* Program */
        Flash_Control->FLASH_ADDR    = address;
        Flash_Control->FLASH_DATA    = data[i];
        Flash_Control->FLASH_NVR_CTL = FLASH_PROGRAM;
        while (Flash_Control->FLASH_INT_STS == 0)
        {
        }
        if (Flash_Control->FLASH_INT_STS & FLASH_PROT)
        {
            Flash_Control->FLASH_INT_STS &= FLASH_PROT;
            return 1;
        }
        else
        {
            Flash_Control->FLASH_INT_STS &= FLASH_PROGRAM;
        }

        address++;
    }

    return 0;
}
