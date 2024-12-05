/*******************************************************************************
 * @file     typec.c
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
 * typec.c
 *
 * Implements the Type-C state machine functions
 */
#include "vif_types.h"
#include "port.h"
#include "typec.h"
#include "protocol.h"
#include "policy.h"
#include "observer.h"
extern volatile bool gSleepInt;
#if CONFIG_ENABLED(LOG)
#include "logging.h"
static const char *const typec_string[] = {
#define STATE_STR(str) #str,
    CREATE_TYPEC_STATE_LIST(STATE_STR)};

#define typec_set_state(port, state, sub)                                                          \
    do                                                                                             \
    {                                                                                              \
        dbg_msg("%d:%s(%d)", port->dev->port_tcpd_id, typec_string[state], sub);                   \
        typec_set_state_func(port, state, sub);                                                    \
    } while (0)
#else
#define typec_set_state(port, state, sub) typec_set_state_func(port, state, sub);
#endif
static inline void typec_set_state_func(struct port *port, TC_STATE_T state, uint32_t substate)
{
    port->tc_state    = state;
    port->tc_substate = substate;
    port->event       = true;
}

static void typec_reset_debounce(struct port *port)
{
    port->cc_term_raw         = CC_STAT_UNDEFINED;
    port->cc_term_cc_debounce = CC_STAT_UNDEFINED;
    port->cc_term_pd_debounce = CC_STAT_UNDEFINED;
    port->vc_term_raw         = CC_STAT_UNDEFINED;
    port->vc_term_cc_debounce = CC_STAT_UNDEFINED;
    port->vc_term_pd_debounce = CC_STAT_UNDEFINED;
    timer_disable(&port->timers[CC_DBNC_TIMER]);
    timer_disable(&port->timers[CC_PDDBNC_TIMER]);
    timer_disable(&port->timers[VC_DBNC_TIMER]);
    timer_disable(&port->timers[VC_PDDBNC_TIMER]);
}

/*
 * @brief checks if status of CC1 and CC2 is one of the known source or sink
 * termination and sets that pin to CC.
 */
static void typec_set_cc_pin(struct port *port)
{
    enum cc_stat_t cc1 = fusbdev_tcpd_cc_stat(port->tcpd_device, CC1);
    enum cc_stat_t cc2 = fusbdev_tcpd_cc_stat(port->tcpd_device, CC2);
    if ((cc1 == CC_STAT_RPUSB) || (cc1 == CC_STAT_RP1p5) || (cc1 == CC_STAT_RP3p0)
        || (cc1 == CC_STAT_RD) || (cc2 == CC_STAT_RA))
    {
        port->cc_pin = CC1;
    }
    else if ((cc2 == CC_STAT_RPUSB) || (cc2 == CC_STAT_RP1p5) || (cc2 == CC_STAT_RP3p0)
             || (cc2 == CC_STAT_RD) || (cc1 == CC_STAT_RA))
    {
        port->cc_pin = CC2;
    }
    else
    {
        port->cc_pin = CC_NONE;
    }
}

void typec_reset(struct port *port)
{
    port->source_or_sink = Source;
    port->tc_enabled     = true;
    typec_set_state(port, Disabled, 0);
    port->src_preferred = port->vif->Type_C_Implements_Try_SRC;
    port->snk_preferred = port->vif->Type_C_Implements_Try_SNK;
    port->acc_support   = port->vif->Type_C_Supports_VCONN_Powered_Accessory
                                || (port->vif->Type_C_Supports_Audio_Accessory) ?
                              true :
                              false;
    port->src_current   = (port->vif->Rp_Value + 1) & 0x3;
    port->cc_pin        = CC_NONE;
    typec_reset_debounce(port);
}

static void typec_clear_state(struct port *port)
{
    policy_pd_disable(port);
    dev_tcpd_port_vbus_src(port, VBUS_OFF, VBUS_OFF, 0);
    dev_tcpd_port_vbus_snk(port, VBUS_OFF, VBUS_OFF, 0);
    dev_tcpd_port_set_vconn(port, false);
    typec_reset_debounce(port);
    timer_disable(&port->timers[TC_TIMER]);
    port->cc_pin = CC_NONE;
}

/*
 * @brief Setup for a source connection.
 */
static inline void typec_set_source(struct port *port)
{
    port->source_or_sink = Source;
    typec_reset_debounce(port);
}
#if (CONFIG_ENABLED(SNK))
/*
 * @brief Setup for a sink connection.
 */
static inline void typec_set_sink(struct port *port)
{
    port->source_or_sink = Sink;
    typec_reset_debounce(port);
}
#if (CONFIG_ENABLED(DRP))
void typec_role_swap_to_sink(struct port *port)
{
    unsigned current = CC_RD_CURRENT(port->cc_term_pd_debounce);
    typec_set_state(port, AttachedSink, 0);
    port->source_or_sink = Sink;
    typec_reset_debounce(port);
    typec_set_sink(port);
    /* Apply Rd - leaving Vconn pin open if needed */
    if (port->is_vconn_source)
    {
        fusbdev_tcpd_set_cc(port->tcpd_device, port->cc_pin, CC_TERM_RD, CC_RP_NOCHANGE,
                            DRP_MODE_NOCHANGE);
    }
    else
    {
        fusbdev_tcpd_set_cc(port->tcpd_device, CC1 | CC2, CC_TERM_RD, CC_RP_NOCHANGE,
                            DRP_MODE_NOCHANGE);
    }
    /* Set up the Sink Disconnect threshold/interrupt */
    fusbdev_tcpd_set_vbus_alarm(port->tcpd_device, VBUS_5V_DISC, 0);
    dev_tcpd_port_vbus_snk(port, VBUS_5V, current, pdoTypeFixed);
    timer_disable(&port->timers[TC_TIMER]);
}
#endif
#endif
#if (CONFIG_ENABLED(DRP))
void typec_role_swap_to_source(struct port *port)
{
    typec_set_state(port, AttachedSource, 0);
    port->source_or_sink = Source;
    typec_reset_debounce(port);
    /* Apply Rp - leaving Vconn pin open if needed */
    if (port->is_vconn_source)
    {
        fusbdev_tcpd_set_cc(port->tcpd_device, port->cc_pin, CC_TERM_RP, port->src_current,
                            DRP_MODE_NOCHANGE);
    }
    else
    {
        fusbdev_tcpd_set_cc(port->tcpd_device, CC1 | CC2, CC_TERM_RP, port->src_current,
                            DRP_MODE_NOCHANGE);
    }
    typec_set_source(port);
    dev_tcpd_port_vbus_src(port, VBUS_5V, CC_RP_CURRENT(port->src_current), pdoTypeFixed);
    timer_disable(&port->timers[TC_TIMER]);
}
#endif
#if (CONFIG_ENABLED(SRC))
void typec_set_attach_wait_source(struct port *port)
{
    typec_set_state(port, AttachWaitSource, 0);
    typec_set_source(port);
    fusbdev_tcpd_set_cc(port->tcpd_device, CC1 | CC2, CC_TERM_RP, port->src_current,
                        DRP_MODE_NOCHANGE);
    /* Enable bleed discharge to ensure vSafe0V */
    fusbdev_tcpd_set_vbus_disch(port->tcpd_device, VBUS_DISCH_DEFAULT);
    timer_disable(&port->timers[TC_TIMER]);
    fusbdev_tcpd_set_vbus_alarm(port->tcpd_device, VBUS_VSAFE0_ALARM, 0);
}

void typec_set_attached_source(struct port *port)
{
    struct event_tc_attached_t evt = {0};
    typec_set_state(port, AttachedSource, 0);
    /* Turn off Bleed - VBus should be below vSafe0V */
    fusbdev_tcpd_set_vbus_disch(port->tcpd_device, VBUS_DISCH_OFF);
    typec_set_source(port);
    typec_set_cc_pin(port);
    fusbdev_tcpd_set_orient(port->tcpd_device, port->cc_pin);
    dev_tcpd_port_vbus_src(port, VBUS_5V, CC_RP_CURRENT(port->src_current), pdoTypeFixed);
    if (port->vif->Type_C_Sources_VCONN)
    {
        dev_tcpd_port_set_vconn(port, true);
    }
    policy_pd_enable(port, true);
    evt.arg.cc  = port->cc_pin;
    evt.arg.src = port->source_or_sink == Source ? true : false;
#if CONFIG_ENABLED(POWER_SHARING) || CONFIG_ENABLED(LEGACY_CHARGING)
    event_notify(EVENT_TC_ATTACHED, port->dev, &evt);
#endif
}

void typec_set_unattached_wait_source(struct port *port)
{
    policy_pd_disable(port);
    typec_set_state(port, UnattachedWaitSource, 0);
    if (port->cc_pin == CC1)
    {
        fusbdev_tcpd_set_cc(port->tcpd_device, CC2, CC_TERM_RD, CC_RP_NOCHANGE, DRP_MODE_NOCHANGE);
    }
    else
    {
        fusbdev_tcpd_set_cc(port->tcpd_device, CC1, CC_TERM_RD, CC_RP_NOCHANGE, DRP_MODE_NOCHANGE);
    }
    timer_start(&port->timers[TC_TIMER], ktVCONNSourceDischarge);
}
#endif
#if (CONFIG_ENABLED(SRC))
void typec_set_unoriented_debug_acc_source(struct port *port)
{
    typec_set_state(port, UnorientedDebugAccessorySource, 0);
    typec_set_source(port);
    dev_tcpd_port_vbus_src(port, VBUS_5V, CC_RP_CURRENT(port->src_current), pdoTypeFixed);
    timer_disable(&port->timers[TC_TIMER]);
}

void typec_set_oriented_debug_acc_source(struct port *port)
{
    typec_set_state(port, OrientedDebugAccessorySource, 0);
    typec_set_source(port);
    policy_pd_enable(port, true);
    timer_disable(&port->timers[TC_TIMER]);
}
#endif
#if (CONFIG_ENABLED(SNK))
void typec_set_attach_wait_sink(struct port *port)
{
    typec_set_state(port, AttachWaitSink, 0);
    typec_set_sink(port);
#if CONFIG_ENABLED(RA_TEST)
    fusbdev_tcpd_set_cc(port->tcpd_device, CC2, CC_TERM_RA, CC_RP_NOCHANGE, DRP_MODE_NOCHANGE);
    fusbdev_tcpd_set_cc(port->tcpd_device, CC1, CC_TERM_RD, CC_RP_NOCHANGE, DRP_MODE_NOCHANGE);
#else
    fusbdev_tcpd_set_cc(port->tcpd_device, CC1 | CC2, CC_TERM_RD, CC_RP_NOCHANGE, DRP_MODE_NOCHANGE);
#endif
    timer_disable(&port->timers[TC_TIMER]);
    fusbdev_tcpd_set_vbus_alarm(port->tcpd_device, 0, VBUS_LOW_THRESHOLD(VBUS_5V));
}

void typec_set_attached_sink(struct port *port)
{
    unsigned                   current = CC_RD_CURRENT(port->cc_term_cc_debounce);
    struct event_tc_attached_t evt     = {0};
    typec_set_state(port, AttachedSink, 0);
    typec_set_sink(port);
    /* Set up the Sink Disconnect threshold/interrupt */
    fusbdev_tcpd_set_vbus_alarm(port->tcpd_device, VBUS_5V_DISC, 0);
    fusbdev_tcpd_set_orient(port->tcpd_device, port->cc_pin);
    policy_pd_enable(port, false);
    /* Enable bleed discharge */
    fusbdev_tcpd_set_vbus_disch(port->tcpd_device, VBUS_DISCH_BLEED);
    dev_tcpd_port_vbus_snk(port, VBUS_5V, current, pdoTypeFixed);
    dev_tcpd_port_set_vconn(port, false);
    evt.arg.cc  = port->cc_pin;
    evt.arg.src = port->source_or_sink == Source ? true : false;
#if CONFIG_ENABLED(POWER_SHARING) || CONFIG_ENABLED(LEGACY_CHARGING)
    event_notify(EVENT_TC_ATTACHED, port->dev, &evt);
#endif
}
void typec_set_debug_acc_sink(struct port *port)
{
    typec_set_state(port, DebugAccessorySink, 0);
    typec_set_sink(port);
    timer_disable(&port->timers[TC_TIMER]);
}

void typec_set_attach_wait_acc(struct port *port)
{
    typec_set_state(port, AttachWaitAccessory, 0);
    fusbdev_tcpd_set_cc(port->tcpd_device, CC1 | CC2, CC_TERM_RP, CC_RP_1p5A, DRP_MODE_NOCHANGE);
    typec_set_source(port);
    timer_disable(&port->timers[TC_TIMER]);
}


void typec_set_powered_acc(struct port *port)
{
    typec_set_state(port, PoweredAccessory, 0);
    fusbdev_tcpd_set_orient(port->tcpd_device, port->cc_pin);
    /* Note that sourcing VBus for powered accessories is not supported in
     * Type-C 1.2, but is done here because not all accessories work without it.
     */
    if (port->vif->Sources_VBus_For_Powered_Accessory)
    {
        dev_tcpd_port_vbus_src(port, VBUS_5V, CC_RP_CURRENT(port->src_current), pdoTypeFixed);
    }
    /* Turn on VConn */
    if (port->vif->Type_C_Sources_VCONN)
    {
        dev_tcpd_port_set_vconn(port, true);
    }
    typec_set_source(port);
    policy_pd_enable(port, true);
    timer_start(&port->timers[TC_TIMER], ktAMETimeout);
}

void typec_set_unsupported_acc(struct port *port)
{
    /* Vbus was enabled in PoweredAccessory - disable it here. */
    if (port->vif->Sources_VBus_For_Powered_Accessory)
    {
        dev_tcpd_port_vbus_src(port, VBUS_OFF, VBUS_OFF, 0);
    }
    if (port->vif->Type_C_Sources_VCONN)
    {
        dev_tcpd_port_set_vconn(port, false);
    }
    typec_set_state(port, UnsupportedAccessory, 0);
    fusbdev_tcpd_set_cc(port->tcpd_device, CC1 | CC2, CC_TERM_RP, CC_RP_DEF, DRP_MODE_NOCHANGE);
    typec_set_source(port);
#if CONFIG_ENABLED(EXTENDED_EVENTS)
    event_notify(EVENT_UNSUPPORTED_ACCESSORY, port->dev, 0);
#endif
    timer_disable(&port->timers[TC_TIMER]);
}
#endif
#if (CONFIG_ENABLED(DRP))
void typec_set_try_sink(struct port *port)
{
    typec_set_state(port, TrySink, 0);
    typec_set_sink(port);
    fusbdev_tcpd_set_cc(port->tcpd_device, CC1 | CC2, CC_TERM_RD, CC_RP_NOCHANGE,
                        DRP_MODE_NOCHANGE);
    timer_start(&port->timers[TC_TIMER], ktDRPTry);
}

void typec_set_try_wait_sink(struct port *port)
{
    policy_pd_disable(port);
    typec_set_state(port, TryWaitSink, 0);
    typec_set_sink(port);
    dev_tcpd_port_set_vconn(port, false);
    dev_tcpd_port_vbus_src(port, VBUS_OFF, VBUS_OFF, 0);
    fusbdev_tcpd_set_vbus_alarm(port->tcpd_device, 0, VBUS_LOW_THRESHOLD(VBUS_5V));
    fusbdev_tcpd_set_cc(port->tcpd_device, CC1 | CC2, CC_TERM_RD, CC_RP_NOCHANGE,
                        DRP_MODE_NOCHANGE);
    timer_start(&port->timers[TC_TIMER], ktCCDebounce);
}

void typec_set_try_source(struct port *port)
{
    typec_set_state(port, TrySource, 0);
    typec_set_source(port);
    fusbdev_tcpd_set_cc(port->tcpd_device, CC1 | CC2, CC_TERM_RP, port->src_current,
                        DRP_MODE_NOCHANGE);
    fusbdev_tcpd_set_vbus_alarm(port->tcpd_device, VBUS_VSAFE0_ALARM, 0);
    timer_start(&port->timers[TC_TIMER], ktDRPTry);
    timer_start(&port->timers[TC_TRY_TIMEOUT_TIMER], ktTryTimeout);
}

void typec_set_try_wait_source(struct port *port)
{
    typec_set_state(port, TryWaitSource, 0);
    typec_set_source(port);
    fusbdev_tcpd_set_cc(port->tcpd_device, CC1 | CC2, CC_TERM_RP, port->src_current,
                        DRP_MODE_NOCHANGE);
    timer_start(&port->timers[TC_TIMER], ktDRPTry);
}

#endif
/*
 *  State Machine Configuration
 */
void typec_set_disabled(struct port *port)
{
    typec_set_state(port, Disabled, 0);
    timer_disable(&port->timers[TC_TIMER]);
    typec_clear_state(port);
    /* Present Open/Open */
    fusbdev_tcpd_set_cc(port->tcpd_device, CC1 | CC2, CC_TERM_OPEN, CC_RP_NOCHANGE,
                        DRP_MODE_NOCHANGE);
#if CONFIG_ENABLED(POWER_SHARING) || CONFIG_ENABLED(LEGACY_CHARGING)
    event_notify(EVENT_TC_DETACHED, port->dev, 0);
#endif
}

void typec_set_error_recovery(struct port *port)
{
    typec_set_state(port, ErrorRecovery, 0);
    typec_clear_state(port);
    timer_start(&port->timers[TC_TIMER], ktErrorRecovery);
    fusbdev_tcpd_cc_events_enable(port->tcpd_device, false);
    /* Present Open/Open for tErrorRecovery */
    fusbdev_tcpd_set_cc(port->tcpd_device, CC1 | CC2, CC_TERM_OPEN, CC_RP_NOCHANGE,
                        DRP_MODE_NOCHANGE);
}

void typec_set_audio_acc(struct port *port)
{
    typec_set_state(port, AudioAccessory, 0);
    typec_set_source(port);
    /* TODO - Add events CC Orientation, Power role and etc... */
    timer_disable(&port->timers[TC_TIMER]);
}

void typec_set_unattached(struct port *port)
{
    typec_clear_state(port);
    timer_disable(&port->timers[TC_TIMER]);
#if CONFIG_ENABLED(POWER_SHARING) || CONFIG_ENABLED(LEGACY_CHARGING)
    event_notify(EVENT_TC_DETACHED, port->dev, 0);
#endif
    fusbdev_tcpd_cc_events_enable(port->tcpd_device, true);
    dev_tcpd_port_vbus_init(port, false);
    //allow for updating of status after setting unattached to account for attach event being missed on vconn discharge timer
    gSleepInt = true;
    /* Reconfigure to look for the next connection */
    if (port->type == TypeC_DRP)
    {
#if CONFIG_ENABLED(DRP)
        /* Config as DRP */
        if (port->tc_state == AttachWaitSink)
        {
            fusbdev_tcpd_set_cc(port->tcpd_device, CC1 | CC2, CC_TERM_RP, port->src_current,
                                DRP_MODE_DRP);
        }
        else
        {
            fusbdev_tcpd_set_cc(port->tcpd_device, CC1 | CC2, CC_TERM_RD, port->src_current,
                                DRP_MODE_DRP);
        }
#endif
    }
    else if (port->type == TypeC_Source)
    {
        /* Config as a source with Rp-Rp */
        fusbdev_tcpd_set_cc(port->tcpd_device, CC1 | CC2, CC_TERM_RP, port->src_current,
                            DRP_MODE_DISABLED);
    }
    else
    {
        /* Config as a sink with Rd-Rd - toggle for acc if supported */
        if (port->acc_support)
        {
            if (port->vif->Type_C_Supports_Audio_Accessory
                && port->vif->Type_C_Supports_VCONN_Powered_Accessory)
            {
                fusbdev_tcpd_set_cc(port->tcpd_device, CC1 | CC2, CC_TERM_RP, port->src_current,
                                    DRP_MODE_SINKVPDACC);
            }
            else if (port->vif->Type_C_Supports_Audio_Accessory)
            {
                fusbdev_tcpd_set_cc(port->tcpd_device, CC1 | CC2, CC_TERM_RP, port->src_current,
                                    DRP_MODE_SINKACC);
            }
            else if (port->vif->Type_C_Supports_VCONN_Powered_Accessory)
            {
                fusbdev_tcpd_set_cc(port->tcpd_device, CC1 | CC2, CC_TERM_RD, port->src_current,
                                    DRP_MODE_SINKVPD);
            }
        }
        else
        {
            fusbdev_tcpd_set_cc(port->tcpd_device, CC1 | CC2, CC_TERM_RD, CC_RP_NOCHANGE,
                                DRP_MODE_DISABLED);
        }
    }

    typec_set_state(port, Unattached, 0);
}

/**
 * @brief Starts two timer for cc and pd debounce on both cc and vconn pin.
 */
static void typec_debounce_cc(struct port *port)
{
    enum cc_stat_t cc1 = fusbdev_tcpd_cc_stat(port->tcpd_device, CC1);
    enum cc_stat_t cc2 = fusbdev_tcpd_cc_stat(port->tcpd_device, CC2);
    enum cc_stat_t cctermcurrent;
    /* *** CC Pin *** */
    /* PD Debounce (filter out PD traffic that could look like CC changes) */
    cctermcurrent = port->cc_pin == CC2 ? cc2 : cc1;
    if (port->cc_term_raw != cctermcurrent)
    {
        /* If our latest value has changed, update and reset timers */
        port->cc_term_raw = cctermcurrent;
        timer_start(&port->timers[CC_PDDBNC_TIMER], ktPDDebounce);
        timer_start(&port->timers[CC_DBNC_TIMER], ktCCDebounce);
    }
    /* If our debounce timer has expired, record the latest values */
    if (timer_expired(&port->timers[CC_PDDBNC_TIMER]))
    {
        port->cc_term_pd_debounce = port->cc_term_raw;
        timer_disable(&port->timers[CC_PDDBNC_TIMER]);
    }
    /* If our debounce timer has expired, record the latest values */
    if (timer_expired(&port->timers[CC_DBNC_TIMER]))
    {
        port->cc_term_cc_debounce = port->cc_term_raw;
        timer_disable(&port->timers[CC_DBNC_TIMER]);
    }
    /* Vconn Pin *** */
    /* PD Debounce (filter out PD traffic that could look like CC changes) */
    cctermcurrent = port->cc_pin == CC2 ? cc1 : cc2;
    if (port->vc_term_raw != cctermcurrent)
    {
        /* If our latest value has changed, update and reset timers */
        port->vc_term_raw = cctermcurrent;
        timer_start(&port->timers[VC_PDDBNC_TIMER], ktPDDebounce);
        timer_start(&port->timers[VC_DBNC_TIMER], ktCCDebounce);
    }
    /* If our debounce timer has expired, record the latest values */
    if (timer_expired(&port->timers[VC_PDDBNC_TIMER]))
    {
        port->vc_term_pd_debounce = port->vc_term_raw;
        timer_disable(&port->timers[VC_PDDBNC_TIMER]);
    }
    /* If our debounce timer has expired, record the latest values */
    if (timer_expired(&port->timers[VC_DBNC_TIMER]))
    {
        port->vc_term_cc_debounce = port->vc_term_raw;
        timer_disable(&port->timers[VC_DBNC_TIMER]);
    }
}
#if (CONFIG_ENABLED(SRC))
static void typec_state_source_unattached_wait(struct port *port)
{
    /* Discharging VConn - wait for vconndischarge timeout */
    if (timer_expired(&port->timers[TC_TIMER]))
    {
        typec_set_unattached(port);
    }
}

static void typec_state_source_attach_wait(struct port *port)
{
    typec_set_cc_pin(port);
    typec_debounce_cc(port);
    if (port->vif->Type_C_Supports_Audio_Accessory && port->cc_term_cc_debounce == CC_STAT_RA
        && port->vc_term_cc_debounce == CC_STAT_RA)
    {
        /* If both pins are Ra, it's an audio accessory */
        typec_set_audio_acc(port);
    }
    else if (port->cc_term_raw == CC_STAT_SRCOPEN
             && (port->vc_term_raw == CC_STAT_SRCOPEN || port->vc_term_raw == CC_STAT_RA))
    {
        typec_set_unattached(port);
    }
    else if (port_vbus_vsafe0v(port))
    {
        if (port->vif->Type_C_Is_Debug_Target_SRC && port->cc_term_cc_debounce == CC_STAT_RD
            && port->vc_term_cc_debounce == CC_STAT_RD)
        {
            /* If both pins are Rd, it's a debug accessory */
            typec_set_unoriented_debug_acc_source(port);
        }
        else if (port->cc_term_cc_debounce == CC_STAT_RD
                 && (port->vc_term_cc_debounce == CC_STAT_SRCOPEN
                     || port->vc_term_cc_debounce == CC_STAT_RA))
        {
#if (CONFIG_ENABLED(DRP) && CONFIG_ENABLED(SNK))
            if (port->snk_preferred)
            {
                /* Sink Attached, but go to Try.SNK */
                typec_set_try_sink(port);
            }
            else
            {
                /* Sink Attached */
                typec_set_attached_source(port);
            }
#else
            typec_set_attached_source(port);
#endif
        }
    }
    else
    {
        fusbdev_tcpd_set_vbus_alarm(port->tcpd_device, VBUS_VSAFE0_ALARM, 0);
    }
}

static void typec_state_source_attached(struct port *port)
{
    typec_debounce_cc(port);
    switch (port->tc_substate)
    {
        case 0:
            /* Look for CC detach */
            if ((port->cc_term_pd_debounce == CC_STAT_SRCOPEN) && (port->is_pr_swap == false))
            {
                if ((CONFIG_ENABLED(DRP)) && (port->type == TypeC_DRP))
                {
                    if (port->src_preferred == true && CONFIG_ENABLED(SNK))
                    {
                        typec_set_try_wait_sink(port);
                    }
                    else
                    {
                        /* Start the disconnect process */
                        typec_set_state(port, AttachedSource, 1);
                        port->pd_active = false;
                        /* Set Rd terminations for transition to unattached.snk */
                        fusbdev_tcpd_set_cc(port->tcpd_device, CC1 | CC2, CC_TERM_RD,
                                            CC_RP_NOCHANGE, DRP_MODE_NOCHANGE);
                        fusbdev_tcpd_set_vbus_alarm(port->tcpd_device, VBUS_VSAFE0_ALARM, 0);
                        dev_tcpd_port_vbus_src(port, VBUS_OFF, VBUS_OFF, 0);
                        /* VBUS discharging from Source Attached when detached is detected
					 * is not implied from specs, will be revisited - TODO!*/
                        fusbdev_tcpd_set_vbus_disch(port->tcpd_device, VBUS_DISCH_DEFAULT);
                        dev_tcpd_port_set_vconn(port, false);
                    }
                }
                else
                {
                    typec_set_state(port, AttachedSource, 1);
                    port->pd_active = false;
                    fusbdev_tcpd_set_vbus_alarm(port->tcpd_device, VBUS_VSAFE0_ALARM, 0);
                    dev_tcpd_port_vbus_src(port, VBUS_OFF, VBUS_OFF, 0);
                    fusbdev_tcpd_set_vbus_disch(port->tcpd_device, VBUS_DISCH_DEFAULT);
                    dev_tcpd_port_set_vconn(port, false);
                }
            }
            break;
        case 1:
            if (port_vbus_vsafe0v(port))
            {
                if (CONFIG_ENABLED(DRP))
                {
                    typec_set_unattached(port);
                }
                else
                {
                    /*revisit this part when VCONN can be disabled while maintaining vconn source
				 * current implementation do not check the following:
				   - if vconn enable or disable
				   - if vconn source
				   - if not vconn source, there was a prior vconn swap
				 TODO!*/
                    typec_set_unattached_wait_source(port);
                }
            }
            else
            {
                fusbdev_tcpd_set_vbus_alarm(port->tcpd_device, VBUS_VSAFE0_ALARM, 0);
            }
            break;
        default:
            typec_set_error_recovery(port);
            break;
    }
}
#endif
#if (CONFIG_ENABLED(SRC))
static void typec_state_source_unoriented_dbgacc(struct port *port)
{
    typec_debounce_cc(port);
    if (port->cc_term_raw == CC_STAT_SRCOPEN || port->vc_term_raw == CC_STAT_SRCOPEN)
    {
        /* Open detected? */
        typec_set_unattached(port);
    }
    else if (port->cc_term_raw != port->vc_term_raw)
    {
        /* Check for orientation */
        if (port->cc_term_raw > port->vc_term_raw)
        {
            port->cc_pin = CC1;
        }
        else
        {
            port->cc_pin = CC2;
        }
        fusbdev_tcpd_set_orient(port->tcpd_device, port->cc_pin);
        typec_set_oriented_debug_acc_source(port);
    }
}

static void typec_state_source_oriented_dbgacc(struct port *port)
{
    typec_debounce_cc(port);
    if (port->cc_term_raw == CC_STAT_SRCOPEN || port->vc_term_raw == CC_STAT_SRCOPEN)
    {
        /* Detach */
        typec_set_unattached(port);
    }
}
#endif

#if (CONFIG_ENABLED(SNK))
static void typec_state_sink_attach_wait_acc(struct port *port)
{
    typec_set_cc_pin(port);
    typec_debounce_cc(port);
    /* There is a chance that RaRa or RdRd is detected and
     * one termination changes.  Swap if necessary.
     */
    if (port->cc_term_raw == CC_STAT_RA && port->vc_term_raw == CC_STAT_RD)
    {
        port->cc_pin = port->cc_pin == CC1 ? CC2 : CC1;
        fusbdev_tcpd_set_orient(port->tcpd_device, port->cc_pin);
        typec_debounce_cc(port);
    }
    if (port->cc_term_cc_debounce == CC_STAT_RA && port->vc_term_cc_debounce == CC_STAT_RA
        && port->vif->Type_C_Supports_Audio_Accessory)
    {
        /* If both pins are Ra, it's an audio accessory */
        typec_set_audio_acc(port);
    }
    else if (port->cc_term_cc_debounce == CC_STAT_RD && port->vc_term_cc_debounce == CC_STAT_RA
             && port->vif->Type_C_Supports_VCONN_Powered_Accessory)
    {
        /* If Rd-Ra, Powered Accessory (VPD) */
        typec_set_powered_acc(port);
    }
    else if (port->cc_term_cc_debounce == CC_STAT_SRCOPEN
             || port->vc_term_cc_debounce == CC_STAT_SRCOPEN)
    {
        typec_set_unattached(port);
    }
}
static void typec_state_sink_attach_wait(struct port *port)
{
    typec_set_cc_pin(port);
    typec_debounce_cc(port);
    if (port->cc_term_pd_debounce == CC_STAT_SNKOPEN
        && port->vc_term_pd_debounce == CC_STAT_SNKOPEN)
    {
        /* Open detected? */
        typec_set_unattached(port);
    }
    else if (port_vbus_sink_min(port))
    {
#if (CONFIG_ENABLED(DRP))
        if (port->vif->Type_C_Is_Debug_Target_SNK
            && (port->cc_term_cc_debounce == CC_STAT_RPUSB
                || port->cc_term_cc_debounce == CC_STAT_RP1p5
                || port->cc_term_cc_debounce == CC_STAT_RP3p0)
            && (port->vc_term_cc_debounce == CC_STAT_RPUSB
                || port->vc_term_cc_debounce == CC_STAT_RP1p5
                || port->vc_term_cc_debounce == CC_STAT_RP3p0))
        {
            /* If both pins are Rp, it's a debug accessory */
            typec_set_debug_acc_sink(port);
        }
        else if ((port->cc_term_cc_debounce == CC_STAT_RPUSB
                  || port->cc_term_cc_debounce == CC_STAT_RP1p5
                  || port->cc_term_cc_debounce == CC_STAT_RP3p0))
        {
            if (CONFIG_ENABLED(DRP) && port->type == TypeC_DRP && port->src_preferred == true)
            {
                /* Source Attached, but go to Try.SRC */
                typec_set_try_source(port);
            }
            else
            {
                /* Source Attached */
                typec_set_attached_sink(port);
            }
        }
#else
        if ((port->cc_term_cc_debounce == CC_STAT_RPUSB
          || port->cc_term_cc_debounce == CC_STAT_RP1p5
          || port->cc_term_cc_debounce == CC_STAT_RP3p0))
		{
			/* Source Attached */
			typec_set_attached_sink(port);
		}
#endif
    }
    else
    {
        fusbdev_tcpd_set_vbus_alarm(port->tcpd_device, 0, VBUS_LOW_THRESHOLD(VBUS_5V));
    }
}

static void typec_state_sink_attached(struct port *port)
{
    /* Debounce both lines */

    static int k             = 0;
    unsigned   FixedVoltage  = 0;
    unsigned   PPSRDOVoltage = 0;
    if (port->is_pr_swap == false)
    {
        typec_debounce_cc(port);
    }
    /* A VBus disconnect should generate an interrupt to wake us up */
    //    if (!port_vbus_above(port, port->sink_selected_voltage) && port->is_pr_swap == false
    //        && port->is_hard_reset == false)
    //    {
    //        /* Start the disconnect process */
    //        typec_set_unattached(port);
    //        return;
    //    }
    FixedVoltage  = port->sink_selected_voltage;
    FixedVoltage = FixedVoltage > 21000 ? 21000 : FixedVoltage;
    uint32_t vbus = fusbdev_tcpd_vbus_value(port->dev->tcpd_device);
    if (port->PDO_Type == pdoTypeFixed && !port->is_pr_swap
        && !port->is_hard_reset
	    && !(port->policy_state == PE_SNK_Select_Capability || port->policy_state == PE_SNK_Transition_Sink)
#if CONFIG_ENABLED(EPR)
		&& !(port->policy_state == PE_SNK_Send_EPR_Mode_Exit || port->policy_state_prev == PE_SNK_Send_EPR_Mode_Exit)
#endif
        && 1)
    {
        if (((port->sink_selected_voltage > VBUS_5V)
                && (vbus < (VBUS_PD_FIXED_DISC(FixedVoltage))))
            || ((port->sink_selected_voltage <= VBUS_5V)
                   && ( vbus < (VBUS_5V_DISC))))
            {
        		if (timer_disabled(&port->timers[UVP_DBNC_TIMER]))
        		{
        			timer_start(&port->timers[UVP_DBNC_TIMER], 100U);
        		}
        		else if (timer_expired(&port->timers[UVP_DBNC_TIMER]))
				{
					fusbdev_tcpd_set_vbus_alarm(port->tcpd_device, port->sink_selected_voltage, 0);
					typec_set_unattached(port);
				}
            }
    }
    else if (FixedVoltage == 0 && !port_vbus_in_range(port, VBUS_PD_FIXED_DISC(5000), ~0))
    {
    	if (timer_disabled(&port->timers[UVP_DBNC_TIMER]))
		{
			timer_start(&port->timers[UVP_DBNC_TIMER], 100U);
		}
		else if (timer_expired(&port->timers[UVP_DBNC_TIMER]))
		{
			fusbdev_tcpd_set_vbus_alarm(port->tcpd_device, port->sink_selected_voltage, 0);
			typec_set_unattached(port);
		}
    }
    else if (timer_expired(&port->timers[UVP_DBNC_TIMER]))
    {
    	timer_disable(&port->timers[UVP_DBNC_TIMER]);
    }
	/* If using PD, sink can monitor CC as well as VBUS to allow detach during a
	* hard rest */
    if (port->pd_active == true && !port->is_pr_swap
        && port->cc_term_pd_debounce == CC_STAT_SNKOPEN)
    {
        typec_set_unattached(port);
        return;
    }

    /* Type-C only connections can idle here -
     * otherwise let the PE handle it */
    if (port->pd_active == false)
    {
    }
}

static void typec_state_sink_dbgacc(struct port *port)
{
    typec_debounce_cc(port);
    if (!port_vbus_above(port, port->sink_selected_voltage))
    {
        typec_set_unattached(port);
    }
    else if (port->cc_term_raw != port->vc_term_raw && port->pd_active == false)
    {
        /* Check for orientation */
        if (port->cc_term_raw > port->vc_term_raw)
        {
            port->cc_pin = CC1;
        }
        else
        {
            port->cc_pin = CC2;
        }
        fusbdev_tcpd_set_orient(port->tcpd_device, port->cc_pin);
        policy_pd_enable(port, false);
    }
}

static void typec_state_powered_acc(struct port *port)
{
    typec_debounce_cc(port);
    if (port->cc_term_raw == CC_STAT_SRCOPEN)
    {
        /* Transition to Unattached.Snk when monitored CC pin is Open */
        typec_set_unattached(port);
    }
#if MODAL_OPERATION_SUPPORTED
    else if (port->mode_entered == true)
    {
        timer_disable(&port->timers[TC_TIMER]);
    }
#endif
    else if (timer_expired(&port->timers[TC_TIMER]))
    {
#if (CONFIG_ENABLED(DRP))
        timer_disable(&port->timers[TC_TIMER]);
        /* Time out and not in alternate mode */
        if (port->policy_has_contract)
        {
            typec_set_unsupported_acc(port);
        }
        else
        {
            dev_tcpd_port_set_vconn(port, false);
            typec_set_try_sink(port);
        }
#else
        typec_set_unsupported_acc(port);
#endif
    }
}

static void typec_state_unsupported_acc(struct port *port)
{
    typec_debounce_cc(port);
    if (port->cc_term_raw == CC_STAT_SRCOPEN)
    {
        /* Transition to Unattached.Snk when monitored CC pin is Open */
        typec_set_unattached(port);
    }
}
#endif
#if (CONFIG_ENABLED(DRP))
static void typec_state_source_try_wait_sink(struct port *port)
{
    typec_debounce_cc(port);
    if (port->cc_term_pd_debounce == CC_STAT_SNKOPEN
        && port->vc_term_pd_debounce == CC_STAT_SNKOPEN)
    {
        /* Open detected? */
        typec_set_unattached(port);
    }
    else if (timer_expired(&port->timers[TC_TIMER]) && port_vbus_vsafe5v(port))
    {
        timer_disable(&port->timers[TC_TIMER]);
        typec_set_attached_sink(port);
    }
    else
    {
        fusbdev_tcpd_set_vbus_alarm(port->tcpd_device, 0, VBUS_LOW_THRESHOLD(VBUS_5V));
    }
}

static void typec_state_sink_try_source(struct port *port)
{
    typec_set_cc_pin(port);
    typec_debounce_cc(port);
    if (port->cc_term_pd_debounce == CC_STAT_RD
        && (port->vc_term_pd_debounce == CC_STAT_SRCOPEN
            || port->vc_term_pd_debounce == CC_STAT_RA))
    {
        /* If Rd is detected on exactly one of the CC pins */
        typec_set_attached_source(port);
    }
    else if (port_vbus_vsafe5v(port) && timer_expired(&port->timers[TC_TIMER]))
    {
        timer_disable(&port->timers[TC_TIMER]);
        /* Transition after tDRPTry if SRC.Rd is not detected and Vbus within vSafe0V */
        typec_set_try_wait_sink(port);
    }
    else
    {
        if (timer_expired(&port->timers[TC_TRY_TIMEOUT_TIMER]))
        {
            timer_disable(&port->timers[TC_TRY_TIMEOUT_TIMER]);
            /* Transition after tTryTimeout if SRC.Rd is not detected */
            typec_set_try_wait_sink(port);
        }
    }
}
static void typec_state_source_try_sink(struct port *port)
{
    typec_set_cc_pin(port);
    typec_debounce_cc(port);
    switch (port->tc_substate)
    {
        case 0:
            if (timer_expired(&port->timers[TC_TIMER]))
            {
                if (port->type == TypeC_Sink)
                {
                    timer_start(&port->timers[TC_TIMER], ktDRPTryWait);
                }
                else if (CONFIG_ENABLED(DRP) && port->type == TypeC_DRP)
                {
                    timer_start(&port->timers[TC_TIMER], ktTryCCDebounce);
                }
                typec_reset_debounce(port);
                typec_set_state(port, TrySink, 1);
                fusbdev_tcpd_set_vbus_alarm(port->tcpd_device, 0, VBUS_LOW_THRESHOLD(VBUS_5V));
            }
            break;
        case 1:
            if (port_vbus_vsafe5v(port)
                && (port->cc_term_pd_debounce == CC_STAT_RPUSB
                    || port->cc_term_pd_debounce == CC_STAT_RP1p5
                    || port->cc_term_pd_debounce == CC_STAT_RP3p0)
                && port->vc_term_pd_debounce == CC_STAT_SNKOPEN)
            {
                /* If the CC pin is Rp for at least tTryCCDebounce... */
                typec_set_attached_sink(port);
            }
            else if (port->type == TypeC_Sink && timer_expired(&port->timers[TC_TIMER]))
            {
                /* TODO Make sure this transition exists for sink+acc*/
                timer_disable(&port->timers[TC_TIMER]);
                typec_set_unsupported_acc(port);
            }
            else if (CONFIG_ENABLED(DRP) && port->type == TypeC_DRP
                     && timer_expired(&port->timers[TC_TIMER]))
            {
                typec_set_try_wait_source(port);
            }
            else
            {
                fusbdev_tcpd_set_vbus_alarm(port->tcpd_device, 0, VBUS_LOW_THRESHOLD(VBUS_5V));
            }
            break;
        default:
            typec_set_error_recovery(port);
            break;
    }
}

static void typec_state_sink_try_wait_source(struct port *port)
{
    typec_debounce_cc(port);
    if (port_vbus_vsafe0v(port) && port->cc_term_pd_debounce == CC_STAT_RD
        && (port->vc_term_pd_debounce == CC_STAT_RA
            || port->vc_term_pd_debounce == CC_STAT_SRCOPEN))
    {
        typec_set_attached_source(port);
    }
    else if (timer_expired(&port->timers[TC_TIMER]) && port->cc_term_pd_debounce != CC_STAT_RD
             && port->vc_term_pd_debounce != CC_STAT_RD)
    {
        /* Go to Unattached.SNK after tDPRTry if neither pins are in SRC.Rd state */
        typec_set_unattached(port);
    }
}
#endif
static void typec_state_audio_acc(struct port *port)
{
    typec_debounce_cc(port);
    if (port->cc_term_cc_debounce == CC_STAT_SRCOPEN
        || port->vc_term_cc_debounce == CC_STAT_SRCOPEN)
    {
        typec_set_unattached(port);
    }
}



static void typec_state_disabled(struct port *port)
{
    /* Wait for detach to reset port state */
    enum cc_t pin = port->cc_pin;
#if (DEVICE_TYPE == FUSB15201P)
    if (fusbdev_tcpd_cc_stat(port->tcpd_device, pin) == CC_STAT_SRCOPEN)
    {
        typec_reset(port);
        policy_reset(port);
        protocol_reset(port);
        port->type = port->vif->Type_C_State_Machine;
        typec_set_unattached(port);
    }
#endif
}

static void typec_state_error_recovery(struct port *port)
{
#if (CONFIG_ENABLED(FAULT_PROTECTION))
	if (!port->fault_active && timer_expired(&port->timers[TC_TIMER]))
#else
    if (timer_expired(&port->timers[TC_TIMER]))
#endif
    {
        timer_disable(&port->timers[TC_TIMER]);
        typec_set_unattached(port);
    }
}

static void typec_state_unattached(struct port *port)
{
    /* Check if attached */
    enum drp_stat_t drp_status = {0};
    enum cc_stat_t  cc1_stat   = fusbdev_tcpd_cc_stat(port->tcpd_device, CC1);
    enum cc_stat_t  cc2_stat   = fusbdev_tcpd_cc_stat(port->tcpd_device, CC2);
#if CONFIG_ENABLED(DRP)
    if (port->type == TypeC_DRP)
    {
        drp_status = fusbdev_tcpd_drp_stat(port->tcpd_device);
        if (drp_status == DRP_ACTIVE)
        {
            return;
        }
        else if (drp_status == DRP_SOURCE && CONFIG_ENABLED(SRC))
        {
            port->source_or_sink = Source;
            typec_set_attach_wait_source(port);
        }
        else if (drp_status == DRP_SINK && CONFIG_ENABLED(SNK))
        {
            typec_set_attach_wait_sink(port);
        }
        else
        {
            typec_set_unattached(port);
        }
    }
    else if (port->type == TypeC_Source && CONFIG_ENABLED(SRC))
    {
        port->source_or_sink = Source;
        if (cc1_stat == CC_STAT_RD || cc2_stat == CC_STAT_RD)
        {
            typec_set_attach_wait_source(port);
        }
        else if (cc1_stat == CC_STAT_RA && cc2_stat == CC_STAT_RA)
        {
            typec_set_attach_wait_source(port);
        }
    }
    else if (port->type == TypeC_Sink && CONFIG_ENABLED(SNK))
    {
        port->source_or_sink = Sink;
        if (cc1_stat < CC_STAT_SNKOPEN && cc1_stat >= CC_STAT_RPUSB)
        {
            /* Detected Rp */
            typec_set_attach_wait_sink(port);
        }
        else if (cc2_stat < CC_STAT_SNKOPEN && cc2_stat >= CC_STAT_RPUSB)
        {
            /* Detected Rp */
            typec_set_attach_wait_sink(port);
        }
        else if (port->vif->Type_C_Supports_Audio_Accessory && cc1_stat == CC_STAT_RA
                 && cc2_stat == CC_STAT_RA)
        {
            typec_set_attach_wait_acc(port);
        }
        else if (port->vif->Type_C_Supports_VCONN_Powered_Accessory && cc1_stat == CC_STAT_RD
                 && cc2_stat == CC_STAT_RA)
        {
            typec_set_attach_wait_acc(port);
        }
        else if (port->vif->Type_C_Supports_VCONN_Powered_Accessory && cc1_stat == CC_STAT_RA
                 && cc2_stat == CC_STAT_RD)
        {
            typec_set_attach_wait_acc(port);
        }
    }
    else
    {
        typec_set_unattached(port);
    }
#elif (CONFIG_ENABLED(SRC))
    port->source_or_sink = Source;
    if (cc1_stat == CC_STAT_RD || cc2_stat == CC_STAT_RD)
    {
        typec_set_attach_wait_source(port);
    }
    else if (cc1_stat == CC_STAT_RA && cc2_stat == CC_STAT_RA)
    {
        typec_set_attach_wait_source(port);
    }
#elif (CONFIG_ENABLED(SNK))
    port->source_or_sink = Sink;
	if (cc1_stat < CC_STAT_SNKOPEN && cc1_stat >= CC_STAT_RPUSB)
	{
		/* Detected Rp */
		typec_set_attach_wait_sink(port);
	}
	else if (cc2_stat < CC_STAT_SNKOPEN && cc2_stat >= CC_STAT_RPUSB)
	{
		/* Detected Rp */
		typec_set_attach_wait_sink(port);
	}
	else if (port->vif->Type_C_Supports_Audio_Accessory && cc1_stat == CC_STAT_RA
			 && cc2_stat == CC_STAT_RA)
	{
		typec_set_attach_wait_acc(port);
	}
	else if (port->vif->Type_C_Supports_VCONN_Powered_Accessory && cc1_stat == CC_STAT_RD
			 && cc2_stat == CC_STAT_RA)
	{
		typec_set_attach_wait_acc(port);
	}
	else if (port->vif->Type_C_Supports_VCONN_Powered_Accessory && cc1_stat == CC_STAT_RA
			 && cc2_stat == CC_STAT_RD)
	{
		typec_set_attach_wait_acc(port);
	}
#endif
    if (port->tc_state != Unattached)
    {
        /* Detected start so initialize cc and vbus */
        typec_set_cc_pin(port);
        typec_debounce_cc(port);
        dev_tcpd_port_vbus_init(port, true);
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
        fusbdev_tcpd_set_drp(port->tcpd_device, DRP_MODE_DISABLED);
#endif
    }
}

const void (*typec_state_run[NUM_TC_STATES])(struct port * port) = {
		typec_state_disabled,
		typec_state_error_recovery,
		typec_state_unattached,
#if (CONFIG_ENABLED(SRC))
		typec_state_source_attach_wait,
		typec_state_source_attached,
		typec_state_source_unattached_wait,
		typec_state_source_unoriented_dbgacc,
		typec_state_source_oriented_dbgacc,
#endif
#if (CONFIG_ENABLED(SNK))
		typec_state_sink_attach_wait,
		typec_state_sink_attached,
		typec_state_sink_attach_wait_acc,
		typec_state_sink_dbgacc,
		typec_state_powered_acc,
		typec_state_unsupported_acc,
#endif
#if (CONFIG_ENABLED(DRP))
		typec_state_sink_try_source,
		typec_state_source_try_wait_sink,
		typec_state_source_try_sink,
		typec_state_sink_try_wait_source,
#endif
		typec_state_audio_acc,
		typec_state_error_recovery, //delayunattached unsupported
		typec_state_error_recovery, //illegalcable unsupported
};
/* Entry point to the Type-C state machine */
void typec_sm(struct port *port)
{
    if (!port->tc_enabled)
    {
        return;
    }
    port->event = false;
    /* PD State Machines */
    if (port->pd_active)
    {
        protocol_engine(port);
        policy_engine(port);
    }
    typec_state_run[port->tc_state](port);
}
