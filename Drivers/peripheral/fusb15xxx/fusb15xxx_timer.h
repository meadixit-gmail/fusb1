/**
 * @file     fusb15xxx_timer.h
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
#ifndef __FUSB15XXX_TIMER_H__
#define __FUSB15XXX_TIMER_H__

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        void (*Configure)(TIM_T *, uint32_t prescale);
        void (*Start)(TIM_T *, uint32_t count, bool periodic);
        void (*Stop)(TIM_T *);
        void (*InterruptEnable)(TIM_T *, bool);
        void (*InterruptClear)(TIM_T *);
        bool (*InterruptStatus)(TIM_T *);
        void (*Load)(TIM_T *, uint32_t, bool);
        uint32_t (*Value)(TIM_T *d);
    } const HAL_TIMER_DRIVER_T;

    /**
 * @brief IRQ Handler for timer
 * @param timer
 */
    void HAL_TIM_IRQHandler(TIM_T *);

#if STACK_MON
 /**
 * @brief Stack Monitoring
 * @param NA
 */
    void HAL_STK_Mon();
#endif

    extern HAL_TIMER_DRIVER_T TIMER_DRIVER;

#ifdef __cplusplus
}
#endif
#endif /** @} __FUSB15XXX_TIMER_H__ */
