/*******************************************************************************
 * @file     port_bc1p2.h
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
 ******************************************************************************/
#ifndef __PORT_BC1P2_H__
#define __PORT_BC1P2_H__

#include "timer.h"
#include "pd_types.h"
#include "legacy_types.h"

/**
 * BC1.2 object
 */
struct legacy_state
{
    struct port_bc1p2 *dev;
    int                event;   /** event flag keeps state machine active */
    int                enabled; /** State machine is enabled */
#if CONFIG_ENABLED(BC1P2_DCP) && CONFIG_ENABLED(BC1P2_CSM) && CONFIG_ENABLED(BC1P2_CDP)
    enum bc1p2_mode    mode;
#endif
    enum bc1p2_state   state;
    int                stateIdx;
    struct ticktimer_t timers[BC1P2_TIMER_COUNT];
    struct fusb_bc1p2_device *bc1p2_device;
    void (*smcb)(struct legacy_state *); //TODO
};

struct port_bc1p2 *dpm_legacy_init(void *dpm_info, int port_id);
void               port_bc1p2_init(void *bc1p2_priv, enum bc1p2_mode m);
/**
 * @brief Gets the current BC1.2 state
 * @param BC1.2 object
 * @return
 */
enum bc1p2_state bc_sm_state(void *);

/**
 * @brief Runs the BC1.2 state machine
 * @param BC1.2 object
 */
void bc_sm(void *);

/**
 * @brief Executes statemachine until ready to yield. This will not return until statemachine
 * has no task or waiting on a timer. If another task need to schedule while statemachine is
 * active use bc_sm.
 * @param BC1.2 object
 */
void bc_sm_exec(void *);

/**
 * @brief Returns the shortest active timer value.
 * @param BC1.2 object
 * @return value of the shortest active timer in milliseconds.
 */
unsigned long port_bc1p2_next_schedule(struct port_bc1p2 *const dev);

/**
 * @brief Informs that there are no active events and the state machine
 * can yield for bc_min_timer().
 * @return false: active event execute statemachine immediately when possible
 *         true: the state machine can yield until timer expires
 */
bool          bc_yield(void *);
unsigned long port_bc1p2_next_schedule(struct port_bc1p2 *const dev);
void          port_bc1p2_sm_disable(void *bc1p2_priv);
void          port_bc1p2_sm_enable(void *bc1p2_priv);

#endif //__PORT_BC1P2_H__
