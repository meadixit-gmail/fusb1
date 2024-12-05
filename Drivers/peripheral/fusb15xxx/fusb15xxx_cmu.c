/**
 * @file     fusb15xxx_cmu.c
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

#if HAL_USE_CMU
static void HAL_CMU_Clock_Select(CMU_T *d, uint32_t clk_sel)
{
    assert(d != 0);
    uint32_t cmu_cfg = d->CLK_CFG;
    if (clk_sel == Clock_Management_Unit_Select_LS_Clock_Osc)
    {
        cmu_cfg &= ~Clock_Management_Unit_CLK_CFG_HS_OSC_EN_Msk;
        cmu_cfg |= (Clock_Management_Unit_CLK_CFG_LS_OSC_EN_Msk
                    | Clock_Management_Unit_CLK_CFG_CLK_SEL_Msk);
    }
    else
    {
        cmu_cfg &= ~Clock_Management_Unit_CLK_CFG_LS_OSC_EN_Msk;
        cmu_cfg |= Clock_Management_Unit_CLK_CFG_HS_OSC_EN_Msk;
        cmu_cfg &= ~Clock_Management_Unit_CLK_CFG_CLK_SEL_Msk;
    }
    d->CLK_CFG = cmu_cfg;
}

HAL_CMU_DRIVER_T CMU_DRIVER = {
    .ClockSelect = HAL_CMU_Clock_Select,
};

#endif
