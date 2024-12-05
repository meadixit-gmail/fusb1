/**
 * @file     main.c
 * @author   USB PD Firmware Team
 * @brief
 * @addtogroup App
 * @mainpage
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
 *
 * @tableofcontents
 *
 * @{
 */
#include "legacy_types.h"
#include "pd_types.h"
#include "fusbdev_hal.h"
#include "timer.h"
#include "FUSB15xxx.h"
#include "dev_tcpd.h"
#include "dpm.h"

#if ((CONFIG_ENABLED(ALT_BATT_NTC)))
#define DEFERRED_SLEEP_TIME_MS 5000
#else
#define DEFERRED_SLEEP_TIME_MS 500
#endif

/**
 * DPM forward declaration
 */
extern struct dpm_t DPM;
extern void         dpm_device_init(void);
extern void         dpm_init();
extern void         dpm_reinit();
extern void         dpm_exec();
unsigned long       dpm_next_wakeup();

/* linker symbol */
extern uint32_t __etext;
extern uint32_t __data_start__;
extern uint32_t __data_end__;
extern uint32_t __bss_start__;
extern uint32_t __bss_end__;

/* Set by interrupt wake up */
volatile bool gSleepInt;
volatile bool gUnattached;
#if defined(__GNUC__)
#define __NO_INLINE       __attribute__((noinline))
#define __SECTION_RAMCODE __attribute__((section(".RAMCODE")))
#else
#define __NO_INLINE
#define __SECTION_RAMCODE
#endif

#if CONFIG_LOG
#include "SEGGER_RTT.h"
int dbg_printf(const char *msg, ...)
{
    int     r;
    va_list paramlist;
    va_start(paramlist, msg);
    r = SEGGER_RTT_printf(0, "%08u:", HAL_SYSTICK_Count());
    if (r >= 0)
    {
        r = SEGGER_RTT_vprintf(0, msg, &paramlist);
    }
    va_end(paramlist);
    return r;
}
#endif

/**
 * @brief override the default implementation
 */
void HAL_Delay(unsigned long ticks)
{
    unsigned long start = HAL_SYSTICK_Count();
    while (start - TIMER_DRIVER.Value(Timer_0) < ticks)
    {
    }
}

unsigned long HAL_SYSTICK_Count()
{
    /* Free running countdown timer */
    return (~0UL - TIMER_DRIVER.Value(Timer_0));
}
void clear_sram()
{
	uint32_t *pDest;

	/* Copy data section  */
	pDest = &__data_start__;
	for (; pDest < &__data_end__;)
	{
		*pDest++ = 0UL;
	}

	/* Zero-init bss */
	pDest = &__bss_start__;
	for (; pDest < &__bss_end__;)
	{
		*pDest++ = 0UL;
	}
}
void reinit_sram()
{
	uint32_t *pSrc, *pDest;

	/* Copy data section  */
	pSrc  = &__etext;
	pDest = &__data_start__;
	for (; pDest < &__data_end__;)
	{
		*pDest++ = *pSrc++;
	}

	/* Zero-init bss */
	pDest = &__bss_start__;
	for (; pDest < &__bss_end__;)
	{
		*pDest++ = 0UL;
	}
}
#if (CONFIG_SLEEP)
/**
 * @brief The routine below witches to the ls_osc and goes to sleep if there are
 * no events. It restores the system clocks after waking up.
 */
#if (DEVICE_TYPE == FUSB15201) || (DEVICE_TYPE == FUSB15201P)
#pragma GCC push_options
#pragma GCC optimize("O0")
#endif
static __NO_INLINE void deep_sleep()
{
#if (DEVICE_TYPE == FUSB15200)
    __disable_irq();
    if (!gSleepInt)
    {
        fusbdev_adc_disable();
        fusbdev_cmu_clock_select(Clock_Management_Unit_Select_LS_Clock_Osc);
        fusbdev_adc_enable();
        __WFI();
        gSleepInt = true;

        fusbdev_adc_disable();
        fusbdev_cmu_clock_select(Clock_Management_Unit_Select_HS_Clock_Osc);
        fusbdev_adc_enable();
    }
    __enable_irq();
#elif (DEVICE_TYPE == FUSB15201P) || (DEVICE_TYPE == FUSB15201)

    __disable_irq();
    if (!gSleepInt)
    {
#if (!CONFIG_ENABLED(NO_SRAM_ON_SLEEP))
        uint32_t adc_ctrl = ADC_Interface->ADC_CTRL & ~(ADC_Interface_ADC_CTRL_adc_en_Msk |
        		            ADC_Interface_ADC_CTRL_adc_scan_Msk);
#endif
        if (gUnattached)
        {
#if (CONFIG_ENABLED(NO_SRAM_ON_SLEEP))
        	/* Only for testing */
        	//clear_sram();
#endif
#if !CONFIG_ENABLED(CUSTOM_APPLICATION)
        	ADC_Interface->ADC_CH_EN = 0U;
            ADC_Interface->ADC_CTRL &= ~(ADC_Interface_ADC_CTRL_adc_en_Msk | ADC_Interface_ADC_CTRL_adc_scan_Msk |
                                         ADC_Interface_ADC_CTRL_adc_loop_Msk);
            ADC_Interface->ADC_INT |= ADC_Interface_ADC_INT_I_ADC_Msk;
#else
            ADC_Interface->ADC_CTRL &= ~ADC_Interface_ADC_CTRL_adc_en_Msk;
#endif
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            Clock_Management_Unit->CLK_CFG |= Clock_Management_Unit_CLK_CFG_CLK_SEL_Msk;
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            Clock_Management_Unit->CLK_CFG &= ~Clock_Management_Unit_CLK_CFG_HS_OSC_EN_Msk;
#if (CONFIG_ENABLED(CUSTOM_APPLICATION))
            __NOP();
			__NOP();
			__NOP();
			__NOP();
            fusbdev_adc_enable();
#endif
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __WFI();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            Clock_Management_Unit->CLK_CFG |= Clock_Management_Unit_CLK_CFG_HS_OSC_EN_Msk;
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            Clock_Management_Unit->CLK_CFG &= ~Clock_Management_Unit_CLK_CFG_CLK_SEL_Msk;
            __NOP();
			__NOP();
			__NOP();
			__NOP();
#if (CONFIG_ENABLED(NO_SRAM_ON_SLEEP))
			reinit_sram();
			dpm_reinit();
#endif
            gSleepInt = true;
#if (CONFIG_ENABLED(CUSTOM_APPLICATION))
            __NOP();
			__NOP();
			__NOP();
			__NOP();
            fusbdev_adc_enable();
#elif (!CONFIG_ENABLED(NO_SRAM_ON_SLEEP))
            ADC_Interface->ADC_CTRL |= adc_ctrl;
#endif
        }
        else
        {
            /*Don't clock switch if any port is not unattached*/
#if (!CONFIG_ENABLED(CUSTOM_APPLICATION))
            //don't sleep if attached w/ PDPS (need to respond faster to pass compliance)
            __WFI();
#endif
            gSleepInt = true;
        }
    }
    __enable_irq();
#endif
}

static void schedule_wut(void)
{
    unsigned int r = dpm_next_wakeup();

    if (r == 0)
    {
        /* Force the state machine to run immediately */
        gSleepInt = true;
    }
    else if (r != ~0)
    {
        /* Start Wake-up timer */
        fusbdev_timer_interrupt_enable(TImer_Device_Type_WakeUpTimer, true);
        fusbdev_timer_start(TImer_Device_Type_WakeUpTimer, r, false);
    }
}

static void deferred_sleep(TICKTIMER_T *timer)
{
    if (timer_disabled(timer))
    {
        timer_start(timer, DEFERRED_SLEEP_TIME_MS);
        schedule_wut();
    }
    else if (timer_expired(timer))
    {
        timer_disable(timer);
        deep_sleep();
    }
}

#endif
int main(void)
{
#if CONFIG_ENABLED(SLEEP)
    TICKTIMER_T sleep_defer_timer;
#endif
    dpm_device_init();
    dpm_init();
    gSleepInt = true;

    while (1)
    {
        /* Port A Processing */
        if (gSleepInt)
        {
            gSleepInt = false;
#if CONFIG_ENABLED(SLEEP)
            timer_disable(&sleep_defer_timer);
            fusbdev_timer_stop(TImer_Device_Type_WakeUpTimer);
#endif
            dpm_exec();
        }
        else
        {
#if CONFIG_ENABLED(SLEEP)
            bool sending_message = false;
            for (uint32_t i = 0; i < TYPE_C_PORT_COUNT; i++)
            {
                sending_message |= port_sending_message(DPM.port_tcpd[i]);
            }
            if (!sending_message)
            {
                gUnattached = true;
                for (uint32_t i = 0; i < TYPE_C_PORT_COUNT; i++)
                {
                    gUnattached &= ((port_tc_state(DPM.port_tcpd[i]) == Unattached)
                                    || !port_tc_enabled(DPM.port_tcpd[i]));
                }
#if (DEVICE_TYPE == FUSB15201P) || (DEVICE_TYPE == FUSB15201)
                // try disabling deep sleep mask
                if (gUnattached)
                {
                    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
                }
                else
                {
                    SCB->SCR &= ~SCB_SCR_SLEEPDEEP_Msk;
                }
#endif
                deferred_sleep(&sleep_defer_timer);
            }
            else
            {
                gSleepInt = true;
            }
#else
            gSleepInt = true;
#endif
        }
    }

    return 0;
}
