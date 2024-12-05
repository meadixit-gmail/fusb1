/**
 * @file     fusb15xxx_timer.c
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


#ifdef HAL_USE_TIMER

#if STACK_MON
uint32_t __attribute__((section (".StackMonSection"))) __Stack_Mon;
#endif

static void HAL_TIM_Init(TIM_T *d, uint32_t prescale)
{
    uint32_t ctrl = (prescale << Timer_0_GPT0_TIMER1CONTROL_timer_prescale_Pos)
                    & Timer_0_GPT0_TIMER1CONTROL_timer_prescale_Msk;
    d->GPT_TIMERCONTROL = ctrl;
}

static void HAL_TIM_InterruptEnable(TIM_T *d, bool en)
{
    if (en)
    {
        d->GPT_TIMERCONTROL |= Timer_0_GPT0_TIMER1CONTROL_timer_int_en_Msk;
    }
    else
    {
        d->GPT_TIMERCONTROL &= ~Timer_0_GPT0_TIMER1CONTROL_timer_int_en_Msk;
    }
}

static void HAL_TIM_Start(TIM_T *d, uint32_t count, bool periodic)
{
    uint32_t ctrl;
    d->GPT_TIMERCONTROL &= ~Timer_0_GPT0_TIMER1CONTROL_timer_enable_Msk;
    ctrl             = d->GPT_TIMERCONTROL;
    d->GPT_TIMERLOAD = count;
    if ((count) & (0xffffU << 16))
    {
        /* 32 bit */
        ctrl |= Timer_0_GPT0_TIMER1CONTROL_timer_size_Msk;
    }
    else
    {
        /* 16-bit */
        ctrl &= ~Timer_0_GPT0_TIMER1CONTROL_timer_size_Msk;
    }
    if (periodic)
    {
        ctrl &= ~Timer_0_GPT0_TIMER1CONTROL_one_shot_cnt_Msk;
        if (ctrl & Timer_0_GPT0_TIMER1CONTROL_timer_int_en_Msk)
        {
            /* Interrupt is periodically generated */
            ctrl |= Timer_0_GPT0_TIMER1CONTROL_timer_mode_Msk;
        }
        else
        {
            /* Free running timer with no interrupt */
            ctrl &= ~Timer_0_GPT0_TIMER1CONTROL_timer_mode_Msk;
        }
    }
    else
    {
        /* single shot */
        ctrl |= Timer_0_GPT0_TIMER1CONTROL_one_shot_cnt_Msk;
        ctrl &= ~Timer_0_GPT0_TIMER1CONTROL_timer_mode_Msk;
    }
    ctrl |= Timer_0_GPT0_TIMER1CONTROL_timer_enable_Msk;
    d->GPT_TIMERCONTROL = ctrl;
}

static void HAL_TIM_IntClr(TIM_T *d)
{
    d->GPT_TIMERINTCLR = Timer_0_GPT0_TIMER1INTCLR_timewr1_int_clr_Msk;
}

static bool HAL_TIM_IntStatus(TIM_T *d)
{
    return d->GPT_TIMERMIS == 0 ? false : true;
}

static void HAL_TIM_Load(TIM_T *d, uint32_t count, bool wait)
{
    if (wait)
    {
        d->GPT_TIMERBGLOAD = count;
    }
    else
    {
        d->GPT_TIMERLOAD = count;
    }
}

static uint32_t HAL_TIM_Value(TIM_T *d)
{
    return d->GPT_TIMERVALUE;
}

static void HAL_TIM_Stop(TIM_T *d)
{
    d->GPT_TIMERCONTROL &= ~Timer_0_GPT0_TIMER1CONTROL_timer_enable_Msk;
}

void HAL_TIM_IRQHandler(TIM_T *d)
{
    if (d->GPT_TIMERCONTROL & Timer_0_GPT0_TIMER1CONTROL_one_shot_cnt_Msk)
    {
        d->GPT_TIMERCONTROL &= ~Timer_0_GPT0_TIMER1CONTROL_timer_enable_Msk;
    }
    d->GPT_TIMERINTCLR = Timer_0_GPT0_TIMER1INTCLR_timewr1_int_clr_Msk;
}

#if STACK_MON
void HAL_STK_Mon()
{
    uint32_t sp;

    __asm volatile("MRS %0, MSP" : "=r"(sp));

    if (__Stack_Mon == 0) {
        __Stack_Mon = sp;
    }else if (__Stack_Mon > sp) {
        __Stack_Mon = sp;
    }
}
#endif

HAL_TIMER_DRIVER_T TIMER_DRIVER = {.Configure       = HAL_TIM_Init,
                                   .Start           = HAL_TIM_Start,
                                   .Stop            = HAL_TIM_Stop,
                                   .InterruptEnable = HAL_TIM_InterruptEnable,
                                   .InterruptClear  = HAL_TIM_IntClr,
                                   .InterruptStatus = HAL_TIM_IntStatus,
                                   .Load            = HAL_TIM_Load,
                                   .Value           = HAL_TIM_Value};
#endif /* HAL_USE_TIMER */
