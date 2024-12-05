/**
 * @file     boot_startup_FUSB15200.c
 * @author   USB PD Firmware Team
 * @brief
 *
 * @copyright @parblock
 * Copyright &copy; 2020 ON Semiconductor &reg;. All rights reserved.
 *
 * This software and/or documentation is licensed by ON Semiconductor under
 * limited terms and conditions. The terms and conditions pertaining to the
 * software and/or documentation are available at [ONSMI_T&C.pdf]
 * (http://www.onsemi.com/site/pdf/ONSEMI_T&C.pdf)
 *
 * ("ON Semiconductor Standard Terms and Conditions of Sale,
 *   Section 8 Software").
 *
 * DO NOT USE THIS SOFTWARE AND/OR DOCUMENTATION UNLESS YOU HAVE CAREFULLY
 * READ AND YOU AGREE TO THE LIMITED TERMS AND CONDITIONS. BY USING THIS
 * SOFTWARE AND/OR DOCUMENTATION, YOU AGREE TO THE LIMITED TERMS AND CONDITIONS.
 * @endparblock
 *
 */

#if 1
#include "FUSB15200.h"
#include "firmware_update.h"

/*----------------------------------------------------------------------------
  Linker generated Symbols
 *----------------------------------------------------------------------------*/
extern uint32_t __StackTop;
extern uint32_t __ApplicationStart;

#define VTABLE_BASE (uint32_t) & __ApplicationStart
#define VECT_ISR(x) ((void (*)(void))(*(uint32_t *)(VTABLE_BASE + (x))))()
/*----------------------------------------------------------------------------
  Exception / Interrupt Handler Function Prototype
 *----------------------------------------------------------------------------*/
typedef void (*pFunc)(void);

/*----------------------------------------------------------------------------
  External References
 *----------------------------------------------------------------------------*/
//extern void _start     (void) __attribute__((noreturn)); /* PreeMain (C library entry point) */

/*----------------------------------------------------------------------------
  Internal References
 *----------------------------------------------------------------------------*/
void Reset_Handler(void) __attribute__((noreturn));

/*----------------------------------------------------------------------------
  User Initial Stack & Heap
 *----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
  Exception / Interrupt Handler
 *----------------------------------------------------------------------------*/
/* Boot Exceptions */
void boot_Default_Handler(void)
{
    VECT_ISR(__get_IPSR() << 2);
}

/*----------------------------------------------------------------------------
  Exception / Interrupt Vector table
 *----------------------------------------------------------------------------*/
extern const pFunc __Boot_Vectors[48];
const pFunc        __Boot_Vectors[48] __attribute__((section(".vectors"))) = {
    (pFunc)(&__StackTop), /*     Initial Stack Pointer */
    Reset_Handler,        /*     Reset Handler */
    boot_Default_Handler, /* -14 NMI Handler */
    boot_Default_Handler, /* -13 Hard Fault Handler */
    boot_Default_Handler, /*     Reserved */
    boot_Default_Handler, /*     Reserved */
    boot_Default_Handler, /*     Reserved */
    boot_Default_Handler, /*     Reserved */
    boot_Default_Handler, /*     Reserved */
    boot_Default_Handler, /*     Reserved */
    boot_Default_Handler, /*     Reserved */
    boot_Default_Handler, /*  -5 SVCall Handler */
    boot_Default_Handler, /*     Reserved */
    boot_Default_Handler, /*     Reserved */
    boot_Default_Handler, /*  -2 PendSV Handler */
    boot_Default_Handler, /*  -1 SysTick Handler */

    /* Interrupts */
    boot_Default_Handler, /*   0 GPIO IRQ */
    boot_Default_Handler, /*   1 WIC IRQ */
    boot_Default_Handler, /*   2 Reserved */
    boot_Default_Handler, /*   3 Reserved */
    boot_Default_Handler, /*   4 ADC IRQ */
    boot_Default_Handler, /*   5 Reserved */
    boot_Default_Handler, /*   6 Flash IRQ */
    boot_Default_Handler, /*   7 Watchdog IRQ */
    boot_Default_Handler, /*   8 System clock configure IRQ */
    boot_Default_Handler, /*   9 Timer0 IRQ */
    boot_Default_Handler, /*  10 Timer1 IRQ */
    boot_Default_Handler, /*  11 HPDA IRQ */
    boot_Default_Handler, /*  12 HPDB IRQ */
    boot_Default_Handler, /*  13 Reserved */
    boot_Default_Handler, /*  14 USBPDA IRQ */
    boot_Default_Handler, /*  15 USBPDB IRQ */
    boot_Default_Handler, /*  16 Reserved */
    boot_Default_Handler, /*  17 Reserved */
    boot_Default_Handler, /*  18 I2C1 IRQ */
    boot_Default_Handler, /*  19 I2C2 IRQ */
    boot_Default_Handler, /*  20 I2C3 IRQ */
    boot_Default_Handler, /*  21 I2C4 IRQ */
    boot_Default_Handler, /*  22 DMA Error IRQ */
    boot_Default_Handler, /*  23 DMA Done IRQ */
    boot_Default_Handler, /*  24 PMU IRQ */
};

/*----------------------------------------------------------------------------
  Reset Handler called on controller reset
 *----------------------------------------------------------------------------*/
void Reset_Handler(void)
{
    uint32_t *pSrc, *pDest;

    __disable_irq();

    __set_MSP((uint32_t)&__StackTop);

    if (firmware_boot_check())
    {
        firmware_update();
    }
    else
    {
        __enable_irq();
        VECT_ISR(0x04);
    }
    while (1)
    {
    }
}
}
#endif
