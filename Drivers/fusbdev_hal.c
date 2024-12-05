/**
 * @file     fusbdev_hal.c
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
 **********************************************************************************/
#include "pd_types.h"
#include "legacy_types.h"
#include "fusb15xxx_hal.h"
#include "fusbdev_hal.h"
#include "dpm.h"
#if (CONFIG_ENABLED(CUSTOM_APPLICATION))
#include "pd_powersupply.h"
#endif
#if (!CONFIG_ENABLED(EPR))
#define VBUS_NUMERATOR 1
#define VBUS_DENOMINATOR 1
#else
#define VBUS_NUMERATOR 14
#define VBUS_DENOMINATOR 10
#endif
FUSB_TCPD_DEVICE_T TCPD_Dev[TYPE_C_PORT_COUNT] = {0};
void fusbdev_timer_start(enum timer_type_t timer_dev, int count, bool periodic)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_timer_start(timer_dev, count, periodic);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_timer_start(timer_dev, count, periodic);
#endif
}

void fusbdev_timer_stop(enum timer_type_t timer_dev)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_timer_stop(timer_dev);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_timer_stop(timer_dev);
#endif
}

void fusbdev_timer_interrupt_enable(enum timer_type_t timer_dev, bool en)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_timer_interrupt_enable(timer_dev, en);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_timer_interrupt_enable(timer_dev);
#endif
}
#if (DEVICE_TYPE != FUSB15101)
void fusbdev_cmu_clock_select(enum clk_sel_t clk_sel)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_cmu_clock_select(clk_sel);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_cmu_clock_select(clk_sel);
#endif
}
#endif
void fusbdev_adc_enable(void)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_adc_enable();
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_adc_enable();
#endif
}

void fusbdev_adc_disable(void)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_adc_disable();
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_adc_disable();
#endif
}
void fusbdev_adc_ch_enable(int ch)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_adc_ch_enable(ch, true);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_adc_ch_enable(ch, true);
#endif
}

void fusbdev_adc_ch_disable(int ch)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_adc_ch_disable(ch, false);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_adc_ch_disable(ch, false);
#endif
}

HAL_ADC_CHx_STAT_T fusbdev_adc_ch_status(int dp_ch)
{
#if defined(FUSB15XXX_DEVICE)
    return (fusb15xxx_adc_ch_status(dp_ch));
#elif defined(FUSB18XXX_DEVICE)
    return ((fusb18xxx_adc_ch_status(dp_ch));
#endif
}
bool fusbdev_adc_ch_ready(int dp_ch)
{
#if defined(FUSB15XXX_DEVICE)
    return (fusb15xxx_adc_ch_ready(dp_ch));
#elif defined(FUSB18XXX_DEVICE)
    return ((fusb18xxx_adc_ch_ready(dp_ch));
#endif
}

unsigned long fusbdev_adc_ch_data(int ch)
{
#if defined(FUSB15XXX_DEVICE)
    return (fusb15xxx_adc_ch_data(ch));
#elif defined(FUSB18XXX_DEVICE)
    return (fusb18xxx_adc_ch_data(ch));
#endif
}

void fusbdev_adc_ch_set_alarm(int ch, uint32_t low, uint32_t high)
{
#if defined(FUSB15XXX_DEVICE)
    return (fusb15xxx_adc_ch_set_alarm(ch, low, high));
#elif defined(FUSB18XXX_DEVICE)
    return (Fusb18xxx_adc_ch_set_alarm(ch, low, high));
#endif
}

void fusbdev_adc_ch_set_low_alarm_interrupt(int ch, bool en)
{
#if defined(FUSB15XXX_DEVICE)
    return (fusb15xxx_adc_ch_set_low_alarm_interrupt(ch, en));
#elif defined(FUSB18XXX_DEVICE)
    return (Fusb18xxx_adc_ch_set_low_alarm_interrupt(ch, en));
#endif
}

void fusbdev_adc_ch_set_high_alarm_interrupt(int ch, bool en)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_adc_ch_set_high_alarm_interrupt(ch, en);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_adc_ch_set_high_alarm_interrupt(ch, en);
#endif
}

uint32_t fusbdev_adc_ch_interrupt_status(int ch)
{
#if defined(FUSB15XXX_DEVICE)
    return (fusb15xxx_adc_ch_interrupt_status(ch));
#elif defined(FUSB18XXX_DEVICE)
    return (Fusb18xxx_adc_ch_interrupt_status(ch));
#endif
}

void fusbdev_adc_ch_interrupt_enable(bool en)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_adc_ch_interrupt_enable(en);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_adc_ch_interrupt_enable(en);
#endif
}

void fusbdev_adc_ch_interrupt_clear(void)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_adc_ch_interrupt_clear();
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_adc_ch_interrupt_clear());
#endif
}
void fusbdev_adc_ch_set_reference(int ch, int refsel)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_adc_ch_set_reference(ch, refsel);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_adc_ch_set_reference(ch), refsel);
#endif
}

void fusbdev_adc_ch_reference(int ch)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_adc_ch_reference(ch);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_adc_ch_reference(ch);
#endif
}
#if (DEVICE_TYPE != FUSB15101)
void fusbdev_adc_ch_autoscale_enable(HAL_ADCHx_T ch, bool en)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_adc_ch_autoscale_enable(ch, en);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_adc_ch_autoscale_enable(ch, en);
#endif
}
#endif
void fusbdev_configure_gpio_pin(HAL_GPIOx_T pin, HAL_GPIOCFG_T *cfg, HAL_GPIO_PORT_T port)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_gpio_initialize(pin, cfg, port);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_gpio_initialize(pin, cfg, port);
#endif
}

void fusbdev_set_gpio_pin(HAL_GPIOx_T pin, HAL_GPIOLVL_T lvl, HAL_GPIO_PORT_T port)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_gpio_set(pin, lvl, port);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_gpio_set(pin, lvl, port);
#endif
}

HAL_GPIOLVL_T fusbdev_state_gpio_pin(HAL_GPIOx_T pin, HAL_GPIODIR_T dir, HAL_GPIO_PORT_T port)
{
#if defined(FUSB15XXX_DEVICE)
    return fusb15xxx_gpio_state(pin, dir, port);
#elif defined(FUSB18XXX_DEVICE)
    return fusb18xxx_gpio_state(pin, dir, port);
#endif
}

void fusbdev_enable_interrupt_gpio_pin(HAL_GPIOx_T pin, HAL_GPIO_IRQ_T type, HAL_GPIO_IRQ_POL_T pol,
                                       HAL_GPIO_PORT_T port)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_gpio_enable_interrupt(pin, type, pol, port);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_gpio_enable_interrupt(pin, type, pol, port);
#endif
}

void fusbdev_disable_interrupt_gpio_pin(HAL_GPIOx_T pin, HAL_GPIO_PORT_T port)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_gpio_disable_interrupt(pin, port);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_gpio_disable_interrupt(pin, port);
#endif
}

void fusbdev_interrupt_status_gpio_pin(HAL_GPIOx_T pin, HAL_GPIO_PORT_T port)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_gpio_interrupt_status(pin, port);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_gpio_interrupt_status(pin, port);
#endif
}

void fusbdev_interrupt_clear_gpio_pin(HAL_GPIOx_T pin, HAL_GPIO_PORT_T port)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_gpio_interrupt_clear(pin, port);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_gpio_interrupt_clear(pin, port);
#endif
}

void fusbdev_configure_port_io(HAL_PORTx_T pin, HAL_PORTCFG_T *cfg, HAL_GPIO_PORT_T port)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_port_initialize(pin, cfg, port);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_port_configure_port(pin, cfg, port);
#endif
}

void fusbdev_enable_interrupt_port_io(HAL_PORTx_T pin, HAL_PORT_IRQ_T pol, HAL_GPIO_PORT_T port)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_port_enable_interrupt(pin, pol, port);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_port_enable_interrupt(pin, pol, port);
#endif
}

void fusbdev_disable_interrupt_port_io(HAL_GPIO_PORT_T port)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_port_disable_interrupt(port);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_port_disable_interrupt(port);
#endif
}

void fusbdev_interrupt_status_port_io(HAL_GPIO_PORT_T port)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_port_interrupt_status(port);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_port_interrupt_status(port);
#endif
}

void fusbdev_interrupt_clean_port_io(HAL_GPIO_PORT_T port)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_port_interrupt_clear(port);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_port_interrupt_clear(port);
#endif
}
#if (CONFIG_ENABLED(NMI))
void fusbdev_enable_nmi_port_io(HAL_NMI_CFG_T nmi, HAL_GPIO_PORT_T port)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_port_enable_nmi(nmi, port);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_port_enable_nmi(nmi, port);
#endif
}

void fusbdev_disable_nmi_port_io(HAL_GPIO_PORT_T port)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_port_disable_nmi(port);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_port_disable_nmi(port);
#endif
}
#endif

bool fusbdev_port_is_swd_enabled()
{
#if defined(FUSB15XXX_DEVICE)
	return fusb15xxx_port_is_swd_enabled();
#else
	return fusb18xxx_port_is_swd_enabled();
#endif
}
void fusbdev_ntc_enable(int ntc_port, bool en)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_enable_ntc(ntc_port, en);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_enable_ntc(ntc_port, en);
#endif
}

void fusbdev_ntc_convert_adc_to_temp(int adc)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_ntc_convert_adc_to_temp(adc);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_ntc_convert_adc_to_temp(adc);
#endif
}

void fusbdev_tcpd_initialize(struct fusb_tcpd_device *dev, bool en, bool reinit)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_tcpd_initialize(dev->tcpd_id, en, reinit);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_tcpd_initialize(dev->tcpd_id, en);
#endif
}

bool fusbdev_tcpd_rx_hardreset(struct fusb_tcpd_device *dev)
{
#if defined(FUSB15XXX_DEVICE)
    return (fusb15xxx_tcpd_rx_hardreset(dev->tcpd));
#elif defined(FUSB18XXX_DEVICE)
    return (fusb18xxx_tcpd_rx_hardreset(dev->tcpd));
#endif
}

void fusbdev_tcpd_tx_hardreset(struct fusb_tcpd_device *dev, SOP_T sop)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_tcpd_tx_hardreset(dev->tcpd, sop);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_tcpd_tx_hardreset(dev->tcpd, sop);
#endif
}

void fusbdev_tcpd_set_rp(struct fusb_tcpd_device *dev, int mask, CC_RP_T rp)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_tcpd_set_rp(dev->tcpd, mask, rp);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_tcpd_set_rp(dev->tcpd, mask, rp);
#endif
}

void fusbdev_tcpd_set_orient(struct fusb_tcpd_device *dev, CC_T cc)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_tcpd_set_orient(dev->tcpd, cc);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_tcpd_set_orient(dev->tcpd, cc);
#endif
}

void fusbdev_tcpd_set_cc(struct fusb_tcpd_device *dev, int mask, CC_TERM_T term, CC_RP_T rp,
                         DRP_MODE_T drp)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_tcpd_set_cc(dev->tcpd, mask, term, rp, drp);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_tcpd_set_cc(dev->tcpd, mask, term, rp, drp);
#endif
}

CC_STAT_T fusbdev_tcpd_cc_stat(struct fusb_tcpd_device *dev, CC_T cc)
{
#if defined(FUSB15XXX_DEVICE)
    return (fusb15xxx_tcpd_cc_stat(dev->tcpd, cc));
#elif defined(FUSB18XXX_DEVICE)
    return (fusb18xxx_tcpd_cc_stat(dev->tcpd, cc));
#endif
}

void fusbdev_tcpd_cc_events_enable(struct fusb_tcpd_device *dev, bool en)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_tcpd_cc_events_enable(dev->tcpd, en);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_tcpd_cc_events_enable(dev->tcpd, en);
#endif
}

bool fusbdev_tcpd_cc_check_error(struct fusb_tcpd_device *dev)
{
#if defined(FUSB15XXX_DEVICE)
    return (fusb15xxx_tcpd_cc_check_error(dev->tcpd));
#elif defined(FUSB18XXX_DEVICE)
    return fusb18xxx_tcpd_cc_check_error(dev->tcpd);
#endif
}
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
void fusbdev_tcpd_set_drp(struct fusb_tcpd_device *dev, DRP_MODE_T drp)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_tcpd_set_drp(dev->tcpd, drp);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_tcpd_set_drp(dev->tcpd, drp);
#endif
}

DRP_STAT_T fusbdev_tcpd_drp_stat(struct fusb_tcpd_device *dev)
{
#if defined(FUSB15XXX_DEVICE)
    return (fusb15xxx_tcpd_drp_stat(dev->tcpd));
#elif defined(FUSB18XXX_DEVICE)
    return (fusb18xxx_tcpd_drp_stat(dev->tcpd));
#endif
}
#endif
void fusbdev_tcpd_vbus_init(struct fusb_tcpd_device *dev)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_tcpd_vbus_init(dev->tcpd);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_tcpd_vbus_init(dev->tcpd);
#endif
}

void fusbdev_tcpd_vbus_deinit(struct fusb_tcpd_device *dev)
{
#if (!CONFIG_ENABLED(LPS))
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_tcpd_vbus_deinit(dev->tcpd);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_tcpd_vbus_deinit(dev->tcpd);
#endif
#else
    fusb15101_set_vbus_source(dev, 0, 0, pdoTypeFixed);
#endif
}

void fusbdev_tcpd_vbus_events(struct fusb_tcpd_device *dev, bool en)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_tcpd_vbus_events(dev->tcpd, en);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_tcpd_vbus_events(dev->tcpd, en);
#endif
}

bool fusbdev_tcpd_set_vbus_source(struct fusb_tcpd_device *dev, unsigned mv, unsigned ma,
                                  PDO_T type)
{
#if (!CONFIG_ENABLED(LPS))
#if defined(FUSB15XXX_DEVICE)
    return (fusb15xxx_tcpd_set_vbus_source(dev->tcpd, mv, ma, type));
#elif defined(FUSB18XXX_DEVICE)
    return (fusb18xxx_tcpd_set_vbus_source(dev->tcpd, mv, ma, type));
#endif
#else
    //ADD fusb15101_set_vbus_source function here, modify it to take fusb_tcpd_device instead of prev. first argument
    return (fusb15101_set_vbus_source(dev, mv, ma, type));
#endif
}

bool fusbdev_tcpd_set_vbus_sink(struct fusb_tcpd_device *dev, unsigned mv, unsigned ma, PDO_T type)
{
#if defined(FUSB15XXX_DEVICE)
    return (fusb15xxx_tcpd_set_vbus_sink(dev->tcpd, mv, ma, type));
#elif defined(FUSB18XXX_DEVICE)
    return (fusb18xxx_tcpd_set_vbus_sink(dev->tcpd, mv, ma, type));
#endif
}
#if (DEVICE_TYPE == FUSB15101)
unsigned fusbdev_tcpd_vbus_pps_value(struct fusb_tcpd_device *dev)
{
#if defined(FUSB15XXX_DEVICE)
    return ((fusb15xxx_tcpd_vbus_pps_value(dev->tcpd))*VBUS_NUMERATOR)/VBUS_DENOMINATOR;
#elif defined(FUSB18XXX_DEVICE)
    return (fusb18xxx_tcpd_vbus__pps_value(dev->tcpd));
#endif
}
#endif
unsigned fusbdev_tcpd_vbus_value(struct fusb_tcpd_device *dev)
{
#if defined(FUSB15XXX_DEVICE)
    return ((fusb15xxx_tcpd_vbus_value(dev->tcpd))*VBUS_NUMERATOR)/VBUS_DENOMINATOR;
#elif defined(FUSB18XXX_DEVICE)
    return (fusb18xxx_tcpd_vbus_value(dev->tcpd));
#endif
}

void fusbdev_tcpd_set_vbus_res(struct fusb_tcpd_device *dev, unsigned max)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_tcpd_set_vbus_res(dev->tcpd, dev->tcpd_id, max);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_tcpd_set_vbus_res(dev->tcpd, max);
#endif
}

void fusbdev_tcpd_set_vbus_alarm(struct fusb_tcpd_device *dev, unsigned low, unsigned high)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_tcpd_set_vbus_alarm(dev->tcpd, low, high);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_tcpd_set_vbus_alarm(dev->tcpd, low, high);
#endif
}

void fusbdev_tcpd_set_vbus_disch(struct fusb_tcpd_device *dev, VBUS_DISCH_T disch)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_tcpd_set_vbus_disch(dev->tcpd, disch);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_tcpd_set_vbus_disch(dev->tcpd, disch);
#endif
}

CC_RP_T fusbdev_tcpd_pullup_value(struct fusb_tcpd_device *dev, CC_T cc)
{
#if defined(FUSB15XXX_DEVICE)
    return (fusb15xxx_tcpd_pullup_value(dev->tcpd, cc));
#elif defined(FUSB18XXX_DEVICE)
    return (fusb18xxx_tcpd_pullup_value(dev->tcpd, cc));
#endif
}

void fusbdev_tcpd_set_term(struct fusb_tcpd_device *dev, int mask, CC_TERM_T term)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_tcpd_set_term(dev->tcpd, mask, term);
#elif defined(FUSB18XXX_DEVICE)
    tcpd_set_term(dev->tcpd, mask, term);
#endif
}

void fusbdev_tcpd_vconn_enable(struct fusb_tcpd_device *dev, bool en, HAL_VCONN_OCP_T ocp)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_tcpd_vconn_enable(dev->tcpd, en, ocp);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_tcpd_vconn_enable(dev->tcpd, en);
#endif
}

void fusbdev_tcpd_pd_enable(struct fusb_tcpd_device *dev, bool en)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_tcpd_pd_enable(dev->tcpd, en);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_tcpd_pd_enable(dev->tcpd, en);
#endif
}

void fusbdev_tcpd_pd_sop_enable(struct fusb_tcpd_device *dev, unsigned mask)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_tcpd_pd_sop_enable(dev->tcpd, mask);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_tcpd_pd_sop_enable(dev->tcpd, mask);
#endif
}

void fusbdev_tcpd_pd_rev(struct fusb_tcpd_device *dev, PD_REV_T rev, SOP_T sop)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_tcpd_pd_rev(dev->tcpd, rev, sop);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_tcpd_pd_rev(dev->tcpd, rev, sop);
#endif
}

void fusbdev_tcpd_set_pd_dfp(struct fusb_tcpd_device *dev, bool dfp_en)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_tcpd_set_pd_dfp(dev->tcpd, dfp_en);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_tcpd_set_pd_dfp(dev->tcpd, dfp_en);
#endif
}

void fusbdev_tcpd_set_pd_source(struct fusb_tcpd_device *dev, bool src_en)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_tcpd_set_pd_source(dev->tcpd, src_en);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_tcpd_set_pd_source(dev->tcpd, src_en);
#endif
}

void fusbdev_tcpd_set_bist_mode(struct fusb_tcpd_device *dev, BIST_MODE_T mode)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_tcpd_set_bist_mode(dev->tcpd, mode);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_tcpd_set_bist_mode(dev->tcpd, mode);
#endif
}

PORT_TX_STAT_T fusbdev_tcpd_port_tx_status(struct fusb_tcpd_device *dev)
{
#if defined(FUSB15XXX_DEVICE)
    return (fusb15xxx_tcpd_port_tx_status(dev->tcpd));
#elif defined(FUSB18XXX_DEVICE)
    return (fusb18xxx_tcpd_port_tx_status(dev->tcpd));
#endif
}

bool fusbdev_tcpd_pd_tx(struct fusb_tcpd_device *dev, uint8_t const *buf, unsigned long len,
                        SOP_T sop, int retries)
{
#if defined(FUSB15XXX_DEVICE)
    return (fusb15xxx_tcpd_pd_tx(dev->tcpd, buf, len, sop, retries));
#elif defined(FUSB18XXX_DEVICE)
    return (fusb18xxx_tcpd_pd_tx(dev->tcpd, buf, len, sop, retries));
#endif
}

bool fusbdev_tcpd_pd_check_rx(struct fusb_tcpd_device *dev)
{
#if defined(FUSB15XXX_DEVICE)
    return (fusb15xxx_tcpd_pd_check_rx(dev->tcpd));
#elif defined(FUSB18XXX_DEVICE)
    return (fusb18xxx_tcpd_pd_check_rx(dev->tcpd));
#endif
}

SOP_T fusbdev_tcpd_pd_rx(struct fusb_tcpd_device *dev, uint8_t *buf, unsigned long len)
{
#if defined(FUSB15XXX_DEVICE)
    return (fusb15xxx_tcpd_pd_rx(dev->tcpd, buf, len));
#elif defined(FUSB18XXX_DEVICE)
    return (fusb18xxx_tcpd_pd_rx(dev->tcpd, buf, len));
#endif
}

#if CONFIG_ENABLED(DCDC)
HAL_I2C_T *fusbdev_i2c_enable(struct fusb_dcdc_device *dev, int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    return (fusb15xxx_i2c_enable(i2c_id));
#elif defined(FUSB18XXX_DEVICE)
    return (fusb18xxx_i2c_enable(dev->i2c));
#endif
}
void fusbdev_i2c_disable(struct fusb_dcdc_device *dev, int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_disable(dev->i2c[i2c_id]);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_disable(dev->i2c[i2c_id]);
#endif
}

void fusbdev_i2c_master_enable(struct fusb_dcdc_device *dev, uint32_t prescale, int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_master_enable(dev->i2c[i2c_id], prescale);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_master_enable(dev->i2c[i2c_id], prescale);
#endif
}

void fusbdev_i2c_master_disable(struct fusb_dcdc_device *dev, int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_master_disable(dev->i2c[i2c_id]);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_master_disable(dev->i2c[i2c_id]);
#endif
}

void fusbdev_i2c_write(struct fusb_dcdc_device *dev, uint8_t addr, uint8_t *pBuf, uint32_t len,
                       uint32_t timeout, int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_write(dev->i2c[i2c_id], addr, pBuf, len, timeout);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_write(dev->i2c[i2c_id], addr, pBuf, len, timeout);
#endif
}

void fusbdev_i2c_read(struct fusb_dcdc_device *dev, uint8_t addr, uint8_t *pBuf, uint32_t len,
                      uint32_t timeout, int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_read(dev->i2c[i2c_id], addr, pBuf, len, timeout);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_read(dev->i2c[i2c_id], addr, pBuf, len, timeout);
#endif
}

void fusbdev_i2c_write_with_interrupt(struct fusb_dcdc_device *dev, uint8_t addr, uint8_t *pBuf,
                                      uint32_t len, HAL_I2CCB_T cb, int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_write_with_interrupt(dev->i2c[i2c_id], addr, pBuf, len, cb);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_write_with_interrupt(dev->i2c[i2c_id], addr, pBuf, len, cb);
#endif
}

void fusbdev_i2c_read_with_interrupt(struct fusb_dcdc_device *dev, uint8_t addr, uint8_t *pBuf,
                                     uint32_t len, HAL_I2CCB_T cb, int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_read_with_interrupt(dev->i2c[i2c_id], addr, pBuf, len, cb);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_read_with_interrupt(dev->i2c[i2c_id], addr, pBuf, len, cb);
#endif
}

#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
void fusbdev_i2c_write_with_dma(struct fusb_dcdc_device *dev, uint8_t addr, uint8_t *pBuf,
                                uint32_t len, HAL_I2CCB_T cb, int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_write_with_dma(dev->i2c[i2c_id], addr, pBuf, len, cb);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_write_with_dma(dev->i2c[i2c_id], addr, pBuf, len, cb);
#endif
}

void fusbdev_i2c_read_with_dma(struct fusb_dcdc_device *dev, uint8_t addr, uint8_t *pBuf,
                               uint32_t len, HAL_I2CCB_T cb, int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_read_with_dma(dev->i2c[i2c_id], addr, pBuf, len, cb);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_read_with_dma(dev->i2c[i2c_id], addr, pBuf, len, cb);
#endif
}
#endif

void fusbdev_i2c_read_register(struct fusb_dcdc_device *dev, uint8_t addr, uint8_t reg,
                               uint8_t *pBuf, uint32_t len, HAL_I2CCB_T cb, int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_read_register(dev->i2c[i2c_id], addr, reg, pBuf, len, cb);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_read_register(dev->i2c[i2c_id], addr, reg, pBuf, len, cb);
#endif
}
#if (CONFIG_ENABLED(I2C_SLAVE))
void fusbdev_i2c_slave_enable(struct fusb_dcdc_device *dev, uint8_t addr, uint32_t prescale,
                              int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_slave_enable(dev->i2c[i2c_id], addr, prescale);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_slave_enable(dev->i2c[i2c_id], addr, prescale);
#endif
}

void fusbdev_i2c_slave_disable(struct fusb_dcdc_device *dev, int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_slave_disable(dev->i2c[i2c_id]);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_slave_disable(dev->i2c[i2c_id]);
#endif
}

void fusbdev_i2c_slave_start(struct fusb_dcdc_device *dev, uint8_t *pBuf, uint32_t size,
                             HAL_SLAVE_CB_T cb, int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_slave_start(dev->i2c[i2c_id], pBuf, size, cb);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_slave_start(dev->i2c[i2c_id], pBuf, size, cb);
#endif
}

void fusbdev_i2c_slave_set_offset(struct fusb_dcdc_device *dev, uint32_t offset, int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_slave_set_offset(dev->i2c[i2c_id], offset);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_slave_set_offset(dev->i2c[i2c_id], offset);
#endif
}

#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
void fusbdev_i2c_slave_setup_rx_dma(struct fusb_dcdc_device *dev, uint8_t *pBuf, uint32_t size,
                                    HAL_SLAVE_CB_T cb, int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_slave_setup_rx_dma(dev->i2c[i2c_id], pBuf, size, cb);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_slave_setup_rx_dma(dev->i2c[i2c_id], pBuf, size, cb);
#endif
}

void fusbdev_i2c_slave_setup_tx_dma(struct fusb_dcdc_device *dev, uint8_t *pBuf, uint32_t size,
                                    HAL_SLAVE_CB_T cb, int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_slave_setup_tx_dma(dev->i2c[i2c_id], pBuf, size, cb);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_slave_setup_tx_dma(dev->i2c[i2c_id], pBuf, size, cb);
#endif
}
#endif
#endif
void fusbdev_i2c_irq_enable(struct fusb_dcdc_device *dev, HAL_I2CINT_T irq, int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_irq_enable(dev->i2c[i2c_id], irq);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_irq_enable(dev->i2c[i2c_id], irq);
#endif
}

void fusbdev_i2c_irq_disable(struct fusb_dcdc_device *dev, int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_irq_disable(dev->i2c[i2c_id]);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_irq_disable(dev->i2c[i2c_id]);
#endif
}

void fusbdev_i2c_irq_set(struct fusb_dcdc_device *dev, uint32_t val, int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_irq_set(dev->i2c[i2c_id], val);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_irq_set(dev->i2c[i2c_id], val);
#endif
}

void fusbdev_i2c_irq_clear(struct fusb_dcdc_device *dev, int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_irq_clear(dev->i2c[i2c_id]);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_irq_clear(dev->i2c[i2c_id]);
#endif
}

void fusbdev_i2c_irq_status(struct fusb_dcdc_device *dev, int i2c_m, int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_irq_status(dev->i2c[i2c_id]);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_irq_status(dev->i2c[i2c_id]);
#endif
}
void fusbdev_i2c_irq_set_callback(struct fusb_dcdc_device *dev, HAL_INTCB_T cb, int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_irq_set_callback(dev->i2c[i2c_id], cb);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_irq_set_callback(dev->i2c[i2c_id], cb);
#endif
}
#elif CONFIG_ENABLED(CUSTOM_APPLICATION)
#if (CONFIG_ENABLED(LEGACY_I2C))
HAL_I2C_T *fusbdev_i2c_enable(I2C_Port_Type *i2c)
{
#if defined(FUSB15XXX_DEVICE)
    return (fusb15xxx_i2c_enable(i2c));
#elif defined(FUSB18XXX_DEVICE)
    return (fusb18xxx_i2c_enable(i2c));
#endif
}
#else
HAL_I2C_T *fusbdev_i2c_enable(int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    return (fusb15xxx_i2c_enable(i2c_id));
#elif defined(FUSB18XXX_DEVICE)
    return (fusb18xxx_i2c_enable(i2c_id));
#endif
}
#endif
void fusbdev_i2c_disable(HAL_I2C_T *i2c, int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_disable(i2c);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_disable(i2c);
#endif
}

void fusbdev_i2c_master_enable(HAL_I2C_T *i2c, uint32_t prescale, int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_master_enable(i2c, prescale);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_master_enable(i2c, prescale);
#endif
}

void fusbdev_i2c_master_disable(HAL_I2C_T *i2c, int i2c_id)
{
#if !(CONFIG_ENABLED(LEGACY_I2C))
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_master_disable(i2c);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_master_disable(i2c);
#endif
#endif
}

void fusbdev_i2c_slave_enable(HAL_I2C_T *i2c, uint8_t addr, uint32_t prescale, int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_slave_enable(i2c, addr, prescale);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_slave_enable(i2c, addr, prescale);
#endif
}

void fusbdev_i2c_slave_disable(HAL_I2C_T *i2c, int i2c_id)
{
#if !(CONFIG_ENABLED(LEGACY_I2C))
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_slave_disable(i2c);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_slave_disable(i2c);
#endif
#endif
}

#if (CONFIG_ENABLED(I2C_BLOCKING))
void fusbdev_i2c_write(HAL_I2C_T *i2c, uint8_t addr, uint8_t *pBuf, uint32_t len, uint32_t timeout,
                       int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_write(i2c, addr, pBuf, len, timeout);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_write(i2c, addr, pBuf, len, timeout);
#endif
}

void fusbdev_i2c_read(HAL_I2C_T *i2c, uint8_t addr, uint8_t *pBuf, uint32_t len, uint32_t timeout,
                      int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_read(i2c, addr, pBuf, len, timeout);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_read(i2c, addr, pBuf, len, timeout);
#endif
}
#endif

void fusbdev_i2c_write_with_interrupt(HAL_I2C_T *i2c, uint8_t addr, uint8_t *pBuf, uint32_t len,
                                      HAL_I2CCB_T cb, int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_write_with_interrupt(i2c, addr, pBuf, len, cb);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_write_with_interrupt(i2c, addr, pBuf, len, cb);
#endif
}

void fusbdev_i2c_read_with_interrupt(HAL_I2C_T *i2c, uint8_t addr, uint8_t *pBuf, uint32_t len,
                                     HAL_I2CCB_T cb, int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_read_with_interrupt(i2c, addr, pBuf, len, cb);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_read_with_interrupt(i2c, addr, pBuf, len, cb);
#endif
}

#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
void fusbdev_i2c_write_with_dma(HAL_I2C_T *i2c, uint8_t addr, uint8_t *pBuf, uint32_t len,
                                HAL_I2CCB_T cb, int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_write_with_dma(i2c, addr, pBuf, len, cb);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_write_with_dma(i2c, addr, pBuf, len, cb);
#endif
}

void fusbdev_i2c_read_with_dma(HAL_I2C_T *i2c, uint8_t addr, uint8_t *pBuf, uint32_t len,
                               HAL_I2CCB_T cb, int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_read_with_dma(i2c, addr, pBuf, len, cb);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_read_with_dma(i2c, addr, pBuf, len, cb);
#endif
}
#endif

void fusbdev_i2c_read_register(HAL_I2C_T *i2c, uint8_t addr, uint8_t reg, uint8_t *pBuf,
                               uint32_t len, HAL_I2CCB_T cb, int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_read_register(i2c, addr, reg, pBuf, len, cb);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_read_register(i2c, addr, reg, pBuf, len, cb);
#endif
}

void fusbdev_i2c_slave_start(HAL_I2C_T *i2c, uint8_t *pBuf, uint32_t size, HAL_SLAVE_CB_T cb,
                             int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_slave_start(i2c, pBuf, size, cb);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_slave_start(i2c, pBuf, size, cb);
#endif
}

void fusbdev_i2c_slave_set_offset(HAL_I2C_T *i2c, uint32_t offset, int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_slave_set_offset(i2c, offset);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_slave_set_offset(i2c, offset);
#endif
}

#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
void fusbdev_i2c_slave_setup_rx_dma(HAL_I2C_T *i2c, uint8_t *pBuf, uint32_t size, HAL_SLAVE_CB_T cb,
                                    int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_slave_setup_rx_dma(i2c, pBuf, size, cb);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_slave_setup_rx_dma(i2c, pBuf, size, cb);
#endif
}

void fusbdev_i2c_slave_setup_tx_dma(HAL_I2C_T *i2c, uint8_t *pBuf, uint32_t size, HAL_SLAVE_CB_T cb,
                                    int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_slave_setup_tx_dma(i2c, pBuf, size, cb);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_slave_setup_tx_dma(i2c, pBuf, size, cb);
#endif
}
#endif

void fusbdev_i2c_irq_enable(HAL_I2C_T *i2c, HAL_I2CINT_T irq, int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_irq_enable(i2c, irq);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_irq_enable(i2c, irq);
#endif
}

void fusbdev_i2c_irq_disable(HAL_I2C_T *i2c, int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_irq_disable(i2c);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_irq_disable(i2c);
#endif
}

void fusbdev_i2c_irq_set(HAL_I2C_T *i2c, uint32_t val, int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_irq_set(i2c, val);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_irq_set(i2c, val);
#endif
}

void fusbdev_i2c_irq_clear(HAL_I2C_T *i2c, int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_irq_clear(i2c);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_irq_clear(i2c);
#endif
}

void fusbdev_i2c_irq_status(HAL_I2C_T *i2c, int i2c_m, int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_irq_status(i2c);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_irq_status(i2c);
#endif
}

void fusbdev_i2c_irq_set_callback(HAL_I2C_T *i2c, HAL_INTCB_T cb, int i2c_id)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_i2c_irq_set_callback(i2c, cb);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_i2c_irq_set_callback(i2c, cb);
#endif
}
#endif

#if CONFIG_ENABLED(LEGACY_CHARGING)
FUSB_BC1P2_DEVICE_T BC1P2_Dev[TYPE_C_PORT_COUNT] = {0};
/**
 * Legacy driver implementation
 */
void fusbdev_usbchg_enable(struct fusb_bc1p2_device const *dev)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_usbchg_enable(dev->usbchg_id);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_usbchg_enable(dev->usbchg_id);
#endif
}

void fusbdev_usbchg_disable(struct fusb_bc1p2_device const *dev)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_usbchg_disable(dev->usbchg);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_usbchg_disable(dev->usbchg);
#endif
}

void fusbdev_usbchg_idp_src(struct fusb_bc1p2_device const *dev, bool en)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_usbchg_idp_src(dev->usbchg, en);
#elif defined(FUSB18XXX_DEVICE)
    (fusb18xxx_usbchg_idp_src(dev->usbchg, en);
#endif
}

void fusbdev_usbchg_idp_snk(struct fusb_bc1p2_device const *dev, bool en)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_usbchg_idp_snk(dev->usbchg, en);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_usbchg_idp_snk(dev->usbchg, en);
#endif
}

void fusbdev_usbchg_rdm_dwn(struct fusb_bc1p2_device const *dev, bool en)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_usbchg_rdm_dwn(dev->usbchg, en);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_usbchg_rdm_dwn(dev->usbchg, en);
#endif
}

void fusbdev_usbchg_rdp_dwn(struct fusb_bc1p2_device const *dev, bool en)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_usbchg_rdp_dwn(dev->usbchg, en);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_usbchg_rdp_dwn(dev->usbchg, en);
#endif
}

void fusbdev_usbchg_vdp_src(struct fusb_bc1p2_device const *dev, bool en)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_usbchg_vdp_src(dev->usbchg, en);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_usbchg_vdp_src(dev->usbchg, en);
#endif
}

void fusbdev_usbchg_vdm_src(struct fusb_bc1p2_device const *dev, bool en)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_usbchg_vdm_src(dev->usbchg, en);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_usbchg_vdp_src(dev->usbchg, en);
#endif
}

void fusbdev_usbchg_idm_sink(struct fusb_bc1p2_device const *dev, bool en)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_usbchg_idm_snk(dev->usbchg, en);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_usbchg_idm_snk(dev->usbchg, en);
#endif
}

void fusbdev_usbchg_host_sw(struct fusb_bc1p2_device const *dev, bool en)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_usbchg_host_sw(dev->usbchg, en);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_usbchg_host_sw(dev->usbchg, en);
#endif
}

void fusbdev_usbchg_rdiv(struct fusb_bc1p2_device const *dev, bool en)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_usbchg_rdiv(dev->usbchg, en);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_usbchg_rdiv(dev->usbchg, en);
#endif
}

void fusbdev_usbchg_rdcp(struct fusb_bc1p2_device const *dev, bool en)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_usbchg_rdcp(dev->usbchg, en);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_usbchg_rdcp(dev->usbchg, en);
#endif
}

void fusbdev_usbchg_rdat(struct fusb_bc1p2_device const *dev, bool en)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_usbchg_rdat(dev->usbchg, en);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_usbchg_rdat(dev->usbchg, en);
#endif
}
#endif

FUSB_TCPD_DEVICE_T *device_tcpd_init(int port, int dev_id, bool en, bool reinit)
{
    HAL_USBPD_T *tcpd = 0;
    int          id   = (dev_id << 8) | DEVICE_TYPE;

#if defined(FUSB15XXX_DEVICE)
    if (dev_id == fusb15_dev)
    {
        TCPD_Dev[port].tcpd_id = TCPD_PORT_ID(port);
        tcpd                   = fusb15xxx_device_tcpd_init(TCPD_Dev[port].tcpd_id, id, en, reinit);
    }
#elif defined(FUSB18XXX_DEVICE)
    if (dev_id == fusb18_dev)
    {
        TCPD_Dev[port].tcpd_id = TCPD_PORT_ID(port);
        tcpd = fusb18xxx_device_tcpd_init(id, en);
    }
#endif

    if (tcpd)
    {
        TCPD_Dev[port].tcpd = tcpd;
        //        TCPD_Dev[port].fusb_driv = &fusb_driver;
    }
    return (FUSB_TCPD_DEVICE_T *)&TCPD_Dev[port];
}

#if CONFIG_ENABLED(LEGACY_CHARGING)
FUSB_BC1P2_DEVICE_T *device_bc1p2_init(int port, int dev_id, bool en)
{
    HAL_USBCHG_T *usbchg = 0;
    int           id     = (dev_id << 8) | DEVICE_TYPE;

#if defined(FUSB15XXX_DEVICE)
    if (dev_id == fusb15_dev)
    {
        BC1P2_Dev[port].usbchg_id = LEGACY_PORT_ID(port);
        usbchg                    = fusb15xxx_device_bc1p2_init(BC1P2_Dev[port].usbchg_id, id, en);
    }
#elif defined(FUSB18XXX_DEVICE)
    if (dev_id == fusb18_dev)
    {
        BC1P2_Dev[port].usbchg_id = LEGACY_PORT_ID(port);
        usbchg                    = fusb18xxx_device_bc1p2_init(BC1P2_Dev[port].usbchg_id, id, en);
    }
#endif

    if (usbchg)
    {
        BC1P2_Dev[port].usbchg = usbchg;
//		BC1P2_Dev[port].fusb_driv = &fusb_driver;
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
        if (BC1P2_Dev[port].usbchg_id == HAL_USBCHG_A)
        {
            BC1P2_Dev[port].dp_ch = ADC_CH1;
            BC1P2_Dev[port].dm_ch = ADC_CH2;
        }
        else
        {
            BC1P2_Dev[port].dp_ch = ADC_CH7;
            BC1P2_Dev[port].dm_ch = ADC_CH8;
        }
#elif (DEVICE_TYPE == FUSB15201P)
        if (BC1P2_Dev[port].usbchg_id == HAL_USBCHG_A)
        {
            BC1P2_Dev[port].dp_ch = ADC_CH1;
            BC1P2_Dev[port].dm_ch = ADC_CH2;
        }
        else
        {
            BC1P2_Dev[port].dp_ch = ADC_CH5;
            BC1P2_Dev[port].dm_ch = ADC_CH6;
        }
#else
        BC1P2_Dev[port].dp_ch = ADC_CH3;
        BC1P2_Dev[port].dm_ch = ADC_CH4;
#endif
    }
    return (FUSB_BC1P2_DEVICE_T *)&BC1P2_Dev[port];
}
#endif //CONFIG_ENABLED(LEGACY_CHARGING)

#if CONFIG_ENABLED(POWER_SHARING)
FUSB_PS_DEVICE_T  PS_Dev[TYPE_C_PORT_COUNT] = {0};
FUSB_PS_DEVICE_T *device_ps_init(int port, int dev_id, bool en)
{
    PS_Dev[port].ps_id     = TCPD_PORT_ID(port);
    PS_Dev[port].tcpd      = TCPD_Dev[port].tcpd;
    PS_Dev[port].fusb_driv = TCPD_Dev[port].fusb_driv;
    return (FUSB_PS_DEVICE_T *)&PS_Dev[port];
}
#endif

#if CONFIG_ENABLED(DCDC)
FUSB_DCDC_DEVICE_T  DCDC_Dev = {0};
FUSB_DCDC_DEVICE_T *device_dcdc_init(int dev_id, bool en)
{
    int id = (dev_id << 8) | DEVICE_TYPE;
#if defined(FUSB15XXX_DEVICE)
    if (dev_id == fusb15_dev)
    {
        //        DCDC_Dev.fusb_driv = &fusb_driver;
    }
#elif defined(FUSB18XXX_DEVICE)
    if (dev_id == fusb18_dev)
    {
        DCDC_Dev[i2c_if].fusb_dcdc_driv = &fusb_dcdc_driver;
    }
#endif
    return (FUSB_DCDC_DEVICE_T *)&DCDC_Dev;
}
#endif

void device_sleep_init(struct fusb_tcpd_device *dev)
{
#if defined(FUSB15XXX_DEVICE)
    fusb15xxx_sleep_init(dev->tcpd);
#elif defined(FUSB18XXX_DEVICE)
    fusb18xxx_sleep_init(dev->tcpd);
#endif
}
/**
 * @brief Initialize all clocks and peripheral system
 */
struct fusb_driver_t *device_init(int dev_id)
{
    int id = (dev_id << 8) | DEVICE_TYPE;
#if defined(FUSB15XXX_DEVICE)
    if (DEVICE_TYPE < 5)
    {
        if ((fusb15xxx_init(id)) == true)
        {
            //        	return &fusb_driver;
        }
    }
#elif defined(FUSB18XXX_DEVICE)
    if (dev_id == fusb18_dev)
    {
        fusb18xxx_init(id);
    }
#endif
    return (0);
}
