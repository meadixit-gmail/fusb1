/*******************************************************************************
 * @file     legacy.c
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
#include "dev_tcpd.h"
#include "legacy.h"
#include "timer.h"
#include "port.h"

#if CONFIG_ENABLED(LEGACY_CHARGING)

#ifndef LEGACY_PORT_COUNT
#error "Define LEGACY_PORT_COUNT in platform.h"
#endif

/**
 * Forward declaration
 */
#if CONFIG_ENABLED(BC1P2_CSM)
static void bc_csm_enable(struct legacy_state *);
static void bc_csm_set_consumer_enable(struct legacy_state *);
static void bc_csm_set_primary_detection(struct legacy_state *);
static void bc_csm_set_sdp_attached(struct legacy_state *);
static void bc_csm_set_proprietary(struct legacy_state *);
static void bc_csm_set_secondary_detection(struct legacy_state *);
static void bc_csm_set_dcp_attached(struct legacy_state *);
static void bc_csm_set_cdp_attached(struct legacy_state *);
#endif
#if CONFIG_ENABLED(BC1P2_CDP)
static void bc_cdp_enable(struct legacy_state *);
static void bc_cdp_disable(struct legacy_state *);
static void bc_cdp_set_pe_emulation2(struct legacy_state *);
static void bc_cdp_set_attached(struct legacy_state *);
#endif
#if CONFIG_ENABLED(BC1P2_DCP)
static void bc_dcp_init(struct legacy_state *);
static void bc_dcp_enable(struct legacy_state *);
static void bc_dcp_disable(struct legacy_state *);
static void bc_dcp_set_ad_div_mode(struct legacy_state *);
static void bc_dcp_set_ad_dcp(struct legacy_state *);
#endif
/*******************************************************************************
 * BC1.2 Consumer states
 ******************************************************************************/
#if CONFIG_ENABLED(BC1P2_CSM)
static void bc_csm_set_disable(struct legacy_state *state)
{
    assert(state != 0);
    assert(state->dev != 0);

    state->enabled = 0;
    state->state   = BC1P2_STATE_DISABLED;
    state->smcb    = 0;

    timer_disable(&state->timers[BC1P2_TIMER0]);
    timer_disable(&state->timers[BC1P2_TIMER1]);
    timer_disable(&state->timers[BC1P2_TIMER2]);

    /* Clear all control - disable */
    fusbdev_usbchg_disable(state->bc1p2_device);
    fusbdev_adc_ch_disable(state->bc1p2_device->dp_ch);
    fusbdev_adc_ch_disable(state->bc1p2_device->dm_ch);
}

static void bc_csm_consumer_enable(struct legacy_state *state)
{
    assert((state != 0) && (state->dev != 0));

    unsigned long vdp = 0;

    if (timer_expired(&state->timers[BC1P2_TIMER1]))
    {
        /* Move to primary detection state */
        bc_csm_set_primary_detection(state);
    }
    else if (timer_expired(&state->timers[BC1P2_TIMER2]))
    {
        if (fusbdev_adc_ch_ready(state->bc1p2_device->dp_ch))
        {
            vdp = fusbdev_adc_ch_data(state->bc1p2_device->dp_ch);
            if (vdp < VLGC_LOW_H)
            {
                bc_csm_set_primary_detection(state);
            }
        }
    }
}

static void bc_csm_set_consumer_enable(struct legacy_state *state)
{
    assert((state != 0) && (state->dev != 0));

    /* Set state variables */
    state->state    = BC1P2_STATE_CSM_ENABLE;
    state->stateIdx = 0;
    state->smcb     = bc_csm_consumer_enable;
    state->event    = true;

    fusbdev_usbchg_idp_src(state->bc1p2_device, true);
    fusbdev_usbchg_rdm_dwn(state->bc1p2_device, true);

    timer_start(&state->timers[BC1P2_TIMER0], TSVLD_CON_PWD);
    timer_start(&state->timers[BC1P2_TIMER1], TDCD_TIMEOUT);
    timer_start(&state->timers[BC1P2_TIMER2], TDCD_DBNC);

    fusbdev_adc_ch_enable(state->bc1p2_device->dp_ch);
}

static void bc_csm_primary_detection(struct legacy_state *state)
{
    assert((state != 0) && (state->dev != 0));
    unsigned long vdm = 0;

    if (timer_expired(&state->timers[BC1P2_TIMER1]))
    {
        timer_disable(&state->timers[BC1P2_TIMER1]);
        /* TVDPSRC_ON expired */
        if (fusbdev_adc_ch_ready(state->bc1p2_device->dm_ch))
        {
            vdm = fusbdev_adc_ch_data(state->bc1p2_device->dm_ch);
            if (vdm < VDAT_REF_H)
            {
                bc_csm_set_sdp_attached(state);
            }
            else if (vdm < VLGC_L && vdm > VDAT_REF_H)
            {
                bc_csm_set_secondary_detection(state);
            }
            else
            {
                bc_csm_set_proprietary(state);
            }
        }
    }
}

static void bc_csm_set_primary_detection(struct legacy_state *state)
{
    assert((state != 0) && (state->dev != 0));

    /* Set state variables */
    state->state    = BC1P2_STATE_CSM_PRIMARY_DETECTION;
    state->stateIdx = 0;
    state->smcb     = bc_csm_primary_detection;
    state->event    = true;

    fusbdev_usbchg_idp_src(state->bc1p2_device, false);
    fusbdev_usbchg_rdm_dwn(state->bc1p2_device, false);
    fusbdev_usbchg_vdp_src(state->bc1p2_device, true);
    fusbdev_usbchg_idm_sink(state->bc1p2_device, true);

    timer_disable(&state->timers[BC1P2_TIMER1]);
    timer_disable(&state->timers[BC1P2_TIMER2]);
    timer_start(&state->timers[BC1P2_TIMER1], TVDxSRC_ON);

    fusbdev_adc_ch_enable(state->bc1p2_device->dm_ch);
}

static void bc_csm_state_sdp_attached(struct legacy_state *state)
{
    assert((state != 0) && (state->dev != 0));
}

static void bc_csm_set_sdp_attached(struct legacy_state *state)
{
    assert((state != 0) && (state->dev != 0));

    /* Cleanup from another states */
    fusbdev_adc_ch_disable(state->bc1p2_device->dp_ch);
    fusbdev_adc_ch_disable(state->bc1p2_device->dm_ch);

    timer_disable(&state->timers[BC1P2_TIMER0]);

    /* Set state variables */
    state->state    = BC1P2_STATE_CSM_SDP_ATTACHED;
    state->stateIdx = 0;
    state->smcb     = bc_csm_state_sdp_attached;
    state->event    = true;

    fusbdev_usbchg_idp_src(state->bc1p2_device, false);
    fusbdev_usbchg_rdm_dwn(state->bc1p2_device, false);
    fusbdev_usbchg_vdp_src(state->bc1p2_device, true);
    fusbdev_usbchg_host_sw(state->bc1p2_device, true);
}

static void bc_csm_secondary_detection(struct legacy_state *state)
{
    assert((state != 0) && (state->dev != 0));

    unsigned long vdp = 0;

    if (timer_expired(&state->timers[BC1P2_TIMER1]))
    {
        timer_disable(&state->timers[BC1P2_TIMER1]);
        if (fusbdev_adc_ch_ready(state->bc1p2_device->dp_ch))
        {
            vdp = fusbdev_adc_ch_data(state->bc1p2_device->dp_ch);
            if (vdp > VDAT_REF_H)
            {
                bc_csm_set_dcp_attached(state);
            }
            else
            {
                bc_csm_set_cdp_attached(state);
            }
        }
    }
}

static void bc_csm_set_secondary_detection(struct legacy_state *state)
{
    assert((state != 0) && (state->dev != 0));

    fusbdev_adc_ch_disable(state->bc1p2_device->dm_ch);

    state->state    = BC1P2_STATE_CSM_SECONDARY_DETECTION;
    state->stateIdx = 0;
    state->smcb     = bc_csm_secondary_detection;
    state->event    = true;

    fusbdev_usbchg_vdp_src(state->bc1p2_device, false);
    fusbdev_usbchg_idm_sink(state->bc1p2_device, false);
    fusbdev_usbchg_vdm_src(state->bc1p2_device, true);
    fusbdev_usbchg_idp_snk(state->bc1p2_device, true);

    timer_start(&state->timers[BC1P2_TIMER1], TVDxSRC_ON);
}

static void bc_csm_dcp(struct legacy_state *state)
{
    assert((state != 0) && (state->dev != 0));
    /* DCP */
}

void bc_csm_set_dcp_attached(struct legacy_state *state)
{
    assert((state != 0) && (state->dev != 0));

    fusbdev_adc_ch_disable(state->bc1p2_device->dp_ch);
    fusbdev_adc_ch_disable(state->bc1p2_device->dm_ch);

    state->state    = BC1P2_STATE_CSM_DCP_ATTACHED;
    state->stateIdx = 0;
    state->smcb     = bc_csm_dcp;
    state->event    = true;

    timer_disable(&state->timers[BC1P2_TIMER0]);

    fusbdev_usbchg_vdm_src(state->bc1p2_device, false);
    fusbdev_usbchg_idp_snk(state->bc1p2_device, false);
    fusbdev_usbchg_vdp_src(state->bc1p2_device, true);
}

static void bc_csm_cdp(struct legacy_state *state)
{
    assert((state != 0) && (state->dev != 0));
    /* CDP */
}

static void bc_csm_set_cdp_attached(struct legacy_state *state)
{
    assert((state != 0) && (state->dev != 0));

    fusbdev_adc_ch_disable(state->bc1p2_device->dp_ch);
    fusbdev_adc_ch_disable(state->bc1p2_device->dm_ch);

    state->state    = BC1P2_STATE_CSM_CDP_ATTACHED;
    state->stateIdx = 0;
    state->smcb     = bc_csm_cdp;
    state->event    = true;

    timer_disable(&state->timers[BC1P2_TIMER0]);

    fusbdev_usbchg_vdm_src(state->bc1p2_device, false);
    fusbdev_usbchg_idp_snk(state->bc1p2_device, false);
    fusbdev_usbchg_host_sw(state->bc1p2_device, true);
}

static void bc_csm_proprietary(struct legacy_state *state)
{
    assert((state != 0) && (state->dev != 0));

    /* Start detecting proprietary charger here. For now just wait out
     * the timer and attach as SDP */
}

static void bc_csm_set_proprietary(struct legacy_state *state)
{
    assert((state != 0) && (state->dev != 0));

    fusbdev_adc_ch_disable(state->bc1p2_device->dp_ch);
    fusbdev_adc_ch_disable(state->bc1p2_device->dm_ch);

    state->state    = BC1P2_STATE_CSM_PROPRIETARY;
    state->stateIdx = 0;
    state->smcb     = bc_csm_proprietary;
    state->event    = true;

    timer_disable(&state->timers[BC1P2_TIMER0]);

    fusbdev_usbchg_vdp_src(state->bc1p2_device, false);
    fusbdev_usbchg_idm_sink(state->bc1p2_device, false);
}

static void bc_csm_init(struct legacy_state *state)
{
    assert((state != 0) && (state->dev != 0));

    state->mode = BC1P2_MODE_CONSUMER;
    bc_csm_set_disable(state);
}

static void bc_csm_enable(struct legacy_state *state)
{
    assert((state != 0) && (state->dev != 0));

    state->enabled = true;
    state->event   = true;

    fusbdev_usbchg_enable(state->bc1p2_device);
    bc_csm_set_consumer_enable(state);
}
#endif
#if CONFIG_ENABLED(BC1P2_CDP)
/*******************************************************************************
 * BC1.2 CDP Charging states
 ******************************************************************************/
static void bc_cdp_pe_emulation1(struct legacy_state *state)
{
    assert((state != 0) && (state->dev != 0));

    unsigned long vdp = 0;

    if (fusbdev_adc_ch_ready(state->bc1p2_device->dp_ch))
    {
        vdp = fusbdev_adc_ch_data(state->bc1p2_device->dp_ch);
        if (vdp > VDAT_REF_H && vdp < VLGC_L)
        {
            bc_cdp_set_pe_emulation2(state);
        }
    }
}

static void bc_cdp_set_pe_emulation1(struct legacy_state *state)
{
    assert((state != 0) && (state->dev != 0));

    state->state    = BC1P2_STATE_CDP_PRIMARY1;
    state->stateIdx = 0;
    state->smcb     = bc_cdp_pe_emulation1;
    state->event    = true;

    fusbdev_usbchg_idp_snk(state->bc1p2_device, true);
    fusbdev_usbchg_rdp_dwn(state->bc1p2_device, true);
    fusbdev_usbchg_rdm_dwn(state->bc1p2_device, true);

    fusbdev_adc_ch_disable(state->bc1p2_device->dp_ch);
}

static void bc_cdp_pe_emulation2(struct legacy_state *state)
{
    assert((state != 0) && (state->dev != 0));

    unsigned long vdp = 0;

    if (fusbdev_adc_ch_ready(state->bc1p2_device->dp_ch))
    {
        vdp = fusbdev_adc_ch_data(state->bc1p2_device->dp_ch);
        if (vdp > VLGC_H || vdp < VDAT_REF_L)
        {
            bc_cdp_set_attached(state);
        }
    }
}

static void bc_cdp_set_pe_emulation2(struct legacy_state *state)
{
    assert((state != 0) && (state->dev != 0));

    state->state    = BC1P2_STATE_CDP_PRIMARY2;
    state->smcb     = bc_cdp_pe_emulation2;
    state->stateIdx = 0;
    state->event    = true;

    fusbdev_usbchg_vdm_src(state->bc1p2_device, true);
}

static void bc_cdp_attached(struct legacy_state *state)
{
    assert((state != 0) && (state->dev != 0));
}

static void bc_cdp_set_attached(struct legacy_state *state)
{
    assert((state != 0) && (state->dev != 0));

    state->state    = BC1P2_STATE_CDP_ATTACHED;
    state->smcb     = bc_cdp_attached;
    state->stateIdx = 0;
    state->event    = true;

    fusbdev_usbchg_vdm_src(state->bc1p2_device, false);
    fusbdev_usbchg_idp_snk(state->bc1p2_device, false);
    fusbdev_usbchg_rdp_dwn(state->bc1p2_device, false);
    fusbdev_usbchg_rdm_dwn(state->bc1p2_device, false);
    fusbdev_usbchg_host_sw(state->bc1p2_device, true);
}

static void bc_cdp_enable(struct legacy_state *state)
{
    assert((state != 0) && (state->dev != 0));

    state->enabled = true;
    state->event   = true;
    fusbdev_usbchg_enable(state->bc1p2_device);
    bc_cdp_set_pe_emulation1(state);
}

static void bc_cdp_disable(struct legacy_state *state)
{
    assert((state != 0) && (state->dev != 0));

    state->enabled = false;
    state->state   = BC1P2_STATE_DISABLED;
    state->smcb    = 0;
    state->event   = false;

    timer_disable(&state->timers[BC1P2_TIMER0]);
    timer_disable(&state->timers[BC1P2_TIMER1]);
    timer_disable(&state->timers[BC1P2_TIMER2]);

    /* Clear all control - disable */
    fusbdev_adc_ch_disable(state->bc1p2_device->dp_ch);
    fusbdev_adc_ch_disable(state->bc1p2_device->dm_ch);
    fusbdev_usbchg_host_sw(state->bc1p2_device, false);
    fusbdev_usbchg_disable(state->bc1p2_device);
}

static void bc_cdp_init(struct legacy_state *state)
{
    assert((state != 0) && (state->dev != 0));

    state->mode = BC1P2_MODE_PROVIDER_CDP;
    bc_cdp_disable(state);
}
#endif
#if CONFIG_ENABLED(BC1P2_DCP)
/*******************************************************************************
 * BC1.2 DCP Charging states
 ******************************************************************************/
static void bc_dcp_ad_div_mode(struct legacy_state *state)
{
    assert((state != 0) && (state->dev != 0));
    unsigned long vdp = 0;
    unsigned long vdm = 0;

    if ((fusbdev_adc_ch_ready(state->bc1p2_device->dp_ch))
        && (fusbdev_adc_ch_ready(state->bc1p2_device->dm_ch)))
    {
        vdp = fusbdev_adc_ch_data(state->bc1p2_device->dp_ch);
        vdm = fusbdev_adc_ch_data(state->bc1p2_device->dm_ch);
        if (vdp > V150mV + VDIV || vdp < VLGC_HI_L || vdm < VLGC_HI_L)
        {
            if (state->stateIdx == 0)
            {
                timer_start(&state->timers[BC1P2_TIMER0], TEXIT_MODE);
                state->stateIdx++;
            }
            else
            {
                if (timer_expired(&state->timers[BC1P2_TIMER0]))
                {
                    bc_dcp_set_ad_dcp(state);
                }
            }
        }
    }
}

static void bc_dcp_set_ad_div_mode(struct legacy_state *state)
{
    assert((state != 0) && (state->dev != 0));

    state->state    = BC1P2_STATE_DCP_ADV_DIV;
    state->smcb     = bc_dcp_ad_div_mode;
    state->stateIdx = 0;
    state->event    = true;

    fusbdev_usbchg_rdiv(state->bc1p2_device, true);
    fusbdev_adc_ch_enable(state->bc1p2_device->dp_ch);
    fusbdev_adc_ch_enable(state->bc1p2_device->dm_ch);
}

static void bc_dcp_ad_dcp(struct legacy_state *state)
{
    assert((state != 0) && (state->dev != 0));

    unsigned long vdp = 0;

    if (state->stateIdx == 0)
    {
        if (timer_expired(&state->timers[BC1P2_TIMER0]))
        {
            timer_disable(&state->timers[BC1P2_TIMER0]);
            state->stateIdx++;
        }
    }

    if (state->stateIdx == 1)
    {
        if (fusbdev_adc_ch_ready(state->bc1p2_device->dp_ch))
        {
            vdp = fusbdev_adc_ch_data(state->bc1p2_device->dp_ch);
            if (vdp < VDAT_REF_L)
            {
                timer_start(&state->timers[BC1P2_TIMER0], TDETACH);
                state->stateIdx++;
            }
        }
    }

    if (state->stateIdx == 2)
    {
        if (timer_expired(&state->timers[BC1P2_TIMER0]))
        {
            timer_disable(&state->timers[BC1P2_TIMER0]);
            if (fusbdev_adc_ch_ready(state->bc1p2_device->dp_ch))
            {
                vdp = fusbdev_adc_ch_data(state->bc1p2_device->dp_ch);
                if (vdp < VDAT_REF_L)
                {
                    bc_dcp_disable(state);
                }
                else
                {
                    state->stateIdx = 1;
                }
            }
        }
    }
}

static void bc_dcp_set_ad_dcp(struct legacy_state *state)
{
    assert((state != 0) && (state->dev != 0));

    state->state    = BC1P2_STATE_DCP_ADV_DCP;
    state->smcb     = bc_dcp_ad_dcp;
    state->stateIdx = 0;
    state->event    = true;

    fusbdev_usbchg_rdiv(state->bc1p2_device, false);
    fusbdev_usbchg_rdcp(state->bc1p2_device, true);
    fusbdev_usbchg_rdat(state->bc1p2_device, true);
	
    fusbdev_adc_ch_enable(state->bc1p2_device->dp_ch);
    fusbdev_adc_ch_enable(state->bc1p2_device->dm_ch);

    timer_start(&state->timers[BC1P2_TIMER0], TBC12);
}

static void bc_dcp_enable(struct legacy_state *state)
{
    assert((state != 0) && (state->dev != 0));

    state->enabled = true;
    state->event   = true;
    fusbdev_usbchg_enable(state->bc1p2_device);
    if (CONFIG_ENABLED(BC1P2_DCP_ADDIV))
    {
        bc_dcp_set_ad_div_mode(state);
    }
    else
    {
        bc_dcp_set_ad_dcp(state);
    }
}

static void bc_dcp_disable(struct legacy_state *state)
{
    assert((state != 0) && (state->dev != 0));

    state->enabled = false;
    state->state   = BC1P2_STATE_DISABLED;
    state->smcb    = 0;

    /* Stop ADC channels */
    fusbdev_adc_ch_disable(state->bc1p2_device->dp_ch);
    fusbdev_adc_ch_disable(state->bc1p2_device->dm_ch);

    /* Disable timers */
    timer_disable(&state->timers[BC1P2_TIMER0]);
    timer_disable(&state->timers[BC1P2_TIMER1]);
    timer_disable(&state->timers[BC1P2_TIMER2]);

    /* Clear all control - disable */
    fusbdev_usbchg_disable(state->bc1p2_device);
}

static void bc_dcp_init(struct legacy_state *state)
{
    assert((state != 0) && (state->dev != 0));
#if CONFIG_ENABLED(BC1P2_DCP) && CONFIG_ENABLED(BC1P2_CSM) && CONFIG_ENABLED(BC1P2_CDP)
    state->mode = BC1P2_MODE_PROVIDER_DCP;
#endif
    bc_dcp_disable(state);
}
#endif

/*******************************************************************************
 * Common API
 ******************************************************************************/

void bc_sm_enable(struct legacy_state *state)
{
    assert((state != 0) && (state->dev != 0));
#if CONFIG_ENABLED(BC1P2_CDP) && CONFIG_ENABLED(BC1P2_DCP) && CONFIG_ENABLED(BC1P2_CSM)
    if (state->mode == BC1P2_MODE_CONSUMER)
	{
		bc_csm_enable(state);
	}
	else if (state->mode == BC1P2_MODE_PROVIDER_CDP)
	{
		bc_cdp_enable(state);
	}
	else if (state->mode == BC1P2_MODE_PROVIDER_DCP)
	{
		bc_dcp_enable(state);
	}
#elif CONFIG_ENABLED(BC1P2_CDP)
    bc_cdp_enable(state);
#elif CONFIG_ENABLED(BC1P2_DCP)
    bc_dcp_enable(state);
#elif CONFIG_ENABLED(BC1P2_CSM)
    bc_csm_set_enable(state);
#endif
}

void bc_sm_disable(struct legacy_state *state)
{
    assert((state != 0) && (state->dev != 0));
#if CONFIG_ENABLED(BC1P2_CDP) && CONFIG_ENABLED(BC1P2_DCP) && CONFIG_ENABLED(BC1P2_CSM)
    if (state->mode == BC1P2_MODE_CONSUMER)
    {
        bc_csm_set_disable(state);
    }
    else if (state->mode == BC1P2_MODE_PROVIDER_CDP)
    {
        bc_cdp_disable(state);
    }
    else if (state->mode == BC1P2_MODE_PROVIDER_DCP)
    {
        bc_dcp_disable(state);
    }
#elif CONFIG_ENABLED(BC1P2_CDP)
    bc_cdp_disable(state);
#elif CONFIG_ENABLED(BC1P2_DCP)
    bc_dcp_disable(state);
#elif CONFIG_ENABLED(BC1P2_CSM)
    bc_csm_set_disable(state);
#endif

}

void bc_sm_init(struct legacy_state *state, enum bc1p2_mode mode)
{
    assert((state != 0) && (state->dev != 0));
    assert(state->dev->port_bc1p2_id < LEGACY_PORT_COUNT);
#if CONFIG_ENABLED(BC1P2_CDP) && CONFIG_ENABLED(BC1P2_DCP) && CONFIG_ENABLED(BC1P2_CSM)
    if (mode == BC1P2_MODE_CONSUMER)
    {
        bc_csm_init(state);
    }
    else if (mode == BC1P2_MODE_PROVIDER_CDP)
    {
        bc_cdp_init(state);
    }
    else if (mode == BC1P2_MODE_PROVIDER_DCP)
    {
        bc_dcp_init(state);
    }
#elif CONFIG_ENABLED(BC1P2_CDP)
    bc_cdp_init(state);
#elif CONFIG_ENABLED(BC1P2_DCP)
    bc_dcp_init(state);
#elif CONFIG_ENABLED(BC1P2_CSM)
    bc_csm_init(state);
#endif
}

#endif /* CONFIG_ENABLED_BC1P2_CHARGING */
