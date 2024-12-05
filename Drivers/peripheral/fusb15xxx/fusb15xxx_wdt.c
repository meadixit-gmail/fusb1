/**
 * @file     fusb15xxx_wdt.c
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
 *
 */
#include "FUSB15xxx.h"

#if HAL_USE_WDT

void HAL_WDT_Initialize(WDT_T *d, uint32_t cfg)
{
    assert(d != 0);

    cfg &=
        (WatchDog_Timer_Registers_WDT_CFG_INT_EN_Msk | WatchDog_Timer_Registers_WDT_CFG_RST_EN_Msk);
    d->WDT_CFG = cfg;
}

void HAL_WDT_Feed(WDT_T *d, uint32_t value)
{
    assert(d != 0);

    d->WDT_LD = value;
}

uint32_t HAL_WDT_Value(WDT_T *d)
{
    assert(d != 0);

    return d->WDT_VAL;
}

bool HAL_WDT_InterruptStatus(WDT_T *d)
{
    assert(d != 0);
    if (d->WDT_INTSTS & WatchDog_Timer_Registers_WDT_INTSTS_INT_STS_Msk)
    {
        return true;
    }
    return false;
}

void HAL_WDT_InterruptClear(WDT_T *d)
{
    assert(d != 0);
#if (DEVICE_TYPE == FUSB15101)
    d->WDT_ITCTL = WatchDog_Timer_Registers__WDT_INTCLR_INTCLR_Msk;
#else
    d->_WDT_INTCLR = WatchDog_Timer_Registers__WDT_INTCLR_INTCLR_Msk;
#endif
}

void HAL_WDT_LockEnable(WDT_T *d, bool en)
{
    assert(d != 0);

    if (!en)
    {
        d->WDT_LOCK = WDT_LOCK_VALUE;
    }
    else
    {
        d->WDT_LOCK = WatchDog_Timer_Registers_WDT_LOCK_WDT_LOCK_Msk;
    }
}

HAL_WDT_DRIVER_T WDT_DRIVER = {.Initialize      = HAL_WDT_Initialize,
                               .Feed            = HAL_WDT_Feed,
                               .Value           = HAL_WDT_Value,
                               .InterruptStatus = HAL_WDT_InterruptStatus,
                               .InterruptClear  = HAL_WDT_InterruptClear,
                               .LockEnable      = HAL_WDT_LockEnable};
#endif /* HAL_USE_WDT */
