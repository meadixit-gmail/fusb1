/**
 * @file     fusb15xxx_tcpd.c
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
#if HAL_USE_TCPD
/******************************************************************************
 * Private type defines
 ******************************************************************************/
#define PORT_EVENTS_MASK (USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_otp_Msk)

#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
#define PD_EVENTS_MASK                                                                             \
    (USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_rx_discard_Msk                                        \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_tx_buferror_Msk                                     \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_pd_activity_Msk                                     \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_tx_success_Msk                                      \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_tx_discard_Msk                                      \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_tx_fail_Msk                                         \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_tx_retry_Msk                                        \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_tx_buf_rdy_Msk                                      \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_rx_cbl_rst_Msk                                      \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_rx_hard_rst_Msk                                     \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_rx_overflow_Msk                                     \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_rx_badcrc_Msk                                       \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_rx_success_Msk                                      \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_rx_buf_rdy_Msk                                      \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_fr_swap_det_Msk)

#define VBUS_EVENTS_MASK                                                                           \
    (USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_vSafe0V_chg_Msk                                       \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_vVBUS_VAL_change_Msk)

#define CC_EVENTS_MASK                                                                             \
    (USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_cc1_chg_Msk                                           \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_cc2_chg_Msk                                         \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_drp_change_Msk                                      \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_vconn_ocp_Msk                                       \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_cc_ovp_Msk)

#elif (DEVICE_TYPE == FUSB15101)
#define PD_EVENTS_MASK                                                                             \
    (USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_rx_discard_Msk                                        \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_tx_buferror_Msk                                     \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_pd_activity_Msk                                     \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_tx_success_Msk                                      \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_tx_discard_Msk                                      \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_tx_fail_Msk                                         \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_tx_retry_Msk                                        \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_tx_buf_rdy_Msk                                      \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_rx_cbl_rst_Msk                                      \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_rx_hard_rst_Msk                                     \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_rx_overflow_Msk                                     \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_rx_badcrc_Msk                                       \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_rx_success_Msk                                      \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_rx_buf_rdy_Msk)

#define VBUS_EVENTS_MASK (USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_vSafe0V_chg_Msk)

#define CC_EVENTS_MASK                                                                             \
    (USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_cc1_chg_Msk                                           \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_cc2_chg_Msk                                         \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_vconn_ocp_Msk                                       \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_cc_ovp_Msk)
#else
#define PD_EVENTS_MASK                                                                             \
    (USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_rx_discard_Msk                                        \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_tx_buferror_Msk                                     \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_pd_activity_Msk                                     \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_tx_success_Msk                                      \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_tx_discard_Msk                                      \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_tx_fail_Msk                                         \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_tx_retry_Msk                                        \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_tx_buf_rdy_Msk                                      \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_rx_cbl_rst_Msk                                      \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_rx_hard_rst_Msk                                     \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_rx_overflow_Msk                                     \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_rx_badcrc_Msk                                       \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_rx_success_Msk                                      \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_rx_buf_rdy_Msk)

#define CC_EVENTS_MASK                                                                             \
    (USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_cc1_chg_Msk                                           \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_cc2_chg_Msk                                         \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_vconn_ocp_Msk                                       \
     | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_cc_ovp_Msk)

#endif

#define HAL_SOP_TOKEN        0
#define HAL_SOP_P_TOKEN      1
#define HAL_SOP_PP_TOKEN     2
#define HAL_SOP_DBG_P_TOKEN  3
#define HAL_SOP_DBG_PP_TOKEN 4

/* Union representing the Tx block that is used by the 152xx PD peripheral */
typedef union
{
    uint32_t object;
    uint16_t word[2];
    uint8_t  byte[4];
    struct
    {
        unsigned msg   :5;
        unsigned rsvd0 :7;
        unsigned len   :3;
        unsigned rsvd1 :1;
        unsigned sop   :3;
        unsigned rsvd  :1;
        unsigned token :12;
    } rx;
} PKT_T;
STATIC_ASSERT(sizeof(PKT_T) == 4, PKT_T);

#ifndef TX_BUF_LEN
#define TX_BUF_LEN 7
#endif

#ifndef RX_BUF_LEN
#define RX_BUF_LEN 16
/* Calculate idx mod 16*/
#define RX_BUF_IDX(idx) ((idx) & (RX_BUF_LEN - 1))
#endif

#define RX_BUF_IDX_INC(idx)                                                                        \
    do                                                                                             \
    {                                                                                              \
        idx = RX_BUF_IDX(idx + 1);                                                                 \
    } while (0)

typedef struct _HAL_USBPD
{
    TCPD_T          *device; ///< Register block
    HAL_USBPD_CHID_T ch_num; ///< Channel Number
    HAL_ADCHx_T      vbus_ch;
    HAL_ADCHx_T      vbus_pps_ch;
    HAL_PORTSTAT_T   stat; ///< Port Status
    HAL_PDSTAT_T     pd_stat;
    HAL_CCSTAT_T     cc_stat;
    HAL_VBUSSTAT_T   vbus_stat;
#if (DEVICE_TYPE == FUSB15101)
    HAL_VBUS_T *vbus;
#endif

    /* Rx Queue */
    struct
    {
        uint32_t buf32[RX_BUF_LEN];
        uint32_t w_idx; /* Write Index */
        uint32_t r_idx; /* Read Index */
        uint32_t w_rst; /* write reset */
    } RxQ;

    /* Tx Queue */
    struct
    {
        uint32_t buf32[TX_BUF_LEN];
        uint32_t w_idx; /* Write Index */
        uint32_t r_idx; /* Read Index */
        uint32_t r_rst; /* read reset */
    } TxQ;
} HAL_USBPD_T;

static HAL_USBPD_T tcpddrivMemPool[TYPE_C_PORT_COUNT] = {0};

/******************************************************************************
 * Driver API definition
 ******************************************************************************/
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201) || (DEVICE_TYPE == FUSB15101)
HAL_USBPD_T *HAL_USBPD_Init(HAL_USBPD_CHID_T id, bool src_snk_enable, bool reinit)
{
    assert(id < HAL_USBPD_CH_COUNT);
    HAL_USBPD_T *d = &tcpddrivMemPool[id];

    HAL_PORTCFG_T portCfg = {.an  = PORT_DIGITAL,
                             .alt = PORT_ALTERNATE,
                             .pu  = PORT_PULLUP_DISABLE,
                             .pd  = PORT_PULLDOWN_DISABLE};
    /* Set the pointer to the correct channel instance */
    if (id == HAL_USBPD_CH_A)
    {
#if (DEVICE_TYPE != FUSB15101)
        d->vbus_ch = ADC_CH0;
#else
        d->vbus_pps_ch = ADC_CH0;
        d->vbus_ch = ADC_CH1;
#endif
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
        d->device = (TCPD_T *)USB_Type_C_PD_Port_A;

        if (src_snk_enable)
        {
            PORT_DRIVER.Configure(PORT1, &portCfg, HAL_GPIO_SHARED);
            PORT_DRIVER.Configure(PORT20, &portCfg, HAL_GPIO_SHARED);
        }
        NVIC_SetPriority(USBPDA_IRQn, 0);
        NVIC_EnableIRQ(USBPDA_IRQn);
#else
        d->device  = USB_Type_C_PD_Port;
        NVIC_SetPriority(USBPD_IRQn, 0);
        NVIC_EnableIRQ(USBPD_IRQn);
#endif
    }

    else
    {
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
        d->device  = (TCPD_T *)USB_Type_C_PD_Port_B;
        d->vbus_ch = ADC_CH6;
        if (src_snk_enable)
        {
            PORT_DRIVER.Configure(PORT14, &portCfg, HAL_GPIO_SHARED);
            PORT_DRIVER.Configure(PORT15, &portCfg, HAL_GPIO_SHARED);
        }
        NVIC_SetPriority(USBPDB_IRQn, 0);
        NVIC_EnableIRQ(USBPDB_IRQn);
#endif
    }

    if (d)
    {
        d->ch_num = id;
        if (!reinit)
        {
			/* Mask All events by default*/
			d->device->USB_INTERRUPT_MASK = ~0U;
			/* Clear all events */
			d->device->USB_INTERRUPT = ~0U;
        }
        /* Reset status */
        d->cc_stat.vconn_ocp = 0;
        d->stat.otp          = 0;

        /* Enable Type-C peripheral */
        d->device->USBC_CTRL |=
            USB_Type_C_PD_Port_USBC_CTRL_USB_EN_Msk | USB_Type_C_PD_Port_USBC_CTRL_CC_OVP_EN_Msk;
    }
#if (DEVICE_TYPE == FUSB15101)
    d->vbus = VBUS_DRIVER.Initialize(id);
#endif
    return d;
}
#if (DEVICE_TYPE == FUSB15101)
static HAL_VBUS_T *HAL_PORT_VbusReference(HAL_USBPD_T *d)
{
    return d->vbus;
}

static void HAL_PORT_VbusEnable(HAL_USBPD_T *d, HAL_VBUSIO_T io)
{
    assert(d != 0 && d->device != 0);

    HAL_VBUS_CFG_T vbus;
    vbus.value         = 0;
    vbus.lgate_en_mask = 1;
    if (io == HAL_VBUSIO_SOURCE)
    {
        VBUS_DRIVER.SetFlags(d->vbus, vbus);
        VBUS_DRIVER.Set(d->vbus);
    }
    else
    {
        VBUS_DRIVER.Clear(d->vbus);
    }
}
#endif

#else
HAL_USBPD_T *HAL_USBPD_Init(HAL_USBPD_CHID_T id, bool src_snk_enable, bool reinit)
{
    assert(id < HAL_USBPD_CH_COUNT);
    HAL_USBPD_T *d = &tcpddrivMemPool[id];

    HAL_PORTCFG_T portCfg       = {.an  = PORT_DIGITAL,
                                   .alt = PORT_ALTERNATE,
                                   .pu  = PORT_PULLUP_DISABLE,
                                   .pd  = PORT_PULLDOWN_DISABLE};
    HAL_GPIOCFG_T outGPIOConfig = {
        .iodir = HAL_GPIO_OUTPUT,
        .ppod  = HAL_GPIO_PUSH_PULL,

    };

    /* Set the pointer to the correct channel instance */
    if (id == HAL_USBPD_CH_A)
    {
        d->device  = USB_Type_C_PD_Port_A;
        d->vbus_ch = ADC_CH0;
        NVIC_SetPriority(USBPDA_IRQn, 0);
        NVIC_EnableIRQ(USBPDA_IRQn);
    }
    else
    {
        d->device  = USB_Type_C_PD_Port_B;
        d->vbus_ch = ADC_CH4;
        NVIC_SetPriority(USBPDB_IRQn, 0);
        NVIC_EnableIRQ(USBPDB_IRQn);
    }

    if (d)
    {
        d->ch_num = id;
        if (!reinit)
        {
			/* Mask All events by default*/
			d->device->USB_INTERRUPT_MASK = ~0U;
			/* Clear all events */
			d->device->USB_INTERRUPT = ~0U;
        }
        /* Reset status */
        d->cc_stat.vconn_ocp = 0;
        d->stat.otp          = 0;

        /* Enable Type-C peripheral */
        d->device->USBC_CTRL |=
            USB_Type_C_PD_Port_USBC_CTRL_USB_EN_Msk | USB_Type_C_PD_Port_USBC_CTRL_CC_OVP_EN_Msk;
    }
    return d;
}
#endif

static void HAL_USBPD_Enable(HAL_USBPD_T *d)
{
    assert(d != 0 && d->device != 0);

/* Enable Type-C peripheral */
#if (DEVICE_TYPE == FUSB15101)
    d->device->USBC_CTRL |=
        USB_Type_C_PD_Port_USBC_CTRL_USB_EN_Msk | USB_Type_C_PD_Port_USBC_CTRL_CC_OVP_EN_Msk;
#else
    d->device->USBC_CTRL |=
        USB_Type_C_PD_Port_USBC_CTRL_USB_EN_Msk | USB_Type_C_PD_Port_USBC_CTRL_CC_OVP_EN_Msk;
#endif
}

static void HAL_USBPD_DeInit(HAL_USBPD_T *d)
{
    assert(d != 0 && d->device != 0);

/* Enable Type-C peripheral */
#if (DEVICE_TYPE == FUSB15101)
    d->device->USBC_CTRL &=
        ~(USB_Type_C_PD_Port_USBC_CTRL_USB_EN_Msk | USB_Type_C_PD_Port_USBC_CTRL_CC_OVP_EN_Msk);

#else

    d->device->USBC_CTRL &=
        ~(USB_Type_C_PD_Port_USBC_CTRL_USB_EN_Msk | USB_Type_C_PD_Port_USBC_CTRL_CC_OVP_EN_Msk);
#endif
}

static HAL_PORTSTAT_T *HAL_USBPD_PortStatus(HAL_USBPD_T *d)
{
    assert(d != 0 && d->device != 0);
    if (d->device->USBC_STS & USB_Type_C_PD_Port_USBC_STS_OTP_STS_Msk)
    {
        d->stat.otp = 1;
    }
    else
    {
        d->stat.otp = 0;
    }
    return &d->stat;
}

static void HAL_PORT_SetTerm(HAL_USBPD_T *d, int cc, HAL_CC_TERM_T term)
{
    assert(d != 0 && d->device != 0);

    uint32_t value;
    value = d->device->USBC_CTRL;
    if (cc & HAL_CC1)
    {
        value &= ~USB_Type_C_PD_Port_USBC_CTRL_CC1_TERM_Msk;
        value |= (term << USB_Type_C_PD_Port_USBC_CTRL_CC1_TERM_Pos)
                 & USB_Type_C_PD_Port_USBC_CTRL_CC1_TERM_Msk;
    }
    if (cc & HAL_CC2)
    {
        value &= ~USB_Type_C_PD_Port_USBC_CTRL_CC2_TERM_Msk;
        value |= (term << USB_Type_C_PD_Port_USBC_CTRL_CC2_TERM_Pos)
                 & USB_Type_C_PD_Port_USBC_CTRL_CC2_TERM_Msk;
    }
    d->device->USBC_CTRL = value;
}

static void HAL_PORT_SetCC(HAL_USBPD_T *d, int cc, HAL_CC_TERM_T term, HAL_CC_RP_T rp,
                           HAL_DRP_MODE_T drp_mode)
{
    assert(d != 0 && d->device != 0);
    uint32_t value;
    value = d->device->USBC_CTRL;
    if (cc & HAL_CC1)
    {
        value &= ~USB_Type_C_PD_Port_USBC_CTRL_CC1_TERM_Msk;
        value |= (term << USB_Type_C_PD_Port_USBC_CTRL_CC1_TERM_Pos)
                 & USB_Type_C_PD_Port_USBC_CTRL_CC1_TERM_Msk;
    }
    if (cc & HAL_CC2)
    {
        value &= ~USB_Type_C_PD_Port_USBC_CTRL_CC2_TERM_Msk;
        value |= (term << USB_Type_C_PD_Port_USBC_CTRL_CC2_TERM_Pos)
                 & USB_Type_C_PD_Port_USBC_CTRL_CC2_TERM_Msk;
    }
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
    if (drp_mode != HAL_DRP_MODE_NOCHANGE)
    {
        value &=
            ~(USB_Type_C_PD_Port_USBC_CTRL_DRP_EN_Msk | USB_Type_C_PD_Port_USBC_CTRL_DRP_MODE_Msk);
        if (drp_mode != HAL_DRP_MODE_DISABLED)
        {
            value |= USB_Type_C_PD_Port_USBC_CTRL_DRP_EN_Msk
                     | ((drp_mode << USB_Type_C_PD_Port_USBC_CTRL_DRP_MODE_Pos)
                        & USB_Type_C_PD_Port_USBC_CTRL_DRP_MODE_Msk);
        }
    }
#endif

    if (rp != HAL_CC_RP_NOCHANGE)
    {
        if (cc & HAL_CC1)
        {
            value &= ~USB_Type_C_PD_Port_USBC_CTRL_RP1_VAL_Msk;
            value |= (rp << USB_Type_C_PD_Port_USBC_CTRL_RP1_VAL_Pos)
                     & USB_Type_C_PD_Port_USBC_CTRL_RP1_VAL_Msk;
        }
        if (cc & HAL_CC2)
        {
            value &= ~USB_Type_C_PD_Port_USBC_CTRL_RP2_VAL_Msk;
            value |= (rp << USB_Type_C_PD_Port_USBC_CTRL_RP2_VAL_Pos)
                     & USB_Type_C_PD_Port_USBC_CTRL_RP2_VAL_Msk;
        }
    }
    d->device->USBC_CTRL = value;
}

static void HAL_PORT_SetRpCurrent(HAL_USBPD_T *d, int cc, HAL_CC_RP_T rp)
{
    assert(d != 0 && d->device != 0);
    uint32_t value = d->device->USBC_CTRL;
    uint32_t mask;

    if (cc & HAL_CC1)
    {
        value &= ~USB_Type_C_PD_Port_USBC_CTRL_RP1_VAL_Msk;
        value |= (rp << USB_Type_C_PD_Port_USBC_CTRL_RP1_VAL_Pos)
                 & USB_Type_C_PD_Port_USBC_CTRL_RP1_VAL_Msk;
    }
    if (cc & HAL_CC2)
    {
        value &= ~USB_Type_C_PD_Port_USBC_CTRL_RP2_VAL_Msk;
        value |= (rp << USB_Type_C_PD_Port_USBC_CTRL_RP2_VAL_Pos)
                 & USB_Type_C_PD_Port_USBC_CTRL_RP2_VAL_Msk;
    }

    d->device->USBC_CTRL = value;
}
static void HAL_PORT_Set_Vconn_Ocp(HAL_USBPD_T *d, HAL_VCONN_OCP_T ocp)
{
    uint32_t ctrl = d->device->USBC_CTRL;
    if (ocp == HAL_VCONN_OCP_OFF)
    {
        ctrl &= ~(USB_Type_C_PD_Port_USBC_CTRL_VCONN_OCP_EN_Msk
                  | USB_Type_C_PD_Port_USBC_CTRL_VCONN_OCP_Msk);
    }
    else
    {
        ctrl &= ~USB_Type_C_PD_Port_USBC_CTRL_VCONN_OCP_Msk;
        ctrl |= (ocp << USB_Type_C_PD_Port_USBC_CTRL_VCONN_OCP_Pos)
                & USB_Type_C_PD_Port_USBC_CTRL_VCONN_OCP_Msk;
        ctrl |= USB_Type_C_PD_Port_USBC_CTRL_VCONN_OCP_EN_Msk;
    }
    d->device->USBC_CTRL = ctrl;
}
static void HAL_PORT_VconnEn(HAL_USBPD_T *d, bool en, HAL_VCONN_OCP_T ocp)
{
    assert(d != 0 && d->device != 0);

    uint32_t ctrl = d->device->USBC_CTRL;
#if (DEVICE_TYPE == FUSB15101) || (DEVICE_TYPE == FUSB15201P)
    /* Changing OCP not suppported */
    if (en)
    {
        if (ocp != HAL_VCONN_OCP_OFF)
        {
            ctrl |= (USB_Type_C_PD_Port_USBC_CTRL_VCONN_OCP_EN_Msk
                     | USB_Type_C_PD_Port_USBC_CTRL_VCONN_EN_Msk);
        }
        else
        {
            ctrl |= (USB_Type_C_PD_Port_USBC_CTRL_VCONN_EN_Msk);
            ctrl &= ~(USB_Type_C_PD_Port_USBC_CTRL_VCONN_OCP_EN_Msk);
        }
    }
    else
    {
        ctrl &= ~(USB_Type_C_PD_Port_USBC_CTRL_VCONN_OCP_EN_Msk
                  | USB_Type_C_PD_Port_USBC_CTRL_VCONN_EN_Msk);
    }
#elif (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
    if (en)
    {
        ctrl |= USB_Type_C_PD_Port_USBC_CTRL_VCONN_EN_Msk;
    }
    else
    {
        ctrl &= ~(USB_Type_C_PD_Port_USBC_CTRL_VCONN_EN_Msk);
    }
#endif
    d->device->USBC_CTRL = ctrl;
}

//TODO: revisit difference with 15201P
static void HAL_VCONN_Discharge(HAL_USBPD_T *d, HAL_VCONN_DISCH_T disch)
{
#if (DEVICE_TYPE == FUSB15101) || (DEVICE_TYPE == FUSB15201P)
    assert(d != 0 && d->device != 0);
    uint32_t orient = d->device->USBC_CTRL & USB_Type_C_PD_Port_USBC_CTRL_CC_ORIENT_Msk;
    uint32_t ctrl   = d->device->USBC_CTRL;
    if (!orient)
    {
        ctrl = ctrl & ~USB_Type_C_PD_Port_USBC_CTRL_CC1_TERM_Msk;
        if (disch == HAL_VCONN_DISCH)
        {
            ctrl |= USB_Type_C_PD_Port_USBC_CTRL_CC1_TERM_RD
                    << USB_Type_C_PD_Port_USBC_CTRL_CC1_TERM_Pos;
        }
        else if (disch == HAL_VCONN_DISCH1)
        {
            ctrl |= USB_Type_C_PD_Port_USBC_CTRL_CC1_TERM_RA
                    << USB_Type_C_PD_Port_USBC_CTRL_CC1_TERM_Pos;
        }
        else
        {
            ctrl |= USB_Type_C_PD_Port_USBC_CTRL_CC1_TERM_OPEN
                    << USB_Type_C_PD_Port_USBC_CTRL_CC1_TERM_Pos;
        }
    }
    else
    {
        ctrl = ctrl & ~USB_Type_C_PD_Port_USBC_CTRL_CC2_TERM_Msk;
        if (disch == HAL_VCONN_DISCH)
        {
            ctrl |= USB_Type_C_PD_Port_USBC_CTRL_CC2_TERM_RD
                    << USB_Type_C_PD_Port_USBC_CTRL_CC2_TERM_Pos;
        }
        else if (disch == HAL_VCONN_DISCH1)
        {
            ctrl |= USB_Type_C_PD_Port_USBC_CTRL_CC2_TERM_RA
                    << USB_Type_C_PD_Port_USBC_CTRL_CC2_TERM_Pos;
        }
        else
        {
            ctrl |= USB_Type_C_PD_Port_USBC_CTRL_CC2_TERM_OPEN
                    << USB_Type_C_PD_Port_USBC_CTRL_CC2_TERM_Pos;
        }
    }
    d->device->USBC_CTRL = ctrl;

#endif
}

static HAL_CCSTAT_T *HAL_PORT_CCStatus(HAL_USBPD_T *d)
{
    assert(d != 0 && d->device != 0);

    uint32_t tcstat = 0;
    uint32_t tcctrl = 0;

    tcstat = d->device->USBC_STS;
    tcctrl = d->device->USBC_CTRL;

    d->cc_stat.cc1 = (tcstat & USB_Type_C_PD_Port_USBC_STS_CC1_STS_Msk)
                     >> USB_Type_C_PD_Port_USBC_STS_CC1_STS_Pos;
    d->cc_stat.cc2 = (tcstat & USB_Type_C_PD_Port_USBC_STS_CC2_STS_Msk)
                     >> USB_Type_C_PD_Port_USBC_STS_CC2_STS_Pos;
    d->cc_stat.vconn = (tcctrl & USB_Type_C_PD_Port_USBC_CTRL_VCONN_EN_Msk) ? 1 : 0;

    /* TODO: Decide which ocp style is preferred */
    d->cc_stat.vconn_ocp = (tcstat & USB_Type_C_PD_Port_USBC_STS_VCONN_OCP_STS_Msk) ? 1 : 0;

    if (tcstat & USB_Type_C_PD_Port_USBC_STS_VCONN_OCP_STS_Msk)
    {
        d->cc_stat.vconn_ocp = 1;
    }

#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
    d->cc_stat.drp = (tcstat & USB_Type_C_PD_Port_USBC_STS_DRP_STS_Msk)
                     >> USB_Type_C_PD_Port_USBC_STS_DRP_STS_Pos;
#elif (DEVICE_TYPE == FUSB15101)
    d->cc_stat.cc_ovp = (tcstat & USB_Type_C_PD_Port_USBC_STS_OVP_STS_Msk) ? 1 : 0;
#elif (DEVICE_TYPE == FUSB15201P)
    d->cc_stat.cc_ovp = (tcstat & USB_Type_C_PD_Port_USBC_STS_CC_OVP_STS_Msk) ? 1 : 0;
#endif
    return &d->cc_stat;
}

//TODO - code refactoring revisit difference with 15201P
static void HAL_PORT_Orient(HAL_USBPD_T *d, HAL_CC_T cc)
{
    assert(d != 0 && d->device != 0);
#if (DEVICE_TYPE == FUSB15101)
    if (cc == HAL_CC1)
    {
        /* Clear the orient bit sets cc1  */
        d->device->USBC_CTRL &= ~USB_Type_C_PD_Port_USBC_CTRL_CC_ORIENT_Msk;
    }
    else if (cc == HAL_CC2)
    {
        /* Set the orient bit to cc2 */
        d->device->USBC_CTRL |= USB_Type_C_PD_Port_USBC_CTRL_CC_ORIENT_Msk;
    }
#elif (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201) || (DEVICE_TYPE == FUSB15201P)

    uint32_t cc2 = d->device->USBC_CTRL & USB_Type_C_PD_Port_USBC_CTRL_CC_ORIENT_Msk;

    if (cc == HAL_CC1 && cc2)
    {
        /* Clear the orient bit sets cc1  */
        d->device->USBC_CTRL &= ~USB_Type_C_PD_Port_USBC_CTRL_CC_ORIENT_Msk;
    }
    else if (cc == HAL_CC2 && !cc2)
    {
        /* Set the orient bit to cc2 */
        d->device->USBC_CTRL |= USB_Type_C_PD_Port_USBC_CTRL_CC_ORIENT_Msk;
    }
    else
    {
        /* Leave as is */
    }
#endif
}

static void HAL_PORT_VbusInit(HAL_USBPD_T *d)
{
    assert(d != 0 && d->device != 0);
    ADC_DRIVER.ChannelEnable(d->vbus_ch, true);
#if (DEVICE_TYPE == FUSB15101)
    ADC_DRIVER.ChannelEnable(d->vbus_pps_ch, true);
#endif
}

static void HAL_PORT_VbusDeInit(HAL_USBPD_T *d)
{
    assert(d != 0 && d->device != 0);
    ADC_DRIVER.ChannelEnable(d->vbus_ch, false);
#if (DEVICE_TYPE == FUSB15101)
    ADC_DRIVER.ChannelEnable(d->vbus_pps_ch, false);
#endif
}

static uint32_t HAL_PORT_VbusValue(HAL_USBPD_T *d)
{
    assert(d != 0 && d->device != 0);
    return ADC_DRIVER.ChannelDataMilliVolt(d->vbus_ch);
}

static uint32_t HAL_PORT_VbusPPSValue(HAL_USBPD_T *d)
{
    assert(d != 0 && d->device != 0);
    return ADC_DRIVER.ChannelDataMilliVolt(d->vbus_pps_ch);
}
#if (DEVICE_TYPE == FUSB15201P)
static void HAL_PORT_SetVbus(HAL_USBPD_T *d, HAL_VBUSIO_T io_msk, uint32_t flags)
{
    HAL_GPIOCFG_T en_cfg = {.iodir = HAL_GPIO_OUTPUT, .ppod = HAL_GPIO_PUSH_PULL};
    if (d->device == USB_Type_C_PD_Port_A)
    {
        /*set to HIGH*/
        GPIO_DRIVER.Configure(HAL_GPIO3, &en_cfg, HAL_GPIO_B);
        GPIO_DRIVER.Set(HAL_GPIO3, HAL_GPIO_HIGH, HAL_GPIO_B); // SRCSNK
    }
    else
    {
        GPIO_DRIVER.Configure(HAL_GPIO2, &en_cfg, HAL_GPIO_B);
        GPIO_DRIVER.Set(HAL_GPIO2, HAL_GPIO_HIGH, HAL_GPIO_B); // SRCSNK
    }
}
static void HAL_PORT_ClearVbus(HAL_USBPD_T *d, HAL_VBUSIO_T io_msk, uint32_t flags)
{
    HAL_GPIOCFG_T dis_cfg = {.iodir = HAL_GPIO_INPUT, .ppod = HAL_GPIO_PUSH_PULL};
    if (d->device == USB_Type_C_PD_Port_A)
    {
        /*set to LOW*/
        GPIO_DRIVER.Set(HAL_GPIO3, HAL_GPIO_LOW, HAL_GPIO_B); // SRCSNK
        GPIO_DRIVER.Configure(HAL_GPIO3, &dis_cfg, HAL_GPIO_B);
    }
    else
    {
        GPIO_DRIVER.Set(HAL_GPIO2, HAL_GPIO_LOW, HAL_GPIO_B); // SRCSNK
        GPIO_DRIVER.Configure(HAL_GPIO2, &dis_cfg, HAL_GPIO_B);
    }
}
#elif (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
static void HAL_PORT_SetVbus(HAL_USBPD_T *d, HAL_VBUSIO_T io_msk, uint32_t flag)
{
    assert(d != 0 && d->device != 0);
    uint32_t mask =
        USB_Type_C_PD_Port_USB_IO_CONFIG_O_SRC_SNK_Msk | USB_Type_C_PD_Port_USB_IO_CONFIG_O_SNK_Msk;

    uint32_t iocfg = d->device->USB_IO_CONFIG;

    /* Set pin specified in io_msk */
    iocfg |= (io_msk << USB_Type_C_PD_Port_USB_IO_CONFIG_O_SRC_SNK_Pos) & mask;
    d->device->USB_IO_CONFIG = iocfg;
}

static void HAL_PORT_ClearVbus(HAL_USBPD_T *d, HAL_VBUSIO_T io_msk, uint32_t flag)
{
    assert(d != 0 && d->device != 0);
    uint32_t mask =
        USB_Type_C_PD_Port_USB_IO_CONFIG_O_SRC_SNK_Msk | USB_Type_C_PD_Port_USB_IO_CONFIG_O_SNK_Msk;

    uint32_t iocfg = d->device->USB_IO_CONFIG;

    /* Clear pin specified in io_msk */
    iocfg &= ~(io_msk << USB_Type_C_PD_Port_USB_IO_CONFIG_O_SRC_SNK_Pos) & mask;
    d->device->USB_IO_CONFIG = iocfg;
}
#else
static void HAL_PORT_SetVbus(HAL_USBPD_T *d, HAL_VBUSIO_T io_msk, uint32_t flags)
{
    HAL_VBUS_CFG_T vbus;
    vbus.value = flags;
    HAL_VBUS_T *v = HAL_PORT_VbusReference(d);
    VBUS_DRIVER.SetFlags(v, vbus);
    VBUS_DRIVER.Set(v);
}

static void HAL_PORT_ClearVbus(HAL_USBPD_T *d, HAL_VBUSIO_T io_msk, uint32_t flags)
{
    HAL_VBUS_CFG_T vbus;
    vbus.value = flags;
    HAL_VBUS_T *v = HAL_PORT_VbusReference(d);
    VBUS_DRIVER.SetFlags(v, vbus);
    VBUS_DRIVER.Clear(v);
}
#endif

static void HAL_PORT_SetResolution(HAL_USBPD_T *d, uint32_t max)
{
    ADC_CHx_T adc_ch    = d->vbus_ch;
    uint32_t  adc_range = ADC_DRIVER.ChannelReference(adc_ch);
    if (max < 9000)
    {
        if (adc_range != ADC_REFSEL_1mV)
        {
            ADC_DRIVER.ChannelSetReference(adc_ch, ADC_REFSEL_1mV);
        }
    }
    else if (max < 20000)
    {
        if (adc_range != ADC_REFSEL_2mV)
        {
            ADC_DRIVER.ChannelSetReference(adc_ch, ADC_REFSEL_2mV);
        }
    }
    else
    {
        if (adc_range != ADC_REFSEL_4mV)
        {
            ADC_DRIVER.ChannelSetReference(adc_ch, ADC_REFSEL_4mV);
        }
    }
#if (DEVICE_TYPE == FUSB15101)
    adc_ch    = d->vbus_pps_ch;
	adc_range = ADC_DRIVER.ChannelReference(adc_ch);
	if (max < 9000)
	{
		if (adc_range != ADC_REFSEL_1mV)
		{
			ADC_DRIVER.ChannelSetReference(adc_ch, ADC_REFSEL_1mV);
		}
	}
	else if (max < 20000)
	{
		if (adc_range != ADC_REFSEL_2mV)
		{
			ADC_DRIVER.ChannelSetReference(adc_ch, ADC_REFSEL_2mV);
		}
	}
	else
	{
		if (adc_range != ADC_REFSEL_4mV)
		{
			ADC_DRIVER.ChannelSetReference(adc_ch, ADC_REFSEL_4mV);
		}
	}
#endif
}

static void HAL_PORT_SetVbusAlarm(HAL_USBPD_T *d, uint32_t low, uint32_t high)
{
    assert(d != 0 && d->device);

    ADC_DRIVER.ChannelSetAlarm(d->vbus_ch, low, high);

    if (low > 0)
    {
        ADC_DRIVER.ChannelSetLowAlarmInterrupt(d->vbus_ch, true);
    }
    else
    {
        ADC_DRIVER.ChannelSetLowAlarmInterrupt(d->vbus_ch, false);
    }

    if (high > 0)
    {
        ADC_DRIVER.ChannelSetHighAlarmInterrupt(d->vbus_ch, true);
    }
    else
    {
        ADC_DRIVER.ChannelSetHighAlarmInterrupt(d->vbus_ch, false);
    }

    if (low == 0 && high == 0)
    {
        NVIC_DisableIRQ(ADC_IRQn);
    }
    else
    {
        NVIC_SetPriority(ADC_IRQn, 1);
        NVIC_EnableIRQ(ADC_IRQn);
    }
}

static void HAL_PORT_DischVbus(HAL_USBPD_T *d, HAL_VBUS_DISCH_T disch)
{
    assert(d != 0 && d->device != 0);
#if (DEVICE_TYPE == FUSB15101)
    HAL_VBUS_T    *v = HAL_PORT_VbusReference(d);
    HAL_VBUS_CFG_T vbus;
    vbus.value            = 0;
    vbus.vbus_bld_en_mask = 1;
    VBUS_DRIVER.SetFlags(v, vbus);
    if (disch == HAL_VBUS_DISCH_NONE)
    {
        VBUS_DRIVER.Clear(v);
    }
    else
    {
        VBUS_DRIVER.Set(v);
    }
#else
    uint32_t mask = d->device->VBUS_CTRL & ~USB_Type_C_PD_Port_VBUS_CTRL_VBUS_DISCH_SEL_Msk;
    mask |= disch << USB_Type_C_PD_Port_VBUS_CTRL_VBUS_DISCH_SEL_Pos;
    d->device->VBUS_CTRL = mask;
#endif
}

static HAL_VBUSSTAT_T *HAL_PORT_VbusStat(HAL_USBPD_T *d)
{
    assert(d != 0 && d->device);
#if (DEVICE_TYPE == FUSB15101)
    HAL_CC_CV_CVSTAT_T cvstat;
    HAL_CC_CV_CCSTAT_T ccstat;
    uint32_t           stat;
    stat                = d->device->USBC_STS;
    cvstat              = CC_CV_DRIVER.CVStatus();
    ccstat              = CC_CV_DRIVER.CCStatus();
    d->vbus_stat.valid  = TCPORT_DRIVER.vbus.Value(d) > 37000;
    d->vbus_stat.ocp    = ccstat.ocp;
    d->vbus_stat.ovp    = cvstat.ovp;
    d->vbus_stat.uvp    = cvstat.uvp;
    d->vbus_stat.vsafe0 = (stat & USB_Type_C_PD_Port_USBC_STS_vSafe0V_STS_Msk) ? 1 : 0;
    /* Alarm flags are set by ADC interrupt */
    return &d->vbus_stat;

#else
    uint32_t stat;
    stat                = d->device->USBC_STS;
#if (DEVICE_TYPE != FUSB15201P)
    d->vbus_stat.valid  = (stat & USB_Type_C_PD_Port_USBC_STS_vVBUS_VAL_STS_Msk) ? 1 : 0;
    d->vbus_stat.vsafe0 = (stat & USB_Type_C_PD_Port_USBC_STS_vSafe0V_STS_Msk) ? 1 : 0;
#endif
    /* Report VBUS OCP and OVP if applicable */
    d->vbus_stat.ocp = 0;
    d->vbus_stat.ovp = 0;
    /* Alarm flags are set by ADC interrupt */
    return &d->vbus_stat;
#endif
}

static void HAL_PORT_PdEnable(HAL_USBPD_T *d)
{
    assert(d != 0 && d->device != 0);
    d->pd_stat.cblrst = d->pd_stat.ccbusy = d->pd_stat.hrdrst = 0;
    d->pd_stat.rxmsg = d->pd_stat.txstat = 0;
    /* Enable the interrupts related to PD */
    d->device->USB_INTERRUPT = PD_EVENTS_MASK;
    d->device->USB_INTERRUPT_MASK &= ~PD_EVENTS_MASK;
    d->device->USBPD_CONFIG |= USB_Type_C_PD_Port_USBPD_CONFIG_EN_PD_Msk;
    d->RxQ.r_idx = d->RxQ.w_idx = d->RxQ.w_rst = 0;
    d->TxQ.r_idx = d->TxQ.w_idx = d->TxQ.r_rst = 0;
}

//TODO - code refactoring difference with 15201P
static void HAL_PORT_PdDisable(HAL_USBPD_T *d)
{
    assert(d != 0 && d->device != 0);
    d->device->USB_INTERRUPT_MASK |= PD_EVENTS_MASK;
#if (DEVICE_TYPE == FUSB15101)
    d->device->USBPD_CONFIG = USB_Type_C_PD_Port_USBPD_CONFIG_PD_REV_REV1;
#else
    /* Default reset value */
    d->device->USBPD_CONFIG = USB_Type_C_PD_Port_USBPD_CONFIG_PD_REV_REV2;
#endif
}

static HAL_PDSTAT_T *HAL_PORT_PdStatus(HAL_USBPD_T *d)
{
    assert(d != 0 && d->device != 0);
    return &d->pd_stat;
}

static void HAL_PORT_DataRoleDfp(HAL_USBPD_T *d, bool en)
{
    assert(d != 0 && d->device != 0);

    if (en)
    {
        d->device->USBPD_CONFIG |= USB_Type_C_PD_Port_USBPD_CONFIG_DATA_ROLE_Msk;
    }
    else
    {
        d->device->USBPD_CONFIG &= ~USB_Type_C_PD_Port_USBPD_CONFIG_DATA_ROLE_Msk;
    }
}

static void HAL_PORT_SetPdRevision(HAL_USBPD_T *d, HAL_PD_REV_T rev)
{
    assert(d != 0 && d->device != 0);

    uint32_t config = d->device->USBPD_CONFIG;
    config &= ~USB_Type_C_PD_Port_USBPD_CONFIG_EN_PD3P0_Msk;
    if (rev == HAL_PD_REV3)
    {
        config = USB_Type_C_PD_Port_USBPD_CONFIG_EN_PD3P0_Msk;
    }
    d->device->USBPD_CONFIG = config;
}

static void HAL_PORT_GoodCRCRevision(HAL_USBPD_T *d, HAL_PD_REV_T rev)
{
    assert(d != 0 && d->device != 0);

    uint32_t config = d->device->USBPD_CONFIG;
    config &= ~USB_Type_C_PD_Port_USBPD_CONFIG_PD_REV_Msk;
    if (rev == HAL_PD_REV1)
    {
        config |= (USB_Type_C_PD_Port_USBPD_CONFIG_PD_REV_REV1
                   << USB_Type_C_PD_Port_USBPD_CONFIG_PD_REV_Pos)
                  & USB_Type_C_PD_Port_USBPD_CONFIG_PD_REV_Msk;
    }
    else if (rev == HAL_PD_REV2)
    {
        config |= (USB_Type_C_PD_Port_USBPD_CONFIG_PD_REV_REV2
                   << USB_Type_C_PD_Port_USBPD_CONFIG_PD_REV_Pos)
                  & USB_Type_C_PD_Port_USBPD_CONFIG_PD_REV_Msk;
    }
    else
    {
        config |= (USB_Type_C_PD_Port_USBPD_CONFIG_PD_REV_REV3
                   << USB_Type_C_PD_Port_USBPD_CONFIG_PD_REV_Pos)
                  & USB_Type_C_PD_Port_USBPD_CONFIG_PD_REV_Msk;
    }
    d->device->USBPD_CONFIG = config;
}

static void HAL_PORT_PwrRoleSrc(HAL_USBPD_T *d, bool en)
{
    assert(d != 0 && d->device != 0);

    if (en)
    {
        d->device->USBPD_CONFIG |= USB_Type_C_PD_Port_USBPD_CONFIG_POWER_ROLE_Msk;
    }
    else
    {
        d->device->USBPD_CONFIG &= ~USB_Type_C_PD_Port_USBPD_CONFIG_POWER_ROLE_Msk;
    }
}

static void HAL_PORT_SopEnable(HAL_USBPD_T *d, HAL_SOP_T sop)
{
    assert(d != 0 && d->device != 0);

    uint32_t val = d->device->USBPD_CONFIG;
    val &=
        ~(USB_Type_C_PD_Port_USBPD_CONFIG_EN_SOP_Msk | USB_Type_C_PD_Port_USBPD_CONFIG_EN_SOP1_Msk
          | USB_Type_C_PD_Port_USBPD_CONFIG_EN_SOP2_Msk
          | USB_Type_C_PD_Port_USBPD_CONFIG_EN_SOP1_DBG_Msk
          | USB_Type_C_PD_Port_USBPD_CONFIG_EN_SOP2_DBG_Msk);

    if (sop != HAL_SOP_DISABLE_ALL)
    {
        /* One of the SOP is enabled then enable hard reset detection */
        val |= USB_Type_C_PD_Port_USBPD_CONFIG_EN_HRD_RST_Msk;
    }
    else
    {
        val &= ~USB_Type_C_PD_Port_USBPD_CONFIG_EN_HRD_RST_Msk;
    }

    if (sop & HAL_SOP_TYPE_SOP)
    {
        val |= USB_Type_C_PD_Port_USBPD_CONFIG_EN_SOP_Msk;
    }
    if (sop & HAL_SOP_TYPE_SOP1)
    {
        val |= USB_Type_C_PD_Port_USBPD_CONFIG_EN_SOP1_Msk;
    }
    if (sop & HAL_SOP_TYPE_SOP2)
    {
        val |= USB_Type_C_PD_Port_USBPD_CONFIG_EN_SOP2_Msk;
    }
    if (sop & HAL_SOP_TYPE_SOP1_DEBUG)
    {
        val |= USB_Type_C_PD_Port_USBPD_CONFIG_EN_SOP1_DBG_Msk;
    }
    if (sop & HAL_SOP_TYPE_SOP2_DEBUG)
    {
        val |= USB_Type_C_PD_Port_USBPD_CONFIG_EN_SOP2_DBG_Msk;
    }

    d->device->USBPD_CONFIG = val;
}

static void HAL_PORT_BistEn(HAL_USBPD_T *d, HAL_BIST_T mode)
{
    assert(d != 0 && d->device != 0);

    if (mode == HAL_BIST_RX)
    {
        d->device->USBPD_CONFIG |= USB_Type_C_PD_Port_USBPD_CONFIG_EN_BIST_RX_MODE_Msk;
    }
    else if (mode == HAL_BIST_TX)
    {
        d->device->USBPD_TRANSMIT |= USB_Type_C_PD_Port_USBPD_TRANSMIT_TX_SOP_BISTCM2
                                     << USB_Type_C_PD_Port_USBPD_TRANSMIT_tx_sop_Pos;
    }
    else if (mode == HAL_BIST_DISABLE)
    {
        HAL_PORT_PdDisable(d);
    }
}

static void HAL_PORT_TxHardRst(HAL_USBPD_T *d, HAL_SOP_T sop)
{
    assert(d != 0 && d->device != 0);

    d->pd_stat.txstat = HAL_TX_BUSY;
    if (sop == HAL_SOP_TYPE_SOP)
    {
        d->device->USBPD_TRANSMIT = USB_Type_C_PD_Port_USBPD_TRANSMIT_TX_SOP_HARD_RESET
                                    << USB_Type_C_PD_Port_USBPD_TRANSMIT_tx_sop_Pos;
    }
    else if (sop == HAL_SOP_TYPE_SOP1)
    {
        d->device->USBPD_TRANSMIT = USB_Type_C_PD_Port_USBPD_TRANSMIT_TX_SOP_CABLE_RESET
                                    << USB_Type_C_PD_Port_USBPD_TRANSMIT_tx_sop_Pos;
    }
}

static HAL_ERROR_T HAL_PORT_TxData(HAL_USBPD_T *d, uint8_t const *buf, uint32_t len, HAL_SOP_T sop,
                                   uint32_t retry)
{
    assert(d != 0 && d->device != 0 && buf != 0 && len > 1 && len < 31);
    uint32_t i;
    uint32_t tx_hdr, txLen;
    tx_hdr = ((len << USB_Type_C_PD_Port_USBPD_TRANSMIT_tx_byte_cnt_Pos)
              & USB_Type_C_PD_Port_USBPD_TRANSMIT_tx_byte_cnt_Msk)
             | ((retry << USB_Type_C_PD_Port_USBPD_TRANSMIT_retry_cnt_Pos)
                & USB_Type_C_PD_Port_USBPD_TRANSMIT_retry_cnt_Msk);

    if (sop & HAL_SOP_TYPE_SOP)
    {
        tx_hdr |= (USB_Type_C_PD_Port_USBPD_TRANSMIT_TX_SOP_SOP
                   << USB_Type_C_PD_Port_USBPD_TRANSMIT_tx_sop_Pos)
                  & USB_Type_C_PD_Port_USBPD_TRANSMIT_tx_sop_Msk;
    }
    else if (sop & HAL_SOP_TYPE_SOP1)
    {
        tx_hdr |= (USB_Type_C_PD_Port_USBPD_TRANSMIT_TX_SOP_SOP_P
                   << USB_Type_C_PD_Port_USBPD_TRANSMIT_tx_sop_Pos)
                  & USB_Type_C_PD_Port_USBPD_TRANSMIT_tx_sop_Msk;
    }
    else if (sop & HAL_SOP_TYPE_SOP2)
    {
        tx_hdr |= (USB_Type_C_PD_Port_USBPD_TRANSMIT_TX_SOP_SOP_PP
                   << USB_Type_C_PD_Port_USBPD_TRANSMIT_tx_sop_Pos)
                  & USB_Type_C_PD_Port_USBPD_TRANSMIT_tx_sop_Msk;
    }
    else if (sop & HAL_SOP_TYPE_SOP1_DEBUG)
    {
        tx_hdr |= (USB_Type_C_PD_Port_USBPD_TRANSMIT_TX_SOP_DBG_P
                   << USB_Type_C_PD_Port_USBPD_TRANSMIT_tx_sop_Pos)
                  & USB_Type_C_PD_Port_USBPD_TRANSMIT_tx_sop_Msk;
    }
    else if (sop & HAL_SOP_TYPE_SOP2_DEBUG)
    {
        tx_hdr |= (USB_Type_C_PD_Port_USBPD_TRANSMIT_TX_SOP_DBG_PP
                   << USB_Type_C_PD_Port_USBPD_TRANSMIT_tx_sop_Pos)
                  & USB_Type_C_PD_Port_USBPD_TRANSMIT_tx_sop_Msk;
    }

    /* Empty out the queue */
    d->TxQ.r_idx = d->TxQ.w_idx = d->TxQ.r_rst = 0;
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201) || (DEVICE_TYPE == FUSB15201P)
    if (len >= 2)
    {
        /* Write the message header */
        d->device->USBPD_MSG_HEADER = buf[0] | buf[1] << 8;
        i                           = 2;
        txLen                       = (len - i) / 4;
        if (txLen > TX_BUF_LEN)
        {
            txLen = TX_BUF_LEN;
        }
        /* Copy the data message  */
        while (d->TxQ.w_idx < txLen)
        {
            d->TxQ.buf32[d->TxQ.w_idx] =
                buf[i] | buf[i + 1] << 8 | buf[i + 2] << 16 | buf[i + 3] << 24;
            d->TxQ.w_idx++;
            i += 4;
        }
        /* Set the read and reset index to pd data message */
        d->TxQ.r_rst = d->TxQ.r_idx = 0;
        /* Write TX register to begin transmission */
        d->pd_stat.txstat         = HAL_TX_BUSY;
        d->device->USBPD_TRANSMIT = tx_hdr;
        /* Load first word of data message in tx buffer */
        d->device->USBPD_DATA_TX = d->TxQ.buf32[d->TxQ.r_idx];
        d->TxQ.r_idx += 1;
    }

    return HAL_SUCCESS;
#else
    if (len >= 2)
    {
        /* Write the message header */
        d->device->USBPD_MSG_HEADER = buf[0] | buf[1] << 8;
        i                           = 2;
        txLen                       = (len - i) / 4;
        if (txLen > TX_BUF_LEN)
        {
            txLen = TX_BUF_LEN;
        }
        /* Copy the data message  */
        while (d->TxQ.w_idx < txLen)
        {
            d->TxQ.buf32[d->TxQ.w_idx] =
                buf[i] | buf[i + 1] << 8 | buf[i + 2] << 16 | buf[i + 3] << 24;
            d->TxQ.w_idx++;
            i += 4;
        }
        /* Set the read and reset index to pd data message */
        d->TxQ.r_rst = d->TxQ.r_idx = 0;
        if (d->pd_stat.rxmsg == false)
        {
            /* Write TX register to begin transmission */
            d->pd_stat.txstat         = HAL_TX_BUSY;
            d->device->USBPD_TRANSMIT = tx_hdr;
            /* Load first word of data message in tx buffer */
            d->device->USBPD_DATA_TX = d->TxQ.buf32[d->TxQ.r_idx];
            d->TxQ.r_idx += 1;
            return HAL_SUCCESS;
        }
    }

    d->pd_stat.txstat = HAL_TX_DISCARD;
    return HAL_ERROR;
#endif
}

static HAL_SOP_T HAL_PORT_RxData(HAL_USBPD_T *d, uint8_t *buf, uint32_t size)
{
    assert(d != 0 && d->device != 0 && buf != 0);

    PKT_T     pckt;
    HAL_SOP_T sop;
    uint32_t  i, j, len;
    uint32_t  r_idx, r_len;

    pckt.object = d->RxQ.buf32[d->RxQ.r_idx];
    r_idx       = d->RxQ.r_idx;
    r_len       = pckt.rx.token;
#if 0
    if (r_len < 2) {
        /* This may not be needed but check integrity. */
        d->RxQ.r_idx = RX_BUF_IDX(r_idx + r_len);
        pckt.object = d->RxQ.buf32[d->RxQ.r_idx];
        r_idx = d->RxQ.r_idx;
        r_len = pckt.rx.token;
    }
#endif
    RX_BUF_IDX_INC(d->RxQ.r_idx);

    switch (pckt.rx.sop)
    {
        case HAL_SOP_TOKEN:
            sop = HAL_SOP_TYPE_SOP;
            break;
        case HAL_SOP_P_TOKEN:
            sop = HAL_SOP_TYPE_SOP1;
            break;
        case HAL_SOP_PP_TOKEN:
            sop = HAL_SOP_TYPE_SOP2;
            break;
        case HAL_SOP_DBG_P_TOKEN:
            sop = HAL_SOP_TYPE_SOP1_DEBUG;
            break;
        case HAL_SOP_DBG_PP_TOKEN:
            sop = HAL_SOP_TYPE_SOP2_DEBUG;
            break;
        default:
            sop = HAL_SOP_TYPE_ERROR;
            break;
    }

    if (size >= 2)
    {
        /* Copy the header */
        buf[0] = pckt.byte[0];
        buf[1] = pckt.byte[1];
        /* Skip header and count available 4 byte aligned buffer */
        len = (size - 2) / 4;
        /* Rx buffer contains data plus header (4 byte aligned) */
        if (r_len - 1 < len)
        {
            len = r_len - 1;
        }
        i = 2;
        for (j = 0; j < len; j++)
        {
            buf[i]     = d->RxQ.buf32[d->RxQ.r_idx] & 0xff;
            buf[i + 1] = (d->RxQ.buf32[d->RxQ.r_idx] >> 8) & 0xff;
            buf[i + 2] = (d->RxQ.buf32[d->RxQ.r_idx] >> 16) & 0xff;
            buf[i + 3] = (d->RxQ.buf32[d->RxQ.r_idx] >> 24) & 0xff;
            i += 4;
            RX_BUF_IDX_INC(d->RxQ.r_idx);
        }
    }
    /* Move the r_idx to end of actual number of bytes stored at the
     * beginning of buffer since there is no guarantee that
     * the message received is completed or header matches
     * the number of bytes received. */
    d->RxQ.r_idx = RX_BUF_IDX(r_idx + r_len);
    /* Clear the RX success interrupt and mask */
    d->device->USB_INTERRUPT = USB_Type_C_PD_Port_USB_INTERRUPT_I_rx_success_Msk;
    d->device->USB_INTERRUPT_MASK &= ~USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_rx_success_Msk;
    return sop;
}

static void HAL_USBPD_EvtEnable(HAL_USBPD_T *d, bool enable)
{
    assert(d != 0 && d->device != 0);

    if (enable)
    {
        d->device->USB_INTERRUPT_MASK &= ~PORT_EVENTS_MASK;
    }
    else
    {
        d->device->USB_INTERRUPT_MASK |= PORT_EVENTS_MASK;
    }
}

static void HAL_USBPD_CCEvtEnable(HAL_USBPD_T *d, bool enable)
{
    assert(d != 0 && d->device != 0);

    if (enable)
    {
        d->device->USB_INTERRUPT_MASK &= ~CC_EVENTS_MASK;
    }
    else
    {
        d->device->USB_INTERRUPT_MASK |= CC_EVENTS_MASK;
    }
}

static void HAL_USBPD_PDEvtEnable(HAL_USBPD_T *d, bool enable)
{
    assert(d != 0 && d->device != 0);

    if (enable)
    {
        d->device->USB_INTERRUPT_MASK &= ~PD_EVENTS_MASK;
    }
    else
    {
        d->device->USB_INTERRUPT_MASK |= PD_EVENTS_MASK;
    }
}

static void HAL_USBPD_VbusEvtEnable(HAL_USBPD_T *d, bool enable)
{
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201) || (DEVICE_TYPE == FUSB15101)
    assert(d != 0 && d->device != 0);

    if (enable)
    {
        d->device->USB_INTERRUPT_MASK &= ~VBUS_EVENTS_MASK;
    }
    else
    {
        d->device->USB_INTERRUPT_MASK |= VBUS_EVENTS_MASK;
    }
#endif
}

#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)

static void HAL_PORT_DrpEn(HAL_USBPD_T *d, HAL_DRP_MODE_T mode)
{
    assert(d != 0 && d->device != 0);
    uint32_t value;
    if (mode != HAL_DRP_MODE_NOCHANGE)
    {
        value = d->device->USBC_CTRL;
        value &=
            ~(USB_Type_C_PD_Port_USBC_CTRL_DRP_EN_Msk | USB_Type_C_PD_Port_USBC_CTRL_DRP_MODE_Msk);
        if (mode != HAL_DRP_MODE_DISABLED)
        {
            value |= USB_Type_C_PD_Port_USBC_CTRL_DRP_EN_Msk
                     | (mode << USB_Type_C_PD_Port_USBC_CTRL_DRP_MODE_Pos);
        }
        d->device->USBC_CTRL = value;
    }
}

static void HAL_USBPD_FRSwapConfigure(HAL_USBPD_T *d, HAL_FRSWAP_T frs, HAL_VBUSIO_T io_msk)
{
    assert(d != 0 && d->device != 0);

    uint32_t val = d->device->USB_IO_CONFIG
                   & ~(USB_Type_C_PD_Port_USB_IO_CONFIG_FR_Mode_Msk
                       | USB_Type_C_PD_Port_USB_IO_CONFIG_FR_O_SNK_Msk
                       | USB_Type_C_PD_Port_USB_IO_CONFIG_FR_O_SRC_SNK_Msk);
    val |= (io_msk << USB_Type_C_PD_Port_USB_IO_CONFIG_FR_O_SRC_SNK_Pos)
           & (USB_Type_C_PD_Port_USB_IO_CONFIG_FR_O_SNK_Msk
              | USB_Type_C_PD_Port_USB_IO_CONFIG_FR_O_SRC_SNK_Msk);
    if (frs == HAL_FRSWAP_DISABLED)
    {
        val &= ~USB_Type_C_PD_Port_USB_IO_CONFIG_FR_Mode_Msk;
        d->device->USB_INTERRUPT_MASK |= USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_fr_swap_det_Msk;
    }
    else if (frs == HAL_FRSWAP_SOURCE_TO_SINK)
    {
        val |= (USB_TYPE_C_PD_PORT_FRSWAP_ENABLED_AS_INITIAL_SRC
                << USB_Type_C_PD_Port_USB_IO_CONFIG_FR_Mode_Pos);
        d->device->USB_INTERRUPT_MASK |= USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_fr_swap_det_Msk;
    }
    else if (HAL_FRSWAP_SINK_TO_SOURCE)
    {
        val |= (USB_TYPE_C_PD_PORT_FRSWAP_ENABLED_AS_INITIAL_SNK
                << USB_Type_C_PD_Port_USB_IO_CONFIG_FR_Mode_Pos);
        d->device->USB_INTERRUPT_MASK &= ~USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_fr_swap_det_Msk;
    }
    else
    {
        /* Reserved */
    }
    d->device->USB_IO_CONFIG = val;
}

static void HAL_USBPD_FRSwapInitiate(HAL_USBPD_T *d)
{
    assert(d != 0 && d->device != 0);
    d->device->USBC_CTRL |= USB_Type_C_PD_Port_USBC_CTRL_FR_SWAP_INIT_Msk;
}
#endif
void HAL_PD_IRQHandler(HAL_USBPD_CHID_T id)
{
    assert(id < HAL_USBPD_CH_COUNT);

    HAL_USBPD_T *d = &tcpddrivMemPool[id];
    uint32_t     intmask;
    PKT_T        pkt;

    intmask = d->device->USB_INTERRUPT_MASK;
    intmask = (~intmask) & d->device->USB_INTERRUPT;

    if (intmask & USB_Type_C_PD_Port_USB_INTERRUPT_I_rx_buf_rdy_Msk)
    {
        pkt.object = d->device->USBPD_DATA_RX;
        if (d->RxQ.w_idx == d->RxQ.w_rst)
        {
            /* Beginning of new message. 4 byte at
             * the beginning to keep track of message
             * length */
            /* Use bit for token to keep track of msg length */
            pkt.rx.token = 0;
        }
        d->RxQ.buf32[d->RxQ.w_idx] = pkt.object;
        RX_BUF_IDX_INC(d->RxQ.w_idx);
        /* Increment the number of message objects */
        pkt.object = d->RxQ.buf32[d->RxQ.w_rst];
        pkt.rx.token++;
        d->RxQ.buf32[d->RxQ.w_rst] = pkt.object;
    }

    if (intmask & USB_Type_C_PD_Port_USB_INTERRUPT_I_tx_success_Msk)
    {
        d->RxQ.w_idx      = d->RxQ.w_rst;
        d->pd_stat.txstat = HAL_TX_SUCCESS;
    }

    if (intmask & USB_Type_C_PD_Port_USB_INTERRUPT_I_tx_retry_Msk)
    {
        d->TxQ.r_idx      = d->TxQ.r_rst;
        d->pd_stat.txstat = HAL_TX_BUSY;
    }

    /* Transmit side */
    if (intmask
        & (USB_Type_C_PD_Port_USB_INTERRUPT_I_tx_buf_rdy_Msk
           | USB_Type_C_PD_Port_USB_INTERRUPT_I_tx_retry_Msk))
    {
        /* Copy another tx word to tx register. */
        pkt.object               = d->TxQ.buf32[d->TxQ.r_idx++];
        d->device->USBPD_DATA_TX = pkt.object;
        d->pd_stat.txstat        = HAL_TX_BUSY;
    }

    if (intmask
        & (USB_Type_C_PD_Port_USB_INTERRUPT_I_tx_discard_Msk
           | USB_Type_C_PD_Port_USB_INTERRUPT_I_tx_buferror_Msk))
    {
        /* Collision or new received message before Tx */
        d->pd_stat.txstat = HAL_TX_DISCARD;
    }

    /* Tx failed */
    if (intmask & USB_Type_C_PD_Port_USB_INTERRUPT_I_tx_fail_Msk)
    {
        /* GoodCRC not received for message sent */
        d->pd_stat.txstat = HAL_TX_FAILED;
    }

    if (intmask & USB_Type_C_PD_Port_USB_INTERRUPT_I_rx_success_Msk)
    {
        d->RxQ.w_rst     = d->RxQ.w_idx;
        d->pd_stat.rxmsg = 1;
        /* Do not clear the interrupt bit and allow application to clear it
           once message is read. This will cause any discard on transmit
           until rx success bit is cleared. */
        intmask &= ~USB_Type_C_PD_Port_USB_INTERRUPT_I_rx_success_Msk;
        d->device->USB_INTERRUPT_MASK |= USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_rx_success_Msk;
    }

    if (intmask
        & (USB_Type_C_PD_Port_USB_INTERRUPT_I_rx_badcrc_Msk
           | USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_rx_discard_Msk
           | USB_Type_C_PD_Port_USB_INTERRUPT_I_rx_overflow_Msk))
    {
        d->RxQ.w_idx = d->RxQ.w_rst;
    }

    if (intmask & USB_Type_C_PD_Port_USB_INTERRUPT_I_rx_hard_rst_Msk)
    {
        d->pd_stat.hrdrst = 1;
    }

    if (intmask & USB_Type_C_PD_Port_USB_INTERRUPT_I_rx_cbl_rst_Msk)
    {
        d->pd_stat.cblrst = 1;
    }

    if (intmask & USB_Type_C_PD_Port_USB_INTERRUPT_I_vconn_ocp_Msk)
    {
        d->device->USB_INTERRUPT_MASK |= USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_vconn_ocp_Msk;
        d->cc_stat.vconn_ocp = 1;
    }

    if (intmask & USB_Type_C_PD_Port_USB_INTERRUPT_I_cc_ovp_Msk)
    {
        d->device->USB_INTERRUPT_MASK |= USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_cc_ovp_Msk;
        //d->cc_stat.cc_ovp = 1;
    }
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
    if (intmask & USB_Type_C_PD_Port_USB_INTERRUPT_I_fr_swap_det_Msk)
    {
        d->device->USB_INTERRUPT_MASK |= USB_Type_C_PD_Port_USB_INTERRUPT_I_fr_swap_det_Msk;
        d->cc_stat.frswap = 1;
    }
#endif

    if (intmask & USB_Type_C_PD_Port_USB_INTERRUPT_I_otp_Msk)
    {
#if FUSB15200_REVB
        d->device->USBC_CTRL |= USB_Type_C_PD_Port_USBC_CTRL_VCONN_EN_Msk;
#elif (DEVICE_TYPE == FUSB15101)
        d->device->USB_INTERRUPT_MASK |= USB_Type_C_PD_Port_USB_INTERRUPT_MASK_M_otp_Msk;
#endif
    }

    /* Reset of the changes can be update lazily during cc, vbus, pd status.
     * Clear other interrupts */
    d->device->USB_INTERRUPT = intmask;
}

HAL_TCPD_DRIVER_T TCPORT_DRIVER = {.Initialize   = HAL_USBPD_Init,
                                   .DeIntialize  = HAL_USBPD_DeInit,
                                   .EventsEnable = HAL_USBPD_EvtEnable,
                                   .Status       = HAL_USBPD_PortStatus,
#if (DEVICE_TYPE == FUSB15101)
                                   .Enable = HAL_USBPD_Enable,
#endif
                                   .cc =
                                       {
                                           .Termination    = HAL_PORT_SetTerm,
                                           .SetCC          = HAL_PORT_SetCC,
                                           .Pullup         = HAL_PORT_SetRpCurrent,
                                           .Orientation    = HAL_PORT_Orient,
                                           .SetVconnOcp    = HAL_PORT_Set_Vconn_Ocp,
                                           .VconnEnable    = HAL_PORT_VconnEn,
                                           .VconnDischarge = HAL_VCONN_Discharge,
                                           .Status         = HAL_PORT_CCStatus,
                                           .EventsEnable   = HAL_USBPD_CCEvtEnable,
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
                                           .Drp                   = HAL_PORT_DrpEn,
                                           .FastRoleSwapConfigure = HAL_USBPD_FRSwapConfigure,
                                           .FastRoleSwap          = HAL_USBPD_FRSwapInitiate,
#endif
                                       },
                                   .pd =
                                       {
                                           .Enable             = HAL_PORT_PdEnable,
                                           .Disable            = HAL_PORT_PdDisable,
                                           .SetDfp             = HAL_PORT_DataRoleDfp,
                                           .SetPdRevision      = HAL_PORT_SetPdRevision,
                                           .SetGoodCRCRevision = HAL_PORT_GoodCRCRevision,
                                           .Source             = HAL_PORT_PwrRoleSrc,
                                           .SopEnable          = HAL_PORT_SopEnable,
                                           .BistEnable         = HAL_PORT_BistEn,
                                           .TransmitHardReset  = HAL_PORT_TxHardRst,
                                           .Receive            = HAL_PORT_RxData,
                                           .Transmit           = HAL_PORT_TxData,
                                           .Status             = HAL_PORT_PdStatus,
                                           .EventsEnable       = HAL_USBPD_PDEvtEnable,
                                       },
                                   .vbus = {
                                       .Initialize    = HAL_PORT_VbusInit,
                                       .Value         = HAL_PORT_VbusValue,
#if (DEVICE_TYPE == FUSB15101)
									   .PPSValue      = HAL_PORT_VbusPPSValue,
#endif
                                       .Set           = HAL_PORT_SetVbus,
                                       .SetResolution = HAL_PORT_SetResolution,
                                       .Clear         = HAL_PORT_ClearVbus,
                                       .Alarm         = HAL_PORT_SetVbusAlarm,
                                       .Discharge     = HAL_PORT_DischVbus,
                                       .Status        = HAL_PORT_VbusStat,
                                       .EventsEnable  = HAL_USBPD_VbusEvtEnable,
#if (DEVICE_TYPE == FUSB15101)
                                       .GetVbus = HAL_PORT_VbusReference,
                                       .Enable  = HAL_PORT_VbusEnable,
#endif
                                   }};

#endif /* HAL_USE_TCPD */
