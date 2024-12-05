/**
 * @file     fusb15xxx_adc.h
 * @author   USB PD Firmware Team
 * @brief    ADC
 * @addtogroup HAL_ADC_Driver
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
 * @addtogroup HAL
 * @{
 */
#ifndef __FUSB15XXX_ADC_H__
#define __FUSB15XXX_ADC_H__

#ifdef __cplusplus
extern "C"
{
#endif

    /**
 * ADC Channels
 */
    typedef enum
    {
        ADC_CH0 = 0,
        ADC_CH1,
        ADC_CH2,
        ADC_CH3,
        ADC_CH4,
        ADC_CH5,
        ADC_CH6,
#if (DEVICE_TYPE == FUSB15200 || DEVICE_TYPE == FUSB15201) || (DEVICE_TYPE == FUSB15201P)
        ADC_CH7,
        ADC_CH8,
#endif
#if (DEVICE_TYPE == FUSB15200 || DEVICE_TYPE == FUSB15201)
        ADC_CH9,
        ADC_CH10,
        ADC_CH11,
#endif
        ADC_NUM_CH,
    } HAL_ADCHx_T;

    /**
 * ADC Reference Selection
 */
    typedef enum
    {
        ADC_REFSEL_1p025mV = 0, //< 1.025 mV
        ADC_REFSEL_1mV     = 0, //< 1 mv
        ADC_REFSEL_2mV     = 1, //< 2 mV
        ADC_REFSEL_4mV     = 2, //< 4 mV
    } HAL_ADCREF_T;

    typedef HAL_ADCHx_T ADC_CHx_T;

    typedef struct
    {
        uint8_t enabled :1; ///< ADC Channel is enabled
        uint8_t done    :1; ///< ADC Channel data is is ready
        uint8_t alarml  :1; ///< ADC Channel Low alarm set
        uint8_t alarmh  :1; ///< ADC Channel High alarm set
        uint8_t         :4;
    } HAL_ADC_CHx_STAT_T;

    /**
 * ADC Driver interface definition
 */
    typedef struct
    {
        void (*Configure)(uint32_t);
        void (*Start)();
        void (*Stop)();
        void (*InterruptEnable)(bool);
        void (*InterruptClear)();
        HAL_ADC_CHx_STAT_T (*ChannelStatus)(HAL_ADCHx_T);
        void (*ChannelEnable)(HAL_ADCHx_T, bool);
        uint32_t (*ChannelData)(HAL_ADCHx_T);
        uint32_t (*ChannelDataMilliVolt)(HAL_ADCHx_T);
        void (*ChannelSetAlarm)(HAL_ADCHx_T, uint32_t, uint32_t);
        void (*ChannelSetLowAlarmInterrupt)(HAL_ADCHx_T, bool);
        void (*ChannelSetHighAlarmInterrupt)(HAL_ADCHx_T, bool);
        void (*ChannelSetReference)(HAL_ADCHx_T, HAL_ADCREF_T);
        uint32_t (*ChannelReference)(HAL_ADCHx_T);
        uint32_t (*InterruptStatus)();
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201) || (DEVICE_TYPE == FUSB15201P)
        void (*ChannelAutoscaleEnable)(HAL_ADCHx_T, bool);
#endif
    } const HAL_ADC_DRIVER_T;

    /**
 * ADC Driver object
 */
    extern HAL_ADC_DRIVER_T ADC_DRIVER;

#ifdef __cplusplus
}
#endif
#endif /** @} __FUSB15XXX_ADC_H__ */
