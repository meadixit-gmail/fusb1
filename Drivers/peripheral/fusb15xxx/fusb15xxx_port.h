/**
 * @file     fusb15xxx_port.h
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
 * @addtogroup HAL
 * @{
 */
#ifndef __FUSB15XXX_PORT_H__
#define __FUSB15XXX_PORT_H__

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * PORT Definitions
 */
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
    typedef enum
    {
        PORT1 = 0,
        PORT2,
        PORT3,
        PORT4,
        PORT5,
        PORT6,
        PORT7,
        PORT8,
        PORT9,
        PORT10,
        PORT11,
        PORT12,
        PORT13,
        PORT14,
        PORT15,
        PORT16,
        PORT17,
        PORT18,
        PORT19,
        PORT20,
        NUM_PORT_PINS = 20,
    } HAL_PORTx_T;

#elif (DEVICE_TYPE == FUSB15201P)
typedef enum
{
    PORT0 = 0, /*GPIOB_0*/
    PORT1,     /*GPIOB_1*/
    PORT2,     /*GPIOB_2*/
    PORT3,     /*GPIOB_3*/
    PORT4,     /*GPIOB_4*/
    PORT5,     /*GPIOB_5*/
    PORT6,     /*GPIOB_6*/
    PORT7,     /*GPIOB_7*/
    NUM_PORT_PINS,
} HAL_PORTx_T;
#else
typedef enum
{
    PORT0 = 0,
    PORT1,
    PORT2,
    PORT3,
    PORT4,
    NUM_PORT_PINS,
} HAL_PORTx_T;
#endif

    /**
 * Select Digital/Analog input/output
 */
    typedef enum
    {
        PORT_DIGITAL, ///< Digtal IO
        PORT_ANALOG,  ///< Analog IO
    } HAL_PORT_AN_T;

    /**
 * Select primary or alternate function
 */
    typedef enum
    {
        PORT_PRIMARY   = 0, ///< Primary function
        PORT_ALTERNATE = 1, ///< Alternation function
        PORT_ENABLE_OVERRIDE =
            2, //SWD configuration overrides both primary and alternate function of the pin.
        PORT_DISABLE_OVERRIDE = 3,
    } HAL_PORT_ALT_T;

    /**
 * Pullup and pull down values
 */
    typedef enum
    {
        PORT_PULLUP_DISABLE = 0, ///< Disable Pullup
        PORT_PULLUP_ENABLE  = 1, ///< Enable Pullup
    } HAL_PULLUP_T;

    /**
 * Pulldown enable/disable
 */
    typedef enum
    {
        PORT_PULLDOWN_DISABLE = 0, ///< Disable Pull-down
        PORT_PULLDOWN_ENABLE  = 1, ///< Enable Pull-down
    } HAL_PULLDOWN_T;

    /**
 * Select Interrupt Polarity
 */
    typedef enum
    {
        PORT_INT_POLARITY_LOW = 0,  ///< Interrupt on low level
        PORT_INT_POLARITY_HIGH = 1, ///< Interrupt on high level
    } HAL_PORT_IRQ_T;

    /**
 * Port Configuration structure
 */
    typedef struct
    {
        HAL_PORT_AN_T  an;
        HAL_PORT_ALT_T alt;
        HAL_PULLUP_T   pu;
        HAL_PULLDOWN_T pd;
    } HAL_PORTCFG_T;

    /**
 * NMI configuration.
 * Example: HAL_NMI_BROWNOUT_STATUS_CHANGE | HAL_NMI_EXTERNAL_INT
 * sets NMI on Brownout status change and external interrupt.
 */
    typedef enum
    {
        HAL_NMI_DISABLED               = 0,
        HAL_NMI_BROWNOUT_STATUS_HIGH   = 1,
        HAL_NMI_BROWNOUT_STATUS_LOW    = 2,
        HAL_NMI_BROWNOUT_STATUS_CHANGE = 3,
        HAL_NMI_EXTERNAL_INT           = 4
    } HAL_NMI_CFG_T;

    /*
 * @brief Initialize port with configuration
 *
 * Example:
 * HAL_PORTCFG_T config = {0};
 * config.pin = PORT_PIN0 | PORT_PIN1;
 * config.anen = PORT_ANAEN_PA0_Disable | PORT_ANAEN_PA1_Disable;
 * config.config = PORT_CONFIG_PA0_Primary | PORT_CONFIG_PA10_Alternate;
 * config.puen = PORT_PULLUP_PA0_Enable;
 * config.pden = PORT_PULLDOWN_PA10_Enable;
 * config.inten = PORT_INTEN_PA0_Disable | PORT_INTEN_PA1_Enable;
 * PORT_Init(PORT, &config);
 * PORT_DeInit(PORT, PORT_PIN1 | PORT_PIN2);
 */

    typedef struct
    {
        void (*Configure)(HAL_PORTx_T, HAL_PORTCFG_T *, HAL_GPIO_PORT_T);
        void (*InterruptEnable)(HAL_PORTx_T, HAL_PORT_IRQ_T, HAL_GPIO_PORT_T);
        void (*InterruptDisable)(HAL_GPIO_PORT_T);
        bool (*InterruptStatus)(HAL_GPIO_PORT_T);
        void (*InterruptClear)(HAL_GPIO_PORT_T);
        void (*EnableNMI)(HAL_NMI_CFG_T, HAL_GPIO_PORT_T);
        void (*DisableNMI)(HAL_GPIO_PORT_T);
        bool (*SWDEnabled)();
    } const HAL_PORT_DRIVER_T;

    extern HAL_PORT_DRIVER_T PORT_DRIVER;

#ifdef __cplusplus
}
#endif
#endif /** @} __FUSB15XXX_PORT_H__ */
