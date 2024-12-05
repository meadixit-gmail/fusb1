/**
 * @file   fusb15xxx_hal.h
 * @author USB Firmware Team
 * @brief  Defines functionality for the Device Policy Manager state machine.
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
 * @addtogroup HAL
 * @{
 *******************************************************************************/
#ifndef __FUSB15XXX_HAL_H__
#define __FUSB15XXX_HAL_H__

#include "pd_types.h"
#include "port.h"
#include "FUSB15xxx.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**************************************************************************
 * TCPD function driver abstraction *
 **************************************************************************/
    void         fusb15xxx_timer_start(enum timer_type_t timer, int count, bool periodic);
    void         fusb15xxx_timer_stop(enum timer_type_t timer);
    void         fusb15xxx_timer_interrupt_enable(enum timer_type_t timer, bool en);
    void         fusb15xxx_cmu_clock_select(enum clk_sel_t clk_sel);
    HAL_USBPD_T *fusb15xxx_tcpd_initialize(int tcpd_id, bool en, bool reinit);
    bool         fusb15xxx_tcpd_rx_hardreset(HAL_USBPD_T *tcpd);
    void         fusb15xxx_tcpd_tx_hardreset(HAL_USBPD_T *tcpd, SOP_T sop);
    void         fusb15xxx_tcpd_set_rp(HAL_USBPD_T *tcpd, int mask, CC_RP_T rp);
    void         fusb15xxx_tcpd_set_orient(HAL_USBPD_T *tcpd, CC_T cc);
    void         fusb15xxx_tcpd_set_cc(HAL_USBPD_T *tcpd, int mask, CC_TERM_T term, CC_RP_T rp,
                                       DRP_MODE_T drp);
    CC_STAT_T    fusb15xxx_tcpd_cc_stat(HAL_USBPD_T *tcpd, CC_T cc);
    void         fusb15xxx_tcpd_cc_events_enable(HAL_USBPD_T *tcpd, bool en);
    bool         fusb15xxx_tcpd_cc_check_error(HAL_USBPD_T *tcpd);
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
    void       fusb15xxx_tcpd_set_drp(HAL_USBPD_T *tcpd, DRP_MODE_T drp);
    DRP_STAT_T fusb15xxx_tcpd_drp_stat(HAL_USBPD_T *tcpd);
#endif
    void fusb15xxx_tcpd_vbus_init(HAL_USBPD_T *tcpd);
    void fusb15xxx_tcpd_vbus_deinit(HAL_USBPD_T *tcpd);
    void fusb15xxx_tcpd_vbus_events(HAL_USBPD_T *tcpd, bool en);
    bool fusb15xxx_tcpd_set_vbus_source(HAL_USBPD_T *tcpd, unsigned mv, unsigned ma, PDO_T type);
    bool fusb15xxx_tcpd_set_vbus_sink(HAL_USBPD_T *tcpd, unsigned mv, unsigned ma, PDO_T type);
    uint32_t fusb15xxx_tcpd_vbus_value(HAL_USBPD_T *tcpd);
#if (DEVICE_TYPE == FUSB15101)
    uint32_t fusb15xxx_tcpd_vbus_pps_value(HAL_USBPD_T *tcpd);
#endif
    void     fusb15xxx_tcpd_set_vbus_res(HAL_USBPD_T *tcpd, HAL_USBPD_CHID_T tcpd_id, unsigned max);
    void     fusb15xxx_tcpd_set_vbus_alarm(HAL_USBPD_T *tcpd, unsigned low, unsigned high);
    void     fusb15xxx_tcpd_set_vbus_disch(HAL_USBPD_T *tcpd, VBUS_DISCH_T disch);
    CC_RP_T  fusb15xxx_tcpd_pullup_value(HAL_USBPD_T *tcpd, CC_T cc);
    void     fusb15xxx_tcpd_set_term(HAL_USBPD_T *tcpd, int mask, CC_TERM_T term);
    void     fusb15xxx_tcpd_vconn_enable(HAL_USBPD_T *tcpd, bool en, HAL_VCONN_OCP_T ocp);
    void     fusb15xxx_tcpd_pd_enable(HAL_USBPD_T *tcpd, bool en);
    void     fusb15xxx_tcpd_pd_sop_enable(HAL_USBPD_T *tcpd, unsigned mask);
    void     fusb15xxx_tcpd_pd_rev(HAL_USBPD_T *tcpd, PD_REV_T rev, SOP_T sop);
    void     fusb15xxx_tcpd_set_pd_dfp(HAL_USBPD_T *tcpd, bool dfp_en);
    void     fusb15xxx_tcpd_set_pd_source(HAL_USBPD_T *tcpd, bool src_en);
    void     fusb15xxx_tcpd_set_bist_mode(HAL_USBPD_T *tcpd, BIST_MODE_T mode);
    PORT_TX_STAT_T fusb15xxx_tcpd_port_tx_status(HAL_USBPD_T *tcpd);
    bool  fusb15xxx_tcpd_pd_tx(HAL_USBPD_T *tcpd, uint8_t const *buf, unsigned long len, SOP_T sop,
                               int retries);
    bool  fusb15xxx_tcpd_pd_check_rx(HAL_USBPD_T *tcpd);
    SOP_T fusb15xxx_tcpd_pd_rx(HAL_USBPD_T *tcpd, uint8_t *buf, unsigned long len);

    /**************************************************************************************
 ** NTC Driver abstraction
 **************************************************************************************/
    void fusb15xxx_enable_ntc(int ntc_port, bool en);
    void fusb15xxx_ntc_convert_adc_to_temp(int adc);

    /**************************************************************************************
 ** DCDC Function Driver abstraction
 **************************************************************************************/
#if (CONFIG_ENABLED(LEGACY_I2C))
    HAL_I2C_T *fusb15xxx_i2c_enable(I2C_Port_Type *);
#else
HAL_I2C_T *fusb15xxx_i2c_enable(int);
#endif
    void        fusb15xxx_i2c_disable(HAL_I2C_T *i2c);
    void        fusb15xxx_i2c_master_enable(HAL_I2C_T *i2c, uint32_t prescale);
    void        fusb15xxx_i2c_master_disable(HAL_I2C_T *const i2c);
    void        fusb15xxx_i2c_slave_enable(HAL_I2C_T *i2c, uint8_t addr, uint32_t prescale);
    void        fusb15xxx_i2c_slave_disable(HAL_I2C_T *i2c);
    HAL_ERROR_T fusb15xxx_i2c_write(HAL_I2C_T *const i2c, uint8_t addr, uint8_t *pBuf, uint32_t len,
                                    uint32_t timeout);
    HAL_ERROR_T fusb15xxx_i2c_read(HAL_I2C_T *const i2c, uint8_t addr, uint8_t *pBuf, uint32_t len,
                                   uint32_t timeout);
    void        fusb15xxx_i2c_write_with_interrupt(HAL_I2C_T *i2c, uint8_t addr, uint8_t *pBuf,
                                                   uint32_t len, HAL_I2CCB_T cb);
    void        fusb15xxx_i2c_read_with_interrupt(HAL_I2C_T *i2c, uint8_t addr, uint8_t *pBuf,
                                                  uint32_t len, HAL_I2CCB_T cb);

#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
    void fusb15xxx_i2c_write_with_dma(HAL_I2C_T *i2c, uint8_t addr, uint8_t *pBuf, uint32_t len,
                                      HAL_I2CCB_T cb);
    void fusb15xxx_i2c_read_with_dma(HAL_I2C_T *i2c, uint8_t addr, uint8_t *pBuf, uint32_t len,
                                     HAL_I2CCB_T cb);
#endif

    void fusb15xxx_i2c_read_register(HAL_I2C_T *i2c, uint8_t addr, uint8_t reg, uint8_t *pBuf,
                                     uint32_t len, HAL_I2CCB_T cb);
    void fusb15xxx_i2c_slave_start(HAL_I2C_T *i2c, uint8_t *pBuf, uint32_t size, HAL_SLAVE_CB_T cb);
    void fusb15xxx_i2c_slave_set_offset(HAL_I2C_T *i2c, uint32_t offset);

#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
    void fusb15xxx_i2c_slave_setup_rx_dma(HAL_I2C_T *i2c, uint8_t *pBuf, uint32_t size,
                                          HAL_SLAVE_CB_T cb);
    void fusb15xxx_i2c_slave_setup_tx_dma(HAL_I2C_T *i2c, uint8_t *pBuf, uint32_t size,
                                          HAL_SLAVE_CB_T cb);
#endif

    void    fusb15xxx_i2c_irq_enable(HAL_I2C_T *i2c, HAL_I2CINT_T irq);
    void    fusb15xxx_i2c_irq_disable(HAL_I2C_T *i2c);
    void    fusb15xxx_i2c_irq_set(HAL_I2C_T *i2c, uint32_t val);
    void    fusb15xxx_i2c_irq_clear(HAL_I2C_T *i2c);
    uint8_t fusb15xxx_i2c_irq_status(HAL_I2C_T *i2c);
    void    fusb15xxx_i2c_irq_set_callback(HAL_I2C_T *i2c, HAL_INTCB_T cb);

    /**************************************************************************
 * ADC  driver abstraction *
 **************************************************************************/
    void               fusb15xxx_adc_enable(void);
    void               fusb15xxx_adc_disable(void);
    void               fusb15xxx_adc_ch_enable(int ch, bool en);
    void               fusb15xxx_adc_ch_disable(int ch, bool en);
    HAL_ADC_CHx_STAT_T fusb15xxx_adc_ch_status(int dp_ch);
    bool               fusb15xxx_adc_ch_ready(int dp_ch);
    unsigned long      fusb15xxx_adc_ch_data(int ch);
    void               fusb15xxx_adc_ch_set_alarm(int ch, int low, int high);
    void               fusb15xxx_adc_ch_set_low_alarm_interrupt(int ch, bool en);
    void               fusb15xxx_adc_ch_set_high_alarm_interrupt(int ch, bool en);
    uint32_t           fusb15xxx_adc_ch_interrupt_status(int ch);
    void               fusb15xxx_adc_ch_interrupt_enable(bool en);
    void               fusb15xxx_adc_ch_interrupt_clear(void);
    void               fusb15xxx_adc_ch_set_reference(int ch, int refsel);
    uint32_t           fusb15xxx_adc_ch_reference(int ch);
    void               fusb15xxx_adc_ch_autoscale_enable(HAL_ADCHx_T ch, bool en);

    /**************************************************************************
 * BC1P2 function driver abstraction *
 **************************************************************************/
    HAL_USBCHG_T *fusb15xxx_usbchg_enable(int usbchg_id);
    void          fusb15xxx_usbchg_disable(HAL_USBCHG_T *usbchg);
    void          fusb15xxx_usbchg_idp_src(HAL_USBCHG_T *usbchg, bool en);
    void          fusb15xxx_usbchg_idp_snk(HAL_USBCHG_T *usbchg, bool en);
    void          fusb15xxx_usbchg_idm_snk(HAL_USBCHG_T *usbchg, bool en);
    void          fusb15xxx_usbchg_rdm_dwn(HAL_USBCHG_T *usbchg, bool en);
    void          fusb15xxx_usbchg_rdp_dwn(HAL_USBCHG_T *usbchg, bool en);
    void          fusb15xxx_usbchg_vdp_src(HAL_USBCHG_T *usbchg, bool en);
    void          fusb15xxx_usbchg_vdm_src(HAL_USBCHG_T *usbchg, bool en);
    void          fusb15xxx_usbchg_host_sw(HAL_USBCHG_T *usbchg, bool en);
    void          fusb15xxx_usbchg_rdiv(HAL_USBCHG_T *usbchg, bool en);
    void          fusb15xxx_usbchg_rdcp(HAL_USBCHG_T *usbchg, bool en);
    void          fusb15xxx_usbchg_rdat(HAL_USBCHG_T *usbchg, bool en);

    /**************************************************************************
 * GPIO function driver abstraction *
 **************************************************************************/
    void fusb15xxx_gpio_initialize(HAL_GPIOx_T pin, HAL_GPIOCFG_T *cfg, HAL_GPIO_PORT_T port);
    void fusb15xxx_gpio_set(HAL_GPIOx_T pin, HAL_GPIOLVL_T lvl, HAL_GPIO_PORT_T port);
    HAL_GPIOLVL_T fusb15xxx_gpio_state(HAL_GPIOx_T pin, HAL_GPIODIR_T dir, HAL_GPIO_PORT_T port);
    void          fusb15xxx_gpio_enable_interrupt(HAL_GPIOx_T pin, HAL_GPIO_IRQ_T type,
                                                  HAL_GPIO_IRQ_POL_T pol, HAL_GPIO_PORT_T port);
    void          fusb15xxx_gpio_disable_interrupt(HAL_GPIOx_T pin, HAL_GPIO_PORT_T port);
    void          fusb15xxx_gpio_interrupt_status(HAL_GPIOx_T pin, HAL_GPIO_PORT_T port);
    void          fusb15xxx_gpio_interrupt_clear(HAL_GPIOx_T pin, HAL_GPIO_PORT_T port);

    /**************************************************************************
 * PORT function driver abstraction *
 **************************************************************************/
    void fusb15xxx_port_initialize(HAL_PORTx_T pin, HAL_PORTCFG_T *cfg, HAL_GPIO_PORT_T port);
    void fusb15xxx_port_enable_interrupt(HAL_PORTx_T pin, HAL_PORT_IRQ_T pol, HAL_GPIO_PORT_T port);
    void fusb15xxx_port_disable_interrupt(HAL_GPIO_PORT_T port);
    void fusb15xxx_port_interrupt_status(HAL_GPIO_PORT_T port);
    void fusb15xxx_port_interrupt_clear(HAL_GPIO_PORT_T port);
    void fusb15xxx_port_enable_nmi(HAL_NMI_CFG_T nmi, HAL_GPIO_PORT_T port);
    void fusb15xxx_port_disable_nmi(HAL_GPIO_PORT_T port);
    bool fusb15xxx_port_is_swd_enabled();

    typedef enum fusb15xx_type
    {
        fusb15101_dev = 0x0F01,
        fusb15200_dev,
        fusb15201_dev,
        fusb15201p_dev,
    } FUSB15DEV_TYPE_T;

    bool          fusb15xxx_init(enum fusb15xx_type dev_id);
    HAL_USBPD_T  *fusb15xxx_device_tcpd_init(int tcpd_id, int dev_id, bool en, bool reinit);
    void          fusb15xxx_sleep_init(HAL_USBPD_T *tcpd);
    HAL_USBCHG_T *fusb15xxx_device_bc1p2_init(int usbchg_id, int dev_id, bool en);
    HAL_I2C_T    *fusb15xxx_device_dcdc_init(int i2c_id, int dev_id, bool en);

#ifdef __cplusplus
}
#endif

#endif /** @} _FUSB15XXX_HAL_H__ */
