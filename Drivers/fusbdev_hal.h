/**
 * @file     fusbdev_hal.h
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
#ifndef __FUSBDEV_HAL_H__
#define __FUSBDEV_HAL_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "pd_types.h"
#define TCPD_PORT_ID(id)   ((id) % TYPE_C_PORT_COUNT)
#define LEGACY_PORT_ID(id) ((id) % LEGACY_PORT_COUNT)
#define I2C_PORT_ID(id)    ((id) % I2C_PORT_COUNT)
    /*
 * Platform specific data
 */
    typedef struct fusb_tcpd_device
    {
        HAL_USBPD_CHID_T                  tcpd_id;
        HAL_USBPD_T                      *tcpd;
        struct fusb_driver_t             *fusb_driv;
        struct fusb15101_state_variables *fusb15101_state;
    } FUSB_TCPD_DEVICE_T;

    /*
 * Platform specific data
 */
    typedef struct fusb_ps_device
    {
        HAL_USBPD_CHID_T      ps_id;
        HAL_USBPD_T          *tcpd;
        struct fusb_driver_t *fusb_driv;
    } FUSB_PS_DEVICE_T;

    /*
 * Platform specific data
 */
    typedef struct fusb_dcdc_device
    {
        HAL_I2C_T            *i2c[I2C_PORT_COUNT];
        struct fusb_driver_t *fusb_driv;
    } FUSB_DCDC_DEVICE_T;

    /*
 * Platform specific data
 */
    typedef struct fusb_bc1p2_device
    {
        HAL_USBCHG_CHID_T     usbchg_id;
        HAL_USBCHG_T         *usbchg;
        int                   dp_ch; ///< Channel info forA DP
        int                   dm_ch; ///< Channel info for DM
        struct fusb_driver_t *fusb_driv;
    } FUSB_BC1P2_DEVICE_T;

#if (DEVICE_TYPE == FUSB15101)

#endif

    /* Function Definition */

    //tcpd related functions
    void fusbdev_tcpd_initialize(struct fusb_tcpd_device *dev, bool en, bool reinit);
    bool fusbdev_tcpd_rx_hardreset(struct fusb_tcpd_device *dev);
    void fusbdev_tcpd_tx_hardreset(struct fusb_tcpd_device *dev, SOP_T sop);
    void fusbdev_tcpd_set_term(struct fusb_tcpd_device *dev, int mask, CC_TERM_T term);
    void fusbdev_tcpd_set_rp(struct fusb_tcpd_device *dev, int mask, CC_RP_T rp);
    void fusbdev_tcpd_set_orient(struct fusb_tcpd_device *dev, CC_T cc);
    void fusbdev_tcpd_set_cc(struct fusb_tcpd_device *dev, int mask, CC_TERM_T term, CC_RP_T rp,
                             DRP_MODE_T drp);
    CC_STAT_T fusbdev_tcpd_cc_stat(struct fusb_tcpd_device *dev, CC_T cc);
    void      fusbdev_tcpd_cc_events_enable(struct fusb_tcpd_device *dev, bool en);
    bool      fusbdev_tcpd_cc_check_error(struct fusb_tcpd_device *dev);
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
    void       fusbdev_tcpd_set_drp(struct fusb_tcpd_device *dev, DRP_MODE_T drp);
    DRP_STAT_T fusbdev_tcpd_drp_stat(struct fusb_tcpd_device *dev);
#endif
    void     fusbdev_tcpd_vbus_init(struct fusb_tcpd_device *dev);
    void     fusbdev_tcpd_vbus_deinit(struct fusb_tcpd_device *dev);
    void     fusbdev_tcpd_vbus_events(struct fusb_tcpd_device *dev, bool en);
    bool     fusbdev_tcpd_set_vbus_source(struct fusb_tcpd_device *dev, unsigned mv, unsigned ma,
                                          PDO_T type);
    bool     fusbdev_tcpd_set_vbus_sink(struct fusb_tcpd_device *dev, unsigned mv, unsigned ma,
                                        PDO_T type);
#if (DEVICE_TYPE == FUSB15101)
    unsigned fusbdev_tcpd_vbus_pps_value(struct fusb_tcpd_device *dev);
#endif
    unsigned fusbdev_tcpd_vbus_value(struct fusb_tcpd_device *dev);
    void     fusbdev_tcpd_set_vbus_res(struct fusb_tcpd_device *dev, unsigned max);
    void     fusbdev_tcpd_set_vbus_alarm(struct fusb_tcpd_device *dev, unsigned low, unsigned high);
    void     fusbdev_tcpd_set_vbus_disch(struct fusb_tcpd_device *dev, VBUS_DISCH_T disch);
    CC_RP_T  fusbdev_tcpd_pullup_value(struct fusb_tcpd_device *dev, CC_T cc);
    void     fusbdev_tcpd_vconn_enable(struct fusb_tcpd_device *dev, bool en, HAL_VCONN_OCP_T ocp);
    void     fusbdev_tcpd_pd_enable(struct fusb_tcpd_device *dev, bool en);
    bool     fusbdev_tcpd_pd_tx(struct fusb_tcpd_device *dev, uint8_t const *buf, unsigned long len,
                                SOP_T sop, int retries);
    bool     fusbdev_tcpd_pd_check_rx(struct fusb_tcpd_device *dev);
    SOP_T    fusbdev_tcpd_pd_rx(struct fusb_tcpd_device *dev, uint8_t *buf, unsigned long len);
    void     fusbdev_tcpd_pd_sop_enable(struct fusb_tcpd_device *dev, unsigned mask);
    void     fusbdev_tcpd_pd_rev(struct fusb_tcpd_device *dev, PD_REV_T rev, SOP_T sop);
    void     fusbdev_tcpd_set_pd_dfp(struct fusb_tcpd_device *dev, bool dfp_en);
    void     fusbdev_tcpd_set_pd_source(struct fusb_tcpd_device *dev, bool src_en);
    void     fusbdev_tcpd_set_bist_mode(struct fusb_tcpd_device *dev, BIST_MODE_T mode);
    PORT_TX_STAT_T fusbdev_tcpd_port_tx_status(struct fusb_tcpd_device *dev);

    //timer related functions
    void fusbdev_timer_start(enum timer_type_t timer_dev, int count, bool periodic);
    void fusbdev_timer_interrupt_enable(enum timer_type_t timer_dev, bool en);
    void fusbdev_timer_stop(enum timer_type_t timer_dev);

//cmu related functions
#if (DEVICE_TYPE != FUSB15101)
    void fusbdev_cmu_clock_select(enum clk_sel_t clk_sel);
#endif

    //adc related functions
    void               fusbdev_adc_enable(void);
    void               fusbdev_adc_disable(void);
    void               fusbdev_adc_ch_enable(int ch);
    void               fusbdev_adc_ch_disable(int ch);
    HAL_ADC_CHx_STAT_T fusbdev_adc_ch_status(int dp_ch);
    bool               fusbdev_adc_ch_ready(int dp_ch);
    unsigned long      fusbdev_adc_ch_data(int ch);
    void               fusbdev_adc_ch_set_alarm(int ch, uint32_t low, uint32_t high);
    void               fusbdev_adc_ch_set_low_alarm_interrupt(int ch, bool en);
    void               fusbdev_adc_ch_set_high_alarm_interrupt(int ch, bool en);
    uint32_t           fusbdev_adc_ch_interrupt_status(int ch);
    void               fusbdev_adc_ch_interrupt_enable(bool en);
    void               fusbdev_adc_ch_interrupt_clear(void);
    void               fusbdev_adc_ch_set_reference(int ch, int refsel);
    void               fusbdev_adc_ch_reference(int ch);
#if (DEVICE_TYPE != FUSB15101)
    void fusbdev_adc_ch_autoscale_enable(HAL_ADCHx_T ch, bool en);
#endif

//i2c related functions
#if CONFIG_ENABLED(DCDC)
    HAL_I2C_T *fusbdev_i2c_enable(struct fusb_dcdc_device *dev, int i2c_id);
    void       fusbdev_i2c_disable(struct fusb_dcdc_device *dev, int i2c_id);
    void fusbdev_i2c_master_enable(struct fusb_dcdc_device *dev, uint32_t prescale, int i2c_id);
    void fusbdev_i2c_master_disable(struct fusb_dcdc_device *dev, int i2c_id);
    void fusbdev_i2c_write(struct fusb_dcdc_device *dev, uint8_t addr, uint8_t *pBuf, uint32_t len,
                           uint32_t timeout, int i2c_id);
    void fusbdev_i2c_read(struct fusb_dcdc_device *dev, uint8_t addr, uint8_t *pBuf, uint32_t len,
                          uint32_t timeout, int i2c_id);
    void fusbdev_i2c_write_with_interrupt(struct fusb_dcdc_device *dev, uint8_t addr, uint8_t *pBuf,
                                          uint32_t len, HAL_I2CCB_T cb, int i2c_id);
    void fusbdev_i2c_read_with_interrupt(struct fusb_dcdc_device *dev, uint8_t addr, uint8_t *pBuf,
                                         uint32_t len, HAL_I2CCB_T cb, int i2c_id);
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
    void fusbdev_i2c_write_with_dma(struct fusb_dcdc_device *dev, uint8_t addr, uint8_t *pBuf,
                                    uint32_t len, HAL_I2CCB_T cb, int i2c_id);
    void fusbdev_i2c_read_with_dma(struct fusb_dcdc_device *dev, uint8_t addr, uint8_t *pBuf,
                                   uint32_t len, HAL_I2CCB_T cb, int i2c_id);
#endif
    void fusbdev_i2c_read_register(struct fusb_dcdc_device *dev, uint8_t addr, uint8_t reg,
                                   uint8_t *pBuf, uint32_t len, HAL_I2CCB_T cb, int i2c_id);
#if (CONFIG_ENABLED(I2C_SLAVE))
    void fusbdev_i2c_slave_enable(struct fusb_dcdc_device *dev, uint8_t addr, uint32_t prescale,
                                  int i2c_id);
    void fusbdev_i2c_slave_disable(struct fusb_dcdc_device *dev, int i2c_id);
    void fusbdev_i2c_slave_start(struct fusb_dcdc_device *dev, uint8_t *pBuf, uint32_t size,
                                 HAL_SLAVE_CB_T cb, int i2c_id);
    void fusbdev_i2c_slave_set_offset(struct fusb_dcdc_device *dev, uint32_t offset, int i2c_id);
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
    void fusbdev_i2c_slave_setup_rx_dma(struct fusb_dcdc_device *dev, uint8_t *pBuf, uint32_t size,
                                        HAL_SLAVE_CB_T cb, int i2c_id);
    void fusbdev_i2c_slave_setup_tx_dma(struct fusb_dcdc_device *dev, uint8_t *pBuf, uint32_t size,
                                        HAL_SLAVE_CB_T cb, int i2c_id);
#endif
#endif

    void fusbdev_i2c_irq_enable(struct fusb_dcdc_device *dev, HAL_I2CINT_T irq, int i2c_id);
    void fusbdev_i2c_irq_disable(struct fusb_dcdc_device *dev, int i2c_id);
    void fusbdev_i2c_irq_set(struct fusb_dcdc_device *dev, uint32_t val, int i2c_id);
    void fusbdev_i2c_irq_clear(struct fusb_dcdc_device *dev, int i2c_id);
    void fusbdev_i2c_irq_status(struct fusb_dcdc_device *dev, int i2c_m, int i2c_id);
    void fusbdev_i2c_irq_set_callback(struct fusb_dcdc_device *dev, HAL_INTCB_T cb, int i2c_id);
#endif

#if CONFIG_ENABLED(CUSTOM_APPLICATION)
#if (CONFIG_ENABLED(LEGACY_I2C))
    HAL_I2C_T *fusbdev_i2c_enable(I2C_Port_Type *i2c);
#else
    HAL_I2C_T *fusbdev_i2c_enable(int i2c_id);
#endif
    void fusbdev_i2c_disable(HAL_I2C_T *i2c, int i2c_id);
    void fusbdev_i2c_master_enable(HAL_I2C_T *i2c, uint32_t prescale, int i2c_id);
    void fusbdev_i2c_master_disable(HAL_I2C_T *i2c, int i2c_id);
#if (CONFIG_ENABLED(I2C_SLAVE))
    void fusbdev_i2c_slave_enable(HAL_I2C_T *i2c, uint8_t addr, uint32_t prescale, int i2c_id);
    void fusbdev_i2c_slave_disable(HAL_I2C_T *i2c, int i2c_id);
#endif
#if (CONFIG_ENABLED(I2C_BLOCKING))
    void fusbdev_i2c_write(HAL_I2C_T *i2c, uint8_t addr, uint8_t *pBuf, uint32_t len,
                           uint32_t timeout, int i2c_id);
    void fusbdev_i2c_read(HAL_I2C_T *i2c, uint8_t addr, uint8_t *pBuf, uint32_t len,
                          uint32_t timeout, int i2c_id);
#else
    void fusbdev_i2c_write_with_interrupt(HAL_I2C_T *i2c, uint8_t addr, uint8_t *pBuf, uint32_t len,
                                          HAL_I2CCB_T cb, int i2c_id);
    void fusbdev_i2c_read_with_interrupt(HAL_I2C_T *i2c, uint8_t addr, uint8_t *pBuf, uint32_t len,
                                         HAL_I2CCB_T cb, int i2c_id);
#endif
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
    void fusbdev_i2c_write_with_dma(HAL_I2C_T *i2c, uint8_t addr, uint8_t *pBuf, uint32_t len,
                                    HAL_I2CCB_T cb, int i2c_id);
    void fusbdev_i2c_read_with_dma(HAL_I2C_T *i2c, uint8_t addr, uint8_t *pBuf, uint32_t len,
                                   HAL_I2CCB_T cb, int i2c_id);
#if (CONFIG_ENABLED(I2C_SLAVE))
    void fusbdev_i2c_slave_setup_rx_dma(HAL_I2C_T *i2c, uint8_t *pBuf, uint32_t size,
                                        HAL_SLAVE_CB_T cb, int i2c_id);
    void fusbdev_i2c_slave_setup_tx_dma(HAL_I2C_T *i2c, uint8_t *pBuf, uint32_t size,
                                        HAL_SLAVE_CB_T cb, int i2c_id);
#endif
#endif
    void fusbdev_i2c_read_register(HAL_I2C_T *i2c, uint8_t addr, uint8_t reg, uint8_t *pBuf,
                                   uint32_t len, HAL_I2CCB_T cb, int i2c_id);
#if (CONFIG_ENABLED(I2C_SLAVE))
    void fusbdev_i2c_slave_start(HAL_I2C_T *i2c, uint8_t *pBuf, uint32_t size, HAL_SLAVE_CB_T cb,
                                 int i2c_id);
    void fusbdev_i2c_slave_set_offset(HAL_I2C_T *i2c, uint32_t offset, int i2c_id);
#endif
    void fusbdev_i2c_irq_enable(HAL_I2C_T *i2c, HAL_I2CINT_T irq, int i2c_id);
    void fusbdev_i2c_irq_disable(HAL_I2C_T *i2c, int i2c_id);
    void fusbdev_i2c_irq_set(HAL_I2C_T *i2c, uint32_t val, int i2c_id);
    void fusbdev_i2c_irq_clear(HAL_I2C_T *i2c, int i2c_id);
    void fusbdev_i2c_irq_status(HAL_I2C_T *i2c, int i2c_m, int i2c_id);
    void fusbdev_i2c_irq_set_callback(HAL_I2C_T *i2c, HAL_INTCB_T cb, int i2c_id);
#endif

//usbchg related functions
#if CONFIG_ENABLED(LEGACY_CHARGING)
    void fusbdev_usbchg_enable(struct fusb_bc1p2_device const *dev);
    void fusbdev_usbchg_disable(struct fusb_bc1p2_device const *dev);
    void fusbdev_usbchg_idp_src(struct fusb_bc1p2_device const *dev, bool en);
    void fusbdev_usbchg_idp_snk(struct fusb_bc1p2_device const *dev, bool en);
    void fusbdev_usbchg_rdm_dwn(struct fusb_bc1p2_device const *dev, bool en);
    void fusbdev_usbchg_rdp_dwn(struct fusb_bc1p2_device const *dev, bool en);
    void fusbdev_usbchg_vdp_src(struct fusb_bc1p2_device const *dev, bool en);
    void fusbdev_usbchg_vdm_src(struct fusb_bc1p2_device const *dev, bool en);
    void fusbdev_usbchg_idm_sink(struct fusb_bc1p2_device const *dev, bool en);
    void fusbdev_usbchg_host_sw(struct fusb_bc1p2_device const *dev, bool en);
    void fusbdev_usbchg_rdiv(struct fusb_bc1p2_device const *dev, bool en);
    void fusbdev_usbchg_rdcp(struct fusb_bc1p2_device const *dev, bool en);
    void fusbdev_usbchg_rdat(struct fusb_bc1p2_device const *dev, bool en);
#endif

    //gpio related functions
    void fusbdev_configure_gpio_pin(HAL_GPIOx_T pin, HAL_GPIOCFG_T *cfg, HAL_GPIO_PORT_T port);
    void fusbdev_set_gpio_pin(HAL_GPIOx_T pin, HAL_GPIOLVL_T lvl, HAL_GPIO_PORT_T port);
    HAL_GPIOLVL_T fusbdev_state_gpio_pin(HAL_GPIOx_T pin, HAL_GPIODIR_T dir, HAL_GPIO_PORT_T port);
    void          fusbdev_enable_interrupt_gpio_pin(HAL_GPIOx_T pin, HAL_GPIO_IRQ_T type,
                                                    HAL_GPIO_IRQ_POL_T pol, HAL_GPIO_PORT_T port);
    void          fusbdev_disable_interrupt_gpio_pin(HAL_GPIOx_T pin, HAL_GPIO_PORT_T port);
    void          fusbdev_interrupt_status_gpio_pin(HAL_GPIOx_T pin, HAL_GPIO_PORT_T port);
    void          fusbdev_interrupt_clear_gpio_pin(HAL_GPIOx_T pin, HAL_GPIO_PORT_T port);

    //port related functions
    void fusbdev_configure_port_io(HAL_PORTx_T pin, HAL_PORTCFG_T *cfg, HAL_GPIO_PORT_T port);
    void fusbdev_enable_interrupt_port_io(HAL_PORTx_T pin, HAL_PORT_IRQ_T pol,
                                          HAL_GPIO_PORT_T port);
    void fusbdev_disable_interrupt_port_io(HAL_GPIO_PORT_T port);
    void fusbdev_interrupt_status_port_io(HAL_GPIO_PORT_T port);
    void fusbdev_interrupt_clean_port_io(HAL_GPIO_PORT_T port);
#if (CONFIG_ENABLED(NMI))
    void fusbdev_enable_nmi_port_io(HAL_NMI_CFG_T nmi, HAL_GPIO_PORT_T port);
    void fusbdev_disable_nmi_port_io(HAL_GPIO_PORT_T port);
#endif
    bool fusbdev_port_is_swd_enabled();

    //ntc related functions
    void fusbdev_ntc_enable(int ntc_port, bool en);
    void fusbdev_ntc_convert_adc_to_temp(int adc);

    typedef enum dev_type
    {
        fusb15_dev  = 0x0F,
        fusb18_dev  = 0x12,
        dev_unknown = -1,
    } DEV_TYPE_T;

    /*Function Declaration*/
    struct fusb_driver_t *device_init(int);
    FUSB_PS_DEVICE_T     *device_ps_init(int port, int dev_id, bool en);
    FUSB_DCDC_DEVICE_T   *device_dcdc_init(int dev_id, bool en);
    FUSB_BC1P2_DEVICE_T  *device_bc1p2_init(int port, int dev_id, bool en);
    FUSB_TCPD_DEVICE_T   *device_tcpd_init(int port, int dev_id, bool en, bool reinit);
    void                  device_sleep_init(struct fusb_tcpd_device *dev);

#define TCPD_TIMER   TYPE_C_PORT_COUNT
#define LEGACY_TIMER 0
#define DCDC_TIMER   0
#if (CONFIG_ENABLED(BATT_NTC) || CONFIG_ENABLED(ALT_BATT_NTC))
#define BNM_TIMER 1
#else
#define BNM_TIMER 0
#endif
#if CONFIG_ENABLED(LEGACY_CHARGING)
#undef LEGACY_TIMER
#define LEGACY_TIMER TYPE_C_PORT_COUNT
#endif

#if CONFIG_ENABLED(DCDC)
#if (DEVICE_TYPE == FUSB15201) || (DEVICE_TYPE == FUSB15201P)
#undef DCDC_TIMER
#define DCDC_TIMER TYPE_C_PORT_COUNT
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif /*__FUSBDEV_HAL_H__*/
