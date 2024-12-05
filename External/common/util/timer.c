/*******************************************************************************
 * @file     timer.c
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
 * timer.c
 *
 * Implements a timer class for time tracking against a system clock and
 * also keeps track timer that are running on a min heap.
 */

#include "timer.h"
#include "logging.h"

struct
{
    tick_timer_cb fn_timer;
    unsigned int  scale;
    unsigned int  div;
} static tick_cfg;

/**
 * Register a timer function
 */
void timer_init(tick_timer_cb handle, unsigned long scale, unsigned long div)
{
    tick_cfg.fn_timer = handle;
    tick_cfg.scale    = 1;
    tick_cfg.div      = 1;
    if (scale > 0)
    {
        tick_cfg.scale = scale;
    }
    if (div > 0)
    {
        tick_cfg.div = div;
    }
}

void timer_start(struct ticktimer_t *obj, unsigned long time)
{
    /* Grab the current time stamp and store the wait period. */
    /* Time must be > 0 */
    obj->start_time = tick_cfg.fn_timer();
    obj->period     = (time * tick_cfg.scale) / tick_cfg.div;
}

void timer_disable(struct ticktimer_t *obj)
{
    /* Zero means disabled */
    obj->start_time = obj->period = 0;
}

bool timer_disabled(struct ticktimer_t *obj)
{
    return (obj->start_time == 0) && (obj->period == 0) ? true : false;
}

bool timer_expired(struct ticktimer_t *obj)
{
    unsigned long time  = tick_cfg.fn_timer();
    unsigned long delta = time - obj->start_time;
    return delta >= obj->period;
}

unsigned long timer_remaining(struct ticktimer_t *obj)
{
    unsigned long time = tick_cfg.fn_timer();
    if (!timer_expired(obj))
    {
        time = (obj->start_time + obj->period) - time;
    }
    else
    {
        time = 0;
    }
    return time;
}
