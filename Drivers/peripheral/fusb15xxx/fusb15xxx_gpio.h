/**
 * @file     fusb15xxx_gpio.h
 * @author   USB PD Firmware Team
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
 * @addtogroup @{
 */
#ifndef __FUSB15XXX_GPIO_H__
#define __FUSB15XXX_GPIO_H__

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        HAL_GPIO1 = 0,
        HAL_GPIO2,
        HAL_GPIO3,
        HAL_GPIO4,
        HAL_GPIO5,
        HAL_GPIO6,
        HAL_GPIO7,
        HAL_GPIO8,
        HAL_GPIO9,
        HAL_GPIO10,
        HAL_GPIO11,
        HAL_GPIO12,
        HAL_GPIO13,
        HAL_GPIO14,
        HAL_GPIO15,
        HAL_GPIO16,
        HAL_GPIO17,
        HAL_GPIO18,
        HAL_GPIO19,
        HAL_GPIO20,
        NUM_GPIO_PINS = 20,
    } HAL_GPIOx_T;

    /**
 * Input/Output mode
 */
    typedef enum
    {
        HAL_GPIO_INPUT,  ///< GPIO Input
        HAL_GPIO_OUTPUT, ///< GPIO Output
    } HAL_GPIODIR_T;

    /**
 * Output Type
 */
    typedef enum
    {
        HAL_GPIO_PUSH_PULL,  ///< Push Pull output
        HAL_GPIO_OPEN_DRAIN, ///< Open Drain output
    } HAL_GPIOOD_T;

    typedef struct
    {
        HAL_GPIODIR_T iodir;
        HAL_GPIOOD_T  ppod;
    } HAL_GPIOCFG_T;

    /**
 * GPIO level
 */
    typedef enum
    {
        HAL_GPIO_LOW  = 0, ///< GPIO Low
        HAL_GPIO_HIGH = 1  ///< GPIO High
    } HAL_GPIOLVL_T;

    /**
 * GPIO interrupt level triggered or edge triggered
 */
    typedef enum
    {
        HAL_GPIO_IRQ_LEVEL = 0, ///< Level triggered
        HAL_GPIO_IRQ_EDGE  = 1  ///< Edge triggered
    } HAL_GPIO_IRQ_T;

    typedef enum
    {
        HAL_GPIO_IRQ_LOW_OR_FALLING = 0, ///< Falling or Low
        HAL_GPIO_IRQ_HIGH_OR_RISING = 1  ///< Rising or High
    } HAL_GPIO_IRQ_POL_T;

    /**
 * PORT GPIO Definition
 *
 */
    typedef enum
    {
        HAL_GPIO_A,
        HAL_GPIO_B,
        HAL_GPIO_SHARED
    } HAL_GPIO_PORT_T;

    typedef struct
    {
        void (*Configure)(HAL_GPIOx_T, HAL_GPIOCFG_T *, HAL_GPIO_PORT_T);
        void (*Set)(HAL_GPIOx_T, HAL_GPIOLVL_T, HAL_GPIO_PORT_T);
        HAL_GPIOLVL_T (*State)(HAL_GPIOx_T, HAL_GPIODIR_T, HAL_GPIO_PORT_T);
        void (*EnableInterrupt)(HAL_GPIOx_T, HAL_GPIO_IRQ_T, HAL_GPIO_IRQ_POL_T, HAL_GPIO_PORT_T);
        void (*InterruptDisable)(HAL_GPIOx_T, HAL_GPIO_PORT_T);
        bool (*InterruptStatus)(HAL_GPIOx_T, HAL_GPIO_PORT_T);
        void (*InterruptClear)(HAL_GPIOx_T, HAL_GPIO_PORT_T);
#if (DEVICE_TYPE == FUSB15101)
        void (*EnableInputBuffer)(HAL_GPIOx_T, bool);
#endif
    } const HAL_GPIO_DRIVER_T;

    extern HAL_GPIO_DRIVER_T GPIO_DRIVER;

#ifdef __cplusplus
}
#endif

#endif /** @} __FUSB15XXX_GPIO_H__ */
