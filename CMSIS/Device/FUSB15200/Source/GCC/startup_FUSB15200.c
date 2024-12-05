/**
 * @file     startup_FUSB15200.c
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

#include "FUSB15xxx.h"

/* typedef external function */
extern void main();

/* linker symbol */
extern uint32_t __etext;
extern uint32_t __data_start__;
extern uint32_t __data_end__;
extern uint32_t __bss_start__;
extern uint32_t __bss_end__;
extern uint32_t __StackTop;
#if STACK_MON
extern uint32_t __Stack_Mon;
#endif
/* Function prototype */
void Default_Handler(void) __attribute__((noreturn));
void Reset_Handler(void) __attribute__((noreturn));
void NMI_Handler(void) __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void) __attribute__((weak, alias("Default_Handler")));
void GPIO_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void WUT_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void ADC_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void FLASH_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void WDT_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void SYSCONFIG_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM0_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void TIM1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void HPDA_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void HPDB_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMAERROR_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void DMADONE_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void USBPDA_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void USBPDB_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void BC12A_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void BC12B_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void I2C1_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void I2C2_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void I2C3_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void I2C4_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));
void PMU_IRQHandler(void) __attribute__((weak, alias("Default_Handler")));

typedef void (*pIrqFunc)(void);

/* Vector table */
extern const pIrqFunc __Vectors[48];
const pIrqFunc        __Vectors[48] __attribute__((section(".vectors"))) = {
    (pIrqFunc)(&__StackTop), /*     Initial Stack Pointer */
    Reset_Handler,           /*     Reset Handler */
    NMI_Handler,             /* -14 NMI Handler */
    HardFault_Handler,       /* -13 Hard Fault Handler */
    0,                       /*     Reserved */
    0,                       /*     Reserved */
    0,                       /*     Reserved */
    0,                       /*     Reserved */
    0,                       /*     Reserved */
    0,                       /*     Reserved */
    0,                       /*     Reserved */
    SVC_Handler,             /*  -5 SVCall Handler */
    0,                       /*     Reserved */
    0,                       /*     Reserved */
    PendSV_Handler,          /*  -2 PendSV Handler */
    SysTick_Handler,         /*  -1 SysTick Handler */
    /* Interrupts */
    GPIO_IRQHandler,      /*   0 GPIO IRQ */
    WUT_IRQHandler,       /*   1 WIC IRQ */
    Default_Handler,      /*   2 Reserved */
    Default_Handler,      /*   3 Reserved */
    ADC_IRQHandler,       /*   4 ADC IRQ */
    Default_Handler,      /*   5 Reserved */
    FLASH_IRQHandler,     /*   6 Flash IRQ */
    WDT_IRQHandler,       /*   7 Watchdog IRQ */
    SYSCONFIG_IRQHandler, /*   8 System clock configure IRQ */
    TIM0_IRQHandler,      /*   9 Timer0 IRQ */
    TIM1_IRQHandler,      /*  10 Timer1 IRQ */
    HPDA_IRQHandler,      /*  11 HPDA IRQ */
    HPDB_IRQHandler,      /*  12 HPDB IRQ */
    Default_Handler,      /*  13 Reserved */
    USBPDA_IRQHandler,    /*  14 USBPDA IRQ */
    USBPDB_IRQHandler,    /*  15 USBPDB IRQ */
    BC12A_IRQHandler,     /*  16 Reserved */
    BC12B_IRQHandler,     /*  17 Reserved */
    I2C1_IRQHandler,      /*  18 I2C1 IRQ */
    I2C2_IRQHandler,      /*  19 I2C2 IRQ */
    I2C3_IRQHandler,      /*  20 I2C3 IRQ */
    I2C4_IRQHandler,      /*  21 I2C4 IRQ */
    DMAERROR_IRQHandler,  /*  22 DMA Error IRQ */
    DMADONE_IRQHandler,   /*  23 DMA Done IRQ */
    PMU_IRQHandler,       /*  24 PMU IRQ */
};

/**
 * @brief Entry point after reset
 */
void Reset_Handler(void)
{
    uint32_t *pSrc, *pDest;

    /* Copy data section  */
    pSrc  = &__etext;
    pDest = &__data_start__;
    for (; pDest < &__data_end__;)
    {
        *pDest++ = *pSrc++;
    }

    /* Zero-init bss */
    pDest = &__bss_start__;
    for (; pDest < &__bss_end__;)
    {
        *pDest++ = 0UL;
    }

#if STACK_MON
    pDest = &__Stack_Mon;
    *pDest = 0UL;
#endif
    /* CMSIS init */
#ifdef HAL_USE_CMSIS
    SystemInit();
#endif
    /* Recover stack from*/
    __set_MSP((uint32_t)&__StackTop);
    main();
    while (1)
    {
    }
}

/**
 * @brief Default handler for any undefined exception / interrupt
 */
void Default_Handler(void)
{
    while (1)
    {
    }
}
