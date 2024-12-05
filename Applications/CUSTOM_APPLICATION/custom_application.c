/*******************************************************************************
 * @file   custom_application.c
 * @author USB Firmware Team
 * @brief  Implements the Device Policy Manager state machine functions.
 *
 * @copyright @parblock
 * Copyright &copy; 2020 ON Semiconductor &reg;. All rights reserved.
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
*******************************************************************************/
#include "fusb15xxx.h"
#include "dev_tcpd.h"
#include "fusbdev_hal.h"
#include "timer.h"
#include "vif_types.h"
#include "vdm.h"
#include "power_sharing.h"
#include "dpm.h"
#include "dc_dc_board.h"
extern volatile bool gSleepInt;
#include "legacy_types.h"
#include "port_bc1p2.h"
#include "port_dcdc.h"
#if (CONFIG_ENABLED(POWER_SHARING))
#include "port_ps.h"
#endif

#if CONFIG_ENABLED(CUSTOM_APPLICATION_INCLUDE)
#include "custom_application.h"

extern HAL_I2C_T    *i2c_master[I2C_PORT_COUNT];
extern volatile bool i2c_porta_interrupt;
extern volatile bool i2c_portb_interrupt;
#endif
#if CONFIG_ALT_BATT_NTC
#include "custom_application_monitor.h"
#endif
#if (CONFIG_ENABLED(CUSTOM_APPLICATION_001)) || (CONFIG_ENABLED(CUSTOM_APPLICATION_002))
#include "get_capacity.h"
#define PPS_QUERY_TIME 50
#endif
/* PDPS OCP */
void dpm_ocp_init()
{
    uint8_t i;
#if (DEVICE_TYPE == FUSB15201)
    HAL_I2C_T *i2c           = fusbdev_i2c_enable(I2C_Port_3);
    i2c_master[2]            = i2c;
    HAL_GPIOx_T gpio         = HAL_GPIO7;
    HAL_PORTx_T port_i2c_int = PORT17;
    IRQn_Type   irqn         = I2C3_IRQn;
#elif (DEVICE_TYPE == FUSB15201P)
    HAL_I2C_T *i2c           = fusbdev_i2c_enable(I2C_Port_1);
    i2c_master[0]            = i2c;
    HAL_GPIOx_T gpio         = HAL_GPIO3;
    HAL_PORTx_T port_i2c_int = PORT4;
    IRQn_Type   irqn         = I2C1_IRQn;
#endif
    /* Set up PortA I2C interrupt */
    HAL_PORTCFG_T portCfg = {.an  = PORT_DIGITAL,
                             .alt = PORT_ALTERNATE,
                             .pu  = PORT_PULLUP_DISABLE,
                             .pd  = PORT_PULLDOWN_DISABLE};

    fusbdev_configure_port_io(port_i2c_int, &portCfg, HAL_GPIO_B);
    fusbdev_i2c_irq_enable(
        i2c, I2C_INT_DIR_IN | I2C_INT_ENABLE | I2C_INT_FALLING_LOW | I2C_INT_LEVEL, 0);
    NVIC_SetPriority(irqn, 1);
    NVIC_EnableIRQ(irqn);

    /* Set up PortB I2C interrupt */
    HAL_GPIOCFG_T cfg = {.iodir = HAL_GPIO_INPUT, .ppod = HAL_GPIO_PUSH_PULL};
    fusbdev_configure_gpio_pin(gpio, &cfg, HAL_GPIO_B);
    fusbdev_enable_interrupt_gpio_pin(gpio, HAL_GPIO_IRQ_LEVEL, HAL_GPIO_IRQ_HIGH_OR_RISING,
                                      HAL_GPIO_B);
    NVIC_SetPriority(GPIO_IRQn, 1);
    NVIC_EnableIRQ(GPIO_IRQn);
}

void dpm_ocp_processing(void)
{
    uint8_t               i;
    MPQ4260_STATUS_FAULTS fault[TYPE_C_PORT_COUNT];
#if (DEVICE_TYPE == FUSB15201)
    HAL_I2C_T  *i2c  = i2c_master[2];
    HAL_GPIOx_T gpio = HAL_GPIO7;
#elif (DEVICE_TYPE == FUSB15201P)
    HAL_I2C_T  *i2c          = i2c_master[0];
    HAL_GPIOx_T gpio         = HAL_GPIO3;
#endif
    struct port *port1 = DPM.port_tcpd[0]->port_tcpd_priv;
    struct port *port2 = DPM.port_tcpd[1]->port_tcpd_priv;
    /* If need to send message, set this to true */
    if (((i2c_porta_interrupt == true) || (i2c_portb_interrupt == true))
        && (!port1->policy_sending_message && !port2->policy_sending_message)
		&& !(port1->policy_state == PE_SRC_Transition_Supply || port2->policy_state == PE_SRC_Transition_Supply))
    {
        i2c_porta_interrupt = false;
        i2c_portb_interrupt = false;
        pdpsReadFaultStatus();
        pdpsReadFaultStatus();
        fusbdev_i2c_irq_enable(
            i2c, I2C_INT_DIR_IN | I2C_INT_ENABLE | I2C_INT_FALLING_LOW | I2C_INT_LEVEL, 0);
        fusbdev_enable_interrupt_gpio_pin(gpio, HAL_GPIO_IRQ_LEVEL, HAL_GPIO_IRQ_LOW_OR_FALLING,
                                          HAL_GPIO_SHARED);

        /* Handle faults */
        for (i = 0; i < TYPE_C_PORT_COUNT; i++)
        {
        	struct dpm_info * dpm_info = (struct dpm_info *)(DPM.dpm_info[i]);
        	struct port * port = (struct port *)(DPM.port_tcpd[i]->port_tcpd_priv);
        	uint32_t vbus_sink_selected = port->sink_selected_voltage;
        	bool old_pps_mode = dpm_info->pps_mode;
            fault[i] = pdpsGetFaultStatus(i);
            if (!DPM.dpm_info[i]->pps_active && fault[i].IOUT_OC_FAULT)
            {
                DPM.dpm_info[i]->ocp_dev.ocp_count++;
#if (CONFIG_ENABLED(PDPS_OCP))
                if (DPM.dpm_info[i]->ocp_dev.ocp_count >= OCP_COUNT_MAX)
                {
                    pdps_ocp_disable_port(&DPM.dpm_info[i]->ocp_dev, &DPM.ps_dev, DPM.port_bc1p2[i],
                                          DPM.port_tcpd[i]);
                }
                else
                {
                    pdps_ocp_reset(&DPM.dpm_info[i]->ocp_dev, &DPM.ps_dev, DPM.port_bc1p2[i],
                                   DPM.port_tcpd[i]);
                }
#endif
            }
            else if (fault[i].IOUT_OC_FAULT)
            {
            	/* means PPS is active and fault triggered, switch to cc mode */
            	dpm_vbus_monitor_set_pps_cl(DPM.port_tcpd[i], vbus_sink_selected);
            }
            else if (DPM.dpm_info[i]->pps_active && !fault[i].IOUT_OC_FAULT)
            {
            	/* means fault is not triggered, swap to CV mode */
            	dpm_vbus_monitor_set_pps_cv(DPM.port_tcpd[i], vbus_sink_selected);
            }
            if (old_pps_mode != dpm_info->pps_mode)
            {
            	port->pps_mode_change = true;
            }
        }
    }
}
void custom_application_dpm_init(void)
{
    __enable_irq();
    PDPS_DRIVER pdps_driver;
#if (DEVICE_TYPE == FUSB15201)
    pdps_driver.i2cNum = 3; /*POWERSUPPLY I2C */
#elif (DEVICE_TYPE == FUSB15201P)
    pdps_driver.i2cNum       = 1; /*POWERSUPPLY I2C */
#endif
    pdps_driver.numPorts = 2; /*Number of ports */
    pdpsInit(&pdps_driver);
}