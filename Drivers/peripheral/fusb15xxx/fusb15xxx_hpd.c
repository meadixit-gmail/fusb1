/**
 * @file     fusb15xxx_hpd.c
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

#if HAL_USE_HPD

enum
{
    HAL_HPDA = 0,
    HAL_HPDB,
    HAL_HPD_COUNT,
};

typedef struct _HAL_HPD_T
{
    HPD_Port_Type *dev;
    HAL_HPDMODE_T  mode;
    HAL_HPDCB_T    cb;
    uint8_t        id;
} HAL_HPD_T;

static HAL_HPD_T memHpd[HPD_PORT_COUNT];

static HAL_HPD_T *HAL_HPD_Init(HPD_Port_Type *d, HAL_HPDMODE_T mode, HAL_HPDCB_T cb)
{
    assert(d != 0);
    HAL_HPD_T *hpd = 0;
    if (d == HPD_Port_A)
    {
        hpd = &memHpd[HAL_HPDA];
    }
    else if (d == HPD_Port_B)
    {
        hpd = &memHpd[HAL_HPDB];
    }

    if (hpd)
    {
        hpd->dev  = d;
        hpd->mode = mode;
        hpd->cb   = cb;

        if (hpd->mode == HPD_TRANSMITTER)
        {
            /* Enable HPD in source mode */
            hpd->dev->HPD_CONFIG =
                HPD_Port_HPD_CONFIG_HPD_MODE_Msk | HPD_Port_HPD_CONFIG_HPD_EN_Msk;
            hpd->dev->HPD_CNTRL   = 0;
            hpd->dev->HPD_INT_MSK = ~0;
        }
        else if (hpd->mode == HPD_RECEIVER)
        {
            /* Enable HPD in sink mode */
            hpd->dev->HPD_CONFIG &= ~HPD_Port_HPD_CONFIG_HPD_MODE_Msk;
            hpd->dev->HPD_CONFIG |= HPD_Port_HPD_CONFIG_HPD_EN_Msk;
            /* Enable interrupt only for HPD high */
            hpd->dev->HPD_INT_MSK = ~HPD_Port_HPD_INT_MSK_M_HPD_HIGH_LEVEL_Msk;
        }
    }

    return hpd;
}

static void HAL_HPD_DeInit(HAL_HPD_T *d)
{
    assert(d != 0 && d->dev != 0);

    d->dev->HPD_CONFIG  = 0;
    d->dev->HPD_INT_MSK = ~0;
}

static void HAL_HPD_SetLevel(HAL_HPD_T *d, HAL_HPDLVL_T lvl)
{
    assert(d != 0 && d->dev != 0);

    if (lvl == HPD_LOW)
    {
        d->dev->HPD_CNTRL &= ~HPD_Port_HPD_CNTRL_HPD_TX_LEVEL_Msk;
    }
    else
    {
        d->dev->HPD_CNTRL |= HPD_Port_HPD_CNTRL_HPD_TX_LEVEL_Msk;
    }
}

static HAL_HPDLVL_T HAL_HPD_Status(HAL_HPD_T *d)
{
    assert(d != 0 && d->dev != 0);

    if (d->dev->HPD_STS & HPD_Port_HPD_STS_HPD_IO_STATUS_Msk)
    {
        return HPD_HIGH;
    }
    return HPD_LOW;
}

static void HAL_HPD_TxIRQ(HAL_HPD_T *d)
{
    assert(d != 0 && d->dev != 0);

    d->dev->HPD_CNTRL |= HPD_Port_HPD_CNTRL_HPD_TX_IRQ_Msk;
}

static void HAL_HPD_EventEnable(HAL_HPD_T *d, HAL_HPDEVT_T evt, bool en)
{
    assert(d != 0 && d->dev != 0);
    uint32_t mask = evt;
    if (en)
    {
        d->dev->HPD_INT_MSK &= ~mask;
    }
    else
    {
        d->dev->HPD_INT_MSK |= mask;
    }
}

static HAL_HPDEVT_T HAL_HPD_Event(HAL_HPD_T *d)
{
    assert(d != 0 && d->dev != 0);
    HAL_HPDEVT_T evt = d->dev->HPD_INT;
    return evt;
}

void HAL_HPD_IRQHandler(HAL_HPD_T *d)
{
    assert(d != 0);

    uint32_t     intval = d->dev->HPD_INT & ~(d->dev->HPD_INT_MSK);
    HAL_HPDEVT_T evt    = HAL_HPD_EVENT_NONE;
    if (d->mode == HPD_RECEIVER)
    {
        if (intval & HPD_Port_HPD_INT_I_HPD_HIGH_LEVEL_Msk)
        {
            d->dev->HPD_INT_MSK |= HPD_Port_HPD_INT_MSK_M_HPD_HIGH_LEVEL_Msk;
            d->dev->HPD_INT_MSK &= ~(HPD_Port_HPD_INT_MSK_M_HPD_LOW_LEVEL_Msk
                                     | HPD_Port_HPD_INT_MSK_M_HPD_IRQ_RCVD_Msk);
            d->dev->HPD_INT = HPD_Port_HPD_INT_I_HPD_HIGH_LEVEL_Msk;
            evt |= HAL_HPD_EVENT_HIGH;
        }
        if (intval & HPD_Port_HPD_INT_I_HPD_LOW_LEVEL_Msk)
        {
            d->dev->HPD_INT_MSK |= (HPD_Port_HPD_INT_MSK_M_HPD_IRQ_RCVD_Msk
                                    | HPD_Port_HPD_INT_MSK_M_HPD_LOW_LEVEL_Msk);
            d->dev->HPD_INT_MSK &= ~HPD_Port_HPD_INT_MSK_M_HPD_HIGH_LEVEL_Msk;
            d->dev->HPD_INT = HPD_Port_HPD_INT_I_HPD_LOW_LEVEL_Msk;
            evt |= HAL_HPD_EVENT_LOW;
        }
        if (intval & HPD_Port_HPD_INT_I_HPD_IRQ_RCVD_Msk)
        {
            d->dev->HPD_INT = HPD_Port_HPD_INT_MSK_M_HPD_IRQ_RCVD_Msk;
            evt             = HAL_HPD_EVENT_IRQRX;
        }
    }
    else if (d->mode == HPD_TRANSMITTER)
    {
        if (intval & HPD_Port_HPD_INT_I_HPD_IRQ_SENT_Msk)
        {
            d->dev->HPD_INT = HPD_Port_HPD_INT_MSK_M_HPD_IRQ_SENT_Msk;
            evt |= HAL_HPD_EVENT_IRQTX;
        }
    }

    if (d->cb)
    {
        d->cb(evt);
    }
}

HAL_HPD_DRIVER_T HPD_DRIVER = {.Enable      = HAL_HPD_Init,
                               .Disable     = HAL_HPD_DeInit,
                               .SetHpd      = HAL_HPD_SetLevel,
                               .HpdStatus   = HAL_HPD_Status,
                               .TransmitIrq = HAL_HPD_TxIRQ,
                               .EventEnable = HAL_HPD_EventEnable,
                               .Event       = HAL_HPD_Event};
#endif /* HAL_USE_HPD */
