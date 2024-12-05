/**
 * @file     fusb15xxx_cmu.h
 * @author   USB PD Firmware Team
 * @brief    Clock Control
 * @addtogroup HAL_Clock_Control
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
#ifndef __FUSB15XXX_CMU_H__
#define __FUSB15XXX_CMU_H__

#ifdef __cplusplus
extern "C"
{
#endif

//Commonly used macros for all projects
#define HAL_CMU_WATCHDOG_TIMER_CLOCK_ENABLE()                                                      \
    HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_WDG_EN_Msk)
#define HAL_CMU_TIMER_0_CLOCK_ENABLE()                                                             \
    HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_TIM0_EN_Msk)
#define HAL_CMU_WAKE_UP_TIMER_CLOCK_ENABLE()                                                       \
    HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_WUT_Msk)
#define HAL_CMU_SYSTICK_TIMER_CLOCK_ENABLE()                                                       \
    HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_SYSTICK_EN_Msk)
#define HAL_CMU_GPIO_CLOCK_ENABLE() HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_GPIO_EN_Msk)
#define HAL_CMU_ADC_CLOCK_ENABLE()  HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_ADC_EN_Msk)
#define HAL_CMU_I2C1_CLOCK_ENABLE() HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_I2C1_EN_Msk)

#define HAL_CMU_WATCHDOG_TIMER_CLOCK_DISABLE()                                                     \
    HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_WDG_EN_Msk)
#define HAL_CMU_TIMER_0_CLOCK_DISABLE()                                                            \
    HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_TIM0_EN_Msk)
#define HAL_CMU_WAKE_UP_TIMER_CLOCK_DISABLE()                                                      \
    HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_WUT_Msk)
#define HAL_CMU_SYSTICK_TIMER_CLOCK_DISABLE()                                                      \
    HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_SYSTICK_EN_Msk)
#define HAL_CMU_GPIO_CLOCK_DISABLE()                                                               \
    HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_GPIO_EN_Msk)
#define HAL_CMU_ADC_CLOCK_DISABLE() HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_ADC_EN_Msk)
#define HAL_CMU_I2C1_CLOCK_DISABLE()                                                               \
    HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_I2C1_EN_Msk)

#if (DEVICE_TYPE == FUSB15101)

#define HAL_CMU_DAC_CLOCK_ENABLE() HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_DAC_EN_Msk)
#define HAL_CMU_CC_CV_CLOCK_ENABLE()                                                               \
    HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_CC_CV_EN_Msk)
#define HAL_CMU_OTP_CLOCK_ENABLE() HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_OTP_EN_Msk)
#define HAL_CMU_VBUS_CTRL_CLOCK_ENABLE()                                                           \
    HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_VBUS_CTRL_EN_Msk)
#define HAL_CMU_USBPDA_LS_CLOCK_ENABLE()                                                           \
    HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_USBPDA_LSCLK_EN_Msk)
#define HAL_CMU_USBPDA_HS_CLOCK_ENABLE()                                                           \
    HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_USBPDA_HSCLK_EN_Msk)
#define HAL_CMU_UART_CLOCK_ENABLE() HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_UART_EN_Msk)
#define HAL_CMU_USBCHG_CLOCK_ENABLE()                                                              \
    HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_USBCHGA_CLK_EN_Msk)
#define HAL_CMU_USBCHGA_CLOCK_ENABLE()                                                             \
    HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_USBCHGA_CLK_EN_Msk)

#define HAL_CMU_DAC_CLOCK_DISABLE() HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_DAC_EN_Msk)
#define HAL_CMU_CC_CV_CLOCK_DISABLE()                                                              \
    HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_CC_CV_EN_Msk)
#define HAL_CMU_OTP_CLOCK_DISABLE() HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_OTP_EN_Msk)
#define HAL_CMU_VBUS_CTRL_CLOCK_DISABLE()                                                          \
    HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_VBUS_CTRL_EN_Msk)
#define HAL_CMU_USBPDA_LS_CLOCK_DISABLE()                                                          \
    HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_USBPDA_LSCLK_EN_Msk)
#define HAL_CMU_USBPDA_HS_CLOCK_DISABLE()                                                          \
    HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_USBPDA_HSCLK_EN_Msk)
#define HAL_CMU_UART_CLOCK_DISABLE()                                                               \
    HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_UART_EN_Msk)
#define HAL_CMU_USBCHG_CLOCK_DISABLE()                                                             \
    HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_USBCHGA_CLK_EN_Msk)
#define HAL_CMU_USBCHGA_CLOCK_DISABLE()                                                            \
    HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_USBCHGA_CLK_EN_Msk)

#endif

#if (DEVICE_TYPE == FUSB15201P)

#define HAL_CMU_DAC_CLOCK_ENABLE()  HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_DAC_EN_Msk)
#define HAL_CMU_I2C2_CLOCK_ENABLE() HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_I2C2_EN_Msk)
#define HAL_CMU_CC_CV_CLOCK_ENABLE()                                                               \
    HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_CC_CV_EN_Msk)
#define HAL_CMU_OTP_CLOCK_ENABLE() HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_OTP_EN_Msk)
#define HAL_CMU_VBUS_CTRL_CLOCK_ENABLE()                                                           \
    HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_VBUS_CTRL_EN_Msk)
#define HAL_CMU_USBPDA_LS_CLOCK_ENABLE()                                                           \
    HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_USBPDA_LSCLK_EN_Msk)
#define HAL_CMU_USBPDA_HS_CLOCK_ENABLE()                                                           \
    HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_USBPDA_HSCLK_EN_Msk)
#define HAL_CMU_USBPDB_LS_CLOCK_ENABLE()                                                           \
    HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_USBPDB_LSCLK_EN_Msk)
#define HAL_CMU_USBPDB_HS_CLOCK_ENABLE()                                                           \
    HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_USBPDB_HSCLK_EN_Msk)
#define HAL_CMU_UART_CLOCK_ENABLE() HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_UART_EN_Msk)
#define HAL_CMU_USBCHG_CLOCK_ENABLE()                                                              \
    HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_USBCHGA_CLK_EN_Msk)
#define HAL_CMU_USBCHGA_CLOCK_ENABLE()                                                             \
    HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_USBCHGA_CLK_EN_Msk)
#define HAL_CMU_USBCHGB_CLOCK_ENABLE()                                                             \
    HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_USBCHGB_CLK_EN_Msk)

#define HAL_CMU_DAC_CLOCK_DISABLE() HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_DAC_EN_Msk)
#define HAL_CMU_I2C2_CLOCK_DISABLE()                                                               \
    HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_I2C2_EN_Msk)
#define HAL_CMU_CC_CV_CLOCK_DISABLE()                                                              \
    HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_CC_CV_EN_Msk)
#define HAL_CMU_OTP_CLOCK_DISABLE() HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_OTP_EN_Msk)
#define HAL_CMU_VBUS_CTRL_CLOCK_DISABLE()                                                          \
    HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_VBUS_CTRL_EN_Msk)
#define HAL_CMU_USBPDA_LS_CLOCK_DISABLE()                                                          \
    HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_USBPDA_LSCLK_EN_Msk)
#define HAL_CMU_USBPDA_HS_CLOCK_DISABLE()                                                          \
    HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_USBPDA_HSCLK_EN_Msk)
#define HAL_CMU_UART_CLOCK_DISABLE()                                                               \
    HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_UART_EN_Msk)
#define HAL_CMU_USBPDB_LS_CLOCK_DISABLE()                                                          \
    HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_USBPDB_LSCLK_EN_Msk)
#define HAL_CMU_USBPDB_HS_CLOCK_DISABLE()                                                          \
    HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_USBPDB_HSCLK_EN_Msk)
#define HAL_CMU_USBCHG_CLOCK_DISABLE()                                                             \
    HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_USBCHGA_CLK_EN_Msk)
#define HAL_CMU_USBCHGA_CLOCK_DISABLE()                                                            \
    HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_USBCHGA_CLK_EN_Msk)
#define HAL_CMU_USBCHGB_CLOCK_DISABLE()                                                            \
    HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_USBCHGB_CLK_EN_Msk)
#endif

#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)

#define HAL_CMU_SYSCLK_TIMER_CLOCK_ENABLE()                                                        \
    HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_SYSCLK_EN_Msk)
#define HAL_CMU_TIMER_1_CLOCK_ENABLE()                                                             \
    HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_TIM1_EN_Msk)
#define HAL_CMU_I2C2_CLOCK_ENABLE() HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_I2C2_EN_Msk)
#define HAL_CMU_I2C3_CLOCK_ENABLE() HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_I2C3_EN_Msk)
#define HAL_CMU_I2C4_CLOCK_ENABLE() HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_I2C4_EN_Msk)
#define HAL_CMU_FLASH_CLOCK_ENABLE()                                                               \
    HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_FLASH_EN_Msk)
#define HAL_CMU_DMA_CLOCK_ENABLE() HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_DMA_EN_Msk)
#define HAL_CMU_USBPDA_LS_CLOCK_ENABLE()                                                           \
    HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_USBPDA_LSCLK_EN_Msk)
#define HAL_CMU_USBPDA_HS_CLOCK_ENABLE()                                                           \
    HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_USBPDA_HSCLK_EN_Msk)
#define HAL_CMU_USBPDB_LS_CLOCK_ENABLE()                                                           \
    HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_USBPDB_LSCLK_EN_Msk)
#define HAL_CMU_USBPDB_HS_CLOCK_ENABLE()                                                           \
    HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_USBPDB_HSCLK_EN_Msk)
#define HAL_CMU_USBCHGA_CLOCK_ENABLE()                                                             \
    HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_USBCHGA_CLK_EN_Msk)
#define HAL_CMU_USBCHGB_CLOCK_ENABLE()                                                             \
    HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_USBCHGB_CLK_EN_Msk)
#define HAL_CMU_HPDA_LS_CLOCK_ENABLE()                                                             \
    HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_HPDA_LSCLK_EN_Msk)
#define HAL_CMU_HPDB_LS_CLOCK_ENABLE()                                                             \
    HAL_CMU_PCLK_ENABLE(Clock_Management_Unit_PCLK_FEN_HPDB_LSCLK_EN_Msk)

#define HAL_CMU_SYSCLK_TIMER_CLOCK_DISABLE()                                                       \
    HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_SYSCLK_EN_Msk)
#define HAL_CMU_I2C2_CLOCK_DISABLE()                                                               \
    HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_I2C2_EN_Msk)
#define HAL_CMU_I2C3_CLOCK_DISABLE()                                                               \
    HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_I2C3_EN_Msk)
#define HAL_CMU_TIMER_1_CLOCK_DISABLE()                                                            \
    HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_TIM1_EN_Msk)
#define HAL_CMU_I2C4_CLOCK_DISABLE()                                                               \
    HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_I2C4_EN_Msk)
#define HAL_CMU_FLASH_CLOCK_DISABLE()                                                              \
    HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_FLASH_EN_Msk)
#define HAL_CMU_DMA_CLOCK_DISABLE() HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_DMA_EN_Msk)
#define HAL_CMU_USBPDA_LS_CLOCK_DISABLE()                                                          \
    HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_USBPDA_LSCLK_EN_Msk)
#define HAL_CMU_USBPDA_HS_CLOCK_DISABLE()                                                          \
    HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_USBPDA_HSCLK_EN_Msk)
#define HAL_CMU_USBPDB_LS_CLOCK_DISABLE()                                                          \
    HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_USBPDB_LSCLK_EN_Msk)
#define HAL_CMU_USBPDB_HS_CLOCK_DISABLE()                                                          \
    HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_USBPDB_HSCLK_EN_Msk)
#define HAL_CMU_USBCHGA_CLOCK_DISABLE()                                                            \
    HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_USBCHGA_CLK_EN_Msk)
#define HAL_CMU_USBCHGB_CLOCK_DISABLE()                                                            \
    HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_USBCHGB_CLK_EN_Msk)
#define HAL_CMU_HPDA_LS_CLOCK_DISABLE()                                                            \
    HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_HPDA_LSCLK_EN_Msk)
#define HAL_CMU_HPDB_LS_CLOCK_DISABLE()                                                            \
    HAL_CMU_PCLK_DISABLE(Clock_Management_Unit_PCLK_FEN_HPDB_LSCLK_EN_Msk)

#define HAL_HSOSC_ENABLE() HAL_CMU_HS_OSC_ENABLE(Clock_Management_Unit_CLK_CFG_HS_OSC_EN_Msk)
#define HAL_LSOSC_ENABLE() HAL_CMU_LS_OSC_ENABLE(Clock_Management_Unit_CLK_CFG_LS_OSC_EN_Msk)

#define HAL_HSOSC_DISABLE() HAL_CMU_HS_OSC_DISABLE(Clock_Management_Unit_CLK_CFG_HS_OSC_EN_Msk)
#define HAL_LSOSC_DISABLE() HAL_CMU_LS_OSC_DISABLE(Clock_Management_Unit_CLK_CFG_LS_OSC_EN_Msk)
#endif

/**
 * @brief       Enables clock sources for given clock on Peripheral Clock Register
 * @param[in]   mask        Determines which clock to enable
 */
#define HAL_CMU_PCLK_ENABLE(mask)                                                                  \
    do                                                                                             \
    {                                                                                              \
        Clock_Management_Unit->PCLK_FEN |= mask;                                                   \
    } while (0)

/**
* @brief       Disables clock sources for given clock on Peripheral Clock Register
* @param[in]   mask        Determines which clock to disable
*/
#define HAL_CMU_PCLK_DISABLE(mask)                                                                 \
    do                                                                                             \
    {                                                                                              \
        Clock_Management_Unit->PCLK_FEN &= ~mask;                                                  \
    } while (0)
/**
* @brief       Enables High Speed Oscillator
*/
#define HAL_CMU_HS_OSC_ENABLE()                                                                    \
    do                                                                                             \
    {                                                                                              \
        Clock_Management_Unit->CLK_CFG |= Clock_Management_Unit_CLK_CFG_HS_OSC_EN_Msk;             \
    } while (0)
/**
* @brief       Enables Low Speed Oscillator
*/
#define HAL_CMU_LS_OSC_ENABLE()                                                                    \
    do                                                                                             \
    {                                                                                              \
        Clock_Management_Unit->CLK_CFG |= Clock_Management_Unit_CLK_CFG_LS_OSC_EN_Msk;             \
    } while (0)
/**
* @brief       Disables High Speed Oscillator
*/
#define HAL_CMU_HS_OSC_DISABLE()                                                                   \
    do                                                                                             \
    {                                                                                              \
        Clock_Management_Unit->CLK_CFG &= ~Clock_Management_Unit_CLK_CFG_HS_OSC_EN_Msk;            \
    } while (0)
/**
* @brief       Disables Low Speed Oscillator
*/
#define HAL_CMU_LS_OSC_DISABLE()                                                                   \
    do                                                                                             \
    {                                                                                              \
        Clock_Management_Unit->CLK_CFG &= ~Clock_Management_Unit_CLK_CFG_LS_OSC_EN_Msk;            \
    } while (0)
/**
 * @brief       Configures System Clock
 * @param[in]   clk          Selects which clock to request the System Clock source (HSOSC or LPOSC)
 * @param[in]   prescale     Selects System Clock divide value
 */
#define HAL_CMU_SYCLK_CLOCK_CONFIGURE(clk, prescale)                                               \
    do                                                                                             \
    {                                                                                              \
        uint32_t mask = Clock_Management_Unit_CLK_CFG_SYSCLK_DIV_Msk                               \
                        | Clock_Management_Unit_CLK_CFG_CLK_SEL_Msk;                               \
        uint32_t value = (clk << Clock_Management_Unit_CLK_CFG_CLK_SEL_Pos)                        \
                         | (prescale << Clock_Management_Unit_CLK_CFG_SYSCLK_DIV_Pos);             \
        Clock_Management_Unit->CLK_CFG =                                                           \
            (Clock_Management_Unit->CLK_CFG & ~mask) | ((value)&mask);                             \
    } while (0)
/**
  * @brief       Configures Watchdog Clock
  * @param[in]   prescale     Selects Watchdog Clock (LPOSC or FCLK)
  */
#define HAL_CMU_WATCHDOG_CLOCK_CONFIGURE(prescale)                                                 \
    do                                                                                             \
    {                                                                                              \
        uint32_t mask  = Clock_Management_Unit_CLK_CFG_WDOG_CLK_SEL_Msk;                           \
        uint32_t value = prescale << Clock_Management_Unit_CLK_CFG_WDOG_CLK_SEL_Pos;               \
        Clock_Management_Unit->CLK_CFG =                                                           \
            (Clock_Management_Unit->CLK_CFG & ~mask) | ((value)&mask);                             \
    } while (0)
/**
  * @brief       Configures ADC Clock
  * @param[in]   clk          Selects ADC Clock source
  * @param[in]   prescale     Selects ADC Clock divide value
  */
#define HAL_CMU_ADC_CLOCK_CONFIGURE(clk, prescale)                                                 \
    do                                                                                             \
    {                                                                                              \
        uint32_t mask = Clock_Management_Unit_ADCCLK_CFG_ADC_CLK_SEL_Msk                           \
                        | Clock_Management_Unit_ADCCLK_CFG_ADC_DIV_Msk;                            \
        uint32_t value = (clk << Clock_Management_Unit_ADCCLK_CFG_ADC_CLK_SEL_Pos)                 \
                         | (prescale << Clock_Management_Unit_ADCCLK_CFG_ADC_DIV_Pos);             \
        Clock_Management_Unit->ADCCLK_CFG =                                                        \
            (Clock_Management_Unit->ADCCLK_CFG & ~mask) | (((value)) & mask);                      \
    } while (0)
/**
  * @brief       Configures Timer0 Clock
  * @param[in]   clk          Selects Timer Clock source
  * @param[in]   prescale     Selects Timer Clock divide value
  */
#define HAL_CMU_TIMER_0_CLOCK_CONFIGURE(clk, prescale)                                             \
    do                                                                                             \
    {                                                                                              \
        uint32_t mask = Clock_Management_Unit_TIMERS_CLK_CFG_GPT0_CLK_DIV_Msk                      \
                        | Clock_Management_Unit_TIMERS_CLK_CFG_GPT0_CLK_SRC_Msk;                   \
        uint32_t value = (clk << Clock_Management_Unit_TIMERS_CLK_CFG_GPT0_CLK_SRC_Pos)            \
                         | (prescale << Clock_Management_Unit_TIMERS_CLK_CFG_GPT0_CLK_DIV_Pos);    \
        Clock_Management_Unit->TIMERS_CLK_CFG =                                                    \
            (Clock_Management_Unit->TIMERS_CLK_CFG & ~mask) | ((value)&mask);                      \
    } while (0)
/**
   * @brief       Configures Timer0 Clock
   * @param[in]   clk          Selects Timer Clock source
   * @param[in]   prescale     Selects Timer Clock divide value
   */
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201) || (DEVICE_TYPE == FUSB15201P)
#define HAL_CMU_TIMER_1_CLOCK_CONFIGURE(clk, prescale)                                             \
    do                                                                                             \
    {                                                                                              \
        uint32_t mask = Clock_Management_Unit_TIMERS_CLK_CFG_GPT1_CLK_DIV_Msk                      \
                        | Clock_Management_Unit_TIMERS_CLK_CFG_GPT1_CLK_SRC_Msk;                   \
        uint32_t value = (clk << Clock_Management_Unit_TIMERS_CLK_CFG_GPT1_CLK_SRC_Pos)            \
                         | (prescale << Clock_Management_Unit_TIMERS_CLK_CFG_GPT1_CLK_DIV_Pos);    \
        Clock_Management_Unit->TIMERS_CLK_CFG =                                                    \
            (Clock_Management_Unit->TIMERS_CLK_CFG & ~mask) | ((value)&mask);                      \
    } while (0)
#endif
/**
   * @brief       Configures Wake Up Timer Clock
   * @param[in]   clk          Selects Wake Up Timer Clock source
   * @param[in]   prescale     Selects Wake Up Timer Clock divide value
   */
#define HAL_CMU_WAKE_UP_TIMER_CLOCK_CONFIGURE(clk, prescale)                                       \
    do                                                                                             \
    {                                                                                              \
        uint32_t mask = Clock_Management_Unit_TIMERS_CLK_CFG_WUT_CLK_DIV_Msk                       \
                        | Clock_Management_Unit_TIMERS_CLK_CFG_WUT_CLK_SRC_Msk;                    \
        uint32_t value = (clk << Clock_Management_Unit_TIMERS_CLK_CFG_WUT_CLK_SRC_Pos)             \
                         | (prescale << Clock_Management_Unit_TIMERS_CLK_CFG_WUT_CLK_DIV_Pos);     \
        Clock_Management_Unit->TIMERS_CLK_CFG =                                                    \
            (Clock_Management_Unit->TIMERS_CLK_CFG & ~mask) | ((value)&mask);                      \
    } while (0)

    /**
 * CMU Driver interface definition
 */
    typedef struct
    {
        void (*ClockSelect)(CMU_T *cmu, uint32_t);
    } const HAL_CMU_DRIVER_T;

    extern HAL_CMU_DRIVER_T CMU_DRIVER;

#ifdef __cplusplus
}
#endif
#endif /* __FUSB15XXX_CMU_H__ */
/** @} */
