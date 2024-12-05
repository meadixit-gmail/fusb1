/*******************************************************************************
 * @file     fusb15201_it.c
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
#include "pdps_batt_ntc_monitor.h"
extern struct batt_ntc_monitor *globalMonitor;
extern bool                     gSleepInt;
extern HAL_I2C_T               *i2c_master[I2C_PORT_COUNT];
#if (CONFIG_ENABLED(CUSTOM_APPLICATION))
extern volatile bool i2c_porta_interrupt;
extern volatile bool i2c_portb_interrupt;
#endif
void SysTick_Handler()
{
}

void GPIO_IRQHandler()
{
#if (CONFIG_ENABLED(CUSTOM_APPLICATION))
#if (DEVICE_TYPE == FUSB15201)
    if ((GPIO_Interface->GPIO_IN_EN & (0x1U << HAL_GPIO7))
        && GPIO_DRIVER.InterruptStatus(HAL_GPIO7, HAL_GPIO_B))
    {
        i2c_portb_interrupt = true;
        GPIO_DRIVER.InterruptDisable(HAL_GPIO7, HAL_GPIO_B);
        GPIO_DRIVER.InterruptClear(HAL_GPIO7, HAL_GPIO_B);
    }
    gSleepInt = true;
#elif (DEVICE_TYPE == FUSB15201P)
    if ((GPIO_Interface->GPIO_INT_EN_PB & (0x1U << HAL_GPIO3))
        && GPIO_DRIVER.InterruptStatus(HAL_GPIO3, HAL_GPIO_B))
    {
        i2c_portb_interrupt = true;
        GPIO_DRIVER.InterruptDisable(HAL_GPIO3, HAL_GPIO_B);
        GPIO_DRIVER.InterruptClear(HAL_GPIO3, HAL_GPIO_B);
    }
    gSleepInt = true;
#endif
#endif
}

void WUT_IRQHandler()
{
    HAL_TIM_IRQHandler(Wake_Up_Timer);
    gSleepInt = true;
}

void WDT_IRQHandler()
{
    WDT_DRIVER.LockEnable(WatchDog_Timer_Registers, false);
    WDT_DRIVER.InterruptClear(WatchDog_Timer_Registers);
    WDT_DRIVER.LockEnable(WatchDog_Timer_Registers, true);
}

void I2C1_IRQHandler()
{
#if (CONFIG_ENABLED(CUSTOM_APPLICATION))
#if (DEVICE_TYPE == FUSB15201P)
    HAL_I2C_IRQHandler(i2c_master[0]);
    if ((I2C_Port_1->I2C_INT_CFG & I2C_INT_ENABLE) && I2C_Port_1->I2C_INT_STS)
    {
        i2c_porta_interrupt = true;
        I2C_Port_1->I2C_INT_CFG &= ~(I2C_INT_ENABLE);
        I2C_Port_1->I2C_INT_STS = 0x1;
    }
    gSleepInt = true;
#endif
#else
    /* Call the I2C IRQ handler with correct i2c driver */
    HAL_I2C_IRQHandler(i2c_master[0]);
    gSleepInt = true;
#endif
}

void I2C2_IRQHandler()
{
    /* Call the I2C IRQ handler with correct i2c driver */
    //HAL_I2C_IRQHandler(1);
}

void I2C3_IRQHandler()
{
#if (CONFIG_ENABLED(CUSTOM_APPLICATION))
    /* Call the I2C IRQ handler with correct i2c driver */
#if (DEVICE_TYPE == FUSB15201)
    HAL_I2C_IRQHandler(i2c_master[2]);
    if ((I2C_Port_3->I2C_INT_CFG & I2C_INT_ENABLE) && I2C_Port_3->I2C_INT_STS)
    {
        i2c_porta_interrupt = true;
        I2C_Port_3->I2C_INT_CFG &= ~(I2C_INT_ENABLE);
        I2C_Port_3->I2C_INT_STS = 0x1;
    }
    gSleepInt = true;
#endif
#endif
}

void I2C4_IRQHandler()
{
    /* Call the I2C IRQ handler with correct i2c driver */
    //HAL_I2C_IRQHandler(3);
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
    HAL_ADC_CHx_STAT_T VBUS_A, VBUS_B, TEMP_A, BATT_B;
    HAL_ADCHx_T        VBUS_A_CH, VBUS_B_CH, TEMP_A_CH, BATT_B_CH;
    gSleepInt = true;
#if (DEVICE_TYPE == FUSB15201)
    VBUS_A_CH = ADC_CH0;
    VBUS_B_CH = ADC_CH6;
    TEMP_A_CH = ADC_CH3;
    BATT_B_CH = ADC_CH9;
#elif (DEVICE_TYPE == FUSB15201P)
    VBUS_A_CH = ADC_CH0;
    VBUS_B_CH = ADC_CH4;
    TEMP_A_CH = ADC_CH3;
    BATT_B_CH = ADC_CH7;
#endif
    VBUS_A = ADC_DRIVER.ChannelStatus(VBUS_A_CH);
    VBUS_B = ADC_DRIVER.ChannelStatus(VBUS_B_CH);
    TEMP_A = ADC_DRIVER.ChannelStatus(TEMP_A_CH);
    BATT_B = ADC_DRIVER.ChannelStatus(BATT_B_CH);
    /* Disable Alarms for now */
    if (VBUS_A.alarml)
    {
        ADC_DRIVER.ChannelSetLowAlarmInterrupt(VBUS_A_CH, false);
    }
    if (VBUS_A.alarmh)
    {
        ADC_DRIVER.ChannelSetHighAlarmInterrupt(VBUS_A_CH, false);
    }
    if (VBUS_B.alarml)
    {
        ADC_DRIVER.ChannelSetLowAlarmInterrupt(VBUS_B_CH, false);
    }
    if (VBUS_B.alarmh)
    {
        ADC_DRIVER.ChannelSetHighAlarmInterrupt(VBUS_B_CH, false);
    }

    /* NTC_A */
    if (TEMP_A.alarml)
    {
        ADC_DRIVER.ChannelSetLowAlarmInterrupt(TEMP_A_CH, false);
        globalMonitor->temperature_low_alarm = true;
    }
    if (TEMP_A.alarmh)
    {
        ADC_DRIVER.ChannelSetHighAlarmInterrupt(TEMP_A_CH, false);
        globalMonitor->temperature_high_alarm = true;
    }

    /* Battery / NTCB */
    if (BATT_B.alarml)
    {
        ADC_DRIVER.ChannelSetLowAlarmInterrupt(BATT_B_CH, false);
        globalMonitor->battery_low_alarm = true;
    }
    if (BATT_B.alarmh)
    {
        ADC_DRIVER.ChannelSetHighAlarmInterrupt(BATT_B_CH, false);
        globalMonitor->battery_high_alarm = true;
    }

    ADC_DRIVER.InterruptClear();
    gSleepInt = true;
}
