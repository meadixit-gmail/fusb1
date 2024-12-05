/**
 * @file     fusb15xxx_adc.c
 * @author   USB PD Firmware Team
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
 */
#include "FUSB15xxx.h"

#if HAL_USE_ADC

/* Convert the channel to index */
#define ADC_CHx_ID(addr)   ((ch >> 16) & 0xFFFF)
#define ADC_CHx_MASK(addr) (ch & 0xFFFF)

static void HAL_ADC_Init(uint32_t cfg)
{
    /* Configure the ADC only do not enable. */
    cfg &= ~ADC_Interface_ADC_CTRL_adc_en_Msk;
    /* Mask all interrupt */
    ADC_Interface->ADC_INT_MSK = ~0;
    ADC_Interface->ADC_INT     = ~0;
    ADC_Interface->ADC_CTRL    = cfg;
}

static void HAL_ADC_Enable()
{
    ADC_Interface->ADC_CTRL |=
        ADC_Interface_ADC_CTRL_adc_en_Msk | ADC_Interface_ADC_CTRL_adc_scan_Msk;
}

static void HAL_ADC_Disable()
{
    ADC_Interface->ADC_CTRL &= ~ADC_Interface_ADC_CTRL_adc_en_Msk;
    HAL_Delay(2);
}

static uint32_t HAL_ADC_INT_Status()
{
    return ADC_Interface->ADC_INT & ADC_Interface_ADC_INT_I_ADC_Msk ? 1 : 0;
}

static void HAL_ADC_INT_Enable(bool b)
{
    if (b)
    {
        ADC_Interface->ADC_INT_MSK &= ~ADC_Interface_ADC_INT_MSK_M_ADC_Msk;
    }
    else
    {
        ADC_Interface->ADC_INT_MSK |= ADC_Interface_ADC_INT_MSK_M_ADC_Msk;
    }
}

static void HAL_ADC_INT_Clear()
{
    ADC_Interface->ADC_INT |= ADC_Interface_ADC_INT_I_ADC_Msk;
}

static HAL_ADC_CHx_STAT_T HAL_ADC_CHx_Status(ADC_CHx_T ch)
{
    HAL_ADC_CHx_STAT_T stat;
    stat.enabled = (ADC_Interface->ADC_CH_EN & (1U << ch)) ? 1 : 0;
    stat.done    = (ADC_Interface->ADC_STS & (1U << ch)) ? 1 : 0;
    stat.alarml  = (ADC_Interface->ADC_INT & (1U << (ch * 2))) ? 1 : 0;
    ;
    stat.alarmh = (ADC_Interface->ADC_INT & (1U << (ch * 2 + 1))) ? 1 : 0;
    return stat;
}

static void HAL_ADC_ChEnable(ADC_CHx_T ch, bool b)
{
    bool en = (ADC_Interface->ADC_CTRL & ADC_Interface_ADC_CTRL_adc_en_Msk) ? true : false;
    HAL_ADC_Disable();
    if (b)
    {
        ADC_Interface->ADC_CH_EN |= 1U << ch;
        en = true;
    }
    else
    {
        ADC_Interface->ADC_CH_EN &= ~(1U << ch);
        /* If no channels are enabled disable ADC */
        if (ADC_Interface->ADC_CH_EN == 0)
        {
            en = false;
        }
    }
    if (en)
    {
        HAL_ADC_Enable();
    }
}
static uint32_t HAL_ADC_CHx_Data(ADC_CHx_T ch)
{
    uint32_t ch_data;
    switch (ch)
    {
        case ADC_CH0:
            ch_data = ADC_Interface->ADC_DATA0;
            break;
        case ADC_CH1:
            ch_data = ADC_Interface->ADC_DATA1;
            break;
        case ADC_CH2:
            ch_data = ADC_Interface->ADC_DATA2;
            break;
        case ADC_CH3:
            ch_data = ADC_Interface->ADC_DATA3;
            break;
        case ADC_CH4:
            ch_data = ADC_Interface->ADC_DATA4;
            break;
        case ADC_CH5:
            ch_data = ADC_Interface->ADC_DATA5;
            break;
        case ADC_CH6:
            ch_data = ADC_Interface->ADC_DATA6;
            break;
#if ((DEVICE_TYPE == FUSB15201P) || (DEVICE_TYPE == FUSB15201) || (DEVICE_TYPE == FUSB15200))
        case ADC_CH7:
            ch_data = ADC_Interface->ADC_DATA7;
            break;
        case ADC_CH8:
            ch_data = ADC_Interface->ADC_DATA8;
            break;
#endif
#if ((DEVICE_TYPE == FUSB15201) || (DEVICE_TYPE == FUSB15200))
        case ADC_CH9:
            ch_data = ADC_Interface->ADC_DATA9;
            break;
        case ADC_CH10:
            ch_data = ADC_Interface->ADC_DATA10;
            break;
        case ADC_CH11:
            ch_data = ADC_Interface->ADC_DATA11;
            break;
#endif
        default:
            ch_data = 0;
            break;
    }
    return ch_data;
}

static inline uint32_t HAL_ADC_CHx_Conv2mV(ADC_CHx_T ch, uint32_t data)
{
#if (DEVICE_TYPE == FUSB15101)
    uint32_t range = ADC_Interface->ADC_RANGE & (0x3U << (2 * ch));
    range          = range >> (2 * ch);
    if (ch == ADC_CH0 || ch == ADC_CH1)
    {
        if (range == ADC_Interface_ADC_RANGE_ADC_RANGE_0_1MV)
        {
            return data * 10;
        }
        else if (range == ADC_Interface_ADC_RANGE_ADC_RANGE_0_2MV)
        {
            return data * 20;
        }
        else if (range == ADC_Interface_ADC_RANGE_ADC_RANGE_0_4MV)
        {
            return data * 40;
        }
        else
        {
            return data;
        }
    }
    else if (ch == ADC_CH2)
    {
        return data * 10;
    }
    else if (ch == ADC_CH5 || ch == ADC_CH6)
    {
        return (data * 125) / 100;
    }
    else
    {
        return data * 4;
    }
#elif (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
    if (ch == ADC_CH3 || ch == ADC_CH9)
    {
        return ((data * 125) / 100);
    }
    else if (ch == ADC_CH0 || ch == ADC_CH6)
    {
        return data * 10;
    }
#else
    if (ch == ADC_CH0 || ch == ADC_CH4)
    {
        return data * 10;
    }
    else if (ch == ADC_CH3 || ch == ADC_CH7)
    {
        return ((data * 125) / 100);
    }
    else
    {
        return data * 4;
    }
#endif
    return data;
}

static inline uint32_t HAL_ADC_CHx_Conv2Adc(ADC_CHx_T ch, uint32_t data)
{
#if (DEVICE_TYPE == FUSB15101)
    uint32_t range = ADC_Interface->ADC_RANGE & (0x3U << (2 * ch));
    range          = range >> (2 * ch);
    if (ch == ADC_CH0 || ch == ADC_CH1)
    {
        if (range == ADC_Interface_ADC_RANGE_ADC_RANGE_0_1MV)
        {
            return data / 10;
        }
        else if (range == ADC_Interface_ADC_RANGE_ADC_RANGE_0_2MV)
        {
            return data / 20;
        }
        else if (range == ADC_Interface_ADC_RANGE_ADC_RANGE_0_4MV)
        {
            return data / 40;
        }
        else
        {
            return data;
        }
    }
    else if (ch == ADC_CH2)
    {
        return data / 10;
    }
    else if (ch == ADC_CH5 || ch == ADC_CH6)
    {
        return (data * 100) / 125;
    }
    else
    {
        return data / 4;
    }
#elif (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
    if (ch == ADC_CH3 || ch == ADC_CH9)
    {
        return ((data * 100) / 125);
    }
    else if (ch == ADC_CH0 || ch == ADC_CH6)
    {
        return data / 10;
    }
#else
    if (ch == ADC_CH0 || ch == ADC_CH4)
    {
        return data / 10;
    }
    else if (ch == ADC_CH3 || ch == ADC_CH7)
    {
        return (data * 100) / 125;
    }
    else
    {
        return data / 4;
    }
#endif
    return data;
}

static uint32_t HAL_ADC_CHx_mV(ADC_CHx_T ch)
{
    uint32_t val = HAL_ADC_CHx_Data(ch);
    return HAL_ADC_CHx_Conv2mV(ch, val);
}

static void HAL_ADC_CHx_SetAlarm(ADC_CHx_T ch, uint32_t low, uint32_t high)
{
    uint32_t           index = ch * 2;
    volatile uint32_t *reg   = &ADC_Interface->ADC_CH0_ALARM_LO;
    if (ch < ADC_NUM_CH)
    {
        reg[index]     = HAL_ADC_CHx_Conv2Adc(ch, low);
        reg[index + 1] = HAL_ADC_CHx_Conv2Adc(ch, high);
    }
}

static void HAL_ADC_CHx_AlarmL(ADC_CHx_T ch, bool b)
{
    volatile uint32_t *reg  = &ADC_Interface->ADC_CH0_ALARM_LO;
    uint32_t           mask = 1U << (ch * 2);

    if (b)
    {
        ADC_Interface->ADC_INT = mask;
        ADC_Interface->ADC_INT_MSK &= ~mask;
    }
    else
    {
        /* Mask interrupt and clear any pending interrupt */
        reg[ch * 2] = 0;
        ADC_Interface->ADC_INT_MSK |= mask;
        ADC_Interface->ADC_INT = mask;
    }
}

static void HAL_ADC_CHx_AlarmH(ADC_CHx_T ch, bool b)
{
    volatile uint32_t *reg  = &ADC_Interface->ADC_CH0_ALARM_LO;
    uint32_t           mask = 1U << ((ch * 2) + 1);

    if (b)
    {
        ADC_Interface->ADC_INT = mask;
        ADC_Interface->ADC_INT_MSK &= ~mask;
    }
    else
    {
        reg[ch * 2 + 1] = 0;
        ADC_Interface->ADC_INT_MSK |= mask;
        ADC_Interface->ADC_INT = mask;
    }
}

static void HAL_ADC_CHx_Refsel(ADC_CHx_T ch, HAL_ADCREF_T ref)
{
    uint32_t offset, mask;
    bool     en = (ADC_Interface->ADC_CTRL & ADC_Interface_ADC_CTRL_adc_en_Msk) ? true : false;

    offset = ch * 2;
    mask   = 0x3U << offset;

    HAL_ADC_Disable();
    ADC_Interface->ADC_RANGE = (ADC_Interface->ADC_RANGE & ~mask) | (ref << offset);
    if (en)
    {
        HAL_ADC_Enable();
    }
}

static uint32_t HAL_ADC_CHx_Range(ADC_CHx_T ch)
{
    uint32_t val = ADC_Interface->ADC_RANGE;
    val          = (val >> (ch * 2)) & 0x3;
    return val;
}
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201) || (DEVICE_TYPE == FUSB15201P)
static void HAL_ADC_CHx_Autoscale(ADC_CHx_T ch, bool b)
{
}
#endif
HAL_ADC_DRIVER_T ADC_DRIVER = {.Configure = HAL_ADC_Init,
                               .Start     = HAL_ADC_Enable,
                               .Stop      = HAL_ADC_Disable,

                               .InterruptEnable = HAL_ADC_INT_Enable,
                               .InterruptClear  = HAL_ADC_INT_Clear,

                               .ChannelStatus        = HAL_ADC_CHx_Status,
                               .ChannelEnable        = HAL_ADC_ChEnable,
                               .ChannelData          = HAL_ADC_CHx_Data,
                               .ChannelDataMilliVolt = HAL_ADC_CHx_mV,

                               .ChannelSetAlarm              = HAL_ADC_CHx_SetAlarm,
                               .ChannelSetLowAlarmInterrupt  = HAL_ADC_CHx_AlarmL,
                               .ChannelSetHighAlarmInterrupt = HAL_ADC_CHx_AlarmH,
                               .ChannelSetReference          = HAL_ADC_CHx_Refsel,
                               .ChannelReference             = HAL_ADC_CHx_Range,
                               .InterruptStatus              = HAL_ADC_INT_Status,
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201) || (DEVICE_TYPE == FUSB15201P)
                               .ChannelAutoscaleEnable = HAL_ADC_CHx_Autoscale
#endif
};
#endif /* HAL_USE_ADC */
