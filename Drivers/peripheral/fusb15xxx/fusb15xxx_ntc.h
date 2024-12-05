/**
 * @file   fusb15xxx_ntc.h
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
 * @addtogroup HAL
 */

#ifndef __FUSB15XXX_NTC_H__
#define __FUSB15XXX_NTC_H__

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        HAL_NTC_A = 0,
        HAL_NTC_B = 1,
    } HAL_NTC_T;

    typedef struct
    {
        void (*Enable)(NTC_T *, HAL_NTC_T, bool);
        uint32_t (*ConvertADC2Temp)(uint16_t);
    } const HAL_NTC_DRIVER_T;

    extern HAL_NTC_DRIVER_T NTC_DRIVER;

#ifdef __cplusplus
}
#endif

#endif /**@} __FUSB15XXX_NTC_H__ */
