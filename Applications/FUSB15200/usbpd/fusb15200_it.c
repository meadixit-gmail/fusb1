/*******************************************************************************
 * @file     fusb15200_it.c
 * @author   USB PD Firmware Team
 *
 * Copyright 2018 ON Semiconductor. All rights reserved.
 *
 * This software and/or documentation is licensed by ON Semiconductor under
 * limited terms and conditions. The terms and conditions pertaining to the
 * software and/or documentation are available at
 * http://www.onsemi.com/site/pdf/ONSEMI_T&C.pdf
 * ("ON Semiconductor Standard Terms and Conditions of Sale,
 *   Section 8 Software").
 *
 * DO NOT USE THIS SOFTWARE AND/OR DOCUMENTATION UNLESS YOU HAVE CAREFULLY
 * READ AND YOU AGREE TO THE LIMITED TERMS AND CONDITIONS. BY USING THIS
 * SOFTWARE AND/OR DOCUMENTATION, YOU AGREE TO THE LIMITED TERMS AND CONDITIONS.
 ******************************************************************************/

#include "FUSB15xxx.h"

extern bool       gSleepInt;
extern HAL_I2C_T *i2c_master[I2C_PORT_COUNT];

void SysTick_Handler()
{
}

void GPIO_IRQHandler()
{
}

void WUT_IRQHandler()
{
    HAL_TIM_IRQHandler(Wake_Up_Timer);
    gSleepInt = true;
}

void I2C1_IRQHandler()
{
    /* Call the I2C IRQ handler with correct i2c driver */
    HAL_I2C_IRQHandler(i2c_master[0]);
}

void I2C2_IRQHandler()
{
    /* Call the I2C IRQ handler with correct i2c driver */
    HAL_I2C_IRQHandler(i2c_master[1]);
}

void I2C3_IRQHandler()
{
    /* Call the I2C IRQ handler with correct i2c driver */
    HAL_I2C_IRQHandler(i2c_master[2]);
}

void I2C4_IRQHandler()
{
    /* Call the I2C IRQ handler with correct i2c driver */
    HAL_I2C_IRQHandler(i2c_master[3]);
}

void DMAERROR_IRQHandler()
{
    //HAL_DMA_ERROR_IRQHandler();
}

void DMADONE_IRQHandler()
{
    //HAL_DMA_DONE_IRQHandler();
}

void USBPDA_IRQHandler()
{
    HAL_PD_IRQHandler(HAL_USBPD_CH_A);
    gSleepInt = true;
}

void USBPDB_IRQHandler()
{
    HAL_PD_IRQHandler(HAL_USBPD_CH_B);
    gSleepInt = true;
}

void TIM0_IRQHandler()
{
}

void TIM1_IRQHandler()
{
#if STACK_MON
    __NOP();
    TIMER_DRIVER.InterruptClear(Timer_1);
    HAL_STK_Mon();
#endif
}

void HPDA_IRQHandler()
{
    // HAL_HPD_IRQHandler(0);
    gSleepInt = true;
}

void HPDB_IRQHandler()
{
    // HAL_HPD_IRQHandler(0);
    gSleepInt = true;
}

void ADC_IRQHandler()
{
    HAL_ADC_CHx_STAT_T stat_ch0, stat_ch6;
    gSleepInt = true;
    stat_ch0  = ADC_DRIVER.ChannelStatus(ADC_CH0);
    stat_ch6  = ADC_DRIVER.ChannelStatus(ADC_CH6);
    /* Disable Alarms for now */
    if (stat_ch0.alarml)
    {
        ADC_DRIVER.ChannelSetLowAlarmInterrupt(ADC_CH0, false);
    }
    if (stat_ch0.alarmh)
    {
        ADC_DRIVER.ChannelSetHighAlarmInterrupt(ADC_CH0, false);
    }
    if (stat_ch6.alarml)
    {
        ADC_DRIVER.ChannelSetLowAlarmInterrupt(ADC_CH6, false);
    }
    if (stat_ch6.alarmh)
    {
        ADC_DRIVER.ChannelSetHighAlarmInterrupt(ADC_CH6, false);
    }
    ADC_DRIVER.InterruptClear();
    gSleepInt = true;
}
