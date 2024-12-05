/**
 * @file     fusb15xxx_port.c
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

#if HAL_USE_PORT

#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201) || (DEVICE_TYPE == FUSB15101)
static void HAL_PORT_Init(HAL_PORTx_T pin, HAL_PORTCFG_T *cfg, HAL_GPIO_PORT_T port)
{
    assert(pin < NUM_PORT_PINS);

    uint32_t pinmask = 0x1U << pin;
    if (cfg->alt == PORT_ENABLE_OVERRIDE)
    {
        PORT_Interface->PORT_CFG |= PORT_Interface_PORT_CFG_SWD_EN_Msk;
    }
    else if (cfg->alt == PORT_DISABLE_OVERRIDE)
    {
        PORT_Interface->PORT_CFG &= ~PORT_Interface_PORT_CFG_SWD_EN_Msk;
    }
    else
    {
        if (cfg->an == PORT_ANALOG)
        {
            PORT_Interface->ANA_EN |= pinmask;
        }
        else
        {
            PORT_Interface->ANA_EN &= ~pinmask;
        }

        if (cfg->alt == PORT_ALTERNATE)
        {
            PORT_Interface->PORT_CFG |= pinmask;
        }
        else
        {
            PORT_Interface->PORT_CFG &= ~pinmask;
        }

        if (cfg->pu == PORT_PULLUP_ENABLE)
        {
            PORT_Interface->PULL_UP |= pinmask;
        }
        else
        {
            PORT_Interface->PULL_UP &= ~pinmask;
        }

        if (cfg->pd == PORT_PULLDOWN_ENABLE)
        {
            PORT_Interface->PULL_DOWN |= pinmask;
        }
        else
        {
            PORT_Interface->PULL_DOWN &= ~pinmask;
        }
    }
}
#else
static void HAL_PORT_Init(HAL_PORTx_T pin, HAL_PORTCFG_T *cfg, HAL_GPIO_PORT_T port)
{
    assert(pin < NUM_PORT_PINS);

    uint32_t pinmask = 0x1U << pin;

    /*different port has different port configuration register*/
    if (port == HAL_GPIO_A)
    {
        if (cfg->alt == PORT_ALTERNATE)
        {
            PORT_Interface->PORT_CFGA |= pinmask;
        }
        else
        {
            PORT_Interface->PORT_CFGA &= ~pinmask;
        }

        if (cfg->pu == PORT_PULLUP_ENABLE)
        {
            PORT_Interface->PULL_UP_PA |= pinmask;
        }
        else
        {
            PORT_Interface->PULL_UP_PA &= ~pinmask;
        }

        if (cfg->pd == PORT_PULLDOWN_ENABLE)
        {
            PORT_Interface->PULL_DOWN_PA |= pinmask;
        }
        else
        {
            PORT_Interface->PULL_DOWN_PA &= ~pinmask;
        }
    }
    else
    {
        /*configure port for digital or analog operation*/
        if (cfg->an == PORT_ANALOG)
        {
            PORT_Interface->ANA_EN |= pinmask;
        }
        else
        {
            PORT_Interface->ANA_EN &= ~pinmask;
        }
        if (cfg->alt == PORT_ENABLE_OVERRIDE)
        {
            PORT_Interface->PORT_CFGB |= PORT_Interface_PORT_CFGB_SWD_EN_Msk;
        }
        else if (cfg->alt == PORT_DISABLE_OVERRIDE)
        {
            PORT_Interface->PORT_CFGB &= ~PORT_Interface_PORT_CFGB_SWD_EN_Msk;
        }
        else if (cfg->alt == PORT_ALTERNATE)
        {
            PORT_Interface->PORT_CFGB |= pinmask;
        }
        else
        {
            PORT_Interface->PORT_CFGB &= ~pinmask;
        }

        if (cfg->pu == PORT_PULLUP_ENABLE)
        {
            PORT_Interface->PULL_UP_PB |= pinmask;
        }
        else
        {
            PORT_Interface->PULL_UP_PB &= ~pinmask;
        }

        if (cfg->pd == PORT_PULLDOWN_ENABLE)
        {
            PORT_Interface->PULL_DOWN_PB |= pinmask;
        }
        else
        {
            PORT_Interface->PULL_DOWN_PB &= ~pinmask;
        }
    }
}
#endif

static void HAL_PORT_IRQ_Enable(HAL_PORTx_T pin, HAL_PORT_IRQ_T pol, HAL_GPIO_PORT_T port)
{
    uint32_t val = 0;
    if (pol == PORT_INT_POLARITY_HIGH)
    {
        val = PORT_Interface_EXT_INT_SEL_EXT_INT_POL_Msk;
    }
#if (DEVICE_TYPE == FUSB15101)
    if (pin == PORT0)
    {
        val |= PORT_Interface_EXT_INT_SEL_EXT_INT_SEL_Msk;
    }
#elif (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
    if (pin == PORT13)
    {
        val |= PORT_Interface_External_Interrupt_Select_GPIO13;
    }
    else if (pin == PORT7)
    {
        val |= PORT_Interface_External_Interrupt_Select_GPIO7;
    }
    else if (pin == PORT15)
    {
        val |= PORT_Interface_External_Interrupt_Select_GPIO15;
    }
    else if (pin == PORT17)
    {
        val |= PORT_Interface_External_Interrupt_Select_GPIO17;
    }
    else
    {
        return;
    }
#else
    if (pin == PORT0)
    {
        val |= PORT_Interface_EXT_INT_SEL_EXT_INT_SEL_Msk;
    }
    else
    {
        return;
    }
#endif

    PORT_Interface->EXT_INT_SEL = val;
    PORT_Interface->EXT_INT_EN  = PORT_Interface_EXT_INT_EN_EXT_INT_EN_Msk;
}

static void HAL_PORT_IRQ_Disable(HAL_GPIO_PORT_T port)
{
    PORT_Interface->EXT_INT_EN = 0;
}

static void HAL_PORT_IRQ_Clear(HAL_GPIO_PORT_T port)
{
    PORT_Interface->EXT_INT_STS = PORT_Interface_EXT_INT_STS_EXT_INT_STS_Msk;
}

static bool HAL_PORT_IRQ_Status(HAL_GPIO_PORT_T port)
{
    return (PORT_Interface->EXT_INT_STS & 0x1) ? true : false;
}

static void HAL_PORT_NMI_Enable(HAL_NMI_CFG_T nmi, HAL_GPIO_PORT_T port)
{
    PORT_Interface->NMI_CFG = nmi;
}

static void HAL_PORT_NMI_Disable(HAL_GPIO_PORT_T port)
{
    PORT_Interface->NMI_CFG = 0;
}

static bool HAL_PORT_SWDEnabled()
{
#if (DEVICE_TYPE == FUSB15101 ||DEVICE_TYPE == FUSB15200 || DEVICE_TYPE == FUSB15201)
	return (PORT_Interface->PORT_CFG & PORT_Interface_PORT_CFG_SWD_EN_Msk);
#else
	return (PORT_Interface->PORT_CFGB & PORT_Interface_PORT_CFGB_SWD_EN_Msk);
#endif
}

HAL_PORT_DRIVER_T PORT_DRIVER = {
    .Configure        = HAL_PORT_Init,
    .InterruptEnable  = HAL_PORT_IRQ_Enable,
    .InterruptDisable = HAL_PORT_IRQ_Disable,
    .InterruptStatus  = HAL_PORT_IRQ_Status,
    .InterruptClear   = HAL_PORT_IRQ_Clear,
    .EnableNMI        = HAL_PORT_NMI_Enable,
    .DisableNMI       = HAL_PORT_NMI_Disable,
	.SWDEnabled       = HAL_PORT_SWDEnabled,
};
#endif /* HAL_USE_PORT */
