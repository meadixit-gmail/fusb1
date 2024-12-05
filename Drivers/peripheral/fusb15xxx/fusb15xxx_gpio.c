/**
 * @file     fusb15xxx_gpio.c
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

#if HAL_USE_GPIO

#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201) || (DEVICE_TYPE == FUSB15101)
static void HAL_GPIO_PINx_Init(HAL_GPIOx_T pin, HAL_GPIOCFG_T *cfg, HAL_GPIO_PORT_T port)
{
    assert(pin < NUM_GPIO_PINS);
    uint32_t pinmask = 0x1U << pin;
    if (cfg->iodir == HAL_GPIO_INPUT)
    {
        GPIO_Interface->GPIO_DIR &= ~pinmask;
        GPIO_Interface->GPIO_IN_EN |= pinmask;
    }
    else if (cfg->iodir == HAL_GPIO_OUTPUT)
    {
        GPIO_Interface->GPIO_DIR |= pinmask;
        GPIO_Interface->GPIO_IN_EN &= ~pinmask;
    }

    if (cfg->ppod == HAL_GPIO_PUSH_PULL)
    {
        GPIO_Interface->GPIO_OD &= ~pinmask;
    }
    else if (cfg->ppod == HAL_GPIO_OPEN_DRAIN)
    {
        GPIO_Interface->GPIO_OD |= pinmask;
    }
}

#else
static void HAL_GPIO_PINx_Init(HAL_GPIOx_T pin, HAL_GPIOCFG_T *cfg, HAL_GPIO_PORT_T port)
{
    assert(pin < NUM_GPIO_PINS);
    uint32_t pinmask = 0x1U << pin;
    if (port == HAL_GPIO_A)
    {
        if (cfg->iodir == HAL_GPIO_INPUT)
        {
            GPIO_Interface->GPIO_DIR_PA &= ~pinmask;
            GPIO_Interface->GPIO_IN_EN_PA |= pinmask;
        }
        else
        {
            GPIO_Interface->GPIO_DIR_PA |= pinmask;
            GPIO_Interface->GPIO_IN_EN_PA &= ~pinmask;
        }
        if (cfg->ppod == HAL_GPIO_PUSH_PULL)
        {
            GPIO_Interface->GPIO_OD_PA &= ~pinmask;
        }
        else
        {
            GPIO_Interface->GPIO_OD_PA |= pinmask;
        }
    }
    else
    {
        if (cfg->iodir == HAL_GPIO_INPUT)
        {
            GPIO_Interface->GPIO_DIR_PB &= ~pinmask;
            GPIO_Interface->GPIO_IN_EN_PB |= pinmask;
        }
        else
        {
            GPIO_Interface->GPIO_DIR_PB |= pinmask;
            GPIO_Interface->GPIO_IN_EN_PB &= ~pinmask;
        }
        if (cfg->ppod == HAL_GPIO_PUSH_PULL)
        {
            GPIO_Interface->GPIO_OD_PB &= ~pinmask;
        }
        else
        {
            GPIO_Interface->GPIO_OD_PB |= pinmask;
        }
    }
}
#endif

#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201) || (DEVICE_TYPE == FUSB15101)
static HAL_GPIOLVL_T HAL_GPIO_PINx_Level(HAL_GPIOx_T pin, HAL_GPIODIR_T dir, HAL_GPIO_PORT_T port)
{
    assert(pin < NUM_GPIO_PINS);
    uint32_t mask = 0x1U << pin;
    if (dir == HAL_GPIO_INPUT)
    {
        return (GPIO_Interface->GPIO_IN & mask) ? HAL_GPIO_HIGH : HAL_GPIO_LOW;
    }
    else
    {
        return (GPIO_Interface->GPIO_OUT & mask) ? HAL_GPIO_HIGH : HAL_GPIO_LOW;
    }
}
#else
static HAL_GPIOLVL_T HAL_GPIO_PINx_Level(HAL_GPIOx_T pin, HAL_GPIODIR_T dir, HAL_GPIO_PORT_T port)
{
    assert(pin < NUM_GPIO_PINS);
    uint32_t mask = 0x1U << pin;
    if (dir == HAL_GPIO_INPUT)
    {
        if (port == HAL_GPIO_A)
        {
            return (GPIO_Interface->GPIO_IN_PA & mask) ? HAL_GPIO_HIGH : HAL_GPIO_LOW;
        }
        else
        {
            return (GPIO_Interface->GPIO_IN_PB & mask) ? HAL_GPIO_HIGH : HAL_GPIO_LOW;
        }
    }
    else
    {
        if (port == HAL_GPIO_A)
        {
            return (GPIO_Interface->GPIO_OUT_PA & mask) ? HAL_GPIO_HIGH : HAL_GPIO_LOW;
        }
        else
        {
            return (GPIO_Interface->GPIO_OUT_PB & mask) ? HAL_GPIO_HIGH : HAL_GPIO_LOW;
        }
    }
}
#endif

#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201) || (DEVICE_TYPE == FUSB15101)
static void HAL_GPIO_PINx_Set(HAL_GPIOx_T pin, HAL_GPIOLVL_T lvl, HAL_GPIO_PORT_T port)
{
    assert(pin < NUM_GPIO_PINS);
    uint32_t mask = 0x1 << pin;
    if (lvl == HAL_GPIO_HIGH)
    {
        GPIO_Interface->GPIO_OUT |= mask;
    }
    else
    {
        GPIO_Interface->GPIO_OUT &= ~mask;
    }
}

#else
static void HAL_GPIO_PINx_Set(HAL_GPIOx_T pin, HAL_GPIOLVL_T lvl, HAL_GPIO_PORT_T port)
{
    assert(pin < NUM_GPIO_PINS);
    uint32_t mask = 0x1 << pin;
    if (lvl == HAL_GPIO_HIGH)
    {
        if (port == HAL_GPIO_A)
        {
            GPIO_Interface->GPIO_OUT_PA |= mask;
        }
        else
        {
            GPIO_Interface->GPIO_OUT_PB |= mask;
        }
    }
    else
    {
        if (port == HAL_GPIO_A)
        {
            GPIO_Interface->GPIO_OUT_PA &= ~mask;
        }
        else
        {
            GPIO_Interface->GPIO_OUT_PB &= ~mask;
        }
    }
}
#endif

#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201) || (DEVICE_TYPE == FUSB15101)
static void HAL_GPIO_IRQ_Enable(HAL_GPIOx_T pin, HAL_GPIO_IRQ_T type, HAL_GPIO_IRQ_POL_T pol,
                                HAL_GPIO_PORT_T port)
{
    assert(pin < NUM_GPIO_PINS);
    uint32_t pinmask = 0x1U << pin;
    if (type == HAL_GPIO_IRQ_LEVEL)
    {
        GPIO_Interface->GPIO_INT_TYP &= ~pinmask;
    }
    else
    {
        GPIO_Interface->GPIO_INT_TYP |= pinmask;
    }

    if (pol == HAL_GPIO_IRQ_LOW_OR_FALLING)
    {
        GPIO_Interface->GPIO_INT_POL &= ~pinmask;
    }
    else
    {
        GPIO_Interface->GPIO_INT_POL |= pinmask;
    }

    GPIO_Interface->GPIO_INT_EN = pinmask;
}
#else
static void HAL_GPIO_IRQ_Enable(HAL_GPIOx_T pin, HAL_GPIO_IRQ_T type, HAL_GPIO_IRQ_POL_T pol,
                                HAL_GPIO_PORT_T port)
{
    assert(pin < NUM_GPIO_PINS);
    uint32_t pinmask = 0x1U << pin;
    if (port == HAL_GPIO_A)
    {
        if (type == HAL_GPIO_IRQ_LEVEL)
        {
            GPIO_Interface->GPIO_INT_TYP_PA &= ~pinmask;
        }
        else
        {
            GPIO_Interface->GPIO_INT_TYP_PA |= pinmask;
        }
        if (pol == HAL_GPIO_IRQ_LOW_OR_FALLING)
        {
            GPIO_Interface->GPIO_INT_POL_PA &= ~pinmask;
        }
        else
        {
            GPIO_Interface->GPIO_INT_POL_PA |= pinmask;
        }
        GPIO_Interface->GPIO_INT_EN_PA = pinmask;
    }
    else
    {
        if (type == HAL_GPIO_IRQ_LEVEL)
        {
            GPIO_Interface->GPIO_INT_TYP_PB &= ~pinmask;
        }
        else
        {
            GPIO_Interface->GPIO_INT_TYP_PB |= pinmask;
        }
        if (pol == HAL_GPIO_IRQ_LOW_OR_FALLING)
        {
            GPIO_Interface->GPIO_INT_POL_PB &= ~pinmask;
        }
        else
        {
            GPIO_Interface->GPIO_INT_POL_PB |= pinmask;
        }
        GPIO_Interface->GPIO_INT_EN_PB = pinmask;
    }
}
#endif

#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201) || (DEVICE_TYPE == FUSB15101)
static void HAL_GPIO_IRQ_Disable(HAL_GPIOx_T pin, HAL_GPIO_PORT_T port)
{
    assert(pin < NUM_GPIO_PINS);
    uint32_t pinmask = 0x1U << pin;
    GPIO_Interface->GPIO_INT_EN &= ~pinmask;
}
#else
static void HAL_GPIO_IRQ_Disable(HAL_GPIOx_T pin, HAL_GPIO_PORT_T port)
{
    assert(pin < NUM_GPIO_PINS);
    uint32_t pinmask = 0x1U << pin;
    if (port == HAL_GPIO_A)
    {
        GPIO_Interface->GPIO_INT_EN_PA &= ~pinmask;
    }
    else
    {
        GPIO_Interface->GPIO_INT_EN_PB &= ~pinmask;
    }
}
#endif

#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201) || (DEVICE_TYPE == FUSB15101)
bool HAL_GPIO_IRQ_Status(HAL_GPIOx_T pin, HAL_GPIO_PORT_T port)
{
    assert(pin < NUM_GPIO_PINS);
    uint32_t pinmask = 0x1U << pin;
    return (GPIO_Interface->GPIO_INT_STS & pinmask) ? true : false;
}
#else
bool HAL_GPIO_IRQ_Status(HAL_GPIOx_T pin, HAL_GPIO_PORT_T port)
{
    assert(pin < NUM_GPIO_PINS);
    uint32_t pinmask = 0x1U << pin;
    if (port == HAL_GPIO_A)
    {
        return (GPIO_Interface->GPIO_INT_STS_PA & pinmask) ? true : false;
    }
    else
    {
        return (GPIO_Interface->GPIO_INT_STS_PB & pinmask) ? true : false;
    }
}
#endif
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201) || (DEVICE_TYPE == FUSB15101)
void HAL_GPIO_IRQ_Clear(HAL_GPIOx_T pin, HAL_GPIO_PORT_T port)
{
    assert(pin < NUM_GPIO_PINS);
    uint32_t pinmask             = 0x1U << pin;
    GPIO_Interface->GPIO_INT_STS = pinmask;
}
#else
void HAL_GPIO_IRQ_Clear(HAL_GPIOx_T pin, HAL_GPIO_PORT_T port)
{
    assert(pin < NUM_GPIO_PINS);
    uint32_t pinmask = 0x1U << pin;
    if (port == HAL_GPIO_A)
    {
        GPIO_Interface->GPIO_INT_STS_PA = pinmask;
    }
    else
    {
        GPIO_Interface->GPIO_INT_STS_PB = pinmask;
    }
}
#endif

#if (DEVICE_TYPE == FUSB15101)
static void HAL_GPIO_EnableInputBuffer(HAL_GPIOx_T pin, bool en)
{
    assert(pin < NUM_GPIO_PINS);

    uint32_t pinmask = 0x1U << pin;
    if (en)
    {
        GPIO_Interface->GPIO_IN_EN |= pinmask;
    }
    else
    {
        GPIO_Interface->GPIO_IN_EN &= ~pinmask;
    }
}
#endif

HAL_GPIO_DRIVER_T GPIO_DRIVER = {.Configure        = HAL_GPIO_PINx_Init,
                                 .Set              = HAL_GPIO_PINx_Set,
                                 .State            = HAL_GPIO_PINx_Level,
                                 .EnableInterrupt  = HAL_GPIO_IRQ_Enable,
                                 .InterruptDisable = HAL_GPIO_IRQ_Disable,
                                 .InterruptStatus  = HAL_GPIO_IRQ_Status,
                                 .InterruptClear   = HAL_GPIO_IRQ_Clear,
#if (DEVICE_TYPE == FUSB15101)
                                 .EnableInputBuffer = HAL_GPIO_EnableInputBuffer
#endif
};
#endif /* HAL_USE_GPIO */
