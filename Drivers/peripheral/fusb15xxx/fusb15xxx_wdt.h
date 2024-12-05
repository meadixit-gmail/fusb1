/**
 * @file     fusb15xxx_wdt.h
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
 * @addtogroup HAL
 * @{
 */
#ifndef __FUSB15XXX_WDT_H__
#define __FUSB15XXX_WDT_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define WDT_LOCK_VALUE 0x1ACCE551

    typedef struct
    {
        void (*Initialize)(WDT_T *, uint32_t);
        void (*Feed)(WDT_T *, uint32_t);
        uint32_t (*Value)(WDT_T *);
        bool (*InterruptStatus)(WDT_T *);
        void (*InterruptClear)(WDT_T *);
        void (*LockEnable)(WDT_T *, bool);
    } const HAL_WDT_DRIVER_T;

    extern HAL_WDT_DRIVER_T WDT_DRIVER;

#ifdef __cplusplus
}
#endif

#endif /** @} __FUSB15XXX_WDT_H__ */
