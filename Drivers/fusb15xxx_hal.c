/**
 * @file     FUSB15xxx_hal.c
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
#if CONFIG_LOG
#include "SEGGER_RTT.h"
#endif

#include "pd_types.h"
#include "FUSB15xxx.h"
#include "fusb15xxx_hal.h"
#include "logging.h"
#include "timer.h"
#if (DEVICE_TYPE == FUSB15101)
#include "fusb15101_hal_vbus.h"
#endif
#if (CONFIG_ENABLED(CUSTOM_APPLICATION))
#include "pd_powersupply.h"
#endif
extern HAL_I2C_T *i2c_master[I2C_PORT_COUNT];
unsigned long     SystickCount = 0;
static int        Dev_Id       = 0;

#ifndef __WEAK
#define __WEAK
#endif

__WEAK unsigned long int HAL_SYSTICK_Count(void)
{
    return SystickCount;
}

__WEAK void HAL_Delay(unsigned long delay)
{
    unsigned long start = SystickCount;
    while (SystickCount - start < delay)
    {
    }
}

void fusb15xxx_timer_start(enum timer_type_t timer, int count, bool periodic)
{
    if (timer == Timer_Device_Type_Timer_0)
    {
        TIMER_DRIVER.Start(Timer_0, count, periodic);
    }
    else if (timer == Timer_Device_Type_Timer_1)
    {
#if (DEVICE_TYPE != FUSB15101)
        TIMER_DRIVER.Start(Timer_1, count, periodic);
#endif
    }
    else
    {
        TIMER_DRIVER.Start(Wake_Up_Timer, count, periodic);
    }
}

void fusb15xxx_timer_stop(enum timer_type_t timer)
{
    if (timer == Timer_Device_Type_Timer_0)
    {
        TIMER_DRIVER.Stop(Timer_0);
    }
    else if (timer == Timer_Device_Type_Timer_1)
    {
#if (DEVICE_TYPE != FUSB15101)
        TIMER_DRIVER.Stop(Timer_1);
#endif
    }
    else
    {
        TIMER_DRIVER.Stop(Wake_Up_Timer);
    }
}

void fusb15xxx_timer_interrupt_enable(enum timer_type_t timer, bool en)
{
    if (timer == Timer_Device_Type_Timer_0)
    {
        TIMER_DRIVER.InterruptEnable(Timer_0, en);
    }
    else if (timer == Timer_Device_Type_Timer_1)
    {
#if (DEVICE_TYPE != FUSB15101)
        TIMER_DRIVER.InterruptEnable(Timer_1, en);
#endif
    }
    else
    {
        TIMER_DRIVER.InterruptEnable(Wake_Up_Timer, en);
    }
}
#if (DEVICE_TYPE != FUSB15101)
void fusb15xxx_cmu_clock_select(enum clk_sel_t clk_sel)
{
    CMU_DRIVER.ClockSelect(Clock_Management_Unit, clk_sel);
}
#endif

void fusb15xxx_adc_enable(void)
{
    ADC_DRIVER.Start();
}

void fusb15xxx_adc_disable(void)
{
    ADC_DRIVER.Stop();
}

void fusb15xxx_gpio_initialize(HAL_GPIOx_T pin, HAL_GPIOCFG_T *cfg, HAL_GPIO_PORT_T port)
{
    GPIO_DRIVER.Configure(pin, cfg, port);
}

void fusb15xxx_gpio_set(HAL_GPIOx_T pin, HAL_GPIOLVL_T lvl, HAL_GPIO_PORT_T port)
{
    GPIO_DRIVER.Set(pin, lvl, port);
}

HAL_GPIOLVL_T fusb15xxx_gpio_state(HAL_GPIOx_T pin, HAL_GPIODIR_T dir, HAL_GPIO_PORT_T port)
{
    HAL_GPIOLVL_T state = HAL_GPIO_LOW;
    state               = GPIO_DRIVER.State(pin, dir, port);
    return state;
}

void fusb15xxx_gpio_enable_interrupt(HAL_GPIOx_T pin, HAL_GPIO_IRQ_T type, HAL_GPIO_IRQ_POL_T pol,
                                     HAL_GPIO_PORT_T port)
{
    GPIO_DRIVER.EnableInterrupt(pin, type, pol, port);
}

void fusb15xxx_gpio_disable_interrupt(HAL_GPIOx_T pin, HAL_GPIO_PORT_T port)
{
    GPIO_DRIVER.InterruptDisable(pin, port);
}

void fusb15xxx_gpio_interrupt_status(HAL_GPIOx_T pin, HAL_GPIO_PORT_T port)
{
    GPIO_DRIVER.InterruptStatus(pin, port);
}

void fusb15xxx_gpio_interrupt_clear(HAL_GPIOx_T pin, HAL_GPIO_PORT_T port)
{
    GPIO_DRIVER.InterruptClear(pin, port);
}

void fusb15xxx_port_initialize(HAL_PORTx_T pin, HAL_PORTCFG_T *cfg, HAL_GPIO_PORT_T port)
{
    PORT_DRIVER.Configure(pin, cfg, port);
}

void fusb15xxx_port_enable_interrupt(HAL_PORTx_T pin, HAL_PORT_IRQ_T pol, HAL_GPIO_PORT_T port)
{
    PORT_DRIVER.InterruptEnable(pin, pol, port);
}

void fusb15xxx_port_disable_interrupt(HAL_GPIO_PORT_T port)
{
    PORT_DRIVER.InterruptDisable(port);
}

void fusb15xxx_port_interrupt_status(HAL_GPIO_PORT_T port)
{
    PORT_DRIVER.InterruptStatus(port);
}

void fusb15xxx_port_interrupt_clear(HAL_GPIO_PORT_T port)
{
    PORT_DRIVER.InterruptClear(port);
}

void fusb15xxx_port_enable_nmi(HAL_NMI_CFG_T nmi, HAL_GPIO_PORT_T port)
{
    PORT_DRIVER.EnableNMI(nmi, port);
}

void fusb15xxx_port_disable_nmi(HAL_GPIO_PORT_T port)
{
    PORT_DRIVER.DisableNMI(port);
}

bool fusb15xxx_port_is_swd_enabled()
{
	return PORT_DRIVER.SWDEnabled();
}
HAL_USBPD_T *fusb15xxx_tcpd_initialize(int tcpd_id, bool en, bool reinit)
{
    return TCPORT_DRIVER.Initialize(tcpd_id, en, reinit);
}

bool fusb15xxx_tcpd_rx_hardreset(HAL_USBPD_T *tcpd)
{
    bool          ret       = false;
    int           device_id = Dev_Id;
    HAL_PDSTAT_T *status    = TCPORT_DRIVER.pd.Status(tcpd);
    if (status->hrdrst)
    {
        status->hrdrst = false;
        ret            = true;
    }
    return ret;
}

void fusb15xxx_tcpd_tx_hardreset(HAL_USBPD_T *tcpd, SOP_T sop)
{
    if (sop == SOP_SOP0)
    {
        TCPORT_DRIVER.pd.TransmitHardReset(tcpd, HAL_SOP_TYPE_SOP);
    }
    else if (sop == SOP_SOP1)
    {
        TCPORT_DRIVER.pd.TransmitHardReset(tcpd, HAL_SOP_TYPE_SOP1);
    }
    else if (sop == SOP_SOP2)
    {
        TCPORT_DRIVER.pd.TransmitHardReset(tcpd, HAL_SOP_TYPE_SOP2);
    }
}
void fusb15xxx_tcpd_set_rp(HAL_USBPD_T *tcpd, int mask, CC_RP_T rp)
{
    HAL_CC_T cc = 0;
    if (CC1 & mask)
    {
        cc |= HAL_CC1;
    }
    if (CC2 & mask)
    {
        cc |= HAL_CC2;
    }
    if (rp == CC_RP_DEF)
    {
        TCPORT_DRIVER.cc.Pullup(tcpd, cc, HAL_CC_RP_DEF);
    }
    else if (rp == CC_RP_1p5A)
    {
        TCPORT_DRIVER.cc.Pullup(tcpd, cc, HAL_CC_RP_1p5A);
    }
    else if (rp == CC_RP_3p0A)
    {
        TCPORT_DRIVER.cc.Pullup(tcpd, cc, HAL_CC_RP_3p0A);
    }
    else if (rp == CC_RP_OFF)
    {
        TCPORT_DRIVER.cc.Pullup(tcpd, cc, HAL_CC_RP_OFF);
    }
    else
    {
    }
}

void fusb15xxx_tcpd_set_orient(HAL_USBPD_T *tcpd, CC_T cc)
{
    if (cc == CC1)
    {
        TCPORT_DRIVER.cc.Orientation(tcpd, HAL_CC1);
    }
    else
    {
        TCPORT_DRIVER.cc.Orientation(tcpd, HAL_CC2);
    }
}

void fusb15xxx_tcpd_set_cc(HAL_USBPD_T *tcpd, int mask, CC_TERM_T term, CC_RP_T rp, DRP_MODE_T drp)
{
    TCPORT_DRIVER.cc.SetCC(tcpd, mask, term, rp, drp);
}

CC_STAT_T fusb15xxx_tcpd_cc_stat(HAL_USBPD_T *tcpd, CC_T cc)
{
    int            device_id = Dev_Id;
    enum cc_stat_t cc_status = CC_STAT_UNDEFINED;
    HAL_CC_STAT_T  cc_stat;
    HAL_CCSTAT_T  *stat;
    stat = TCPORT_DRIVER.cc.Status(tcpd);
    if (cc == CC1)
    {
        cc_stat = stat->cc1;
    }
    else
    {
        cc_stat = stat->cc2;
    }
    if (cc_stat == HAL_CC_STAT_RA)
    {
        cc_status = CC_STAT_RA;
    }
    else if (cc_stat == HAL_CC_STAT_RD)
    {
        cc_status = CC_STAT_RD;
    }
    else if (cc_stat == HAL_CC_STAT_RPUSB)
    {
        cc_status = CC_STAT_RPUSB;
    }
    else if (cc_stat == HAL_CC_STAT_RP1p5)
    {
        cc_status = CC_STAT_RP1p5;
    }
    else if (cc_stat == HAL_CC_STAT_RP3p0)
    {
        cc_status = CC_STAT_RP3p0;
    }
    else if (cc_stat == HAL_CC_STAT_SNKOPEN)
    {
        cc_status = CC_STAT_SNKOPEN;
    }
    else if (cc_stat == HAL_CC_STAT_SRCOPEN)
    {
        cc_status = CC_STAT_SRCOPEN;
    }
    return cc_status;
}

void fusb15xxx_tcpd_cc_events_enable(HAL_USBPD_T *tcpd, bool en)
{
    TCPORT_DRIVER.cc.EventsEnable(tcpd, en);
}

bool fusb15xxx_tcpd_cc_check_error(HAL_USBPD_T *tcpd)
{
    bool            ret = false;
    HAL_CCSTAT_T   *ccstat;
    HAL_PORTSTAT_T *portstat;
    ccstat   = TCPORT_DRIVER.cc.Status(tcpd);
    portstat = TCPORT_DRIVER.Status(tcpd);
    //can this be expanded for other error??
    if (ccstat->cc_ovp || portstat->otp)
    {
        ret = true;
    }
    return ret;
}

#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
void fusb15xxx_tcpd_set_drp(HAL_USBPD_T *tcpd, DRP_MODE_T drp)
{
    if (drp == DRP_MODE_DISABLED)
    {
        TCPORT_DRIVER.cc.Drp(tcpd, HAL_DRP_MODE_DISABLED);
    }
    else if (drp == DRP_MODE_DRP)
    {
        TCPORT_DRIVER.cc.Drp(tcpd, HAL_DRP_MODE_DRP);
    }
    else if (drp == DRP_MODE_SINKACC)
    {
        TCPORT_DRIVER.cc.Drp(tcpd, HAL_DRP_MODE_SINKACC);
    }
    else if (drp == DRP_MODE_SINKVPD)
    {
        TCPORT_DRIVER.cc.Drp(tcpd, HAL_DRP_MODE_SINKVPD);
    }
    else if (drp == DRP_MODE_SINKVPDACC)
    {
        TCPORT_DRIVER.cc.Drp(tcpd, HAL_DRP_MODE_SINKVPDACC);
    }
}

DRP_STAT_T fusb15xxx_tcpd_drp_stat(HAL_USBPD_T *tcpd)
{
    int             device_id = Dev_Id;
    enum drp_stat_t drp_stat  = DRP_MODE_DISABLED;
    HAL_CCSTAT_T   *ccstat    = TCPORT_DRIVER.cc.Status(tcpd);
    if (ccstat->drp == DRP_INACTIVE)
    {
        drp_stat = DRP_INACTIVE;
    }
    else if (ccstat->drp == DRP_ACTIVE)
    {
        drp_stat = DRP_ACTIVE;
    }
    else if (ccstat->drp == DRP_SOURCE)
    {
        drp_stat = DRP_SOURCE;
    }
    else if (ccstat->drp == DRP_SINK)
    {
        drp_stat = DRP_SINK;
    }
    return drp_stat;
}
#endif
void fusb15xxx_tcpd_vbus_init(HAL_USBPD_T *tcpd)
{
#if (DEVICE_TYPE != FUSB15101)
    TCPORT_DRIVER.vbus.Initialize(tcpd);
    /* Default intial state of output to switch */
    TCPORT_DRIVER.vbus.Clear(tcpd, HAL_VBUSIO_SOURCE | HAL_VBUSIO_SINK, 0);
#endif
}

void fusb15xxx_tcpd_vbus_deinit(HAL_USBPD_T *tcpd)
{
#if (DEVICE_TYPE != FUSB15101)
    TCPORT_DRIVER.vbus.EventsEnable(tcpd, false);
    /* Default intial state of output to switch */
    TCPORT_DRIVER.vbus.Clear(tcpd, HAL_VBUSIO_SOURCE | HAL_VBUSIO_SINK, 0);
#endif
}

void fusb15xxx_tcpd_vbus_events(HAL_USBPD_T *tcpd, bool en)
{
    TCPORT_DRIVER.vbus.EventsEnable(tcpd, en);
}

bool fusb15xxx_tcpd_set_vbus_source(HAL_USBPD_T *tcpd, unsigned mv, unsigned ma, PDO_T type)
{
    bool ret = false;
    if (mv == 0)
    {
        /* Turn off vbus switch */
        TCPORT_DRIVER.vbus.Clear(tcpd, HAL_VBUSIO_SOURCE | HAL_VBUSIO_SINK, 0);
    }
    else
    {
        TCPORT_DRIVER.vbus.Set(tcpd, HAL_VBUSIO_SOURCE, 0);
        ret = true;
    }

    return ret;
}

bool fusb15xxx_tcpd_set_vbus_sink(HAL_USBPD_T *tcpd, unsigned mv, unsigned ma, PDO_T type)
{
    bool ret = false;
#if (DEVICE_TYPE != FUSB15101)
    if (mv == 0)
    {
        TCPORT_DRIVER.vbus.Clear(tcpd, HAL_VBUSIO_SOURCE | HAL_VBUSIO_SINK, 0);
    }
    else
    {
        TCPORT_DRIVER.vbus.Set(tcpd, HAL_VBUSIO_SINK, 0);
    }
    ret = true;
#endif
    return ret;
}
#if (DEVICE_TYPE == FUSB15101)
uint32_t fusb15xxx_tcpd_vbus_pps_value(HAL_USBPD_T *tcpd)
{
    int      device_id  = Dev_Id;
    uint32_t vbus_value = 0;
    vbus_value = TCPORT_DRIVER.vbus.PPSValue(tcpd);
    return vbus_value;
}
#endif
uint32_t fusb15xxx_tcpd_vbus_value(HAL_USBPD_T *tcpd)
{
    int      device_id  = Dev_Id;
    uint32_t vbus_value = 0;
    vbus_value = TCPORT_DRIVER.vbus.Value(tcpd);
    return vbus_value;
}

void fusb15xxx_tcpd_set_vbus_res(HAL_USBPD_T *tcpd, HAL_USBPD_CHID_T tcpd_id, unsigned max)
{
    TCPORT_DRIVER.vbus.SetResolution(tcpd, max);
}

void fusb15xxx_tcpd_set_vbus_alarm(HAL_USBPD_T *tcpd, unsigned low, unsigned high)
{
    int device_id = Dev_Id;
#if (DEVICE_TYPE == FUSB15101)
    HAL_VBUS_STAT_T vbus_stat;
#endif
#if (DEVICE_TYPE == FUSB15101)
    vbus_stat = VBUS_DRIVER.Status(TCPORT_DRIVER.vbus.GetVbus(tcpd));
    if (vbus_stat.lgate_en)
    {
        ADC_DRIVER.ChannelSetAlarm(ADC_CH0, low, high);
        if (high)
        {
            ADC_DRIVER.ChannelSetHighAlarmInterrupt(ADC_CH0, true);
        }
        if (low)
        {
            ADC_DRIVER.ChannelSetLowAlarmInterrupt(ADC_CH0, true);
        }
    }
    else
    {
        if (vbus_stat.vbus_bld_en)
        {
            /* Bleed creates a voltage divider */
            low /= 2;
            high /= 2;
        }
        ADC_DRIVER.ChannelSetAlarm(ADC_CH1, low, high);
        if (high)
        {
            ADC_DRIVER.ChannelSetHighAlarmInterrupt(ADC_CH1, true);
        }
        if (low)
        {
            ADC_DRIVER.ChannelSetLowAlarmInterrupt(ADC_CH1, true);
        }
    }
#else
    TCPORT_DRIVER.vbus.Alarm(tcpd, low, high);
#endif
}

void fusb15xxx_tcpd_set_vbus_disch(HAL_USBPD_T *tcpd, VBUS_DISCH_T disch)
{
    HAL_VBUS_DISCH_T vdisch;
    if (disch == VBUS_DISCH_FAST)
    {
        vdisch = HAL_VBUS_DISCH0;
    }
    else if (disch == VBUS_DISCH_BLEED)
    {
        vdisch = HAL_VBUS_BLEED;
    }
    else if (disch == VBUS_DISCH_DEFAULT)
    {
        vdisch = HAL_VBUS_DISCH1;
    }
    else
    {
        vdisch = HAL_VBUS_DISCH_NONE;
    }
    TCPORT_DRIVER.vbus.Discharge(tcpd, vdisch);
}

CC_RP_T fusb15xxx_tcpd_pullup_value(HAL_USBPD_T *tcpd, CC_T cc)
{
    int           device_id = Dev_Id;
    HAL_CC_RP_T   rp        = HAL_CC_RP_NOCHANGE;
    enum cc_rp_t  rp_stat;
    HAL_CCSTAT_T *stat = TCPORT_DRIVER.cc.Status(tcpd);
    if (cc == CC1)
    {
        rp = stat->cc1;
    }
    else
    {
        rp = stat->cc2;
    }
    if (rp == HAL_CC_STAT_RPUSB)
    {
        rp_stat = CC_RP_DEF;
    }
    else if (rp == HAL_CC_STAT_RP1p5)
    {
        rp_stat = CC_RP_1p5A;
    }
    else if (rp == HAL_CC_STAT_RP3p0)
    {
        rp_stat = CC_RP_3p0A;
    }
    else if (rp == HAL_CC_STAT_SNKOPEN)
    {
        rp_stat = CC_RP_OFF;
    }
    else
    {
        rp_stat = HAL_CC_RP_NOCHANGE;
    }
    return rp_stat;
}

void fusb15xxx_tcpd_set_term(HAL_USBPD_T *tcpd, int mask, CC_TERM_T term)
{
    HAL_CC_T cc = 0;
    if (mask & CC1)
    {
        cc |= HAL_CC1;
    }
    if (mask & CC2)
    {
        cc |= HAL_CC2;
    }

    if (term == CC_TERM_OPEN)
    {
        TCPORT_DRIVER.cc.Termination(tcpd, cc, HAL_CC_TERM_OPEN);
    }
    else if (term == CC_TERM_RA)
    {
        TCPORT_DRIVER.cc.Termination(tcpd, cc, HAL_CC_TERM_RA);
    }
    else if (term == CC_TERM_RD)
    {
        TCPORT_DRIVER.cc.Termination(tcpd, cc, HAL_CC_TERM_RD);
    }
    else if (term == CC_TERM_RP)
    {
        TCPORT_DRIVER.cc.Termination(tcpd, cc, HAL_CC_TERM_RP);
    }
}

void fusb15xxx_tcpd_vconn_enable(HAL_USBPD_T *tcpd, bool en, HAL_VCONN_OCP_T ocp)
{
#if (DEVICE_TYPE == FUSB15101)
    if (en)
    {
        TCPORT_DRIVER.cc.VconnEnable(tcpd, en, HAL_VCONN_OCP_50mA);
    }
    else
    {
        TCPORT_DRIVER.cc.VconnEnable(tcpd, en, HAL_VCONN_OCP_OFF);
    }
#else
    TCPORT_DRIVER.cc.VconnEnable(tcpd, en, ocp);
#endif
}

void fusb15xxx_tcpd_pd_enable(HAL_USBPD_T *tcpd, bool en)
{
    if (en)
    {
        TCPORT_DRIVER.pd.Enable(tcpd);
    }
    else
    {
        TCPORT_DRIVER.pd.Disable(tcpd);
    }
}

void fusb15xxx_tcpd_pd_sop_enable(HAL_USBPD_T *tcpd, unsigned mask)
{
    TCPORT_DRIVER.pd.SopEnable(tcpd, mask);
}

void fusb15xxx_tcpd_pd_rev(HAL_USBPD_T *tcpd, PD_REV_T rev, SOP_T sop)
{
    if (rev == PD_REV3)
    {
        TCPORT_DRIVER.pd.SetPdRevision(tcpd, HAL_PD_REV3);
    }
    else
    {
        TCPORT_DRIVER.pd.SetPdRevision(tcpd, HAL_PD_REV2);
    }
    TCPORT_DRIVER.pd.SetGoodCRCRevision(tcpd, HAL_PD_REV2);
}

void fusb15xxx_tcpd_set_pd_dfp(HAL_USBPD_T *tcpd, bool dfp_en)
{
    TCPORT_DRIVER.pd.SetDfp(tcpd, dfp_en);
}

void fusb15xxx_tcpd_set_pd_source(HAL_USBPD_T *tcpd, bool src_en)
{
    TCPORT_DRIVER.pd.Source(tcpd, src_en);
}

void fusb15xxx_tcpd_set_bist_mode(HAL_USBPD_T *tcpd, BIST_MODE_T mode)
{
    if (mode == BIST_MODE_TX)
    {
        TCPORT_DRIVER.pd.BistEnable(tcpd, HAL_BIST_TX);
    }
    else if (mode == BIST_MODE_RX)
    {
        TCPORT_DRIVER.pd.BistEnable(tcpd, HAL_BIST_RX);
    }
    else
    {
        TCPORT_DRIVER.pd.BistEnable(tcpd, HAL_BIST_DISABLE);
    }
}

PORT_TX_STAT_T fusb15xxx_tcpd_port_tx_status(HAL_USBPD_T *tcpd)
{
    enum port_tx_stat_t tx_stat = TX_FAILED;
    HAL_PDSTAT_T       *stat;
    stat = TCPORT_DRIVER.pd.Status(tcpd);
    if (stat->txstat == HAL_TX_BUSY)
    {
        tx_stat = TX_BUSY;
    }
    else if (stat->txstat == HAL_TX_SUCCESS)
    {
        tx_stat = TX_SUCCESS;
    }
    else if (stat->txstat == HAL_TX_DISCARD)
    {
        tx_stat = TX_DISCARD;
    }
    else if (stat->txstat == HAL_TX_FAILED)
    {
        tx_stat = TX_FAILED;
    }
    else if (stat->txstat == HAL_TX_RETRY)
    {
        tx_stat = TX_RETRY;
    }
    else
    {
        tx_stat = HAL_TX_FAILED;
    }
    return tx_stat;
}

bool fusb15xxx_tcpd_pd_tx(HAL_USBPD_T *tcpd, uint8_t const *buf, unsigned long len, SOP_T sop,
                          int retries)
{
    HAL_ERROR_T error;
    HAL_SOP_T   txsop;
    bool        ret = true;
    if (sop == SOP_SOP0)
    {
        txsop = HAL_SOP_TYPE_SOP;
    }
    else if (sop == SOP_SOP1)
    {
        txsop = HAL_SOP_TYPE_SOP1;
    }
    else if (sop == SOP_SOP2)
    {
        txsop = HAL_SOP_TYPE_SOP2;
    }
    else
    {
        ret = false;
    }
    if (ret == true)
    {
        error = TCPORT_DRIVER.pd.Transmit(tcpd, buf, len, txsop, retries);
        if (error == HAL_ERROR)
        {
            ret = false;
        }
    }
    return ret;
}

bool fusb15xxx_tcpd_pd_check_rx(HAL_USBPD_T *tcpd)
{
    bool          ret = false;
    HAL_PDSTAT_T *stat;
    stat = TCPORT_DRIVER.pd.Status(tcpd);
    if (stat->rxmsg)
    {
        ret = true;
    }
    return ret;
}

SOP_T fusb15xxx_tcpd_pd_rx(HAL_USBPD_T *tcpd, uint8_t *buf, unsigned long len)
{
    enum sop_t sop_stat = SOP_ERROR;
    HAL_SOP_T  sop;
    TCPORT_DRIVER.pd.Status(tcpd)->rxmsg = false;
    sop                                  = TCPORT_DRIVER.pd.Receive(tcpd, buf, len);
    if (sop == HAL_SOP_TYPE_SOP)
    {
        sop_stat = SOP_SOP0;
    }
    else if (sop == HAL_SOP_TYPE_SOP1)
    {
        sop_stat = SOP_SOP1;
    }
    else if (sop == HAL_SOP_TYPE_SOP2)
	{
		sop_stat = SOP_SOP2;
	}
    return sop_stat;
}

/**************************************************************************************
 ** DCDC Function Driver abstraction
 **************************************************************************************/
#if (CONFIG_ENABLED(LEGACY_I2C))
HAL_I2C_T *fusb15xxx_i2c_enable(I2C_Port_Type *i2c_p)
{
    return I2C_DRIVER.Initialze(i2c_p);
}
#else
HAL_I2C_T *fusb15xxx_i2c_enable(int i2c_id)
{
    return I2C_DRIVER.Initialize(i2c_id);
}
#endif
void fusb15xxx_i2c_disable(HAL_I2C_T *i2c)
{
    I2C_DRIVER.Disable(i2c);
}

void fusb15xxx_i2c_master_enable(HAL_I2C_T *i2c, uint32_t prescale)
{
    I2C_DRIVER.master.Configure(i2c, prescale);
}
#if (DEVICE_TYPE != FUSB15101) && !(CONFIG_ENABLED(LEGACY_I2C))
void fusb15xxx_i2c_master_disable(HAL_I2C_T *const i2c)
{
    I2C_DRIVER.master.Disable(i2c);
}
#endif
void fusb15xxx_i2c_slave_enable(HAL_I2C_T *i2c, uint8_t addr, uint32_t prescale)
{
    I2C_DRIVER.slave.Configure(i2c, addr, prescale);
}
#if (DEVICE_TYPE != FUSB15101) && !(CONFIG_ENABLED(LEGACY_I2C))
void fusb15xxx_i2c_slave_disable(HAL_I2C_T *i2c)
{
    I2C_DRIVER.slave.Disable(i2c);
}
#endif
HAL_ERROR_T fusb15xxx_i2c_write(HAL_I2C_T *const i2c, uint8_t addr, uint8_t *pBuf, uint32_t len,
                                uint32_t timeout)
{
    return I2C_DRIVER.master.Write(i2c, addr, pBuf, len, timeout);
}

HAL_ERROR_T fusb15xxx_i2c_read(HAL_I2C_T *i2c, uint8_t addr, uint8_t *pBuf, uint32_t len,
                               uint32_t timeout)
{
    return I2C_DRIVER.master.Read(i2c, addr, pBuf, len, timeout);
}
void fusb15xxx_i2c_write_with_interrupt(HAL_I2C_T *i2c, uint8_t addr, uint8_t *pBuf, uint32_t len,
                                        HAL_I2CCB_T cb)
{
    I2C_DRIVER.master.WriteWithInterrupt(i2c, addr, pBuf, len, cb);
}

void fusb15xxx_i2c_read_with_interrupt(HAL_I2C_T *i2c, uint8_t addr, uint8_t *pBuf, uint32_t len,
                                       HAL_I2CCB_T cb)
{
    I2C_DRIVER.master.ReadWithInterrupt(i2c, addr, pBuf, len, cb);
}

#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
void fusb15xxx_i2c_write_with_dma(HAL_I2C_T *i2c, uint8_t addr, uint8_t *pBuf, uint32_t len,
                                  HAL_I2CCB_T cb)
{
    I2C_DRIVER.master.WriteWithDMA(i2c, addr, pBuf, len, cb);
}

void fusb15xxx_i2c_read_with_dma(HAL_I2C_T *i2c, uint8_t addr, uint8_t *pBuf, uint32_t len,
                                 HAL_I2CCB_T cb)
{
    I2C_DRIVER.master.ReadWithDMA(i2c, addr, pBuf, len, cb);
}
#endif

void fusb15xxx_i2c_read_register(HAL_I2C_T *i2c, uint8_t addr, uint8_t reg, uint8_t *pBuf,
                                 uint32_t len, HAL_I2CCB_T cb)
{
    I2C_DRIVER.master.ReadRegister(i2c, addr, reg, pBuf, len, cb);
}

void fusb15xxx_i2c_slave_start(HAL_I2C_T *i2c, uint8_t *pBuf, uint32_t size, HAL_SLAVE_CB_T cb)
{
    I2C_DRIVER.slave.Start(i2c, pBuf, size, cb);
}

void fusb15xxx_i2c_slave_set_offset(HAL_I2C_T *i2c, uint32_t offset)
{
    I2C_DRIVER.slave.SetOffset(i2c, offset);
}

#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
void fusb15xxx_i2c_slave_setup_rx_dma(HAL_I2C_T *i2c, uint8_t *pBuf, uint32_t size,
                                      HAL_SLAVE_CB_T cb)
{
    I2C_DRIVER.slave.SetupDMARxBuffer(i2c, pBuf, size, cb);
}

void fusb15xxx_i2c_slave_setup_tx_dma(HAL_I2C_T *i2c, uint8_t *pBuf, uint32_t size,
                                      HAL_SLAVE_CB_T cb)
{
    I2C_DRIVER.slave.SetupDMATxBuffer(i2c, pBuf, size, cb);
}
#endif

void fusb15xxx_i2c_irq_enable(HAL_I2C_T *i2c, HAL_I2CINT_T irq)
{
    I2C_DRIVER.irq.Enable(i2c, irq);
}

void fusb15xxx_i2c_irq_disable(HAL_I2C_T *i2c)
{
    I2C_DRIVER.irq.Disable(i2c);
}

void fusb15xxx_i2c_irq_set(HAL_I2C_T *i2c, uint32_t val)
{
    I2C_DRIVER.irq.Set(i2c, val);
}

void fusb15xxx_i2c_irq_clear(HAL_I2C_T *i2c)
{
    I2C_DRIVER.irq.Clear(i2c);
}

uint8_t fusb15xxx_i2c_irq_status(HAL_I2C_T *i2c)
{
    return I2C_DRIVER.irq.Status(i2c);
}

void fusb15xxx_i2c_irq_set_callback(HAL_I2C_T *i2c, HAL_INTCB_T cb)
{
    I2C_DRIVER.irq.SetCallback(i2c, cb);
}

/**************************************************************************************
 ** BC1P2 Function Driver abstraction
 **************************************************************************************/
void fusb15xxx_adc_ch_enable(int ch, bool en)
{
    ADC_DRIVER.ChannelEnable((ADC_CHx_T)ch, en);
}

void fusb15xxx_adc_ch_disable(int ch, bool en)
{
    ADC_DRIVER.ChannelEnable((ADC_CHx_T)ch, en);
}

HAL_ADC_CHx_STAT_T fusb15xxx_adc_ch_status(int dp_ch)
{
    return ADC_DRIVER.ChannelStatus((ADC_CHx_T)dp_ch);
}

bool fusb15xxx_adc_ch_ready(int dp_ch)
{
    HAL_ADC_CHx_STAT_T stat = ADC_DRIVER.ChannelStatus((ADC_CHx_T)dp_ch);
    return (stat.done == 1) ? true : false;
}

unsigned long fusb15xxx_adc_ch_data(int ch)
{
    return ADC_DRIVER.ChannelData(ch);
}

void fusb15xxx_adc_ch_set_alarm(int ch, int low, int high)
{
    ADC_DRIVER.ChannelSetAlarm(ch, low, high);
}

void fusb15xxx_adc_ch_set_low_alarm_interrupt(int ch, bool en)
{
    ADC_DRIVER.ChannelSetLowAlarmInterrupt(ch, en);
}

void fusb15xxx_adc_ch_set_high_alarm_interrupt(int ch, bool en)
{
    ADC_DRIVER.ChannelSetHighAlarmInterrupt(ch, en);
}

uint32_t fusb15xxx_adc_ch_interrupt_status(int ch)
{
    return ADC_DRIVER.InterruptStatus(ch);
}

void fusb15xxx_adc_ch_interrupt_enable(bool en)
{
    ADC_DRIVER.InterruptEnable(en);
}

void fusb15xxx_adc_ch_interrupt_clear(void)
{
    ADC_DRIVER.InterruptClear();
}

void fusb15xxx_adc_ch_set_reference(int ch, int refsel)
{
    ADC_DRIVER.ChannelSetReference(ch, refsel);
}

uint32_t fusb15xxx_adc_ch_reference(int ch)
{
    return ADC_DRIVER.ChannelReference(ch);
}

#if (DEVICE_TYPE != FUSB15101)
void fusb15xxx_adc_ch_autoscale_enable(HAL_ADCHx_T ch, bool en)
{
    ADC_DRIVER.ChannelAutoscaleEnable(ch, en);
}
#endif
void fusb15xxx_enable_ntc(int ntc_port, bool en)
{
    NTC_DRIVER.Enable(NTC_Port_A_B_Interface, ntc_port, en);
}
void fusb15xxx_ntc_convert_adc_to_temp(int adc)
{
    NTC_DRIVER.ConvertADC2Temp(adc);
}

HAL_USBCHG_T *fusb15xxx_usbchg_enable(int usbchg_id)
{
    return USBCHG_DRIVER.Enable(usbchg_id);
}

void fusb15xxx_usbchg_disable(HAL_USBCHG_T *usbchg)
{
    USBCHG_DRIVER.Disable(usbchg);
}

void fusb15xxx_usbchg_idp_src(HAL_USBCHG_T *usbchg, bool en)
{
    USBCHG_DRIVER.SetIdpSource(usbchg, en);
}

void fusb15xxx_usbchg_idp_snk(HAL_USBCHG_T *usbchg, bool en)
{
    USBCHG_DRIVER.SetIdpSink(usbchg, en);
}

void fusb15xxx_usbchg_rdm_dwn(HAL_USBCHG_T *usbchg, bool en)
{
    USBCHG_DRIVER.SetRdmDown(usbchg, en);
}

void fusb15xxx_usbchg_rdp_dwn(HAL_USBCHG_T *usbchg, bool en)
{
    USBCHG_DRIVER.SetRdpDown(usbchg, en);
}

void fusb15xxx_usbchg_vdp_src(HAL_USBCHG_T *usbchg, bool en)
{
    USBCHG_DRIVER.SetVdpSource(usbchg, en);
}

void fusb15xxx_usbchg_vdm_src(HAL_USBCHG_T *usbchg, bool en)
{
    USBCHG_DRIVER.SetVdmSource(usbchg, en);
}

void fusb15xxx_usbchg_idm_snk(HAL_USBCHG_T *usbchg, bool en)
{
    USBCHG_DRIVER.SetIdmSink(usbchg, en);
}

void fusb15xxx_usbchg_host_sw(HAL_USBCHG_T *usbchg, bool en)
{
    USBCHG_DRIVER.HostSwitchEnable(usbchg, en);
}

void fusb15xxx_usbchg_rdiv(HAL_USBCHG_T *usbchg, bool en)
{
    USBCHG_DRIVER.SetRdiv(usbchg, en);
}

void fusb15xxx_usbchg_rdcp(HAL_USBCHG_T *usbchg, bool en)
{
    USBCHG_DRIVER.SetRdcp(usbchg, en);
}

void fusb15xxx_usbchg_rdat(HAL_USBCHG_T *usbchg, bool en)
{
    USBCHG_DRIVER.SetRdpLeakage(usbchg, en);
    USBCHG_DRIVER.SetRdmLeakage(usbchg, en);
}

/********************************************************************************
 * FUSB5101 Device Initialization
 *
 ********************************************************************************/
#if (DEVICE_TYPE == FUSB15101)
static void fusb15101_init(void)
{
    /* Peripheral clock configurations */
    HAL_CMU_ADC_CLOCK_CONFIGURE(Clock_Management_Unit_ADCCLK_CFG_ADC_CLK_SEL_CLOCK_SLOW,
                                Clock_Management_Unit_ADCCLK_CFG_ADC_DIV_DIV1);
    HAL_CMU_TIMER_0_CLOCK_CONFIGURE(Clock_Management_Unit_TIMERS_CLK_CFG_GPT0_CLK_SRC_CLK_SLOW,
                                    Clock_Management_Unit_TIMERS_CLK_CFG_GPT0_CLK_DIV_DIV12);
    HAL_CMU_WAKE_UP_TIMER_CLOCK_CONFIGURE(Clock_Management_Unit_TIMERS_CLK_CFG_WUT_CLK_SRC_CLK_SLOW,
                                          Clock_Management_Unit_TIMERS_CLK_CFG_WUT_CLK_DIV_DIV12);
    HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_ADC_EN_Msk
                        | Clock_Management_Unit_PCLK_FEN_USBPDA_HSCLK_EN_Msk
                        | Clock_Management_Unit_PCLK_FEN_USBPDA_LSCLK_EN_Msk
                        | Clock_Management_Unit_PCLK_FEN_VBUS_CTRL_EN_Msk
                        | Clock_Management_Unit_PCLK_FEN_DAC_EN_Msk
                        | Clock_Management_Unit_PCLK_FEN_CC_CV_EN_Msk
                        | Clock_Management_Unit_PCLK_FEN_TIM0_EN_Msk
                        | Clock_Management_Unit_PCLK_FEN_USBCHGA_CLK_EN_Msk
                        | Clock_Management_Unit_PCLK_FEN_WUT_Msk);

    /* Enable peripherals */
    ADC_DRIVER.Configure(ADC_Interface_ADC_CTRL_adc_loop_Msk
                         | ADC_Interface_ADC_CTRL_adc_sleep_op_Msk);

    /* Systick counter is Timer0 and resolution of 0.1ms. 240KHz/DIV24 */
    TIMER_DRIVER.Configure(Timer_0, Timer_0_GPT0_TIMER1CONTROL_TIMER_PRESCALE_DIV1);
    TIMER_DRIVER.Configure(Wake_Up_Timer, Timer_0_GPT0_TIMER1CONTROL_TIMER_PRESCALE_DIV1);
    TIMER_DRIVER.InterruptEnable(Wake_Up_Timer, true);
    TIMER_DRIVER.Start(Timer_0, ~0UL, true);
    timer_init(HAL_SYSTICK_Count, 10, 1);

    ADC_DRIVER.ChannelEnable(ADC_CH0, true);
    ADC_DRIVER.ChannelEnable(ADC_CH1, true);
    ADC_DRIVER.ChannelEnable(ADC_CH2, true);
    NVIC_EnableIRQ(ADC_IRQn);
    NVIC_EnableIRQ(WUT_IRQn);

#if CONFIG_ENABLED(FAULT_TEMP)
    HAL_PORTCFG_T portntc = {.an = PORT_ANALOG, .alt = PORT_ALTERNATE};
    PORT_DRIVER.Configure(PORT3, &portntc);
    PORT_DRIVER.Configure(PORT4, &portntc);
    NTC_DRIVER.Enable(HAL_NTC_A, TRUE);
    NTC_DRIVER.Enable(HAL_NTC_B, TRUE);
    ADC_DRIVER.ChannelEnable(ADC_CH5, TRUE);
    ADC_DRIVER.ChannelEnable(ADC_CH6, TRUE);
#endif

    /*Enable BC1.2 to allow enabling of D+- OVP protection*/
    USBCHG_DRIVER.Enable(HAL_USBCHG_A);

    SCB->SCR = SCB_SCR_SLEEPDEEP_Msk;
    dbg_msg("Sys Init\r\n", 0);
}

#elif (DEVICE_TYPE == FUSB15200)
static void fusb15101_init(void)
{
}
/********************************************************************************
 * FUSB5200 Device Initialization
 *
 ********************************************************************************/
static void fusb15200_init(void)
{
    uint32_t pclk_mask = 0;

    HAL_GPIOCFG_T cfg = {.iodir = HAL_GPIO_OUTPUT, .ppod = HAL_GPIO_PUSH_PULL};

    /** @todo Add pulldown on SWD. Move this to bootloader. */
    PORT_Interface->PORT_CFG &= ~(1U << 30);
    PORT_Interface->PULL_DOWN |= (0x02);
    /* Init deep sleep */
    SCB->SCR = SCB_SCR_SLEEPDEEP_Msk;

    /* Peripheral clock configurations */
    HAL_CMU_ADC_CLOCK_CONFIGURE(Clock_Management_Unit_ADCCLK_CFG_ADC_CLK_SEL_CLOCK_SLOW,
                                Clock_Management_Unit_ADCCLK_CFG_ADC_DIV_DIV1);
    /*Initialize Timer 0*/
    HAL_CMU_TIMER_0_CLOCK_CONFIGURE(Clock_Management_Unit_TIMERS_CLK_CFG_GPT_CLK_SRC_CLK_SLOW,
                                    Clock_Management_Unit_TIMERS_CLK_CFG_GPT_CLK_DIV_DIV12);

    HAL_CMU_WAKE_UP_TIMER_CLOCK_CONFIGURE(Clock_Management_Unit_TIMERS_CLK_CFG_WUT_CLK_SRC_CLK_SLOW,
                                          Clock_Management_Unit_TIMERS_CLK_CFG_WUT_CLK_DIV_DIV12);

    /* Peripheral clock configurations */
    pclk_mask =
        (Clock_Management_Unit_PCLK_FEN_USBPDA_HSCLK_EN_Msk
         | Clock_Management_Unit_PCLK_FEN_USBPDB_HSCLK_EN_Msk
         | Clock_Management_Unit_PCLK_FEN_USBPDA_LSCLK_EN_Msk
         | Clock_Management_Unit_PCLK_FEN_USBPDB_LSCLK_EN_Msk
         | Clock_Management_Unit_PCLK_FEN_ADC_EN_Msk
         | Clock_Management_Unit_PCLK_FEN_TIM0_EN_Msk
#if STACK_MON
         | Clock_Management_Unit_PCLK_FEN_TIM1_EN_Msk
#endif
         | Clock_Management_Unit_PCLK_FEN_WUT_Msk
         | Clock_Management_Unit_PCLK_FEN_GPIO_EN_Msk
         | Clock_Management_Unit_PCLK_FEN_WDG_EN_Msk);

#if CONFIG_ENABLED(LEGACY_CHARGING)
    pclk_mask |= (Clock_Management_Unit_PCLK_FEN_USBCHGA_CLK_EN_Msk
                  | Clock_Management_Unit_PCLK_FEN_USBCHGB_CLK_EN_Msk);
#endif
    HAL_CMU_PCLK_ENABLE(pclk_mask);

    /* Enable peripherals */
    ADC_DRIVER.Configure(ADC_Interface_ADC_CTRL_adc_loop_Msk
                         | ADC_Interface_ADC_CTRL_adc_sleep_op_Msk);
    TIMER_DRIVER.Configure(Timer_0, Timer_GPT_TIMER1CONTROL_TIMER_PRESCALE_DIV1);
    TIMER_DRIVER.Configure(Wake_Up_Timer, Timer_GPT_TIMER1CONTROL_TIMER_PRESCALE_DIV1);
    GPIO_DRIVER.Configure(HAL_GPIO6, &cfg, HAL_GPIO_SHARED);
    GPIO_DRIVER.Configure(HAL_GPIO7, &cfg, HAL_GPIO_SHARED);
    GPIO_DRIVER.Configure(HAL_GPIO16, &cfg, HAL_GPIO_SHARED);
    GPIO_DRIVER.Set(HAL_GPIO16, false, HAL_GPIO_SHARED);
    TIMER_DRIVER.InterruptEnable(Wake_Up_Timer, true);
    TIMER_DRIVER.Start(Timer_0, ~0U, true);
    timer_init(HAL_SYSTICK_Count, 10, 1);

    Power_Management_Unit->PMU_INT_MSK = ~0;
    Power_Management_Unit->PMU_INT = ~0;

    NVIC_ClearPendingIRQ(PMU_IRQn);
    NVIC_SetPriority(WUT_IRQn, 1);
    NVIC_EnableIRQ(WUT_IRQn);

#if STACK_MON
    /* Initialize Timer 1 used for stack monitoring */
    HAL_CMU_TIMER_1_CLOCK_CONFIGURE(Clock_Management_Unit_TIMERS_CLK_CFG_GPT_CLK_SRC_CLK_SLOW,
                                    Clock_Management_Unit_TIMERS_CLK_CFG_GPT_CLK_DIV_DIV12);
    TIMER_DRIVER.Configure(Timer_1, Timer_GPT_TIMER1CONTROL_TIMER_PRESCALE_DIV1);
    TIMER_DRIVER.InterruptEnable(Timer_1, true);
    TIMER_DRIVER.Start(Timer_1, TIMER_1_ONE_SEC, true);
    NVIC_SetPriority(TIM1_IRQn, 1);
    NVIC_EnableIRQ(TIM1_IRQn);
#endif

#if CONFIG_LOG
    SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_TRIM);
#endif
    dbg_msg("Sys Init\r\n");
}

#elif (DEVICE_TYPE == FUSB15201)
void fusb15101_init(void)
{
}
void fusb15200_init(void)
{
}
static void fusb15201_init(void)
{
    uint32_t      pclk_mask = 0;
    HAL_PORTCFG_T portcfg   = {.an = PORT_DIGITAL, .alt = PORT_PRIMARY, .pd = PORT_PULLDOWN_ENABLE};
    PORT_DRIVER.Configure(PORT2, &portcfg, HAL_GPIO_SHARED);
    /** @todo Add pulldown on SWD. Move this to bootloader. */
    PORT_Interface->PORT_CFG &= ~(1U << 30);
    PORT_Interface->PULL_DOWN |= 0x02;
    /* Init deep sleep */
    SCB->SCR = SCB_SCR_SLEEPDEEP_Msk;

    /* Peripheral clock configurations */
    HAL_CMU_ADC_CLOCK_CONFIGURE(Clock_Management_Unit_ADCCLK_CFG_ADC_CLK_SEL_CLOCK_SLOW,
                                Clock_Management_Unit_ADCCLK_CFG_ADC_DIV_DIV1);
    /*Initialize Timer 0*/
    HAL_CMU_TIMER_0_CLOCK_CONFIGURE(Clock_Management_Unit_TIMERS_CLK_CFG_GPT_CLK_SRC_CLK_SLOW,
                                    Clock_Management_Unit_TIMERS_CLK_CFG_GPT_CLK_DIV_DIV12);
    HAL_CMU_WAKE_UP_TIMER_CLOCK_CONFIGURE(Clock_Management_Unit_TIMERS_CLK_CFG_WUT_CLK_SRC_CLK_SLOW,
                                          Clock_Management_Unit_TIMERS_CLK_CFG_WUT_CLK_DIV_DIV12);

    /* Peripheral clock configurations */
    pclk_mask =
        (Clock_Management_Unit_PCLK_FEN_USBPDA_HSCLK_EN_Msk
         | Clock_Management_Unit_PCLK_FEN_USBPDB_HSCLK_EN_Msk
         | Clock_Management_Unit_PCLK_FEN_USBPDA_LSCLK_EN_Msk
         | Clock_Management_Unit_PCLK_FEN_USBPDB_LSCLK_EN_Msk
         | Clock_Management_Unit_PCLK_FEN_ADC_EN_Msk
         | Clock_Management_Unit_PCLK_FEN_TIM0_EN_Msk
#if STACK_MON
         | Clock_Management_Unit_PCLK_FEN_TIM1_EN_Msk
#endif
         | Clock_Management_Unit_PCLK_FEN_WUT_Msk
         | Clock_Management_Unit_PCLK_FEN_GPIO_EN_Msk
         | Clock_Management_Unit_PCLK_FEN_WDG_EN_Msk);

#if CONFIG_ENABLED(LEGACY_CHARGING)
    pclk_mask |= (Clock_Management_Unit_PCLK_FEN_USBCHGA_CLK_EN_Msk
                  | Clock_Management_Unit_PCLK_FEN_USBCHGB_CLK_EN_Msk);
#endif
    HAL_CMU_PCLK_ENABLE(pclk_mask);

    /* Enable peripherals */
#if CONFIG_ENABLED(CUSTOM_APPLICATION)
    ADC_DRIVER.Configure(ADC_Interface_ADC_CTRL_adc_loop_Msk
                         | ADC_Interface_ADC_CTRL_adc_sleep_op_Msk);
#else
    ADC_DRIVER.Configure(ADC_Interface_ADC_CTRL_adc_loop_Msk);
#endif
    /* enable averaging ADC */
    ADC_Interface->ADC_AVG = ADC_Interface_ADC_AVG_adc_avg_Msk;
    TIMER_DRIVER.Configure(Timer_0, Timer_GPT_TIMER1CONTROL_TIMER_PRESCALE_DIV1);
    TIMER_DRIVER.Configure(Wake_Up_Timer, Timer_GPT_TIMER1CONTROL_TIMER_PRESCALE_DIV1);
    TIMER_DRIVER.InterruptEnable(Wake_Up_Timer, true);
    TIMER_DRIVER.Start(Timer_0, ~0U, true);
    timer_init(HAL_SYSTICK_Count, 10, 1);

    Power_Management_Unit->PMU_INT_MSK = ~0;
    Power_Management_Unit->PMU_INT     = ~0;

    NVIC_ClearPendingIRQ(PMU_IRQn);
    NVIC_SetPriority(WUT_IRQn, 1);
    NVIC_EnableIRQ(WUT_IRQn);
#if (CONFIG_ENABLED(CUSTOM_APPLICATION))
    i2c_master[2] = I2C_DRIVER.Initialze(I2C_Port_3);
#endif

#if STACK_MON
    /* Initialize Timer 1 used for stack monitoring */
    HAL_CMU_TIMER_1_CLOCK_CONFIGURE(Clock_Management_Unit_TIMERS_CLK_CFG_GPT_CLK_SRC_CLK_SLOW,
                                    Clock_Management_Unit_TIMERS_CLK_CFG_GPT_CLK_DIV_DIV12);
    TIMER_DRIVER.Configure(Timer_1, Timer_GPT_TIMER1CONTROL_TIMER_PRESCALE_DIV1);
    TIMER_DRIVER.InterruptEnable(Timer_1, true);
    TIMER_DRIVER.Start(Timer_1, TIMER_1_ONE_SEC, true);
    NVIC_SetPriority(TIM1_IRQn, 1);
    NVIC_EnableIRQ(TIM1_IRQn);
#endif

#if CONFIG_LOG
    SEGGER_RTT_ConfigUpBuffer(0, NULL, NULL, 0, SEGGER_RTT_MODE_NO_BLOCK_TRIM);
#endif
    __enable_irq();
    dbg_msg("Sys Init\r\n");
}

#elif (DEVICE_TYPE == FUSB15201P)
void fusb15101_init(void)
{
}
void fusb15200_init(void)
{
}
void fusb15201_init(void)
{
}
static void fusb15201p_init(void)
{
    /* Peripheral clock configurations */
    HAL_CMU_ADC_CLOCK_CONFIGURE(Clock_Management_Unit_ADCCLK_CFG_ADC_CLK_SEL_CLOCK_SLOW,
                                Clock_Management_Unit_ADCCLK_CFG_ADC_DIV_DIV1);
    /*TODO: to change CLK_DIV1 to CLK_DIV12 when already available*/
    HAL_CMU_TIMER_0_CLOCK_CONFIGURE(
        Clock_Management_Unit_TIMERS_CLK_CFG_GPT0_CLK_SRC_CLK_SLOW,
        //	                                Clock_Management_Unit_TIMERS_CLK_CFG_GPT0_CLK_DIV_DIV1);
        Clock_Management_Unit_TIMERS_CLK_CFG_GPT0_CLK_DIV_DIV12);

    /*TODO:to change CLK_DIV1 to CLK_DIV12 when already available*/
    HAL_CMU_WAKE_UP_TIMER_CLOCK_CONFIGURE(
        Clock_Management_Unit_TIMERS_CLK_CFG_WUT_CLK_SRC_CLK_SLOW,
        //	                                     Clock_Management_Unit_TIMERS_CLK_CFG_WUT_CLK_DIV_DIV1);
        Clock_Management_Unit_TIMERS_CLK_CFG_WUT_CLK_DIV_DIV12);

    HAL_CMU_PCLK_ENABLE(
        Clock_Management_Unit_PCLK_FEN_USBCHGA_CLK_EN_Msk
        | Clock_Management_Unit_PCLK_FEN_USBCHGB_CLK_EN_Msk
        | Clock_Management_Unit_PCLK_FEN_USBPDA_HSCLK_EN_Msk
        | Clock_Management_Unit_PCLK_FEN_USBPDB_HSCLK_EN_Msk
        | Clock_Management_Unit_PCLK_FEN_USBPDA_LSCLK_EN_Msk
        | Clock_Management_Unit_PCLK_FEN_USBPDB_LSCLK_EN_Msk
        | Clock_Management_Unit_PCLK_FEN_ADC_EN_Msk | Clock_Management_Unit_PCLK_FEN_TIM0_EN_Msk
        | Clock_Management_Unit_PCLK_FEN_WUT_Msk | Clock_Management_Unit_PCLK_FEN_GPIO_EN_Msk
        | Clock_Management_Unit_PCLK_FEN_WDG_EN_Msk);

    /* Enable peripherals */
#if CONFIG_ENABLED(CUSTOM_APPLICATION)
    ADC_DRIVER.Configure(ADC_Interface_ADC_CTRL_adc_loop_Msk
                         | ADC_Interface_ADC_CTRL_adc_sleep_op_Msk);
#else
    ADC_DRIVER.Configure(ADC_Interface_ADC_CTRL_adc_loop_Msk);
#endif
	/* enable averaging ADC */
	ADC_Interface->ADC_AVG = ADC_Interface_ADC_AVG_adc_avg_Msk;
    /* Systick counter is Timer0 and resolution of 0.1ms. 240KHz/DIV24 */
    TIMER_DRIVER.Configure(Timer_0, Timer_0_GPT0_TIMER1CONTROL_TIMER_PRESCALE_DIV1);
    TIMER_DRIVER.Configure(Wake_Up_Timer, Timer_0_GPT0_TIMER1CONTROL_TIMER_PRESCALE_DIV1);

    TIMER_DRIVER.InterruptEnable(Wake_Up_Timer, true);
    TIMER_DRIVER.Start(Timer_0, ~0UL, true);

    /*TODO: change to correct scale value when CLock divide 12 becomes available */
    //timer_init(HAL_SYSTICK_Count, 120, 1);
    timer_init(HAL_SYSTICK_Count, 10, 1);

    /*Enable Interrupt*/
    NVIC_EnableIRQ(ADC_IRQn);
    NVIC_SetPriority(WUT_IRQn, 1);
    NVIC_EnableIRQ(WUT_IRQn);

#if CONFIG_ENABLED(FAULT_TEMP)
    HAL_PORTCFG_T portntc = {.an = PORT_ANALOG, .alt = PORT_ALTERNATE};
    PORT_DRIVER.Configure(PORT3, &portntc, HAL_GPIO_B);
    PORT_DRIVER.Configure(PORT7, &portntc, HAL_GPIO_B);
    NTC_DRIVER.Enable(HAL_NTC_A, true);
    NTC_DRIVER.Enable(HAL_NTC_B, true);
    ADC_DRIVER.ChannelEnable(ADC_CH3, true);
    ADC_DRIVER.ChannelEnable(ADC_CH7, true);
#endif
    /* Configure I/O */
    HAL_GPIOCFG_T outGPIOConfig = {
        .iodir = HAL_GPIO_OUTPUT,
        .ppod  = HAL_GPIO_PUSH_PULL,
    };
    HAL_PORTCFG_T portCfg = {.an  = PORT_DIGITAL,
                             .alt = PORT_PRIMARY,
                             .pu  = PORT_PULLUP_DISABLE,
                             .pd  = PORT_PULLDOWN_DISABLE};

    /*configure IO for Port A VBUS source enable */
    PORT_DRIVER.Configure(PORT2, &portCfg, HAL_GPIO_B);
    GPIO_DRIVER.Configure(HAL_GPIO3, &outGPIOConfig, HAL_GPIO_B);
#if (!DEBUG_PORTB)
    portCfg.alt = PORT_DISABLE_OVERRIDE;
#endif
    /*configure IO for Port B VBUS source enable*/
    PORT_DRIVER.Configure(PORT1, &portCfg, HAL_GPIO_B);
    GPIO_DRIVER.Configure(HAL_GPIO2, &outGPIOConfig, HAL_GPIO_B);
    /* End configure I/O */
    /*Enable BC1.2 to allow enabling of D+- OVP protection*/
    USBCHG_DRIVER.Enable(HAL_USBCHG_A);
    USBCHG_DRIVER.Enable(HAL_USBCHG_B);

    Clock_Management_Unit->CLK_CFG |= Clock_Management_Unit_CLK_CFG_AUTO_CLK_SWITCH_Msk;
    SCB->SCR = SCB_SCR_SLEEPDEEP_Msk;
}
#endif

bool fusb15xxx_init(enum fusb15xx_type dev_id)
{
#if (DEVICE_TYPE == FUSB15101)
    fusb15101_init();
#elif (DEVICE_TYPE == FUSB15200)
    fusb15200_init();
#elif (DEVICE_TYPE == FUSB15201)
    fusb15201_init();
#elif (DEVICE_TYPE == FUSB15201P)
    fusb15201p_init();
#else
    return false;
#endif
    return true;
}
void fusb15xxx_sleep_init(HAL_USBPD_T *tcpd)
{
    fusb15xxx_tcpd_cc_events_enable(tcpd, true);
}

HAL_I2C_T *fusb15xxx_device_dcdc_init(int i2c_id, int dev_id, bool en)
{
#if (!CONFIG_ENABLED(LEGACY_I2C))
    return (fusb15xxx_i2c_enable(i2c_id));
#else
    return 0;
#endif
}

HAL_USBCHG_T *fusb15xxx_device_bc1p2_init(int usbchg_id, int dev_id, bool en)
{
    return (fusb15xxx_usbchg_enable(usbchg_id));
}

HAL_USBPD_T *fusb15xxx_device_tcpd_init(int tcpd_id, int dev_id, bool en, bool reinit)
{
    return (fusb15xxx_tcpd_initialize(tcpd_id, en, reinit));
}
