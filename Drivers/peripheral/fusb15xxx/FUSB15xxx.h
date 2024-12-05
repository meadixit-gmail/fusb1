/*******************************************************************************
 * @file   FUSB15xxx.h
 * @author USB Firmware Team
 * @brief  Global include file for FUSB152xx.h family or products
 *
 * @copyright @parblock
 * Copyright &copy; 2022 ON Semiconductor &reg;. All rights reserved.
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
*******************************************************************************/
#ifndef __FUSB15XXX_H__
#define __FUSB15XXX_H__

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef _RTE_
#include "RTE_Components.h"
#endif

#if defined(FUSB15101)
#undef FUSB15101
#define FUSB15101   1
#define DEVICE_TYPE FUSB15101
#include "FUSB15101.h"
#elif defined(FUSB15200)
#undef FUSB15200
#define FUSB15200   2
#define DEVICE_TYPE FUSB15200
#include "FUSB15200.h"
#elif defined(FUSB15201)
#undef FUSB15201
#define FUSB15201   3
#define DEVICE_TYPE FUSB15201
#include "FUSB15200.h"
#elif defined(FUSB15201P)
#undef FUSB15201P
#define FUSB15201P  4
#define DEVICE_TYPE FUSB15201P
#include "FUSB15201P.h"
#else
#error "Device not selected"
#endif

#if ((DEVICE_TYPE == FUSB15201) || (DEVICE_TYPE == FUSB15200))
    typedef Timer_Type                      TIM_T;
    typedef WatchDog_Timer_Registers_Type   WDT_T;
    typedef NTC_Port_A_B_Interface_Type     NTC_T;
    typedef USB_Type_C_PD_Port_Type         TCPD_T;
    typedef Legacy_DP_DM_Protocol_Port_Type USBCHG_T;
    typedef Clock_Management_Unit_Type      CMU_T;
    typedef Power_Management_Unit_Type      PMU_T;
    typedef PORT_Interface_Type             PIT_T;
    typedef GPIO_Interface_Type             GPIO_T;
    typedef ADC_Interface_Type              ADC_T;
    typedef I2C_Port_Type                   I2C_T;
    typedef DMA_Type                        DMA_T;
    typedef Flash_Control_Type              FLASH_T;
    typedef HPD_Port_Type                   HPD_T;
    typedef System_Control_Block_Type       SCB_T;

#elif (DEVICE_TYPE == FUSB15101)
typedef Timer_Type                      TIM_T;
typedef WatchDog_Timer_Registers_Type   WDT_T;
typedef NTC_Port_A_B_Interface_Type     NTC_T;
typedef USB_Type_C_PD_Port_Type         TCPD_T;
typedef Legacy_DP_DM_Protocol_Port_Type USBCHG_T;
typedef GPIO_Interface_Type             GPIO_T;
typedef UART_Type                       UART_T;
typedef ADC_Interface_Type              ADC_T;
typedef PORT_Interface_Type             PIT_T;
typedef Clock_Management_Unit_Type      CMU_T;
typedef I2C_Port_Type                   I2C_T;
typedef VBUS_CTRL_Type                  VBUS_T;
typedef System_Control_Block_Type       SCB_T;
#else
typedef Timer_Type                      TIM_T;
typedef WatchDog_Timer_Registers_Type   WDT_T;
typedef NTC_Port_A_B_Interface_Type     NTC_T;
typedef USB_Type_C_PD_Port_Type         TCPD_T;
typedef Legacy_DP_DM_Protocol_Port_Type USBCHG_T;
typedef Clock_Management_Unit_Type      CMU_T;
typedef Power_Management_Unit_Type      PMU_T;
typedef PORT_Interface_Type             PIT_T;
typedef GPIO_Interface_Type             GPIO_T;
typedef ADC_Interface_Type              ADC_T;
typedef I2C_Port_Type                   I2C_T;
typedef System_Control_Block_Type       SCB_T;

#endif

#ifndef _RTE_

#ifndef LEGACY_PORT_COUNT
#define LEGACY_PORT_COUNT 2
#endif

/*PORT COUNT*/
#ifndef TYPE_C_PORT_COUNT
/*only 1 port for FUSB15101*/
#if defined(FUSB15101)
#define TYPE_C_PORT_COUNT 1
/*2 ports for FUSB15200/FUSB15201/FUSB15201P*/
#elif defined(FUSB15200) || defined(FUSB15201) || defined(FUSB15201P)
#define TYPE_C_PORT_COUNT 2
#endif
#endif

/*I2C INTERFACE*/
#ifndef I2C_PORT_COUNT
#if defined(FUSB15101)
#define I2C_PORT_COUNT 1
#elif defined(FUSB15200) || defined(FUSB15201)
#define I2C_PORT_COUNT 4
#elif defined(FUSB15201P)
#define I2C_PORT_COUNT 2
#endif
#endif

#ifndef HPD_PORT_COUNT
#define HPD_PORT_COUNT 2
#endif

#ifndef HAL_USE_CMU
#define HAL_USE_CMU 1
#endif

#ifndef HAL_USE_ADC
#define HAL_USE_ADC 1
#endif

#ifndef HAL_USE_I2C
#define HAL_USE_I2C 1
#endif

#if defined(FUSB15201) || defined(FUSB15200)
#ifndef HAL_USE_DMA
#define HAL_USE_DMA 1
#endif

#ifndef HAL_USE_HPD
#define HAL_USE_HPD 1
#endif
#endif

#ifndef HAL_USE_GPIO
#define HAL_USE_GPIO 1
#endif

#ifndef HAL_USE_PORT
#define HAL_USE_PORT 1
#endif

#ifndef HAL_USE_TIMER
#define HAL_USE_TIMER 1
#endif

#ifndef HAL_USE_TCPD
#define HAL_USE_TCPD 1
#endif

#ifndef HAL_USE_USBCHG
#define HAL_USE_USBCHG 1
#endif

#ifndef HAL_USE_WDT
#define HAL_USE_WDT 1
#endif

#ifndef HAL_USE_NTC
#define HAL_USE_NTC 1
#endif

#if (DEVICE_TYPE == FUSB15101)
#ifndef HAL_USE_VBUS
#define HAL_USE_VBUS 1
#endif

#ifndef HAL_USE_UART
#define HAL_USE_UART 1
#endif

#ifndef HAL_USE_DAC
#define HAL_USE_DAC 1
#endif
#endif

#endif /* _RTE_ */

#if defined(FUSB15101)
#include "fusb15xxx_defines.h"
#include "fusb15101_hal_vbus.h"

#if HAL_USE_CMU
#include "fusb15xxx_cmu.h"
#endif

#if HAL_USE_ADC
#include "fusb15xxx_adc.h"
#endif

#if HAL_USE_USBCHG
#include "fusb15xxx_usbchg.h"
#endif

#if HAL_USE_GPIO
#include "fusb15xxx_gpio.h"
#endif

#if HAL_USE_PORT
#include "fusb15xxx_port.h"
#endif

#if HAL_USE_TIMER
#include "fusb15xxx_timer.h"
#endif

#if HAL_USE_TCPD
#include "fusb15xxx_tcpd.h"
#endif

#if HAL_USE_I2C
#include "fusb15xxx_i2c.h"
#endif

#if HAL_USE_WDT
#include "fusb15xxx_wdt.h"
#endif

#if HAL_USE_NTC
#include "fusb15xxx_ntc.h"
#endif

#include "fusb15101_hal_uart.h"
#include "fusb15101_hal_dac.h"
#include "fusb15101_hal_cccv.h"

#elif defined(FUSB15200) || defined(FUSB15201) || defined(FUSB15201P)
#include "fusb15xxx_defines.h"

#if HAL_USE_CMU
#include "fusb15xxx_cmu.h"
#endif

#if HAL_USE_ADC
#include "fusb15xxx_adc.h"
#endif

#if HAL_USE_DMA
#include "fusb15xxx_dma.h"
#endif

#if HAL_USE_USBCHG
#include "fusb15xxx_usbchg.h"
#endif

#if HAL_USE_GPIO
#include "fusb15xxx_gpio.h"
#endif

#if HAL_USE_HPD
#include "fusb15xxx_hpd.h"
#endif

#if HAL_USE_PORT
#include "fusb15xxx_port.h"
#endif

#if HAL_USE_TIMER
#include "fusb15xxx_timer.h"
#endif

#if HAL_USE_TCPD
#include "fusb15xxx_tcpd.h"
#endif

#if HAL_USE_I2C
#include "fusb15xxx_i2c.h"
#endif

#if HAL_USE_WDT
#include "fusb15xxx_wdt.h"
#endif

#if HAL_USE_NTC
#include "fusb15xxx_ntc.h"
#endif
#endif

#ifdef __cplusplus
}
#endif
#endif /* @} __FUSB15XXX_H__ */
