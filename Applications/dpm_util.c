/*******************************************************************************
 * @file   dpm_util.c
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
#include "dpm_util.h"
#include "dpm.h"
unsigned fw_version_check(uint16_t *current_ver, uint16_t new_ver_0, uint16_t new_ver_1,
                          uint16_t new_ver_2, uint16_t new_ver_3)
{
    uint8_t ret = 0;
    if (new_ver_0 > current_ver[0])
    {
        ret = 1;
    }
    else if (new_ver_1 > current_ver[1])
    {
        ret = 1;
    }
    else if (new_ver_2 > current_ver[2])
    {
        ret = 1;
    }
    else if (new_ver_3 > current_ver[3])
    {
        ret = 1;
    }
    return ret;
}

void enable_swd(void *dpm_priv, bool en)
{
#if (DEBUG_PORTB)
    struct dpm_t *dpm = (struct dpm_t *)dpm_priv;

    HAL_PORTCFG_T portcfg = {
        .an = PORT_DIGITAL, .alt = PORT_ENABLE_OVERRIDE, .pd = PORT_PULLDOWN_ENABLE};
    HAL_GPIOCFG_T outGPIOConfig = {
        .iodir = HAL_GPIO_OUTPUT,
        .ppod  = HAL_GPIO_PUSH_PULL,
    };
#if DEVICE_TYPE == FUSB15201 || DEVICE_TYPE == FUSB15200
    HAL_PORTx_T port = PORT2;
#else
    HAL_PORTx_T port = PORT1;
#endif
    if (en == true)
    {
    	fusbdev_configure_port_io(port, &portcfg, HAL_GPIO_B);
    }
    else
    {
        portcfg.alt = PORT_DISABLE_OVERRIDE;
        fusbdev_configure_port_io(port, &portcfg, HAL_GPIO_B);
        fusbdev_configure_gpio_pin(HAL_GPIO2, &outGPIOConfig, HAL_GPIO_B);
    }
#endif
}

bool dpm_vbus_valid(void *dpm_priv, uint32_t id, uint32_t low, uint32_t high)
{
    struct dpm_t *dpm = (struct dpm_t *)dpm_priv;
    uint32_t      mv  = fusbdev_tcpd_vbus_value(dpm->port_tcpd[id]->tcpd_device);
    return (mv > low && mv < high);
}

void dpm_vbus_disch(void *dpm_priv, uint32_t id, VBUS_DISCH_T disch)
{
    struct dpm_t *dpm = (struct dpm_t *)dpm_priv;
    fusbdev_tcpd_set_vbus_disch(dpm->port_tcpd[id]->tcpd_device, disch);
}
