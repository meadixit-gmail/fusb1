/**
 * @file     fusb15xxx_usbchg.c
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

#ifdef HAL_USE_USBCHG

typedef struct _HAL_USBCHG
{
    USBCHG_T *device; ///< Register block
} HAL_USBCHG_T;

static HAL_USBCHG_T usbchgdrivMemPool[TYPE_C_PORT_COUNT] = {0};

HAL_USBCHG_T *USBCHG_Init(HAL_USBCHG_CHID_T usbchg_id)
{
    HAL_USBCHG_T *d = &usbchgdrivMemPool[usbchg_id];
#if (DEVICE_TYPE == FUSB15101)
    d->device              = Legacy_DP_DM_Protocol_Port;
    d->device->USBCHG_CTRL = Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_CHG_Msk
                             | Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_RDCP_Msk
                             | Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_BC12_SW_Msk;
#else
    if (usbchg_id == HAL_USBCHG_A)
    {
        d->device              = Legacy_DP_DM_Protocol_Port_A;
        d->device->USBCHG_CTRL = Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_CHG_Msk
                                 | Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_BC12_SW_Msk;
    }
    else
    {
        d->device              = Legacy_DP_DM_Protocol_Port_B;
        d->device->USBCHG_CTRL = Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_CHG_Msk
                                 | Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_BC12_SW_Msk;
    }
#endif
    return d;
}

static void USBCHG_Disable(HAL_USBCHG_T *d)
{
    d->device->USBCHG_CTRL = 0U;
}

static void USBCHG_Switch(HAL_USBCHG_T *d, bool en)
{
    if (en)
    {
        d->device->USBCHG_CTRL |= Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_BC12_SW_Msk;
    }
    else
    {
        d->device->USBCHG_CTRL &= ~Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_BC12_SW_Msk;
    }
}

static void USBCHG_HostSwitch(HAL_USBCHG_T *d, bool en)
{
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
    if (en)
    {
        d->device->USBCHG_CTRL |= Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_HOST_SW_Msk;
    }
    else
    {
        d->device->USBCHG_CTRL &= ~Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_HOST_SW_Msk;
    }
#elif (DEVICE_TYPE == FUSB15201P)

#endif
}

static void USBCHG_SetRdiv(HAL_USBCHG_T *d, bool en)
{
    if (en)
    {
        d->device->USBCHG_CTRL |= Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_RDIV_Msk;
    }
    else
    {
        d->device->USBCHG_CTRL &= ~Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_RDIV_Msk;
    }
}

static void USBCHG_SetIdpSrc(HAL_USBCHG_T *d, bool en)
{
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
    if (en)
    {
        d->device->USBCHG_CTRL |= Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_IDP_SRC_Msk;
#ifdef FUSB15200_REVA
        if ((HPDA->CONFIG & HPD_CONFIG_EN_Mask) == 0)
        {
            HPDA->CONFIG |= HPD_CONFIG_EN_Enable;
        }
#endif
    }
    else
    {
        d->device->USBCHG_CTRL &= ~Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_IDP_SRC_Msk;
    }
#elif (DEVICE_TYPE == FUSB15201P)
    USBCHG_SetRdiv(d, true);
#endif
}

static void USBCHG_SetVdpSrc(HAL_USBCHG_T *d, bool en)
{
    if (en)
    {
        d->device->USBCHG_CTRL |= (Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_VDx_SRC_DP
                                   << Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_VDx_SRC_Pos)
                                  & Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_VDx_SRC_Msk;
    }
    else
    {
        d->device->USBCHG_CTRL &= ~(Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_VDx_SRC_DP
                                    << Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_VDx_SRC_Pos)
                                  & Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_VDx_SRC_Msk;
    }
}

static void USBCHG_SetVdmSrc(HAL_USBCHG_T *d, bool en)
{
    if (en)
    {
        d->device->USBCHG_CTRL |= (Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_VDx_SRC_DM
                                   << Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_VDx_SRC_Pos)
                                  & Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_VDx_SRC_Msk;
    }
    else
    {
        d->device->USBCHG_CTRL &= ~(Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_VDx_SRC_DM
                                    << Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_VDx_SRC_Pos)
                                  & Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_VDx_SRC_Msk;
    }
}

static void USBCHG_SetIdpSnk(HAL_USBCHG_T *d, bool en)
{
    if (en)
    {
        d->device->USBCHG_CTRL |= (Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_IDx_SNK_DP
                                   << Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_IDx_SNK_Pos)
                                  & Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_IDx_SNK_Msk;
    }
    else
    {
        d->device->USBCHG_CTRL &= ~(Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_IDx_SNK_DP
                                    << Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_IDx_SNK_Pos)
                                  & Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_IDx_SNK_Msk;
    }
}

static void USBCHG_SetIdmSnk(HAL_USBCHG_T *d, bool en)
{
    if (en)
    {
        d->device->USBCHG_CTRL |= (Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_IDx_SNK_DM
                                   << Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_IDx_SNK_Pos)
                                  & Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_IDx_SNK_Msk;
    }
    else
    {
        d->device->USBCHG_CTRL &= ~(Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_IDx_SNK_DM
                                    << Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_IDx_SNK_Pos)
                                  & Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_IDx_SNK_Msk;
    }
}

static void USBCHG_SetRdpDwn(HAL_USBCHG_T *d, bool en)
{
    if (en)
    {
        d->device->USBCHG_CTRL |= Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_RDP_DWN_Msk;
    }
    else
    {
        d->device->USBCHG_CTRL &= ~Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_RDP_DWN_Msk;
    }
}

static void USBCHG_SetRdmDwn(HAL_USBCHG_T *d, bool en)
{
    if (en)
    {
        d->device->USBCHG_CTRL |= Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_RDM_DWN_Msk;
    }
    else
    {
        d->device->USBCHG_CTRL &= ~Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_RDM_DWN_Msk;
    }
}

static void USBCHG_SetRdcp(HAL_USBCHG_T *d, bool en)
{
    if (en)
    {
        d->device->USBCHG_CTRL |= Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_RDCP_Msk;
    }
    else
    {
        d->device->USBCHG_CTRL &= ~Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_RDCP_Msk;
    }
}

static void USBCHG_SetDpLkg(HAL_USBCHG_T *d, bool en)
{
    if (en)
    {
        d->device->USBCHG_CTRL |= Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_RDAT_LKG_P_Msk;
    }
    else
    {
        d->device->USBCHG_CTRL &= ~Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_RDAT_LKG_P_Msk;
    }
}

static void USBCHG_SetDmLkg(HAL_USBCHG_T *d, bool en)
{
    if (en)
    {
        d->device->USBCHG_CTRL |= Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_RDAT_LKG_M_Msk;
    }
    else
    {
        d->device->USBCHG_CTRL &= ~Legacy_DP_DM_Protocol_Port_USBCHG_CTRL_EN_RDAT_LKG_M_Msk;
    }
}

static void USBCHG_MoisDtct(HAL_USBCHG_T *d, bool en)
{
    if (en)
    {
        d->device->USB_PROT_CTRL |= Legacy_DP_DM_Protocol_Port_USB_PROT_CTRL_EN_USB_MOIS_Msk;
    }
    else
    {
        d->device->USB_PROT_CTRL &= ~Legacy_DP_DM_Protocol_Port_USB_PROT_CTRL_EN_USB_MOIS_Msk;
    }
}

static void USBCHG_MoisRp(HAL_USBCHG_T                                             *d,
                          Legacy_DP_DM_Protocol_Port_USB_PROT_CTRL_EN_RPMOS_HI_Enum rp)
{
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)

#elif (DEVICE_TYPE == FUSB15201P) || (DEVICE_TYPE == FUSB15101)
    if (rp == Legacy_DP_DM_Protocol_Port_USB_PROT_CTRL_EN_RPMOS_HI_RPU_MOS_LO)
    {
        d->device->USB_PROT_CTRL &= ~Legacy_DP_DM_Protocol_Port_USB_PROT_CTRL_EN_RPMOS_HI_Msk;
    }
    else
    {
        d->device->USB_PROT_CTRL |= Legacy_DP_DM_Protocol_Port_USB_PROT_CTRL_EN_RPMOS_HI_Msk;
    }
#endif
}

static void USBCHG_OvpEnable(HAL_USBCHG_T *d, bool en)
{
    if (en)
    {
        d->device->USB_PROT_CTRL |= Legacy_DP_DM_Protocol_Port_USB_PROT_CTRL_EN_USB_OVP_Msk;
        ;
    }
    else
    {
        d->device->USB_PROT_CTRL &= ~Legacy_DP_DM_Protocol_Port_USB_PROT_CTRL_EN_USB_OVP_Msk;
    }
}

static void USBCHG_OVP_Interrupt_Enable(HAL_USBCHG_T *d, bool en)
{
    if (en)
    {
        d->device->USBOVP_MSK &= ~Legacy_DP_DM_Protocol_Port_USBOVP_MSK_M_USB_OVP_Msk;
    }
    else
    {
        d->device->USBOVP_MSK |= Legacy_DP_DM_Protocol_Port_USBOVP_MSK_M_USB_OVP_Msk;
    }
}

static bool USBCHG_OVP_Interrupt_Status(HAL_USBCHG_T *d)
{
    if (d->device->USBOVP_INT & Legacy_DP_DM_Protocol_Port_USBOVP_INT_I_USB_OVP_Msk)
    {
        return true;
    }
    return false;
}

static void USBCHG_OVP_Interrupt_Clear(HAL_USBCHG_T *d)
{
    d->device->USBOVP_INT = Legacy_DP_DM_Protocol_Port_USBOVP_INT_I_USB_OVP_Msk;
}

static void USBCHG_OVP_Debounce_Interrupt_Enable(HAL_USBCHG_T *d, bool en)
{
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)

#elif (DEVICE_TYPE == FUSB15201P) || (DEVICE_TYPE == FUSB15101)
    if (en)
    {
        d->device->USBOVP_MSK &= ~Legacy_DP_DM_Protocol_Port_USBOVP_MSK_M_OVP_DBNC_DONE_Msk;
    }
    else
    {
        d->device->USBOVP_MSK |= Legacy_DP_DM_Protocol_Port_USBOVP_MSK_M_OVP_DBNC_DONE_Msk;
    }
#endif
}

static bool USBCHG_OVP_Debounce_Interrupt_Status(HAL_USBCHG_T *d)
{
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)

#elif (DEVICE_TYPE == FUSB15201P) || (DEVICE_TYPE == FUSB15101)
    if (d->device->USBOVP_INT & Legacy_DP_DM_Protocol_Port_USBOVP_INT_I_OVP_DBNC_DONE_Msk)
    {
        return true;
    }
    return false;
#endif
}

static void USBCHG_OVP_Debounce_Interrupt_Clear(HAL_USBCHG_T *d)
{
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)

#elif (DEVICE_TYPE == FUSB15201P) || (DEVICE_TYPE == FUSB15101)
    d->device->USBOVP_INT = Legacy_DP_DM_Protocol_Port_USBOVP_INT_I_OVP_DBNC_DONE_Msk;
#endif
}

#if (DEVICE_TYPE == FUSB15101)
static void USBCHG_DP_DetachEnable(HAL_USBCHG_T                                               *d,
                                   Legacy_DP_DM_Protocol_Port_USB_PROT_CTRL_DPDETACH_DBNC_Enum dbnc)
{
    uint32_t prot_ctrl = d->device->USB_PROT_CTRL;
    prot_ctrl &= ~Legacy_DP_DM_Protocol_Port_USB_PROT_CTRL_DPDETACH_DBNC_Msk;
    prot_ctrl |= Legacy_DP_DM_Protocol_Port_USB_PROT_CTRL_EN_DPDETACH_Msk;
    prot_ctrl |= (dbnc << Legacy_DP_DM_Protocol_Port_USB_PROT_CTRL_DPDETACH_DBNC_Pos)
                 & Legacy_DP_DM_Protocol_Port_USB_PROT_CTRL_DPDETACH_DBNC_Msk;
    d->device->USB_PROT_CTRL = prot_ctrl;
}

static void USBCHG_DP_DetachInterruptEnable(HAL_USBCHG_T *d, bool en)
{
    if (en)
    {
        d->device->USBOVP_MSK &= ~Legacy_DP_DM_Protocol_Port_USBOVP_MSK_M_DPDETACH_Msk;
    }
    else
    {
        d->device->USBOVP_MSK |= Legacy_DP_DM_Protocol_Port_USBOVP_MSK_M_DPDETACH_Msk;
    }
}

static bool USBCHG_DP_DetachInterruptStatus(HAL_USBCHG_T *d)
{
    if (d->device->USBOVP_INT & Legacy_DP_DM_Protocol_Port_USBOVP_INT_I_DPDETACH_Msk)
    {
        return true;
    }
    return false;
}

static void USBCHG_DP_DetachInterruptClear(HAL_USBCHG_T *d)
{
    d->device->USBOVP_INT = Legacy_DP_DM_Protocol_Port_USBOVP_INT_I_DPDETACH_Msk;
}

static bool USBCHG_DP_DetachedStatus(HAL_USBCHG_T *d)
{
    if (d->device->USB_STAT & Legacy_DP_DM_Protocol_Port_USB_STAT_DPDETACH_STAT_Msk)
    {
        return true;
    }
    return false;
}

static void USBCHG_DP_DetachedDisable(HAL_USBCHG_T *d)
{
    d->device->USB_PROT_CTRL &= ~Legacy_DP_DM_Protocol_Port_USB_PROT_CTRL_EN_DPDETACH_Msk;
}
#endif

static bool USBCHG_SwitchStatus(HAL_USBCHG_T *d)
{
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)

#elif (DEVICE_TYPE == FUSB15201P) || (DEVICE_TYPE == FUSB15101)
    return d->device->USB_STAT & Legacy_DP_DM_Protocol_Port_USB_STAT_BC12_SW_STAT_Msk ? true :
                                                                                        false;
#endif
}

HAL_USBCHG_DRIVER_T USBCHG_DRIVER = {
    .Enable                     = USBCHG_Init,
    .Disable                    = USBCHG_Disable,
    .Switch                     = USBCHG_Switch,
    .HostSwitchEnable           = USBCHG_HostSwitch,
    .SetIdpSource               = USBCHG_SetIdpSrc,
    .SetVdpSource               = USBCHG_SetVdpSrc,
    .SetVdmSource               = USBCHG_SetVdmSrc,
    .SetIdpSink                 = USBCHG_SetIdpSnk,
    .SetIdmSink                 = USBCHG_SetIdmSnk,
    .SetRdpDown                 = USBCHG_SetRdpDwn,
    .SetRdmDown                 = USBCHG_SetRdmDwn,
    .SetRdiv                    = USBCHG_SetRdiv,
    .SetRdcp                    = USBCHG_SetRdcp,
    .SetRdpLeakage              = USBCHG_SetDpLkg,
    .SetRdmLeakage              = USBCHG_SetDmLkg,
    .SetMoistureDetection       = USBCHG_MoisDtct,
    .SetMoisturePullup          = USBCHG_MoisRp,
    .OVPEnable                  = USBCHG_OvpEnable,
    .OVPInterruptEnable         = USBCHG_OVP_Interrupt_Enable,
    .OVPInterruptStatus         = USBCHG_OVP_Interrupt_Status,
    .OVPInterruptClear          = USBCHG_OVP_Interrupt_Clear,
    .OVPDebounceInterruptEnable = USBCHG_OVP_Debounce_Interrupt_Enable,
    .OVPDebounceInterruptStatus = USBCHG_OVP_Debounce_Interrupt_Status,
    .OVPDebounceInterruptClear  = USBCHG_OVP_Debounce_Interrupt_Clear,
    .SwitchStatus               = USBCHG_SwitchStatus,
#if (DEVICE_TYPE == FUSB15101)
    .DPDetachEnable          = USBCHG_DP_DetachEnable,
    .DPDetachInterruptEnable = USBCHG_DP_DetachInterruptEnable,
    .DPDetachInterruptStatus = USBCHG_DP_DetachInterruptStatus,
    .DPDetachInterruptClear  = USBCHG_DP_DetachInterruptClear,
    .DPDetachStatus          = USBCHG_DP_DetachedStatus,
    .DPDetachDisable         = USBCHG_DP_DetachedDisable
#endif
};

#endif /* HAL_USE_USBCHG */
