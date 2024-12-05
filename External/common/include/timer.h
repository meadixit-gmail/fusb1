/*******************************************************************************
 * @file     timer.h
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
 ******************************************************************************/
/*
 * Defines a timer class for time tracking against a system clock
 */
#ifndef _TIMER_H_
#define _TIMER_H_

#include "platform.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
 *  Struct object to contain the timer related members
 */
    typedef struct ticktimer_t
    {
        unsigned long start_time; ///< Time-stamp when timer started
        unsigned long period;     ///< Timer period
    } TICKTIMER_T;

    typedef unsigned long (*tick_timer_cb)(void);

    /**
 * @brief Register a timer function
 * @param[in] scale the value (multiply)
 * @param[in] divide the value
 */
    void timer_init(tick_timer_cb, unsigned long, unsigned long);

    /**
 * @brief Start the timer using the argument in microseconds.
 * time must be greater than 0.
 */
    void timer_start(struct ticktimer_t *, unsigned long);

    /**
 * @brief Set time and period to zero to indicate no current period.
 */
    void timer_disable(struct ticktimer_t *);

    /**
 * @brief Set time and period to zero to indicate no current period.
 */
    bool timer_disabled(struct ticktimer_t *);

    /**
 * @brief Returns true when the time passed to Start is up.
 */
    bool timer_expired(struct ticktimer_t *);

    /*
 * @brief Returns the time remaining in milliseconds, or zero if disabled/done.
 */
    unsigned long timer_remaining(struct ticktimer_t *);

#ifdef __cplusplus
}
#endif

#endif /* _TIMER_H_ */
