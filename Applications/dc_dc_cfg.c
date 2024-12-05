/*******************************************************************************
 * @file   dc_dc_cfg.c
 * @author USB Firmware Team
 * @brief  Implements the Device Policy Manager state machine functions.
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
*******************************************************************************/
#include "dc_dc_board.h"

#if CONFIG_ENABLED(DCDC)

struct dc_dc_master DCDC_master[I2C_PORT_COUNT] = {
    {
#if (DEVICE_TYPE == FUSB15201) && CONFIG_ENABLED(DCDC) && defined(I2C3_BOARD)
        .i2cm_id = I2C_iface_3,
#else
        .i2cm_id     = I2C_iface_1,
#endif
        .prescalar = I2C_Port_I2C_CFG_MASTER_PRESCALE_48,
    },
    {
        0,
        0,
    },
#if (DEVICE_TYPE != FUSB15201P) && (DEVICE_TYPE != FUSB15201)
    {
        0,
        0,
    },
    {
        0,
        0,
    },
#endif
};

struct dc_dc_data DCDC_data[TYPE_C_PORT_COUNT] = {
    {
#if (DEVICE_TYPE == FUSB15201) && CONFIG_ENABLED(DCDC) && defined(I2C3_BOARD)
        .i2c_m = I2C_iface_3,
#else
        .i2c_m       = I2C_iface_1,
#endif
        .slave_address = VBUS_ADDR_PORTA,
        .pot_address   = POT_ADDR_PORTA,
#if (DEVICE_TYPE == FUSB15200)
		.enable_gpio = HAL_GPIO16,
		.port        = HAL_GPIO_SHARED,
#elif (DEVICE_TYPE == FUSB15201)
        .enable_gpio = HAL_GPIO7,
        .port        = HAL_GPIO_SHARED,
#elif (DEVICE_TYPE == FUSB15201P)
        .enable_gpio = HAL_GPIO3,
        .port        = HAL_GPIO_B,
#endif
    },
#if (DEVICE_TYPE != FUSB15101)
    {
#if (DEVICE_TYPE == FUSB15201) && CONFIG_ENABLED(DCDC) && defined(I2C3_BOARD)
        .i2c_m = I2C_iface_3,
#else
        .i2c_m       = I2C_iface_1,
#endif
        .slave_address = VBUS_ADDR_PORTB,
        .pot_address   = POT_ADDR_PORTB,
#if (DEVICE_TYPE == FUSB15200)
		.enable_gpio = HAL_GPIO13,
		.port        = HAL_GPIO_SHARED,
#elif (DEVICE_TYPE == FUSB15201)
        .enable_gpio = HAL_GPIO2,
        .port        = HAL_GPIO_SHARED,
#elif (DEVICE_TYPE == FUSB15201P)
        .enable_gpio = HAL_GPIO2,
        .port        = HAL_GPIO_B,
#endif
    },
#endif
};

#endif //CONFIG_ENABLED(DCDC)
