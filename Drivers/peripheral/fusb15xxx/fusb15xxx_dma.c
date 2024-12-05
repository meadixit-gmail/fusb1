/**
 * @file     fusb15xxx_dma.c
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

#if HAL_USE_DMA
/**
 * @brief 
 * 
 */
 /* 0...3 Primary and 4...7 Alternate
 */
#define DMA_NUM_CH               4U
#define HAL_DMA_ADDR2CH(addr)    (addr & (DMA_NUM_CH - 1))
#define HAL_DMA_CHANNEL_MASK(ch) (1U << (HAL_DMA_ADDR2CH(ch)))

#define HAL_CREATE_DMA_DRIV(OBJ, INST)                                                             \
    __ALIGNED(128) HAL_DMACTRL_T OBJ##_DMA_CFG[DMA_NUM_CH * 2] = {0};                              \
    HAL_DMAXFER_T                OBJ##_DMA_XFER[DMA_NUM_CH]    = {0};                              \
    HAL_DMA_T                    OBJ = HAL_DMA_DRIV(INST, OBJ##_DMA_CFG, OBJ##_DMA_XFER)

#define HAL_DMA_DRIV(INST, CFG, BUF)                                                               \
    {                                                                                              \
        .dev = INST, .active = 0, .pCfg = CFG, .pChXfer = BUF                                      \
    }

HAL_CREATE_DMA_DRIV(DMA_DRIV, DMA);

HAL_ERROR_T HAL_DMA_Enable()
{
    assert(DMA_DRIV.dev != 0);

    /* Store the address of configuration */
    DMA_DRIV.dev->DMA_CTL_BASE_PTR = (uint32_t)(DMA_DRIV.pCfg);
    DMA_DRIV.dev->DMA_CFG |= DMA_DMA_CFG_EN_Msk;

    return HAL_SUCCESS;
}
void HAL_DMA_Disable()
{
    DMA_DRIV.dev->DMA_CFG = 0;
}

void HAL_DMA_StartXfer(const HAL_DMACHADDR_T chAddr)
{
    assert(DMA_DRIV.dev != 0);
    assert(chAddr < 2 * DMA_NUM_CH);

    uint32_t chMask;
    uint32_t chOf = HAL_DMA_ADDR2CH(chAddr);
    if (chAddr < HAL_DMACHADDR_MAX)
    {
        chMask = HAL_DMA_CHANNEL_MASK(HAL_DMA_ADDR2CH(chAddr));
        DMA_DRIV.active |= (1U << chAddr);
        /* Set the current active channels */
        DMA_DRIV.dev->DMA_EN_SET      = chMask;
        DMA_DRIV.pChXfer[chOf].offset = DMA_DRIV.pChXfer[chOf].len;
    }
}

HAL_ERROR_T HAL_DMA_SetupXfer(HAL_DMACHADDR_T chAddr, const HAL_DMAXFER_T *const xfer)
{
    assert(DMA_DRIV.dev != 0);
    assert(xfer != 0);
    assert(chAddr < HAL_DMACHADDR_MAX);

    HAL_ERROR_T    error = HAL_ERROR;
    HAL_DMACTRL_T *cbCtrl;
    uint32_t       chOf = HAL_DMA_ADDR2CH(chAddr);

    if (xfer->len > 0 && chAddr < HAL_DMACHADDR_MAX)
    {
        /* Decode offset */
        DMA_DRIV.pChXfer[chOf].len     = xfer->len;
        DMA_DRIV.pChXfer[chOf].offset  = xfer->offset;
        DMA_DRIV.pChXfer[chOf].control = xfer->control;
        DMA_DRIV.pChXfer[chOf].dir     = xfer->dir;
        DMA_DRIV.pChXfer[chOf].cb      = xfer->cb;
        DMA_DRIV.pChXfer[chOf].cbArg   = xfer->cbArg;
        DMA_DRIV.pChXfer[chOf].pSrc    = xfer->pSrc;
        DMA_DRIV.pChXfer[chOf].pDst    = xfer->pDst;
        /* Copy the data to dma control block */
        cbCtrl          = &DMA_DRIV.pCfg[chAddr];
        cbCtrl->control = xfer->control;
        cbCtrl->control |= (((xfer->len - 1) << HAL_DMACHCTRL_NM1_Offset) & HAL_DMACHCTRL_NM1_Mask);
        if (xfer->dir == HAL_DMAXFERDIR_M2M)
        {
            cbCtrl->pDstEnd = (uint32_t)(xfer->pDst + xfer->len - 1);
            cbCtrl->pSrcEnd = (uint32_t)(xfer->pSrc + xfer->len - 1);
            /* Allow peripherals to request dma data */
            DMA_DRIV.dev->DMA_REQ_MASK_CLR = HAL_DMA_CHANNEL_MASK(chAddr);
            /* Start the software request */
            DMA_DRIV.dev->DMA_SW_REQ = HAL_DMA_CHANNEL_MASK(chAddr);
            /* Not supported for the device */
            error = HAL_ERROR;
        }
        else
        {
            if (xfer->dir == HAL_DMAXFERDIR_P2M)
            {
                cbCtrl->pDstEnd = (uint32_t)(xfer->pDst + xfer->len - 1);
                cbCtrl->pSrcEnd = (uint32_t)xfer->pSrc;
                DMA_DRIV.dev->DMA_INT_EN |= HAL_DMA_CHANNEL_MASK(chAddr);
                error = HAL_SUCCESS;
            }
            else if (xfer->dir == HAL_DMAXFERDIR_M2P)
            {
                cbCtrl->pDstEnd = (uint32_t)xfer->pDst;
                cbCtrl->pSrcEnd = (uint32_t)(xfer->pSrc + xfer->len - 1);
                DMA_DRIV.dev->DMA_INT_EN |= HAL_DMA_CHANNEL_MASK(chAddr);
                error = HAL_SUCCESS;
            }
        }
    }
    return error;
}

uint32_t HAL_DMA_XferCount(HAL_DMACHADDR_T chAddr)
{
    assert(chAddr < HAL_DMACHADDR_MAX);

    uint32_t n_minus_1 = 0;
    if (chAddr < HAL_DMACHADDR_MAX)
    {
        if (DMA_DRIV.pCfg[chAddr].control & HAL_DMACHCTRL_CYCCTRL_Mask)
        {
            n_minus_1 = (DMA_DRIV.pCfg[chAddr].control & HAL_DMACHCTRL_NM1_Mask)
                        >> HAL_DMACHCTRL_NM1_Offset;
            return n_minus_1 + 1;
        }
    }
    return n_minus_1;
}

void HAL_DMA_StopXfer(HAL_DMACHADDR_T chAddr)
{
    assert(DMA_DRIV.dev != 0);
    assert(chAddr < HAL_DMACHADDR_MAX);

    uint32_t chOf, chMask;
    if (chAddr < HAL_DMACHADDR_MAX)
    {
        chOf   = HAL_DMA_ADDR2CH(chAddr);
        chMask = HAL_DMA_CHANNEL_MASK(chOf);
        /* Disable the channel first */
        DMA_DRIV.dev->DMA_INT_EN &= ~chMask;
        DMA_DRIV.dev->DMA_EN_CLR = chMask;
        DMA_DRIV.active &= ~(1U << chAddr);
    }
}

static void HAL_DMA_ErrorHandler(HAL_DMACHADDR_T chAddr)
{
    assert(DMA_DRIV.dev != 0);
    assert(chAddr < HAL_DMACHADDR_MAX);

    uint32_t       chOf;
    HAL_DMAXFER_T *chXfer;
    if (chAddr < HAL_DMACHADDR_MAX)
    {
        chOf   = HAL_DMA_ADDR2CH(chAddr);
        chXfer = &DMA_DRIV.pChXfer[chOf];
        if (chXfer->cb)
        {
            chXfer->cb(chXfer->cbArg, HAL_ERROR);
        }
        DMA_DRIV.dev->DMA_INT_EN &= ~HAL_DMA_CHANNEL_MASK(chOf);
    }
}

static void HAL_DMA_DoneHandler(HAL_DMACHADDR_T chAddr)
{
    assert(DMA_DRIV.dev != 0);
    assert(chAddr < HAL_DMACHADDR_MAX);

    uint32_t       chOf;
    HAL_DMAXFER_T *chXfer;
    if (chAddr < HAL_DMACHADDR_MAX)
    {
        chOf   = HAL_DMA_ADDR2CH(chAddr);
        chXfer = &DMA_DRIV.pChXfer[chOf];
        if (chXfer->cb)
        {
            chXfer->cb(chXfer->cbArg, HAL_SUCCESS);
        }
        DMA_DRIV.dev->DMA_INT_EN &= ~HAL_DMA_CHANNEL_MASK(chOf);
    }
}

void HAL_DMA_DONE_IRQHandler()
{
    assert(DMA_DRIV.dev != 0);

    uint32_t status = DMA->DMA_INT_STS;
    if (DMA->DMA_INT_STS & 0x1)
    {
        HAL_DMA_DoneHandler(0);
    }
    if (DMA->DMA_INT_STS & 0x2)
    {
        HAL_DMA_DoneHandler(1);
    }
    if (DMA->DMA_INT_STS & 0x4)
    {
        HAL_DMA_DoneHandler(2);
    }
    if (DMA->DMA_INT_STS & 0x8)
    {
        HAL_DMA_DoneHandler(3);
    }

    DMA->DMA_INT_STS = status;
}

void HAL_DMA_ERROR_IRQHandler()
{
    assert(DMA_DRIV.dev != 0);

    uint32_t status = DMA->DMA_INT_STS;
    if (DMA->DMA_INT_STS & 0x1)
    {
        HAL_DMA_ErrorHandler(0);
    }
    if (DMA->DMA_INT_STS & 0x2)
    {
        HAL_DMA_ErrorHandler(1);
    }
    if (DMA->DMA_INT_STS & 0x4)
    {
        HAL_DMA_ErrorHandler(2);
    }
    if (DMA->DMA_INT_STS & 0x8)
    {
        HAL_DMA_ErrorHandler(3);
    }
    DMA->DMA_INT_STS = status;
}
#endif /* HAL_USE_DMA */
