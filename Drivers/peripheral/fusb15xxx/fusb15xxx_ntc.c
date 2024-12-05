/**
 * @file   fusb15xxx_ntc.c
 * @author USB Firmware Team
 * @brief  NTC Port driver
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

#if HAL_USE_NTC
/** Table below provides ADC values for Temperature 0 Celsius to 45 Celsius when
 * for RNTC = 100kOhm(1%) and R_P 100kOhm(1%) with NTC beta of 4300.
 */
static uint16_t NTC_ADC_TABLE[] = {
    908, 906, 903, 900, 897, 894, 890, 887, 883, 880, 876, 872, 868, 863, 859, 854, 850, 845,
    840, 834, 829, 823, 818, 812, 806, 800, 793, 787, 780, 773, 766, 759, 751, 744, 736, 729,
    721, 713, 704, 696, 688, 679, 671, 662, 653, 644, 635, 626, 617, 608, 598, 589, 580, 570,
    561, 551, 542, 533, 523, 514, 504, 495, 486, 476, 467, 458, 449, 440, 431, 422, 413, 404,
    396, 387, 379, 371, 362, 354, 346, 338, 331, 323, 316, 308, 301, 294, 287, 280, 273, 267,
    260, 254, 248, 242, 236, 230, 224, 219, 213, 208, 203, 198, 193, 188, 183, 178, 174, 170,
    165, 161, 157, 153, 149, 145, 142, 138, 135, 131, 128, 125, 121, 118, 115, 112, 110, 107,
    104, 102, 99,  97,  94,  92,  89,  87,  85,  83,  81,  79,  77,  75,  73,  71,  70,  68,
    66,  65,  63,  62,  60,  59,  57,  56,  55,  53,  52,  51,  50,  48,  47,  46,  45};

#define NTC_ADC_LEN (sizeof(NTC_ADC_TABLE) / sizeof(NTC_ADC_TABLE[0]))

void HAL_NTC_Enable(NTC_T *d, HAL_NTC_T ntc, bool en)
{
    uint32_t mask = 0;
    if (ntc == HAL_NTC_A)
    {
        mask = NTC_Port_A_B_Interface_NTC_CTRL_ntc_a_en_Msk;
    }
    else if (ntc == HAL_NTC_B)
    {
        mask = NTC_Port_A_B_Interface_NTC_CTRL_ntc_b_en_Msk;
    }
    if (en)
    {
        d->NTC_CTRL |= mask;
    }
    else
    {
        d->NTC_CTRL &= ~mask;
    }
}

uint32_t HAL_NTC_ADC2Temp(uint16_t adc)
{
    uint32_t left  = 0;
    uint32_t right = NTC_ADC_LEN - 1;
    uint32_t middle;
    uint16_t val;

    /* Do a quick binary search to either find the argument adc code,
       or narrow down to two values greater than(left) and less than(right) argument */
    middle = (left + right) / 2;

    /* If left < index an element is in between */
    while (left < middle)
    {
        val = NTC_ADC_TABLE[middle];
        if (adc == val)
        {
            /* value found */
            return middle;
        }
        if (adc > val)
        {
            right = middle;
        }
        else
        {
            left = middle;
        }
        middle = (left + right) / 2;
    }

    /* Here adc code is between two values left and right (left + 1) inclusive */
    if (adc > NTC_ADC_TABLE[right])
    {
        /* Value is either left or less than right */
        return left;
    }
    /* adc code is right */
    return right;
}

HAL_NTC_DRIVER_T NTC_DRIVER = {.Enable = HAL_NTC_Enable, .ConvertADC2Temp = HAL_NTC_ADC2Temp};
#endif
