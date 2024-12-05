/*******************************************************************************
 * @file     port_bc1p2.c
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

#include "port.h"
#include "legacy.h"


#if CONFIG_ENABLED(LEGACY_CHARGING)

static struct legacy_state STATES[LEGACY_PORT_COUNT];
static struct port_bc1p2   PORTS_BC1P2[TYPE_C_PORT_COUNT];

struct port_bc1p2 *dpm_legacy_init(void *dpm_info, int port_id)
{
    struct legacy_state *states = &STATES[port_id];
    struct port_bc1p2   *dev    = &PORTS_BC1P2[port_id];

    /*save dpm_info*/
    dev->dpm_priv = dpm_info;
    /*save port id*/
    dev->port_bc1p2_id = port_id;
    /*save port bc1p device*/
    states->dev = dev;
    /*save port info*/
    dev->port_bc1p2_priv = states;
    return dev;
}

void port_bc1p2_init(void *bc1p2_priv, enum bc1p2_mode m)
{
    struct legacy_state *state = (struct legacy_state *)bc1p2_priv;
    struct port_bc1p2 *dev = state->dev;
    state->bc1p2_device = dev->bc1p2_device;
    bc_sm_init(state, m);
}

void port_bc1p2_sm_enable(void *bc1p2_priv)
{
    struct legacy_state *state = (struct legacy_state *)bc1p2_priv;
    bc_sm_enable(state);
}

void port_bc1p2_sm_disable(void *bc1p2_priv)
{
    struct legacy_state *state = (struct legacy_state *)bc1p2_priv;
    bc_sm_disable(state);
}

enum bc1p2_state bc_sm_state(void *bc1p2_priv)
{
    assert(bc1p2_priv != 0);

    struct legacy_state *state = (struct legacy_state *)bc1p2_priv;
    return state->state;
}

void bc_sm(void *bc1p2_priv)
{
    assert(bc1p2_priv != 0);

    struct legacy_state *state = (struct legacy_state *)bc1p2_priv;

    if (state->enabled)
    {
        state->event = false;
        if (state->smcb)
        {
            state->smcb(state);
        }
    }
}

void bc_sm_exec(void *bc1p2_priv)
{
    assert(bc1p2_priv != 0);

    struct legacy_state *state = (struct legacy_state *)bc1p2_priv;

    if (state->enabled)
    {
        do
        {
            bc_sm(state);
        } while (state->event);
    }
}

unsigned long port_bc1p2_next_schedule(struct port_bc1p2 *const dev)
{
    struct legacy_state *state = (struct legacy_state *)dev->port_bc1p2_priv;
    unsigned long        min   = ~0;
    struct ticktimer_t  *t;
    unsigned long        r, i;

    for (i = 0; i < 3; i++)
    {
        t = &state->timers[i];
        if (!timer_disabled(t))
        {
            r = timer_remaining(t);
            if (r < min)
            {
                min = r;
            }
        }
    }
    return min;
}

bool bc_yield(void *bc1p2_priv)
{
    assert(bc1p2_priv != 0);

    struct legacy_state *state = (struct legacy_state *)bc1p2_priv;
    return !state->event;
}

#endif
