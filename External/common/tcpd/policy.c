/*******************************************************************************
 * @file     policy.c
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
 * policy.c
 *
 * Implements the Policy state machine functions
 */
#include "extended_power_range.h"
#include "port_tcpd.h"
#include "dev_tcpd.h"
#include "vif_types.h"
#include "policy.h"
#include "typec.h"
#include "protocol.h"
#include "vdm.h"
#include "observer.h"
#include "logging.h"
#include "fusbdev_hal.h"

#define VALID_SOPMASK 7U
static void policy_state_send_hardreset(struct port *port);
static void policy_state_source_evaluate_vconn_swap(struct port *port);
static void policy_state_source_send_vconn_swap(struct port *port);
static void policy_state_source_evaluate_drswap(struct port *port);
static void policy_state_source_send_drswap(struct port *port);
static void policy_state_sink_evaluate_vconnswap(struct port *port);
static void policy_state_sink_send_vconnswap(struct port *port);
static void policy_state_sink_evaluate_drswap(struct port *port);
static void policy_state_sink_send_drswap(struct port *port);
static void policy_process_vdmrx(struct port *port);
static enum policy_state_t policy_state_ready(struct port * port)
{
#if (CONFIG_ENABLED(DRP))
    return port->policy_is_source ? PE_SRC_Ready : PE_SNK_Ready;
#elif (CONFIG_ENABLED(SRC))
    return PE_SRC_Ready;
#elif (CONFIG_ENABLED(SNK))
    return PE_SNK_Ready;
#endif
}

static enum policy_state_t policy_state_send_soft_reset(struct port * port)
{
#if (CONFIG_ENABLED(DRP))
    return port->policy_is_source ? PE_SRC_Send_Soft_Reset : PE_SNK_Send_Soft_Reset;
#elif (CONFIG_ENABLED(SRC))
    return PE_SRC_Send_Soft_Reset;
#elif (CONFIG_ENABLED(SNK))
    return PE_SNK_Send_Soft_Reset;
#endif
}

static enum policy_state_t policy_state_soft_reset(struct port * port)
{
#if (CONFIG_ENABLED(DRP))
    return port->policy_is_source ? PE_SRC_Soft_Reset : PE_SNK_Soft_Reset;
#elif (CONFIG_ENABLED(SRC))
    return PE_SRC_Send_Soft_Reset;
#elif (CONFIG_ENABLED(SNK))
    return PE_SNK_Send_Soft_Reset;
#endif
}

static enum policy_state_t policy_state_transition_to_default(struct port * port)
{
#if (CONFIG_ENABLED(DRP))
    return port->policy_is_source ? PE_SRC_Transition_To_Default : PE_SNK_Transition_To_Default;
#elif (CONFIG_ENABLED(SRC))
    return PE_SRC_Transition_To_Default;
#elif (CONFIG_ENABLED(SNK))
    return PE_SNK_Transition_To_Default;
#endif
}
static bool policy_state_is_ready(enum policy_state_t state)
{
#if (CONFIG_ENABLED(DRP))
    return state == PE_SRC_Ready || state == PE_SNK_Ready;
#elif (CONFIG_ENABLED(SRC))
    return state == PE_SRC_Ready;
#elif (CONFIG_ENABLED(SNK))
    return state == PE_SNK_Ready;
#endif
}

static void policy_state_send_drswap(struct port * port)
{
#if (CONFIG_ENABLED(DRP))
    if (port->policy_is_source)
    {
        policy_state_source_send_drswap(port);
    }
    else
    {
        policy_state_sink_send_drswap(port);
    }
#elif (CONFIG_ENABLED(SRC))
    policy_state_source_send_drswap(port);
#elif (CONFIG_ENABLED(SNK))
    policy_state_sink_send_drswap(port);
#endif
}

static void policy_state_evaluate_drswap(struct port * port)
{
#if (CONFIG_ENABLED(DRP))
    if (port->policy_is_source)
    {
        policy_state_source_evaluate_drswap(port);
    }
    else
    {
        policy_state_sink_evaluate_drswap(port);
    }
#elif (CONFIG_ENABLED(SRC))
    policy_state_source_evaluate_drswap(port);
#elif (CONFIG_ENABLED(SNK))
    policy_state_sink_evaluate_drswap(port);
#endif
}

static void policy_state_send_vconn_swap(struct port * port)
{
#if (CONFIG_ENABLED(DRP))
    if (port->policy_is_source)
    {
        policy_state_source_send_vconn_swap(port);
    }
    else
    {
        policy_state_sink_send_vconnswap(port);
    }
#elif (CONFIG_ENABLED(SRC))
    policy_state_source_send_vconn_swap(port);
#elif (CONFIG_ENABLED(SNK))
    policy_state_sink_send_vconnswap(port);
#endif
}

static void policy_state_evaluate_vconn_swap(struct port * port)
{
#if (CONFIG_ENABLED(DRP))
    if (port->policy_is_source)
    {
        policy_state_source_evaluate_vconn_swap(port);
    }
    else
    {
        policy_state_sink_evaluate_vconnswap(port);
    }
#elif (CONFIG_ENABLED(SRC))
    policy_state_source_evaluate_vconn_swap(port);
#elif (CONFIG_ENABLED(SNK))
    policy_state_sink_evaluate_vconnswap(port);
#endif
}

void policy_set_state_func(struct port *port, enum policy_state_t state, uint32_t sub)
{
    if (state != port->policy_state || port->policy_substate != sub)
    {
        port->policy_sending_message = false;
    }
    if (policy_state_is_ready(state))
    {
        /* Should indicate that we are done with an AMS */
        enum sink_tx_state_t oldSinkTx = dev_tcpd_port_sinktx_state(port);
        port->policy_ams_start         = false;
        port->policy_ams_active        = false;
#if (CONFIG_ENABLED(SRC))
        if (state == PE_SRC_Ready)
        {
            dev_tcpd_port_set_sinktx(port, SinkTxOK);
            /*disable running SinkTxNG timer in case transition to SRC_Ready is due to interruption*/
            timer_disable(&port->timers[SINKTX_TIMER]);
            /* update condition to make more sense. If we are not newly entering SRC_Ready, don't reset timer */
            if (port_pd_ver(port, SOP_SOP0) == PD_REV3)
            {
                if (port->policy_state != state && oldSinkTx == SinkTxNG)
                {
                    timer_start(&port->timers[RP_CHANGE_TIMER], ktRPValueChange);
                }
                else
                {
                    timer_disable(&port->timers[RP_CHANGE_TIMER]);
                }
            }
#if CONFIG_ENABLED(EPR)
            if (port->epr_mode && port->policy_state != state)
            {
                timer_start(&port->timers[EPR_KEEPALIVE_TIMER], ktEPRSourceKeepAlive);
            }
#endif
			if (port->source_is_apdo && port->policy_has_contract)
			{
				timer_start(&port->timers[PPS_TIMER], ktPPSTimeout);
			}
        }
#endif
#if (CONFIG_ENABLED(SNK))
        if (state == PE_SNK_Ready)
        {
#if CONFIG_ENABLED(EPR)
            if (port->epr_mode && port->policy_state != state)
            {
                timer_start(&port->timers[EPR_KEEPALIVE_TIMER], ktEPRSinkKeepAlive);
            }
#endif
        }
#endif
    }
    if (port->policy_state != state)
    {
        /* Check if need to set port->policy_ams variables */
        if (policy_state_is_ready(port->policy_state))
        {
            port->policy_ams_start = true;
#if CONFIG_ENABLED(EPR)
            timer_disable(&port->timers[EPR_KEEPALIVE_TIMER]);
#endif
            timer_disable(&port->timers[PPS_TIMER]);
        }
        /* Change in policy state only not sub-state. used for going back to last state */
        port->policy_state_prev = port->policy_state;
    }
    port->event           = true;
    port->policy_state    = state;
    port->policy_substate = sub;
}

static void pps_enable_monitor(struct port *port, bool enable)
{
    EVENT_PPS_MONITOR_ENABLE_T pps_monitor = {0};
    pps_monitor.arg.pps_active             = enable;
    pps_monitor.ret.success                = EVENT_STATUS_DEFAULT;
    event_notify(EVENT_PPS_MONITOR, port->dev, &pps_monitor);
}

static bool pps_set_alarm(struct port *port, bool enable)
{
    bool dpm_handler = false;
    if (enable)
    {
        EVENT_PPS_ALARM_T pps_alarm = {0};
        /*set pps shutdown range when in pps contract*/
        pps_alarm.arg.mv_low  = PERCENT(PPS_MIN_V, UV_PERCENT);
        pps_alarm.arg.mv_high = VBUS_LOW_THRESHOLD(port->usb_pd_contract.PPSRDO.OpVoltage * 20);
        pps_alarm.ret.success = EVENT_STATUS_DEFAULT;
        event_notify(EVENT_PPS_ALARM, port->dev, &pps_alarm);
        if (pps_alarm.ret.success == EVENT_STATUS_SUCCESS)
        {
            dpm_handler = true;
        }
    }
    return dpm_handler;
}

void policy_reset(struct port *port)
{
    unsigned i = 0;
    port->policy_sending_message = false;
    port->pd_rev[SOP_SOP0]       = port->dev->vif->PD_Specification_Revision;
    port->pd_rev[SOP_SOP1]       = port->pd_rev[SOP_SOP0];
    port->pd_rev[SOP_SOP2]       = port->pd_rev[SOP_SOP0];
    port->is_pr_swap             = false;
    port->is_vconn_swap          = false;
    port->pd_active              = false;
    port->pd_enabled             = port->dev->vif->USB_PD_Support;
    port->is_hard_reset          = false;
    port->alert_response         = false;

    port->policy_state      = PE_SRC_Disabled;
    port->policy_state_prev = PE_SRC_Disabled;
    port->policy_substate   = 0;
    port->policy_ams_start  = false;
    port->policy_ams_active = false;
    port->msgtx.value = 0;
    port->policy_is_source  = true;
    port->policy_is_dfp     = true;
    port->is_contract_valid = false;
#if (CONFIG_ENABLED(USB4))
    port->usb_mode       = false;
    port->msgtx.booltxmsg.enterusb = false;
    port->usb_supported_sopx = 0;
    port->enter_usb_rejected = 0;
    port->enter_usb_sent_sopx = 0;
    port->enter_usb_waited_sopx = 0;
    port->enter_usb_timeout = false;
    port->disc_id_sent_sopx = 0;
#if MODAL_OPERATION_SUPPORTED
    port->svid_enabled = false;
    port->disc_svid_received_sopx = 0;
    port->disc_modes_received_sopx = 0;
    port->disc_svid_sent_sopx = 0;
    port->disc_modes_sent_sopx = 0;
#endif 
#endif

    port->hard_reset_counter       = 0;
    port->caps_counter             = 0;
    port->policy_has_contract      = false;
    port->needs_goto_min           = false;
    port->renegotiate              = false;
    port->sink_selected_voltage    = VBUS_5V;
    port->partner_caps.object      = 0;
    port->partner_caps_available   = false;
    port->source_is_apdo           = false;
    port->bat_stat_ref             = 0;
    port->cbl_rst_state            = CBL_RST_DISABLED;
    port->current_limited          = false;
    port->vdm_check_cbl            = false;
    port->higher_cable_cap         = false;
    port->vdm_cbl_present          = false;
    port->discv_id_counter         = 0;
    port->cable_vdo_cable_type_ufp = VDM_ID_UFP_SOP_UNDEFINED;
    port->cable_vdo_cable_current = VDM_CABLE_MAX_CURRENT_3A; /* VDM_CABLE_MAX_CURRENT_3A; */
    port->cable_vdo_cable_voltage = VDM_CABLE_MAX_VOLTAGE_20V;
    port->cable_vdo_cable_highest_usb_speed = VDM_UFP_VDO_USB_2;
#if CONFIG_ENABLED(EPR)
    /* New EPR variables */
    port->captive_cable           = false;
    port->epr_capable_pdo         = false;
    port->epr_capable_rdo         = false;
    port->epr_capable_cable       = false;
    port->epr_mode                = false;
    port->epr_mode_message.object = 0;
    port->negotiate_epr_contract  = true;
#if (CONFIG_ENABLED(SNK))
    port->epr_try_entry = true;
    port->epr_try_exit  = false;
#endif
#endif

    pps_enable_monitor(port, false);
    timer_disable(&port->timers[POLICY_TIMER]);
    timer_disable(&port->timers[SINKTX_TIMER]);
    timer_disable(&port->timers[PD_RSP_TIMER]);
    timer_disable(&port->timers[SWAP_SOURCE_TIMER]);
    timer_disable(&port->timers[PPS_TIMER]);
    timer_disable(&port->timers[RP_CHANGE_TIMER]);
    timer_disable(&port->timers[DISCOVER_IDENTITY_TIMER]);
    timer_disable(&port->timers[UVP_DBNC_TIMER]);
#if CONFIG_ENABLED(USB4)
    timer_disable(&port->timers[VCONN_DISCHARGE_TIMER]);
    timer_disable(&port->timers[VCONN_POWER_TIMER]);
#endif

    for (i = 0; i < 7; ++i)
    {
        port->policy_pd_data[i].object = 0;
        port->caps_source[i].object    = 0;
    }

    if (CONFIG_ENABLED(SRC)
        || (CONFIG_ENABLED(SNK) && port->vif->Type_C_Supports_VCONN_Powered_Accessory))
    {
        port->caps_header_source.sop.NumDataObjects = port->vif->Num_Src_PDOs;
        port->caps_header_source.sop.PortDataRole   = 0;
        port->caps_header_source.sop.PortPowerRole  = 1;
        port->caps_header_source.sop.SpecRevision   = port->vif->PD_Specification_Revision;
        EVENT_PD_POP_SRC_CAPS_T caps_evt            = {0};
        caps_evt.arg.caps                           = port->caps_source;
        caps_evt.arg.caps_count                     = port->dev->vif->Num_Src_PDOs;
        caps_evt.ret.success                        = EVENT_STATUS_DEFAULT;
        event_notify(EVENT_PD_POPULATE_SRC_CAPS, port->dev, &caps_evt);
        if (caps_evt.ret.success != EVENT_STATUS_SUCCESS)
        {
            for (i = 0; i < port->dev->vif->Num_Src_PDOs; i++)
            {
                port->caps_source[i] = port->dev->vif->src_caps[i];
            }
        }
    }

    if (port->vif->Attempts_DiscvId_SOP_P_First)
    {
        port->vdm_check_cbl = true;
    }

#if CONFIG_ENABLED(VDM)  && !CONFIG_ENABLED(VDM_MINIMAL)
    timer_disable(&port->timers[VDM_TIMER]);
    port->vdm_success_ps    = PE_SRC_Disabled;
    port->vdm_auto_state = true;
    port->vdm_msg_length = 0;
    port->vdm_msg_tx_sop = SOP_SOP0;
#if MODAL_OPERATION_SUPPORTED
    port->mode_entered   = false;
#endif
#endif

    /* Set up the capabilities objects */
    if (CONFIG_ENABLED(SNK))
    {
        for (i = 0; i < 7; ++i)
        {
            port->caps_sink[i].object     = 0;
            port->caps_received[i].object = 0;
        }
        port->sink_request[0].object = 0;
        port->sink_request[1].object = 0;
        port->sink_selected_voltage = 5000;
        port->caps_header_sink.sop.NumDataObjects = port->vif->Num_Snk_PDOs;
        port->caps_header_sink.sop.PortDataRole   = 0;
        port->caps_header_sink.sop.PortPowerRole  = 0;
        port->caps_header_sink.sop.SpecRevision   = port->vif->PD_Specification_Revision;
        for (i = 0; i < port->caps_header_sink.sop.NumDataObjects; i++)
        {
            port->caps_sink[i].object = port->vif->snk_caps[i].object;
        }
    }
}

void policy_reset_message_queue(struct port * port)
{
    port->msgtx.value = 0;
	if (port->policy_is_source)
	{
		port->msgtx.booltxmsg.getbatterycap    = true;
		port->msgtx.booltxmsg.getbatterystatus = true;
		port->msgtx.booltxmsg.getsinkcapext    = true;
		port->msgtx.booltxmsg.get_snk_caps     = true;
		port->msgtx.booltxmsg.getstatus        = true;
	}
	else
	{
		port->msgtx.booltxmsg.getsourceinfo    = true;
		port->msgtx.booltxmsg.getrevision      = true;
	}
	port->alert_response = false;
}
static uint8_t a = 0;
void policy_pd_enable(struct port *port, bool source)
{

	protocol_reset(port);
    port->is_hard_reset                = false;
    port->is_pr_swap                   = false;
    port->renegotiate                  = false;
    port->needs_goto_min               = false;
    port->policy_ams_start             = false;
    port->policy_ams_active            = false;
#if CONFIG_ENABLED(FAULT_PROTECTION)
    port->fault_active = false;
    port->uvp_active = false;
#endif
    port->req_dr_swap_to_dfp_as_sink       = port->vif->Attempt_DR_Swap_to_Dfp_As_Sink;
    port->req_dr_swap_To_ufp_as_src        = port->vif->Attempt_DR_Swap_to_Ufp_As_Src;
    port->req_vconn_swap_to_on_as_sink     = port->vif->Attempt_Vconn_Swap_to_On_As_Sink;
    port->req_vconn_swap_to_off_as_src     = port->vif->Attempt_Vconn_Swap_to_Off_As_Src;
    port->req_pr_swap_as_src               = port->vif->Requests_PR_Swap_As_Src;
    port->req_pr_swap_as_snk               = port->vif->Requests_PR_Swap_As_Snk;
    port->cable_vdo_cable_type_ufp = VDM_ID_UFP_SOP_UNDEFINED;
    port->cable_vdo_cable_current = VDM_CABLE_MAX_CURRENT_3A; /* VDM_CABLE_MAX_CURRENT_3A; */
    port->cable_vdo_cable_voltage = VDM_CABLE_MAX_VOLTAGE_20V;
    if (port->vif->Attempts_DiscvId_SOP_P_First)
    {
        port->vdm_check_cbl = true;
    }

    if (CONFIG_EXTMSG)
    {
        port->prl_ext_req_chunk    = false;
        port->prl_ext_send_chunk   = false;
        port->prl_ext_state_active = false;
        port->prl_msg_rx_pending = false;
    }

    if (port->pd_enabled == true)
    {
        if (port->cc_pin != CC_NONE)
        {
            port->pd_active = true;
            timer_disable(&port->timers[PD_RSP_TIMER]);
            timer_disable(&port->timers[POLICY_TIMER]);
            timer_disable(&port->timers[SINKTX_TIMER]);
            timer_disable(&port->timers[SWAP_SOURCE_TIMER]);
            timer_disable(&port->timers[DISCOVER_IDENTITY_TIMER]);
            port->policy_is_source    = source;
            port->policy_is_dfp       = source;
            port->policy_substate     = 0;
            port->policy_has_contract = false;
            pps_enable_monitor(port, false);
            port->source_is_apdo            = false;
            port->usb_pd_contract.object    = 0;
            port->caps_header_received.word = 0;
#if (CONFIG_ENABLED(DRP))
            if (port->policy_is_source)
            {
                policy_set_state(port, PE_SRC_Startup, 0);
            }
            else
            {
                policy_set_state(port, PE_SNK_Startup, 0);
            }
#elif (CONFIG_ENABLED(SRC))
            policy_set_state(port, PE_SRC_Startup, 0);
#elif (CONFIG_ENABLED(SNK))
            policy_set_state(port, PE_SNK_Startup, 0);
#endif
#if CONFIG_ENABLED(EPR)
            port->epr_mode = false;
#endif
            dev_tcpd_port_pd_set_ver(port, SOP_SOP0, port->vif->PD_Specification_Revision);
            dev_tcpd_port_pd_set_ver(port, SOP_SOP1, port->vif->PD_Specification_Revision);
            port->pd_rev[0] = port->vif->PD_Specification_Revision;

        }
    }
    policy_reset_message_queue(port);
}

void policy_pd_disable(struct port *port)
{
    policy_reset(port);
    protocol_reset(port);
    fusbdev_tcpd_pd_enable(port->tcpd_device, false);
    policy_set_state(port, PE_SRC_Disabled, 0);
}

static void policy_set_send_soft_reset(struct port * port)
{
#if (CONFIG_ENABLED(DRP))
    policy_set_state(port, port->policy_is_source ? PE_SRC_Send_Soft_Reset : PE_SNK_Send_Soft_Reset, 0);
#elif CONFIG_ENABLED(SRC)
    policy_set_state(port, PE_SRC_Send_Soft_Reset, 0);
#elif (CONFIG_ENABLED(SNK))
    policy_set_state(port, PE_SNK_Send_Soft_Reset, 0);
#endif
}

/**
 * @brief Send PD message
 */
int policy_send(struct port *port, unsigned msg, uint8_t *data, unsigned long len, enum sop_t sop,
                bool ext, int succ_state, int succ_idx, int fail_state, int fail_idx)
{
    int status = 0;

    struct protocol_tx_t prot_tx = {0};
    port->policy_sending_message = true;
    prot_tx.type                 = msg;
    prot_tx.data                 = data;
    prot_tx.len                  = len;
    prot_tx.sop                  = sop;
    prot_tx.ams                  = port->policy_ams_start;
    prot_tx.ext                  = ext;

    if (!prot_tx.ams
        || (timer_disabled(&port->timers[RP_CHANGE_TIMER])
            || timer_expired(&port->timers[RP_CHANGE_TIMER])))
    {
        timer_disable(&port->timers[RP_CHANGE_TIMER]);
        status = protocol_send(port, &prot_tx);
    }
    if (status == STAT_SUCCESS)
    {
        if (port->policy_ams_start)
        {
            port->policy_ams_start  = false;
            port->policy_ams_active = true;
        }
        /* Goto next state if successful */
        policy_set_state(port, succ_state, succ_idx);
    }
    else if (status == STAT_ERROR)
    {
        if (port->policy_ams_active)
        {
            port->policy_ams_active = false;
            policy_set_send_soft_reset(port);
        }
        else
        {
            policy_set_state(port, fail_state, fail_idx);
        }
    }
    else if (status == STAT_ABORT)
    {
        policy_set_state(port, fail_state, fail_idx);
    }
    return status;
}

#if CONFIG_ENABLED(VDM) && !CONFIG_ENABLED(VDM_MINIMAL)
/**
 * PD 3.0 must be vconn source
 * PD 2.0 must be dfp
 */
static void policy_send_vdm(struct port *port, SOP_T sop, uint32_t *arr, uint32_t length,
                            enum policy_state_t success_state)
{
    unsigned i = 0;
    timer_disable(&port->timers[VDM_TIMER]);
    port->vdm_msg_length = length * 4;
    port->vdm_success_ps    = success_state;
    for (i = 0; i < length; i++)
    {
        port->policy_pd_data[i].object = arr[i];
    }
    port->vdm_msg_tx_sop = sop;
    policy_set_state(port, PE_GIVE_VDM, 0);
}
#endif
static void policy_send_notsupported(struct port *port)
{
#if (CONFIG_ENABLED(DRP))
    policy_set_state(port, port->policy_is_source ? PE_SRC_Send_Not_Supported : PE_SNK_Send_Not_Supported, 0);
#elif CONFIG_ENABLED(SRC)
    policy_set_state(port, PE_SRC_Send_Not_Supported, 0);
#elif (CONFIG_ENABLED(SNK))
    policy_set_state(port, PE_SNK_Send_Not_Supported, 0);
#endif
    port->policy_sending_message = true;
}
void policy_receive_hardreset(struct port *port)
{
#if (CONFIG_ENABLED(EPR))
    port->epr_mode = false;
#endif
#if (CONFIG_ENABLED(DRP))
    if (port->policy_is_source)
    {
        policy_pd_enable(port, true);
        timer_start(&port->timers[POLICY_TIMER], ktPSHardReset);
        policy_set_state(port, PE_SRC_Transition_To_Default, 0);
    }
    else
    {
        policy_pd_enable(port, false);
        timer_disable(&port->timers[POLICY_TIMER]);
        policy_set_state(port, PE_SNK_Transition_To_Default, 0);
    }
#elif (CONFIG_ENABLED(SRC))
    policy_pd_enable(port, true);
    timer_start(&port->timers[POLICY_TIMER], ktPSHardReset);
    policy_set_state(port, PE_SRC_Transition_To_Default, 0);
#elif (CONFIG_ENABLED(SNK))
    policy_pd_enable(port, false);
    timer_disable(&port->timers[POLICY_TIMER]);
    policy_set_state(port, PE_SNK_Transition_To_Default, 0);
#endif
}
#if (CONFIG_ENABLED(SRC))
void policy_send_source_hardreset(struct port *port)
{
    policy_set_state(port, PE_SRC_Hard_Reset, 0);
    policy_state_send_hardreset(port);
    port->policy_sending_message = true;
}
#endif
#if (CONFIG_ENABLED(SNK))
static void policy_send_sink_hardreset(struct port *port)
{
    policy_set_state(port, PE_SNK_Hard_Reset, 0);
    policy_state_send_hardreset(port);
    port->policy_sending_message = true;
}
#endif
static unsigned policy_check_comm_allowed(struct port *port, unsigned sop)
{
    unsigned int mask = 0;
    if (sop & SOP_MASK_SOP0)
    {
        if (port->vif->SOP_Capable)
        {
            mask |= SOP_MASK_SOP0;
        }
    }
    if ((sop & SOP_MASK_SOP1) || (sop & SOP_MASK_SOP2))
    {
        if (port_pd_ver(port, SOP_SOP0) > PD_REV2)
        {
            if (port->is_vconn_source)
            {
                if (port->vif->SOP_P_Capable)
                {
                    mask |= SOP_MASK_SOP1;
                }
                if (port->vif->SOP_PP_Capable)
                {
                    mask |= SOP_MASK_SOP2;
                }
            }
        }
        else
        {
            if (port->policy_is_dfp)
            {
                if (port->vif->SOP_P_Capable)
                {
                    mask |= SOP_MASK_SOP1;
                }
                if (port->vif->SOP_PP_Capable)
                {
                    mask |= SOP_MASK_SOP2;
                }
            }
        }
    }

    return mask & sop;
}

/**
 * @brief Check vif rules to see if response to vdm is allowed.
 */
static inline bool policy_check_vdm_resp_allowed(struct port *port)
{
    bool response = true;
    if (!policy_check_comm_allowed(port, SOP_MASK_SOP0))
    {
        /* Response to SOP not allowed */
        response = false;
    }
    else if (!(policy_state_is_ready(port->policy_state)))
    {
        /* Neither sink ready or source ready state */
        response = false;
    }
    return response;
}

static inline bool policy_check_vdm_requires_response(struct port *port, enum sop_t sop)
{
    if (sop == SOP_SOP0)
    {
        return policy_check_vdm_resp_allowed(port);
    }
    else
    {
        if (policy_check_comm_allowed(port, SOP_MASK_SOP1 | SOP_MASK_SOP2))
        {
            return true;
        }
    }
    return false;
}

static inline bool policy_check_pps_change(struct port *port)
{
    int index = port->policy_pd_data[0].FVRDO.ObjectPosition - 1;
    if ((port->usb_pd_contract.object != 0)
        && (port->policy_pd_data[0].FVRDO.ObjectPosition
            == port->usb_pd_contract.FVRDO.ObjectPosition)
        && (port->caps_source[index].FPDOSupply.SupplyType == pdoTypeAugmented)
        && (port->caps_source[index].APDO.APDOType == apdoTypePPS))
    {
        /* If contract already exists and it is PPS increase/decrease
         * in power then don't start the ktSrcTransition timer. */
        return true;
    }
    return false;
    ;
}

static void policy_set_vconn(struct port *port, bool en)
{
    unsigned mask = 0;
    dev_tcpd_port_set_vconn(port, en);
    mask = policy_check_comm_allowed(port, SOP_MASK_SOP0 | SOP_MASK_SOP1 | SOP_MASK_SOP2);
    fusbdev_tcpd_pd_sop_enable(port->tcpd_device, mask);
}

static void policy_set_dfp(struct port *port, bool en)
{
    unsigned mask       = 0;
    port->policy_is_dfp = en;
    fusbdev_tcpd_set_pd_dfp(port->tcpd_device, en);
    mask = policy_check_comm_allowed(port, SOP_MASK_SOP0 | SOP_MASK_SOP1 | SOP_MASK_SOP2);
    fusbdev_tcpd_pd_sop_enable(port->tcpd_device, mask);
}

static void policy_set_pd_source(struct port *port, bool en)
{
    port->policy_is_source = en;
    fusbdev_tcpd_set_pd_source(port->tcpd_device, en);
}

static inline void policy_set_port_roles(struct port *port)
{
    policy_set_dfp(port, port->policy_is_dfp);
    policy_set_pd_source(port, port->policy_is_source);
}

static void policy_set_partner_cap(struct port *port, bool source_cap)
{
    unsigned i = 0;
#if CONFIG_ENABLED(DRP)
#if CONFIG_ENABLED(EPR)
		if (!port->epr_mode)
		{
			port->caps_header_received.word = port->policy_pd_header.word;
			for (i = 0; i < port->caps_header_received.sop.NumDataObjects; i++)
			{
				port->caps_received[i].object = port->policy_pd_data[i].object;
			}
		}

		else
		{
			for (i = 0; i < port->prl_ext_num_bytes / 4; i++)
			{
				port->caps_received[i].byte[0] = port->policy_pd_data[i].byte[2];
				port->caps_received[i].byte[1] = port->policy_pd_data[i].byte[3];
				port->caps_received[i].byte[2] = port->policy_pd_data[i + 1].byte[0];
				port->caps_received[i].byte[3] = port->policy_pd_data[i + 1].byte[1];
			}
		}
#else
    	port->caps_header_received.word = port->policy_pd_header.word;
    	for (i = 0; i < port->caps_header_received.sop.NumDataObjects; i++)
		{
			port->caps_received[i].object = port->policy_pd_data[i].object;
		}

		port->msgtx.booltxmsg.prswap = port->req_pr_swap_as_src && port->partner_caps.FPDOSink.DualRolePower == 1;
#endif
#else
		port->caps_header_received.word = port->policy_pd_header.word;
		for (i = 0; i < port->caps_header_received.sop.NumDataObjects; i++)
		{
			port->caps_received[i].object = port->policy_pd_data[i].object;
		}
#endif
}

static void policy_process_cable_reset(struct port *port)
{
    switch (port->cbl_rst_state)
    {
        case CBL_RST_START:
            if (port->policy_is_dfp)
            {
                if (port->is_vconn_source)
                {
                    port->cbl_rst_state = CBL_RST_SEND;
                }
                else
                {
                    port->cbl_rst_state = CBL_RST_VCONN_SOURCE;
                    /* Must be dfp and vconn source*/
                    policy_set_state(port, PE_VCS_Send_Swap, 0);
                }
            }
            else
            {
                port->cbl_rst_state = CBL_RST_DR_DFP;
                /* Must be dfp and vconn source*/
                policy_set_state(
                    port,
                    port->policy_is_source ? PE_DRS_DFP_UFP_Send_Swap : PE_DRS_UFP_DFP_Send_Swap,
                    0);
            }
            break;
        case CBL_RST_DR_DFP:
            if (port->policy_is_dfp)
            {
                if (port->is_vconn_source)
                {
                    port->cbl_rst_state = CBL_RST_SEND;
                }
                else
                {
                    port->cbl_rst_state = CBL_RST_VCONN_SOURCE;
                    /* Must be dfp and vconn source*/
                    policy_set_state(port, PE_VCS_Send_Swap, 0);
                }
            }
            else
            {
                /* DR Swap might have failed */
                port->cbl_rst_state = CBL_RST_DISABLED;
            }
            break;
        case CBL_RST_VCONN_SOURCE:
            if (port->is_vconn_source)
            {
                port->cbl_rst_state = CBL_RST_SEND;
            }
            else
            {
                /* VCONN Swap might have failed */
                port->cbl_rst_state = CBL_RST_DISABLED;
            }
            break;
        case CBL_RST_SEND:
            if (port->policy_is_dfp && port->is_vconn_source)
            {
                policy_set_state(port, PE_DFP_CBL_Send_Cable_Reset, 0);
            }
            else
            {
                port->cbl_rst_state = CBL_RST_DISABLED;
            }
            break;
        case CBL_RST_DISABLED:
            break;
        default:
            break;
    }
}

static void policy_process_bist_message(struct port *port)
{
    BIST_SHARED_TEST_MODE_T ctx = {0};
    uint8_t                 bdo = port->policy_pd_data[0].byte[3] >> 4;

    if (port->usb_pd_contract.FVRDO.ObjectPosition == 1)
    {
        switch (bdo)
        {
            case BDO_BIST_Carrier_Mode_2:
                policy_set_state(port, PE_BIST_Carrier_Mode, 0);
                port->prl_state = PRLIdle;
                break;
            case BDO_BIST_Test_Data:
                fusbdev_tcpd_set_bist_mode(port->tcpd_device, BIST_MODE_RX);
                policy_set_state(port, PE_BIST_Test_Data, 0);
                port->prl_state = PRLDisabled;
                break;
            case BDO_BIST_Shared_Test_Mode_Enter:
                ctx.arg.enter = true;
                event_notify(EVENT_BIST_SHARED_TEST_MODE, port->dev, &ctx);
                break;
            case BDO_BIST_Shared_Test_Mode_Exit:
                ctx.arg.enter = false;
                event_notify(EVENT_BIST_SHARED_TEST_MODE, port->dev, &ctx);
                break;
            default:
                break;
        }
    }
}

#if CONFIG_ENABLED(VDM)
/**
 * @brief Check to see if the VDM is enabled and call structured or unstructured
 * handlers.
 */
static void policy_process_vdmrx_engine(struct port *port)
{
    struct pd_msg_t vdm = {0};
    vdm.object          = port->policy_pd_data[0].object;

    if (vdm.SVDM.HEADER.VDMType == VDM_STRUCTURED)
    {
        if (vdm.SVDM.HEADER.CommandType == VDM_CMD_TYPE_REQ)
        {
            /*check if Respond to Discover UFP/DFP are set in vif, else check PD version*/
            if ((port->vif->Responds_To_Discov_SOP_DFP == false)
                && (port->vif->Responds_To_Discov_SOP_UFP == false))
            {
                /* Do not respond or ignore for PD 2.0, send not supported for PD3.0 */
                if (port_pd_ver(port, SOP_SOP0) == PD_REV3)
                {
                    policy_send_notsupported(port);
                }
                return;
            }
        }
#if !CONFIG_ENABLED(VDM_MINIMAL)
        policy_process_vdmrx(port);
#endif
    }
    else
    {
        /* Unstructured VDM's not supported at this time */
        if (port_pd_ver(port, SOP_SOP0) == PD_REV3)
        {
            /* Not supported in PD3.0, ignored in PD2.0 */
        	policy_send_notsupported(port);
        }
    }
}

#if !CONFIG_ENABLED(VDM_MINIMAL)
/**
 * @brief when any of the VDM drivers become active this function is run from
 * policy source and sink ready until the VDM driver completes by setting
 * enabled flag to false.
 */
static void policy_process_vdm_auto_discv(struct port *port)
{
    uint32_t            vdo[7] = {0};
    struct vdm_sm_arg_t arg    = {0};
    bool                done   = false;

    arg.dev       = port->dev;
    arg.sop       = (1U << SOP_SOP0) | (1U << SOP_SOP1) | (1U << SOP_SOP2);
    arg.pdrev     = port_pd_ver(port, port->prl_msg_rx_sop);
    arg.tx_msg    = false;
    arg.buf.vdo   = vdo;
    arg.buf.len   = sizeof(vdo);
    arg.buf.count = 0;

    done = vdm_list_run_active(&arg);
    if (arg.tx_msg)
    {
        policy_send_vdm(port, arg.sop, arg.buf.vdo, arg.buf.count, port->policy_state);
        port->policy_ams_start = true;
    }
    if (done)
    {
        /* indicate that all vdm sm are done */
        port->vdm_auto_state = false;
    }
    else
    {
        port->event = true;
    }
}


/**
 * @brief Handles the incoming structured VDMs checks for the
 * message applicability and then calls the VDM driver. If the VDM drivers
 * tries to send the response then this function sends the response message.
 */
static void policy_process_vdmrx(struct port *port)
{
    uint32_t                buf[7] = {0};
    uint32_t                i      = 0;
    struct pd_msg_t        *vdm_in;
    struct vdm_obj_t const *obj;
    struct vdm_sm_arg_t     arg = {0};

    /* Argument to VDM driver */
    arg.dev       = port->dev;
    arg.tx_msg    = false;
    arg.buf.vdo   = buf;
    arg.buf.count = port->policy_pd_header.sop.NumDataObjects;
    arg.buf.len   = sizeof(buf) / sizeof(uint32_t);
    arg.sop       = port->prl_msg_rx_sop;
    arg.pdrev     = port_pd_ver(port, port->prl_msg_rx_sop);

    /* Copy VDOs */
    for (i = 0; i < port->policy_pd_header.sop.NumDataObjects; i++)
    {
        buf[i] = port->policy_pd_data[i].object;
    }
    vdm_in = (struct pd_msg_t *)buf;
    update_svdm_version_minor(port->dev, vdm_in->SVDM.HEADER.VersionMinor);
    obj    = vdm_list_find_svid(port->dev, vdm_in->SVDM.HEADER.SVID, 1U << arg.sop);

    /*check if valid svid*/
    if (!obj)
    {
        /* No SVID driver found. NAK any message, and ignore attention */
        if (vdm_in->SVDM.HEADER.Command == VDM_CMD_ATTENTION)
        {
            arg.buf.count = 0;
        }
        else
        {
            /* NAK response */
            vdm_in->SVDM.HEADER.CommandType = VDM_CMD_TYPE_NAK;
            arg.buf.count                   = 1;
            arg.tx_msg                      = true;
        }
    }
    else
    {
        if (vdm_in->SVDM.HEADER.CommandType == VDM_CMD_TYPE_REQ)
        {
            if (policy_check_vdm_requires_response(port, arg.sop))
            {
                obj->svdmh->vdm_process(&arg);
                /*update command type response here before sending if needed*/
                if (port_pd_ver(port, SOP_SOP0) < PD_REV3)
                {
                    /*In PD2.0, NAK response will be sent if any of the VIF field is set to 1*/
                    if (port->policy_is_source)
                    {
                        if ((port->vif->Responds_To_Discov_SOP_DFP)
                            || (port->vif->Responds_To_Discov_SOP_UFP))
                        {
                            /*only respond to NAK with Discover Identity command, other should be ignored*/
                            if (vdm_in->SVDM.HEADER.Command == VDM_CMD_DISCOVER_IDENTITY)
                            {
                                /* NAK response */
                                vdm_in->SVDM.HEADER.CommandType = VDM_CMD_TYPE_NAK;
                                arg.buf.count                   = 1;
                                arg.tx_msg                      = true;
                            }
                        }
                    }
                    else
                    {
                        if ((port->policy_is_dfp == false)
                            && (!port->vif->Responds_To_Discov_SOP_UFP))
                        {
                            if (vdm_in->SVDM.HEADER.Command == VDM_CMD_DISCOVER_IDENTITY)
                            {
                                /* NAK response */
                                vdm_in->SVDM.HEADER.CommandType = VDM_CMD_TYPE_NAK;
                                arg.buf.count                   = 1;
                                arg.tx_msg                      = true;
                            }
                        }
                    }
                }
                else
                {
                    /*In PD3.0, NAK response depends on values from VIF field*/
                    if (((port->policy_is_dfp == true) && (!port->vif->Responds_To_Discov_SOP_DFP))
                        || ((port->policy_is_dfp == false) && (!port->vif->Responds_To_Discov_SOP_UFP)))
                    {
                        if (vdm_in->SVDM.HEADER.Command == VDM_CMD_DISCOVER_IDENTITY)
                        {
                            /* NAK response */
                            vdm_in->SVDM.HEADER.CommandType = VDM_CMD_TYPE_NAK;
                            arg.buf.count                   = 1;
                            arg.tx_msg                      = true;
                        }
                    }
                }
            }
            else
            {
                arg.buf.count = 0;
            }
        }
        else
        {
            /* USB4 addition, copy USB4 information into port from DFP VDO */
            /*  */
            if (obj)
            {
#if CONFIG_ENABLED(USB4)
                if (vdm_in->SVDM.HEADER.Command == VDM_CMD_DISCOVER_IDENTITY && vdm_in->SVDM.HEADER.CommandType == VDM_CMD_TYPE_ACK)
                {
                    uint32_t product_type_dfp = 0;
                    uint32_t product_type_ufp = 0;
                    product_type_dfp = vdm_in[VDM_DISCVID_INDEX_ID_HEADER].SVDM.ID.type_dfp;
                    product_type_ufp = vdm_in[VDM_DISCVID_INDEX_ID_HEADER].SVDM.ID.type_ufp;
                    if (product_type_dfp && product_type_ufp && (product_type_ufp != 3))
                    {
                        /* Should be at PRODUCT_TYPE3 */
                        /* usb4 capable, skip for now (Enter_USB message gives all the information we need about) */
                    	port->partner_product_type_ufp.object =
                    	                            vdm_in[VDM_DISCVID_INDEX_PRODUCT_TYPE1].object;
                    	port->usb_supported_sopx |= (port->partner_product_type_ufp.SVDM.UFPVDO.device_cap_usb_4) ? (1U  << port->prl_msg_rx_sop) : 0U;
                    }
                    else if (product_type_dfp)
                    {

                    }
                    else if (product_type_ufp && (product_type_ufp != 3))
                    {
                        /* Should be at PRODUCT_TYPE1 */
                        port->partner_product_type_ufp.object =
                            vdm_in[VDM_DISCVID_INDEX_PRODUCT_TYPE1].object;
                    	port->usb_supported_sopx |= (port->partner_product_type_ufp.SVDM.UFPVDO.device_cap_usb_4) ? (1U  << port->prl_msg_rx_sop) : 0U;
                        /* usb4 mode capable + USB highest speed + alternate modes, use for Enter_USB */
                    }
                    port->disc_id_sent_sopx |= (1U  << port->prl_msg_rx_sop);

                }
#endif
#if MODAL_OPERATION_SUPPORTED
                /* if we get the vdm discover */
				if (vdm_in->SVDM.HEADER.Command == VDM_CMD_DISCOVER_SVIDS && vdm_in->SVDM.HEADER.CommandType == VDM_CMD_TYPE_ACK)
				{
					port->disc_svid_received_sopx |= (1U << port->vdm_msg_tx_sop);
				}
#endif
                obj->svdmh->vdm_process(&arg);
            }
            else if (port->vdm_auto_state)
            {
                /* If a response for message not sent disable auto vdm discovery until
                 * activated */
                port->vdm_auto_state = false;
                vdm_disable_all(port->dev);
            }
            arg.buf.count = 0;
        }
    }

    /* If message activates the VDM driver set auto execution from SRC/SNK ready */
    if (obj && obj->active(port->dev))
    {
        port->vdm_auto_state = true;
        port->event          = true;
    }

    /* Check if there is message to send */
    if (arg.tx_msg)
    {
        policy_send_vdm(port, arg.sop, arg.buf.vdo, arg.buf.count, port->policy_state);
    }
}

static void policy_state_give_vdm(struct port *port)
{
    uint8_t status     = 0;
    uint8_t type       = 0;
    uint8_t cmd        = 0;
    uint8_t structured = 0;
    uint8_t fail_state = 0;
    if (port->policy_pd_data[0].SVDM.HEADER.CommandType == VDM_CMD_TYPE_REQ)
    {
    	fail_state = port->vdm_success_ps;
    }
    else
    {
		fail_state = policy_state_send_soft_reset(port);
    }
    switch (port->policy_substate)
    {
        case 0:
            status = policy_send(port, DMTVendorDefined, port->policy_pd_data[0].byte,
                                 port->vdm_msg_length, port->vdm_msg_tx_sop, false, PE_GIVE_VDM, 1,
								 fail_state, 0);
            if (status != STAT_BUSY)
            {
                port->msgtx.booltxmsg.vdm = false;
            }
            if (status == STAT_SUCCESS)
            {
            	port->previous_vdm_tx_header.object = port->policy_pd_data[0].object;
                /* All AMS are now NOT interruptible, so ams_active needs to be false */
                port->policy_ams_active = true;
                structured              = port->policy_pd_data[0].SVDM.HEADER.VDMType;
                type                    = port->policy_pd_data[0].SVDM.HEADER.CommandType;
                cmd                     = port->policy_pd_data[0].SVDM.HEADER.Command;
                if (structured)
                {
                    if (type == VDM_CMD_TYPE_REQ)
                    {
                        if (cmd == VDM_CMD_ATTENTION)
                        {
                            policy_set_state(port, port->vdm_success_ps, 0);
                        }
                        else if (cmd == VDM_CMD_ENTER_MODE || cmd == VDM_CMD_EXIT_MODE)
                        {
                            timer_start(&port->timers[VDM_TIMER], ktVDMWaitModeEntry);
                        }
                        else
                        {
                            timer_start(&port->timers[VDM_TIMER], ktVDMSenderResponse);
                        }
                    }
                    else
                    {
#if MODAL_OPERATION_SUPPORTED
                        /* Response received. Just set global flags for all vdms */
                        if (cmd == VDM_CMD_ENTER_MODE)
                        {
                            port->mode_entered = true;
                        }
                        else if (cmd == VDM_CMD_EXIT_MODE)
                        {
                            port->mode_entered = false;
                        }
#endif
                        policy_set_state(port, port->vdm_success_ps, 0);
                    }
                }
                else
                {
                    policy_set_state(port, port->vdm_success_ps, 0);
                }
            }
            if (status == STAT_ERROR)
            {
            	vdm_disable_driver(port->dev, port->vdm_tx_svid, (1U << port->vdm_msg_tx_sop));
            }
            break;
        case 1:
            if (port->prl_msg_rx)
            {
                timer_disable(&port->timers[VDM_TIMER]);
                protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                                 sizeof(port->policy_pd_data));
                port->prl_msg_rx = false;
                if (port->vdm_msg_tx_sop > port->prl_msg_rx_sop)
                {
                	port->prl_msg_rx = true;
                	policy_set_state(port, port->vdm_success_ps, 0);
                	/* Interrupting SOP' messaging should disable the driver for now */
                	vdm_disable_driver(port->dev, port->vdm_tx_svid, port->vdm_msg_tx_sop);
                }
                else if (port->policy_pd_header.sop.NumDataObjects > 0 && port->policy_pd_header.sop.Extended == 0
                    && port->policy_pd_header.sop.MessageType == DMTVendorDefined)
                {
                	if (port->previous_vdm_tx_header.SVDM.HEADER.CommandType == VDM_CMD_TYPE_REQ &&
                			 port->policy_pd_data[0].SVDM.HEADER.CommandType != VDM_CMD_TYPE_REQ)
                	{
                		policy_process_vdmrx_engine(port);
						policy_set_state(port, port->vdm_success_ps, 0);
                	}
                	else
                	{
                		/* If REQ->REQ, soft reset */
                		policy_set_state(port, policy_state_send_soft_reset(port), 0);
                	}
                }
                else
                {
                    policy_set_state(port, policy_state_send_soft_reset(port), 0);
                }
            }
            else if (timer_expired(&port->timers[VDM_TIMER]))
            {
                /* No response received */
                policy_set_state(port, PE_GIVE_VDM, 2);
                /* Disable driver if no response received */
                vdm_disable_driver(port->dev, port->vdm_tx_svid, port->vdm_msg_tx_sop);
            }
            break;
        case 2:
            /* Create a NAK message for no response to the VDM driver. */
            port->policy_pd_header.word                     = 0;
            port->policy_pd_header.sop.NumDataObjects       = 1;
            port->policy_pd_header.sop.MessageType          = DMTVendorDefined;
            port->policy_pd_data[0].SVDM.HEADER.CommandType = VDM_CMD_TYPE_NAK;
            policy_process_vdmrx_engine(port);
            policy_set_state(port, policy_state_send_soft_reset(port), 0);
            break;
        default:
            policy_set_state(port, policy_state_send_soft_reset(port) , 0);
            break;
    }
}
#endif
#endif
/////////////////////////////////////////////////////////////////////////
// Source States
/////////////////////////////////////////////////////////////////////////
static void policy_state_disabled(struct port *port)
{
    port->usb_pd_contract.object = 0;
    if (timer_expired(&port->timers[PD_RSP_TIMER]))
    {
        /* TODO Add "had_contract" port item? */
        timer_disable(&port->timers[PD_RSP_TIMER]);
    }
}
#if CONFIG_ENABLED(SRC)
static void policy_state_source_startup(struct port *port)
{
    switch (port->policy_substate)
    {
        case 0:
            port->caps_counter           = 0;
            port->usb_pd_contract.object = 0;
            port->partner_caps.object    = 0;
            port->is_pr_swap             = false;
            port->policy_is_source       = true;
            port->policy_has_contract    = false;
            pps_enable_monitor(port, false);
            port->source_is_apdo            = false;
            port->usb_pd_contract.object    = 0;
            port->partner_caps_available    = false;
            port->caps_header_received.word = 0;
            port->sink_selected_voltage     = VBUS_5V;
            protocol_reset_sop(port, SOP_MASK_SOP0);
            vdm_list_reset_all(port->dev, 0, ((1U << SOP_SOP0) | (1U << SOP_SOP1) | (1U << SOP_SOP2)), 0);
            policy_set_port_roles(port);
            fusbdev_tcpd_pd_enable(port->tcpd_device, true);

            timer_start(&port->timers[POLICY_TIMER], ktSrcStartupVbus);
            timer_disable(&port->timers[PPS_TIMER]);
#if MODAL_OPERATION_SUPPORTED
            port->mode_entered   = false;
#endif
            port->vdm_auto_state = false;

 #if CONFIG_ENABLED(VDM) && !CONFIG_ENABLED(VDM_MINIMAL)
                port->vdm_auto_state = true;
#endif

            policy_set_state(port, PE_SRC_Startup, 1);
            fusbdev_tcpd_set_vbus_alarm(port->tcpd_device, 0, VBUS_LOW_THRESHOLD(VBUS_5V));
            policy_reset_message_queue(port);
            break;
        case 1:
            /* Wait until we reach vSafe5V and delay if coming from PR Swap */
            if (port_vbus_vsafe5v(port)
                && (timer_expired(&port->timers[SWAP_SOURCE_TIMER])
                    || timer_expired(&port->timers[POLICY_TIMER])))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                timer_disable(&port->timers[SWAP_SOURCE_TIMER]);

                policy_set_state_func(port, PE_SRC_Send_Capabilities, 0);
#if (CONFIG_ENABLED(VDM))
                if (port->vdm_check_cbl && policy_check_comm_allowed(port, SOP_MASK_SOP1))
                {
                    policy_set_state(port, PE_CBL_Query, 0);
                    port->policy_ams_active = false;
                }
#endif
            }
            else
            {
                fusbdev_tcpd_set_vbus_alarm(port->tcpd_device, 0, VBUS_LOW_THRESHOLD(VBUS_5V));
            }
            break;
        default:
            policy_set_state(port, PE_ErrorRecovery, 0);
            break;
    }
}
#endif
#if CONFIG_ENABLED(VDM)
static void policy_state_source_cable_query(struct port *port)
{
    int             status     = 0;
    unsigned        i          = 0;
#if CONFIG_ENABLED(EPR) && CONFIG_ENABLED(SRC)
    int             next_state = port->policy_state_prev == PE_SRC_EPR_Mode_Discover_Cable ?
                                     PE_SRC_EPR_Mode_Evaluate_Cable_EPR :
                                     port->policy_state_prev;
#else
    int next_state = port->policy_state_prev;
#endif
    struct pd_msg_t vdo        = {0};
    struct event_pd_cbl_id_received_t evt                             = {0};
    unsigned int                      prev_max_amp                    = 0;
    struct pd_msg_t                   cable_vdos[VDM_DISCVID_MAX_LEN] = {0};

    vdo.object              = 0;
    vdo.SVDM.HEADER.SVID    = 0xFF00;
    vdo.SVDM.HEADER.VersionMajor = VDM_STRUCTURED_VER(port_pd_ver(port, SOP_SOP1));
    vdo.SVDM.HEADER.VersionMinor = VDM_STRUCTURED_MAX_VER_MINOR(port_pd_ver(port, SOP_SOP1));
    vdo.SVDM.HEADER.VDMType = 1;
    vdo.SVDM.HEADER.ObjPos  = 0;
    vdo.SVDM.HEADER.Command = VDM_CMD_DISCOVER_IDENTITY;

    switch (port->policy_substate)
    {
        case 0:
            if ((port->discv_id_counter < MAX_DISC_ID_COUNT))
            {
                /* SOP' discovery has not completed so request again*/
                port->discv_id_counter++;
                policy_set_state_func(port, PE_CBL_Query, 1);
            }
            else
            {
                port->vdm_check_cbl = false;
                policy_set_state(port, next_state, 0);
            }
            break;
        case 1:
            if (timer_expired(&port->timers[DISCOVER_IDENTITY_TIMER])
                || timer_disabled(&port->timers[DISCOVER_IDENTITY_TIMER]))
            {
                timer_disable(&port->timers[DISCOVER_IDENTITY_TIMER]);
                status = policy_send(port, DMTVendorDefined, vdo.byte, 4, SOP_SOP1, false,
                                     PE_CBL_Query, 2, next_state, 0);
                if (status == STAT_SUCCESS)
                {
                    timer_start(&port->timers[POLICY_TIMER], ktSenderResponse);
                    timer_start(&port->timers[DISCOVER_IDENTITY_TIMER], ktDiscoverIdentity);
                }
                if (status != STAT_BUSY)
                {
                	port->vdm_check_cbl = false;
                }
            }
            break;
        case 2:
            if (port->prl_msg_rx || timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                if (port->prl_msg_rx_sop == SOP_SOP0)
                {
                    /* Return to PE_SRC_Ready/PE_SNK_Ready and parse */
                    policy_set_state(port, next_state, 0);
                    /* TODO: Finish implementing other cases */
                    /* If from PE_SRC_Startup/PE_SRC_Send_Capabilities, HardReset */
                    /* If in EPR_Mode, Abort EPR_Mode_Entry and soft/hardreset? */
                }
                else if (port->prl_msg_rx_sop == SOP_SOP1)
                {
                    protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                                     sizeof(port->policy_pd_data));
                    port->prl_msg_rx = false;
                    if (port->policy_pd_header.sop.MessageType == DMTVendorDefined &&
                        port->policy_pd_header.sop.NumDataObjects >= 4)
                    {
                        if (port->policy_pd_data[VDM_DISCVID_INDEX_HEADER].SVDM.HEADER.CommandType == VDM_CMD_TYPE_ACK)
                        {
                            vdm_enable_driver(port->dev, 0xFF00, 1U << port->prl_msg_rx_sop);
                            if ((port->policy_pd_data[VDM_DISCVID_INDEX_ID_HEADER].SVDM.ID.type_ufp == VDM_ID_UFP_SOP_1_PASSIVE_CABLE ||
                                 port->policy_pd_data[VDM_DISCVID_INDEX_ID_HEADER].SVDM.ID.type_ufp == VDM_ID_UFP_SOP_1_ACTIVE_CABLE))
                            {
#if CONFIG_ENABLED(USB4)
                        	    port->disc_id_sent_sopx |= (1U  << port->prl_msg_rx_sop);
#endif
                                port->vdm_cbl_present = true;
                                port->vdm_check_cbl   = false;
                                prev_max_amp          = port_max_cable_current(port);
                                /* Save cable capabilities */
                                for (i = 0; i < port->policy_pd_header.sop.NumDataObjects
                                            && i < VDM_DISCVID_MAX_LEN;
                                     i++)
                                {
                                    cable_vdos[i] = port->policy_pd_data[i];
                                }
                                evt.arg.cable_cap = cable_vdos;
                                port->cable_vdo_cable_type_ufp =
                                    cable_vdos[VDM_DISCVID_INDEX_ID_HEADER].SVDM.ID.type_ufp;
                                port->cable_vdo_cable_current =
                                    cable_vdos[VDM_DISCVID_INDEX_PRODUCT_TYPE1].SVDM.PCBL.current;
                                port->cable_vdo_cable_voltage =
                                    cable_vdos[VDM_DISCVID_INDEX_PRODUCT_TYPE1].SVDM.PCBL.voltage;
#if (CONFIG_ENABLED(EPR))
                                port->epr_capable_cable =
                                    cable_vdos[VDM_DISCVID_INDEX_PRODUCT_TYPE1].SVDM.PCBL.EPRCapable;
                                port->captive_cable =
                                    cable_vdos[VDM_DISCVID_INDEX_PRODUCT_TYPE1].SVDM.PCBL.connector
                                    == CAPTIVE_CABLE;
#endif
#if (CONFIG_ENABLED(USB4))
                                port->cable_vdo_cable_highest_usb_speed = (cable_vdos[VDM_DISCVID_INDEX_PRODUCT_TYPE1].SVDM.PCBL.speed);
                                if (port->cable_vdo_cable_type_ufp == VDM_ID_UFP_SOP_1_ACTIVE_CABLE
                                    && cable_vdos[VDM_DISCVID_INDEX_PRODUCT_TYPE2].object != 0)
                                {
                                    /* store highest USB mode supported */
                                    port->cable_vdo_cable_highest_usb2_mode =
                                        !cable_vdos[VDM_DISCVID_INDEX_PRODUCT_TYPE2].SVDM.ACBL2.usb_2;
                                    port->cable_vdo_cable_highest_usb3_2_mode =
                                        !cable_vdos[VDM_DISCVID_INDEX_PRODUCT_TYPE2].SVDM.ACBL2.usb_3_2;
                                    port->cable_vdo_cable_highest_usb4_mode =
                                        !cable_vdos[VDM_DISCVID_INDEX_PRODUCT_TYPE2].SVDM.ACBL2.usb_4;
                                    /* get SOP' and SOP'' support for Enter_USB */
                                    port->usb_supported_sopx |= (port->cable_vdo_cable_highest_usb4_mode || port->cable_vdo_cable_highest_usb3_2_mode || port->cable_vdo_cable_highest_usb3_2_mode) ? (1U << port->prl_msg_rx_sop) : 0;
                                    port->usb_supported_sopx |= (port->usb_supported_sopx & (1U << port->prl_msg_rx_sop)) ? (cable_vdos[VDM_DISCVID_INDEX_PRODUCT_TYPE1].SVDM.ACBL1.sopp << SOP_SOP2) : 0;
                                    /* Reuse TBT3 VDO for retimer / optical isolated info */
                                    port->tbt3vdo.SVDM.TBT3MODEVDO.Active = 1;
                                    port->tbt3vdo.SVDM.TBT3MODEVDO.Retimer = cable_vdos[VDM_DISCVID_INDEX_PRODUCT_TYPE2].SVDM.ACBL2.active_element;
                                    port->tbt3vdo.SVDM.TBT3MODEVDO.Optical = cable_vdos[VDM_DISCVID_INDEX_PRODUCT_TYPE2].SVDM.ACBL2.optically_isolated;
                                }
                                else if (port->cable_vdo_cable_type_ufp == VDM_ID_UFP_SOP_1_PASSIVE_CABLE)
                                {
                                	port->tbt3vdo.SVDM.TBT3MODEVDO.Active = 0;
                                	port->usb_supported_sopx |= (port->cable_vdo_cable_highest_usb_speed != 0) ? (1U << port->prl_msg_rx_sop) : 0;
                                }
#endif
#if (CONFIG_ENABLED(EXTENDED_EVENT))
                                    event_notify(EVENT_PD_CBL_ID_RECEIVED, port->dev, &evt);
#endif

                                if (port_max_cable_current(port) > prev_max_amp)
                                {
                                    port->higher_cable_cap = true;
                                }

#if (CONFIG_ENABLED(EPR)) && (CONFIG_ENABLED(SRC))
                                if (next_state == PE_SRC_EPR_Mode_Evaluate_Cable_EPR)
                                {
                                    port->epr_mode_message.EPRMDO.action = EprModeEnterSucc;
                                    port->epr_mode_message.EPRMDO.data   = 0;
                                }
#endif
                            }
                            else
                            {
                                /* ACK */
                                /* not passive or active cable, */
                                /* means cable not present */
                                port->vdm_cbl_present = false;
                                port->vdm_check_cbl   = false;
                            }
                        }
                        else
                        {
                            /* no cable */
                            /* not an ACK and size >= 4 ==> stop sending SOP' and assume */
                            port->vdm_cbl_present = false;
                            port->vdm_check_cbl   = false;
                        }
                    }
                    else if (port->policy_pd_data[0].SVDM.HEADER.CommandType == VDM_CMD_TYPE_NAK &&
                             port->policy_pd_header.sop.NumDataObjects == 1)
                    {
                        /* Downgrade cable caps if we receive a NAK.
                     * Note that this is not standard behavior and is implemented
                     * to ensure that cables without higher capabilities can still
                     * be communicated with.
                     */
                        port->pd_rev[SOP_SOP1] = PD_REV2;
                    }
                    else
                    {
                        /* means length is less than 4 */
                        /*  and NOT a valid NAK */
                        port->vdm_cbl_present = false;
                        port->vdm_check_cbl   = false;
                    }
                }
                else
                {
                    /* Retry with rev1 on the last attempt. */
                    if (port->discv_id_counter >= MAX_DISC_ID_COUNT - 1)
                        port->pd_rev[SOP_SOP1] = PD_REV2;
                }
                policy_set_state(port, next_state, 0);
            }
            break;
        default:
            policy_set_state(port, next_state, 0);
            break;
    }
}
#endif
#if CONFIG_ENABLED(SRC)
static void policy_state_source_discovery(struct port *port)
{
    switch (port->policy_substate)
    {
        case 0:
        	port->prl_msg_rx &= port->prl_msg_rx_sop == SOP_SOP0;
            if (port->hard_reset_counter > HARD_RESET_COUNT
                && timer_expired(&port->timers[PD_RSP_TIMER]))
            {
                timer_disable(&port->timers[PD_RSP_TIMER]);
                timer_disable(&port->timers[POLICY_TIMER]);
                if (port->policy_has_contract == true)
                {
                    policy_set_state(port, PE_ErrorRecovery, 0);
                }
                else
                {
                    policy_set_state(port, PE_SRC_Disabled, 0);
                }
            }
            else if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                /* ktTypeCSendSourceCap */
                timer_disable(&port->timers[POLICY_TIMER]);
                if (port->caps_counter > MAX_CAPS_COUNT)
                {
                    /* No PD sink connected */
                    policy_set_state(port, PE_SRC_Disabled, 0);
                }
                else
                {
                    policy_set_state(port, PE_SRC_Send_Capabilities, 0);
#if CONFIG_ENABLED(VDM)
                    if (port->vdm_check_cbl && policy_check_comm_allowed(port, SOP_MASK_SOP1))
                    {
                        policy_set_state(port, PE_CBL_Query, 0);
                        port->policy_ams_active = false;
                    }
#endif
                }
            }
            break;
        default:
            policy_set_state(port, PE_ErrorRecovery, 0);
            break;
    }
}

static void policy_process_source_capibilties(struct port *port)
{
    unsigned      i     = 0;
    int           count = 0;
    enum pd_rev_t rev   = port_pd_ver(port, SOP_SOP0);
    enum pdo_t    type  = {0};

    unsigned int max_amp       = port_max_cable_current(port);
    unsigned int max_volt      = 0;
    unsigned int max_power     = 0;
    unsigned int pdo_max_amp   = 0;
    unsigned int pdo_max_power = 0;
    port->current_limited      = false;

    for (i = 0; i < port->caps_header_source.sop.NumDataObjects; i++)
    {
        type = port->caps_source[i].PDO.SupplyType;
        if (type == pdoTypeAugmented)
        {
            if (rev == PD_REV3)
            {
                pdo_max_amp = PD_MA_STEP_TO_50MA_STEP(max_amp);
                /* Copy PPS PDO and limit current if necessary */
                if (pdo_max_amp < port->caps_source[i].PPSAPDO.MaxCurrent)
                {
                    port->caps_source[i].PPSAPDO.MaxCurrent = pdo_max_amp;
                    port->current_limited                   = true;
                }
                count++;
            }
            else
            {
                /* End check after PPS for older rev */
                break;
            }
        }
        else if (type == pdoTypeVariable)
        {
            pdo_max_amp = PD_MA_STEP_TO_10MA_STEP(max_amp);
            /* Copy Variable PDO and limit current if necessary */
            if (pdo_max_amp < port->caps_source[i].VPDO.MaxCurrent)
            {
                port->caps_source[i].VPDO.MaxCurrent = pdo_max_amp;
                port->current_limited                = true;
            }
            count++;
        }
        else if (type == pdoTypeBattery)
        {
            max_volt      = PD_50MV_STEP_TO_MV(port->caps_source[i].BPDO.MaxVoltage);
            max_power     = (max_volt * max_amp) / 1000;
            pdo_max_power = PD_MW_STEP_TO_250MW_STEP(max_power);
            /* Copy Battery PDO and limit power if necessary*/
            if (pdo_max_power < port->caps_source[i].BPDO.MaxPower)
            {
                port->caps_source[i].BPDO.MaxPower = pdo_max_power;
                port->current_limited              = true;
            }
            count++;
        }
        else
        {
            pdo_max_amp = PD_MA_STEP_TO_10MA_STEP(max_amp);
            /* Copy Fixed PDO and limit current if needed */
            if (pdo_max_amp < port->caps_source[i].FPDOSupply.MaxCurrent)
            {
                port->caps_source[i].FPDOSupply.MaxCurrent = pdo_max_amp;
                port->current_limited                      = true;
            }
            count++;
        }
    }

    port->caps_header_source.sop.NumDataObjects = (count > 7) ? 7 : count;
}
static void policy_state_source_give_source_capabilities(struct port *port)
{
    int                           status         = 0;
    unsigned                      i              = 0;
    struct event_pd_get_src_cap_t evt            = {0};
    uint16_t                      previous_state = port->policy_state_prev;
    switch (port->policy_substate)
    {
        case 0:
            evt.arg.pd_rev  = port_pd_ver(port, SOP_SOP0);
            evt.arg.src_cap = port->caps_source;
            evt.arg.max_ma  = port_max_cable_current(port);
            evt.ret.success = EVENT_STATUS_DEFAULT;
            /* Copy the PDOs from VIF */
            port->caps_header_source.sop.NumDataObjects = port->vif->Num_Src_PDOs;
            EVENT_PD_POP_SRC_CAPS_T caps_evt            = {0};
            caps_evt.arg.caps                           = port->caps_source;
            caps_evt.arg.caps_count                     = port->dev->vif->Num_Src_PDOs;
            caps_evt.ret.success                        = EVENT_STATUS_DEFAULT;
            event_notify(EVENT_PD_POPULATE_SRC_CAPS, port->dev, &caps_evt);
            if (caps_evt.ret.success != EVENT_STATUS_SUCCESS)
            {
                for (i = 0; i < port->dev->vif->Num_Src_PDOs; i++)
                {
                    port->caps_source[i] = port->dev->vif->src_caps[i];
                }
#if (CONFIG_ENABLED(EPR))
                if (!port->epr_mode)
                {
                    for (i = 7; i < 7 +port->dev->vif->Num_Src_EPR_PDOs; i++)
                    {
                        port->caps_source[i] = port->dev->vif->src_caps[i];
                    }
                }
#endif
            }
            /* Let DPM handle any processing if needed */
            event_notify(EVENT_PD_GET_SRC_CAP, port->dev, &evt);
#if CONFIG_ENABLED(EPR)
            port->epr_capable_pdo = evt.arg.src_cap[0].FPDOSupply.EPRCapable;
            if (!port->epr_mode)
            {
                /*if in epr mode 0 all source caps between k and 7 */
                for (int i = evt.ret.num; i < 7; i++)
                {
                    port->caps_source[i].object = 0;
                }
                /*if in epr mode add all epr capabilities */
                event_notify(EVENT_PD_GET_SRC_EPR_CAP, port->dev, &evt);
                if (evt.ret.success == EVENT_STATUS_SUCCESS)
                {
                    port->epr_src_cap_len = evt.ret.num;
                }
            }
#else
            if (evt.ret.success == EVENT_STATUS_SUCCESS)
            {
                port->caps_header_source.sop.NumDataObjects = evt.ret.num;
            }
#endif
            /* Pre-process the source caps to confirm revision and max cable current */
            policy_process_source_capibilties(port);
            policy_set_state(port, PE_SRC_Give_Source_Cap, 1);
            break;
        case 1:
#if CONFIG_ENABLED(EPR)
            if (!port->epr_mode)
            {
                 status = policy_send(port, EMTEPRSourceCapabilities, (uint8_t *)&port->caps_source,
                                                     (uint32_t)port->epr_src_cap_len * 4,
                                                     SOP_SOP0, true, PE_SRC_Ready, 0, PE_SRC_Ready, 0);

            }
            else
            {
                status = policy_send(port, DMTSourceCapabilities, (uint8_t *)&port->caps_source,
                                      (uint32_t)port->caps_header_source.sop.NumDataObjects * 4,
                                      SOP_SOP0, false, PE_SRC_Ready, 0, PE_SRC_Ready, 0);
            }
#else
            status = policy_send(port, DMTSourceCapabilities, (uint8_t *)&port->caps_source,
                                 (uint32_t)port->caps_header_source.sop.NumDataObjects * 4,
                                 SOP_SOP0, false, PE_SRC_Ready, 0, PE_SRC_Ready, 0);

#endif
            if (status == STAT_SUCCESS)
            {
                timer_start(&port->timers[EXTENDED_TIMER], ktExtendedMessage);
                port->is_hard_reset      = false;
                port->hard_reset_counter = 0;
                port->caps_counter       = 0;
                timer_disable(&port->timers[PD_RSP_TIMER]);
            }
            if (status != STAT_BUSY)
            {
                port->msgtx.booltxmsg.src_caps = false;
            }
            break;
    }
}
static void policy_state_source_send_capabilities(struct port *port)
{
    int                           status = 0;
    unsigned                      i      = 0;
    struct event_pd_get_src_cap_t evt    = {0};
    switch (port->policy_substate)
    {
        case 0:
            evt.arg.pd_rev  = port_pd_ver(port, SOP_SOP0);
            evt.arg.src_cap = port->caps_source;
            evt.arg.max_ma  = port_max_cable_current(port);
            evt.ret.success = EVENT_STATUS_DEFAULT;
            /* Copy the PDOs from VIF */
            port->caps_header_source.sop.NumDataObjects = port->vif->Num_Src_PDOs;
            EVENT_PD_POP_SRC_CAPS_T caps_evt            = {0};
            caps_evt.arg.caps                           = port->caps_source;
            caps_evt.arg.caps_count                     = port->dev->vif->Num_Src_PDOs;
            caps_evt.ret.success                        = EVENT_STATUS_DEFAULT;
            event_notify(EVENT_PD_POPULATE_SRC_CAPS, port->dev, &caps_evt);
            if (caps_evt.ret.success != EVENT_STATUS_SUCCESS)
            {
                for (i = 0; i < port->dev->vif->Num_Src_PDOs; i++)
                {
                    port->caps_source[i] = port->dev->vif->src_caps[i];
                }
#if (CONFIG_ENABLED(EPR))
                if (port->epr_mode)
                {
                    for (i = 7; i < 7 +port->dev->vif->Num_Src_EPR_PDOs; i++)
                    {
                        port->caps_source[i] = port->dev->vif->src_caps[i];
                    }
                }
#endif
            }
            /* Let DPM handle any processing if needed */
            event_notify(EVENT_PD_GET_SRC_CAP, port->dev, &evt);
#if CONFIG_ENABLED(EPR)
            port->epr_capable_pdo = evt.arg.src_cap[0].FPDOSupply.EPRCapable;
            if (port->epr_mode)
            {
                /*if in epr mode 0 all source caps between k and 7 */
                for (int i = evt.ret.num; i < 7; i++)
                {
                    port->caps_source[i].object = 0;
                }
                /*if in epr mode add all epr capabilities */
                event_notify(EVENT_PD_GET_SRC_EPR_CAP, port->dev, &evt);
                if (evt.ret.success == EVENT_STATUS_SUCCESS)
                {
                    port->epr_src_cap_len = evt.ret.num;
                }
            }
#else
            if (evt.ret.success == EVENT_STATUS_SUCCESS)
            {
                port->caps_header_source.sop.NumDataObjects = evt.ret.num;
            }
#endif
            /* Pre-process the source caps to confirm revision and max cable current */
            policy_process_source_capibilties(port);
            policy_set_state(port, PE_SRC_Send_Capabilities, 1);

            break;
        case 1:
#if CONFIG_ENABLED(EPR)
            if (!port->epr_mode)
            {
                status =
                    policy_send(port, DMTSourceCapabilities, (uint8_t *)&port->caps_source,
                                (uint32_t)port->caps_header_source.sop.NumDataObjects * 4, SOP_SOP0,
                                false, PE_SRC_Send_Capabilities, 2, PE_SRC_Discovery, 0);
            }
            else
            {
                status = policy_send(port, EMTEPRSourceCapabilities, (uint8_t *)&port->caps_source,
                                     (uint32_t)(port->epr_src_cap_len * 4), SOP_SOP0, true,
                                     PE_SRC_Send_Capabilities, 2, PE_SRC_Discovery, 0);
            }
#else
            status = policy_send(port, DMTSourceCapabilities, (uint8_t *)&port->caps_source,
                                 (uint32_t)port->caps_header_source.sop.NumDataObjects * 4,
                                 SOP_SOP0, false, PE_SRC_Send_Capabilities, 2, PE_SRC_Discovery, 0);
#endif
            if (status == STAT_SUCCESS)
            {
                port->is_hard_reset      = false;
                port->hard_reset_counter = 0;
                port->caps_counter       = 0;
                timer_disable(&port->timers[PD_RSP_TIMER]);
                timer_start(&port->timers[POLICY_TIMER], ktSenderResponse);
#if CONFIG_ENABLED(USB4)
                /* start enter_usb timer for both DFP and UFP */
				timer_start(&port->timers[ENTER_USB_TIMER], ktEnterUSB);
				port->enter_usb_timeout = false;
				if (!port->policy_is_dfp)
				{
				}
				/* if no discover identity sent on SOP', and port is NOT VCONN SOURCE but IS DFP, vconn swap */
				else if (!port->is_vconn_source && !(port->disc_id_sent_sopx & 1U << SOP_SOP1 == 1U << SOP_SOP1))
				{
					port->msgtx.booltxmsg.vconnswap = true;
				}
#endif
#if CONFIG_ENABLED(POWER_SHARING) || CONFIG_ENABLED(LEGACY_CHARGING)
                event_notify(EVENT_PD_DEVICE, port->dev, 0);
#endif
            }
            else if (status == STAT_ERROR || status == STAT_ABORT)
            {
                port->caps_counter++;
                timer_start(&port->timers[POLICY_TIMER], ktTypeCSendSourceCap);
            }
            if (status != STAT_BUSY)
            {
            	port->msgtx.booltxmsg.src_caps = false;
            }
            break;
        case 2:
            if (port->prl_msg_rx)
            {
                protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                                 sizeof(port->policy_pd_data));
                port->prl_msg_rx = false;
                timer_disable(&port->timers[POLICY_TIMER]);
                if (port_pd_ver(port, SOP_SOP0) > port->policy_pd_header.sop.SpecRevision)
                {
                    dev_tcpd_port_pd_set_ver(port, SOP_SOP0,
                                             port->policy_pd_header.sop.SpecRevision);
                }
#if (CONFIG_ENABLED(EPR))
                if ((port->policy_pd_header.sop.NumDataObjects == 1) &&
                     port->policy_pd_header.sop.Extended == 0 &&
                     port->policy_pd_header.sop.MessageType == DMTRequest &&
                     !port->epr_mode)
#else
                	if ((port->policy_pd_header.sop.NumDataObjects == 1) &&
						 port->policy_pd_header.sop.Extended == 0 &&
						 port->policy_pd_header.sop.MessageType == DMTRequest)
#endif
                {
                    policy_set_state(port, PE_SRC_Negotiate_Capability, 0);
                }
#if CONFIG_ENABLED(EPR)
                else if ((port->policy_pd_header.sop.NumDataObjects == 2) &&
                         (port->policy_pd_header.sop.MessageType == DMTEPRequest) &&
                          port->policy_pd_header.sop.Extended == 0 &&
                          port->epr_mode)
                {
                    policy_set_state(port, PE_SRC_Negotiate_Capability, 0);
                }
#endif
                else if ((port->policy_pd_header.sop.NumDataObjects != 0)
                         && ((port->policy_pd_header.sop.MessageType == DMTEPRequest) ||
                              port->policy_pd_header.sop.MessageType == DMTRequest))
                {
                    policy_state_invalid(port);
                }
                else
                {
                    /* Unexpected message */
                    policy_set_state(port, PE_SRC_Send_Soft_Reset, 0);
                }
            }
            else if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                port->prl_msg_rx = false;
                policy_send_source_hardreset(port);
            }
            break;
        default:
            policy_set_state(port, PE_ErrorRecovery, 0);
            break;
    }
}

static void policy_state_source_transition_supply(struct port *port)
{
    uint32_t status        = 0;
    uint32_t req_idx       = 0;
    unsigned voltage       = 0;
    unsigned current       = 0;
    PDO_T    supply_type   = {0};
    bool     valid_voltage = false;
    EVENT_PD_SRC_EVAL_SNK_REQ_T req;
    req.ret.success = EVENT_STATUS_DEFAULT;
    switch (port->policy_substate)
    {
        case 0:
            if (port->needs_goto_min)
            {
                status = policy_send(port, CMTGotoMin, 0, 0, SOP_SOP0, false,
                                     PE_SRC_Transition_Supply, 3, PE_SRC_Hard_Reset, 0);
                if (status == STAT_SUCCESS)
                {
                    timer_start(&port->timers[POLICY_TIMER], ktSrcTransition);
                    port->needs_goto_min = false;
                }
            }
            else
            {
                status = policy_send(port, CMTAccept, 0, 0, SOP_SOP0, false,
                                     PE_SRC_Transition_Supply, 1, PE_SRC_Hard_Reset, 0);
                if (status == STAT_SUCCESS)
                {
                    timer_disable(&port->timers[POLICY_TIMER]);
                    if (!policy_check_pps_change(port))
                    {
                        /* Not a PPS voltage/current change  */
                        timer_start(&port->timers[POLICY_TIMER], ktSrcTransition);
                    }
                    /* Store current contract */
                    port->usb_pd_contract.object = port->policy_pd_data[0].object;
                }
                else if (status == STAT_ERROR || status == STAT_ABORT)
                {
                    policy_send_source_hardreset(port);
                }
            }
            break;
        case 1:
            if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                req_idx        = port->usb_pd_contract.PPSRDO.ObjectPosition - 1;
                supply_type    = port->caps_source[req_idx].APDO.SupplyType;
                port->PDO_Type = supply_type;
                if (supply_type == pdoTypeAugmented)
                {
                    port->source_is_apdo = true;
                    voltage = PD_20MV_STEP_TO_MV(port->usb_pd_contract.PPSRDO.OpVoltage);
                    current = PD_50MA_STEP_TO_MA(port->usb_pd_contract.PPSRDO.OpCurrent);
                }
                else if (supply_type == pdoTypeBattery)
                {
                    port->source_is_apdo = false;
                    voltage = PD_50MV_STEP_TO_MV(port->caps_source[req_idx].BPDO.MaxVoltage);
                    /* Multiply by 1000 for 1mA resolution */
                    current = (1000 * PD_250MW_STEP_TO_MW(port->usb_pd_contract.BRDO.MinMaxPower))
                              / voltage;
                }
                else if (supply_type == pdoTypeVariable)
                {
                    port->source_is_apdo = false;
                    voltage = PD_50MV_STEP_TO_MV(port->caps_source[req_idx].VPDO.MinVoltage);
                    current = PD_10MA_STEP_TO_MA(port->usb_pd_contract.FVRDO.MinMaxCurrent);
                }
                else
                {
                    /* Fixed type */
                    port->source_is_apdo = false;
                    voltage = PD_50MV_STEP_TO_MV(port->caps_source[req_idx].FPDOSupply.Voltage);
                    current = PD_10MA_STEP_TO_MA(port->caps_source[req_idx].FPDOSupply.MaxCurrent);
                }
                if (port->source_is_apdo)
                {
                    /*set pps alarm if handled in dpm*/
                    if (!(pps_set_alarm(port, port->source_is_apdo)))
                    {
                        /*set alarm using regular threshold in case no DPM handler*/
                        fusbdev_tcpd_set_vbus_alarm(port->tcpd_device, VBUS_HIGH_THRESHOLD(voltage),
                                                    VBUS_LOW_THRESHOLD(voltage));
                    }
                    else
                    {
                        /*enable pps monitor as contract is apdo and DPM handles the monitoring*/
                        pps_enable_monitor(port, true);
                    }
                }
                else
                {
                    fusbdev_tcpd_set_vbus_alarm(port->tcpd_device, VBUS_HIGH_THRESHOLD(voltage),
                                                VBUS_LOW_THRESHOLD(voltage));
                }
                dev_tcpd_port_vbus_src(port, voltage, current, supply_type);
                if (ABS(voltage - port->sink_selected_voltage) > vPpsSmallStep)
                {
                    /* tPpsSrcTransLarge(275ms) doubles for tSrcReady(285ms) */
                    timer_start(&port->timers[POLICY_TIMER], ktPpsSrcTransLarge);
                }
                else
                {
                    /* Okay for non-pps contracts that don't change voltage */
                    timer_start(&port->timers[POLICY_TIMER], ktPpsSrcTransSmall);
                }
                port->sink_selected_voltage = voltage;
                port->sink_selected_current = current;
                policy_set_state(port, PE_SRC_Transition_Supply, 2);
            }
            break;
        case 2:
            /* Verify we've hit our target voltage, or the transition timer expires */
            event_notify(EVENT_PPS_CL, port->dev, &req);
            req_idx     = port->usb_pd_contract.PPSRDO.ObjectPosition - 1;
            supply_type = port->caps_source[req_idx].APDO.SupplyType;
            /* grab a new vbus value before running through code */
#if (CONFIG_ENABLED(VBUS_VALID))
            if (supply_type == pdoTypeFixed ){
                valid_voltage = (port->sink_selected_voltage == 5000) ? port_vbus_vsafe5v(port) : port_vbus_valid(port, port->sink_selected_voltage);
            } else if (supply_type == pdoTypeAugmented) {
                valid_voltage = port_vbus_in_range(port, 0, VBUS_HIGH_THRESHOLD(port->sink_selected_voltage));
            }
#else
            valid_voltage = true;
#endif
            if (timer_expired(&port->timers[POLICY_TIMER]) && valid_voltage)
            {
                /* Optional delay to allow for external switching delays */
                policy_set_state(port, PE_SRC_Transition_Supply, 3);
            }
            else
            {
                if (port->source_is_apdo)
                {
                    /*re-enable alarm in case pps handling is in dpm*/
                    if (!(pps_set_alarm(port, port->source_is_apdo)))
                    {
                        /*set alarm using regular threshold in case no DPM handler*/
                        fusbdev_tcpd_set_vbus_alarm(
                            port->tcpd_device, VBUS_HIGH_THRESHOLD(port->sink_selected_voltage),
                            VBUS_LOW_THRESHOLD(port->sink_selected_voltage));
                    }
                }
                else
                {
                    fusbdev_tcpd_set_vbus_alarm(port->tcpd_device,
                                                VBUS_HIGH_THRESHOLD(port->sink_selected_voltage),
                                                VBUS_LOW_THRESHOLD(port->sink_selected_voltage));
                }
            }
            break;
        case 3:
            status = policy_send(port, CMTPS_RDY, 0, 0, SOP_SOP0, false, PE_SRC_Ready, 0,
                                 PE_SRC_Hard_Reset, 0);
            if (status == STAT_SUCCESS)
            {
                if (port->policy_has_contract == false
#if (CONFIG_ENABLED(EPR))
                    || port->epr_mode
#endif
                )
                {
                    dev_tcpd_port_set_sinktx(port, SinkTxNG);
                }
                port->policy_has_contract = true;
                if (port->source_is_apdo)
                {
                    timer_start(&port->timers[PPS_TIMER], ktPPSTimeout);
                }
                else
                {
                    timer_disable(&port->timers[PPS_TIMER]);
                }
                port->is_contract_valid = true;
                event_notify(EVENT_PD_NEW_CONTRACT, port->dev, 0);
            }
            else if (status == STAT_ABORT || status == STAT_ERROR)
            {
                policy_send_source_hardreset(port);
            }
            break;
        default:
            policy_set_state(port, PE_ErrorRecovery, 0);
            break;
    }
}

static void policy_state_source_transition_default(struct port *port)
{
    switch (port->policy_substate)
    {
        case 0:
            if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                port->is_hard_reset       = true;
                port->policy_has_contract = false;
                pps_enable_monitor(port, false);
                port->source_is_apdo = false;
                policy_set_state(port, PE_SRC_Transition_To_Default, 1);
                /* disable vbus and set alarm for vSafe0V */
                fusbdev_tcpd_set_vbus_alarm(port->tcpd_device, VBUS_VSAFE0_ALARM, 0);
                dev_tcpd_port_vbus_src(port, VBUS_OFF, VBUS_OFF, 0);
#if DEVICE_TYPE != FUSB15101
                fusbdev_tcpd_set_vbus_disch(port->tcpd_device, VBUS_DISCH_DEFAULT);
#endif
                policy_set_vconn(port, false);
            }
            break;
        case 1:
            if (port_vbus_vsafe0v(port))
            {
                fusbdev_tcpd_set_vbus_alarm(port->tcpd_device, VBUS_0V, VBUS_0V);
#if (CONFIG_ENABLED(EPR))
                if (port->epr_mode)
                {
                    timer_start(&port->timers[POLICY_TIMER], ktSrcRecoverEPR);
                }
                else
                {
                    timer_start(&port->timers[POLICY_TIMER], ktSrcRecover);
                }
#else
                timer_start(&port->timers[POLICY_TIMER], ktSrcRecover);
#endif
                policy_set_state(port, PE_SRC_Transition_To_Default, 2);
            }
            else
            {
                fusbdev_tcpd_set_vbus_alarm(port->tcpd_device, VBUS_VSAFE0_ALARM, 0);
            }
            break;
        case 2:
            if (timer_expired(&port->timers[POLICY_TIMER]))
            {
#if CONFIG_ENABLED(FAULT_PROTECTION)
				port->uvp_active = false;
#endif
                timer_disable(&port->timers[POLICY_TIMER]);
#if DEVICE_TYPE != FUSB15101
                fusbdev_tcpd_set_vbus_disch(port->tcpd_device, VBUS_DISCH_OFF);
#endif
                dev_tcpd_port_vbus_src(port, VBUS_5V, CC_RP_CURRENT(port->src_current),
                                       pdoTypeFixed);
                if (port->vif->Type_C_Sources_VCONN)
                {
                    policy_set_vconn(port, true);
                }

                policy_pd_enable(port, port->policy_is_source);
                timer_start(&port->timers[PD_RSP_TIMER], ktNoResponse);
            }
            break;
        default:
            policy_set_state(port, PE_ErrorRecovery, 0);
            break;
    }
}

static void policy_state_source_negotiate_capability(struct port *port)
{
    EVENT_PD_SRC_EVAL_SNK_REQ_T req = {0};
    /* Default accept. Defer to dpm if implemented */
    req.ret.success = EVENT_STATUS_DEFAULT;
    req.arg.req     = port->policy_pd_data;
    req.arg.caps    = port->caps_source;
    port->sink_request[0].object = port->policy_pd_data[0].object;
#if (CONFIG_ENABLED(EPR))
    if (port->epr_mode)
    {
        req.arg.caps_count = port->epr_src_cap_len;
    }
    else
    {
        req.arg.caps_count = port->caps_header_source.sop.NumDataObjects;
    }
#else
    req.arg.caps_count = port->caps_header_source.sop.NumDataObjects;
#endif
    req.arg.pd_rev = port_pd_ver(port, SOP_SOP0);
    event_notify(EVENT_PD_SRC_EVAL_SNK_REQ, port->dev, &req);
    if (req.ret.success != EVENT_STATUS_FAILED)
    {
        policy_set_state(port, PE_SRC_Transition_Supply, 0);
        /* New EPR checks - check RDO is EPR capable and keep track of whether we are in EPR or SPR contract */
#if CONFIG_ENABLED(EPR)
        port->epr_capable_rdo = req.arg.req->EPRCAPRDO.EPRCapable;
        if (port->epr_mode)
        {
            port->epr_pdo = (port->epr_mode) ? true : false;
            port->sink_request[1].object = port->policy_pd_data[1].object;
        }
#endif
    }
    else
    {
        policy_set_state(port, PE_SRC_Capability_Response, 0);
    }
}

static void policy_state_source_capability_response(struct port *port)
{
    if (port->policy_has_contract)
    {
        if (port->is_contract_valid)
        {
            policy_send(port, CMTReject, 0, 0, SOP_SOP0, false, PE_SRC_Ready, 0, PE_SRC_Ready, 0);
        }
        else
        {
            policy_send(port, CMTReject, 0, 0, SOP_SOP0, false, PE_SRC_Hard_Reset, 0,
                        PE_SRC_Hard_Reset, 0);
        }
    }
    else
    {
        policy_send(port, CMTReject, 0, 0, SOP_SOP0, false, PE_SRC_Wait_New_Capabilities, 0,
                    PE_SRC_Hard_Reset, 0);
    }
}

static void policy_state_source_get_sink_cap(struct port *port)
{
    uint32_t                    status    = 0;
    EVENT_PD_SNK_CAP_RECEIVED_T snk_event = {0};

    snk_event.arg.success = false;

    switch (port->policy_substate)
    {
        case 0:
#if CONFIG_ENABLED(EPR)
            if (!port->epr_mode)
            {
                status = policy_send(port, CMTGetSinkCap, 0, 0, SOP_SOP0, false,
                                     PE_SRC_Get_Sink_Cap, 1, PE_SRC_Ready, 0);
                if (status == STAT_SUCCESS)
                {
                    port->partner_caps.object = -1;
                    port->policy_ams_start    = true;
                    timer_start(&port->timers[POLICY_TIMER], ktSenderResponse);
                }
                if (status != STAT_BUSY)
                {
                	port->msgtx.booltxmsg.get_snk_caps = false;
                }
            }
            else
            {
                struct ext_control_t msg = {0};
                msg.type                 = EPR_Get_Sink_Cap;
                msg.data                 = 0;
                int status = policy_send(port, EMTExtendedControl, msg.bytes, EXT_CONTROL_LEN,
                                         SOP_SOP0, true, PE_SRC_Get_Sink_Cap, 1, PE_SRC_Ready, 0);
                if (status == STAT_SUCCESS)
                {
                    port->policy_ams_start = true;
                    timer_start(&port->timers[POLICY_TIMER], ktSenderResponse);

                    port->partner_caps.object = -1;
                }
                if (status != STAT_BUSY)
                {
                    port->msgtx.booltxmsg.get_snk_caps = false;
                }
            }
#else
            status = policy_send(port, CMTGetSinkCap, 0, 0, SOP_SOP0, false, PE_SRC_Get_Sink_Cap, 1,
                                 PE_SRC_Ready, 0);
            if (status == STAT_SUCCESS)
            {
                port->partner_caps.object = -1;
                port->policy_ams_start    = true;
                timer_start(&port->timers[POLICY_TIMER], ktSenderResponse);
            }
            if (status != STAT_BUSY)
            {
                port->msgtx.booltxmsg.get_snk_caps = false;
            }

#endif
            break;
        case 1:
            if (port->prl_msg_rx)
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                                 sizeof(port->policy_pd_data));
                port->prl_msg_rx = false;
                if ((port->policy_pd_header.sop.NumDataObjects > 0)
                    && (port->policy_pd_header.sop.MessageType == DMTSinkCapabilities))
                {
                    policy_set_partner_cap(port, false);
                    port->partner_caps_available = true;

                    snk_event.arg.pd_rev = port_pd_ver(port, SOP_SOP0);
                    ;
                    snk_event.arg.snk_cap = port->policy_pd_data;
                    snk_event.arg.num     = port->policy_pd_header.sop.NumDataObjects;
                    snk_event.arg.success = true;

                    policy_set_state(port, PE_SRC_Ready, 0);
                }
#if CONFIG_ENABLED(EPR)
                else if (port->policy_pd_header.sop.Extended && port->epr_mode
                         && port->policy_pd_header.sop.MessageType == EMTEPRSinkCapabilities)
                {
                    policy_set_partner_cap(port, false);
                    port->partner_caps_available = true;

                    snk_event.arg.pd_rev = port_pd_ver(port, SOP_SOP0);
                    snk_event.arg.snk_cap = port->policy_pd_data;
                    snk_event.arg.num     = port->policy_pd_header.sop.NumDataObjects;
                    snk_event.arg.success = true;

                    policy_set_state(port, PE_SRC_Ready, 0);
                }
#endif
                else if (port->policy_pd_header.sop.NumDataObjects == 0
                         && port->policy_pd_header.sop.MessageType == CMTNotSupported)
                {
                    policy_set_state(port, PE_SRC_Ready, 0);
                }
                else
                {
                    /* No valid sink caps message */
                    policy_set_state(port, PE_SRC_Send_Soft_Reset, 0);
                }

                event_notify(EVENT_PD_SNK_CAP_RECEIVED, port->dev, &snk_event);
            }
            else if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                event_notify(EVENT_PD_SNK_CAP_RECEIVED, port->dev, &snk_event);
                policy_set_state(port, PE_SRC_Ready, 0);
            }
            break;
        default:
            policy_set_state(port, PE_ErrorRecovery, 0);
            break;
    }
}

static void policy_state_source_give_sink_cap(struct port *port)
{
    EVENT_PD_GET_SNK_CAP_T evt = {0};
    int status = 0;
#if (CONFIG_ENABLED(DRP))
    if (port->type == TypeC_DRP)
    {
        if (port->policy_substate == 0)
        {
            evt.ret.success = EVENT_STATUS_DEFAULT;
            evt.arg.snk_cap = port->caps_sink;
            evt.arg.pd_rev  = port_pd_ver(port, SOP_SOP0);
            if (evt.ret.success == EVENT_STATUS_SUCCESS)
            {
                port->caps_header_sink.sop.NumDataObjects = evt.ret.num;
            }
            policy_set_state(port, PE_DR_SRC_Give_Sink_Cap, 1);
        }
        else
        {
        	status = policy_send(port, DMTSinkCapabilities, port->caps_sink[0].byte,
                        port->caps_header_sink.sop.NumDataObjects * 4, SOP_SOP0, false,
                        PE_SRC_Ready, 0, PE_SRC_Ready, 0);
            if (status != STAT_BUSY)
            {
            	port->msgtx.booltxmsg.snk_caps = false;
            }
        }
    }
    else
    {
        policy_send_notsupported(port);
    }
#else
    policy_send_notsupported(port);
#endif
}
#if (CONFIG_ENABLED(DRP))
static void policy_state_source_get_sourcecap(struct port *port)
{
    uint32_t status = 0;
    switch (port->policy_substate)
    {
        case 0:
            status = policy_send(port, CMTGetSourceCap, 0, 0, SOP_SOP0, false,
                                 PE_DR_SRC_Get_Source_Cap, 1, PE_SRC_Ready, 0);
            if (status == STAT_SUCCESS)
            {
                timer_start(&port->timers[POLICY_TIMER], ktSenderResponse);
            }
            if (status != STAT_BUSY)
            {
                port->msgtx.booltxmsg.get_src_caps = false;
            }
            break;
        case 1:
            if (port->prl_msg_rx)
            {
                protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                                 sizeof(port->policy_pd_data));
                port->prl_msg_rx = false;
                if (port->policy_pd_header.sop.NumDataObjects > 0
                    && port->policy_pd_header.sop.MessageType == DMTSourceCapabilities)
                {
                    policy_set_partner_cap(port, true);
                    port->partner_caps_available = true;
                    policy_set_state(port, PE_SRC_Ready, 0);
                }
                else if (port->policy_pd_header.sop.NumDataObjects == 0
                         && (port->policy_pd_header.sop.MessageType == CMTReject
                             || port->policy_pd_header.sop.MessageType == CMTNotSupported))
                {
                    policy_set_state(port, PE_SRC_Ready, 0);
                }
                else
                {
                    policy_set_state(port, PE_SRC_Send_Soft_Reset, 0);
                }
            }
            else if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                policy_set_state(port, PE_SRC_Ready, 0);
            }
            break;
        default:
            policy_set_state(port, PE_ErrorRecovery, 0);
            break;
    }
}
#endif
static void policy_state_source_send_ping(struct port *port)
{
    policy_send(port, CMTPing, 0, 0, SOP_SOP0, false, PE_SRC_Ready, 0, PE_SRC_Ready, 0);
}

static void policy_state_source_send_drswap(struct port *port)
{
    int status = 0;
    switch (port->policy_substate)
    {
        case 0:
            status = policy_send(port, CMTDR_Swap, 0, 0, SOP_SOP0, false, PE_DRS_DFP_UFP_Send_Swap,
                                 1, PE_SRC_Ready, 0);
            if (status == STAT_SUCCESS)
            {
                timer_start(&port->timers[POLICY_TIMER], ktSenderResponse);
                port->req_dr_swap_To_ufp_as_src = false;
            }
            break;
        case 1:
            if (port->prl_msg_rx)
            {
                protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                                 sizeof(port->policy_pd_data));
                port->prl_msg_rx = false;
                if (port->policy_pd_header.sop.NumDataObjects == 0)
                {
                    /* Received a control message */
                    if (port->policy_pd_header.sop.MessageType == CMTAccept)
                    {
                        port->policy_is_dfp = !port->policy_is_dfp;
                        policy_set_dfp(port, port->policy_is_dfp);
#if (CONFIG_ENABLED(USB4))
                        port->usb_mode       = false;
                        /* start enter_usb timer for both DFP and UFP */
                        timer_start(&port->timers[ENTER_USB_TIMER], ktEnterUSB);
                        port->enter_usb_timeout = false;
                        if (!port->policy_is_dfp)
                        {
                        }
            			else if (!port->is_vconn_source && !(port->disc_id_sent_sopx & 1U << SOP_SOP1 == 1U << SOP_SOP1))
            			{
            				port->msgtx.booltxmsg.vconnswap = true;
            			}
#endif
                        policy_set_port_roles(port);
                        policy_set_state(port, PE_SRC_Ready, 0);
                    }
                    else if (port->policy_pd_header.sop.MessageType == CMTReject
                             || port->policy_pd_header.sop.MessageType == CMTWait
                             || port->policy_pd_header.sop.MessageType == CMTNotSupported)
                    {
                        policy_set_state(port, PE_SRC_Ready, 0);
                    }
                    else
                    {
                        policy_set_state(port, PE_SRC_Send_Soft_Reset, 0);
                    }
                }
                else
                {
                    /* Received data message */
                    policy_set_state(port, PE_SRC_Send_Soft_Reset, 0);
                }
            }
            else if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                policy_set_state(port, PE_SRC_Ready, 0);
            }
            break;
        default:
            policy_set_state(port, PE_ErrorRecovery, 0);
            break;
    }
}

static void policy_state_source_evaluate_drswap(struct port *port)
{
    int status = 0;
#if MODAL_OPERATION_SUPPORTED || CONFIG_ENABLED(USB4)
    if (port->mode_entered)
    {
        policy_send_source_hardreset(port);
        return;
    }
#endif
    if (port->prl_msg_rx_sop != SOP_SOP0
        || (port->policy_is_dfp && !port->vif->DR_Swap_To_UFP_Supported)
        || (!port->policy_is_dfp && !port->vif->DR_Swap_To_DFP_Supported))
    {
        policy_send_notsupported(port);
    }
    else
    {
        status =
            policy_send(port, CMTAccept, 0, 0, SOP_SOP0, false, PE_SRC_Ready, 0, PE_SRC_Ready, 0);
        if (status == STAT_SUCCESS)
        {
            port->policy_is_dfp = !port->policy_is_dfp;
            policy_set_dfp(port, port->policy_is_dfp);
#if (CONFIG_ENABLED(USB4))
            port->usb_mode       = false;
            /* start enter_usb timer for both DFP and UFP */
            timer_start(&port->timers[ENTER_USB_TIMER], ktEnterUSB);
            port->enter_usb_timeout = false;
			if (!port->policy_is_dfp)
			{
			}
			else if (!port->is_vconn_source && !(port->disc_id_sent_sopx & 1U << SOP_SOP1 == 1U << SOP_SOP1))
			{
				port->msgtx.booltxmsg.vconnswap = true;
			}
#endif
        }
    }
}

static void policy_state_source_send_vconn_swap(struct port *port)
{
    int status     = 0;
#if CONFIG_ENABLED(EPR)
    int next_state = (port->policy_state_prev == PE_SRC_EPR_Mode_Entry_ACK) ?
                         PE_SRC_EPR_Mode_Discover_Cable :
                         PE_SRC_Ready;
#else
    int next_state = PE_SRC_Ready;
#endif
    switch (port->policy_substate)
    {
        case 0:
            status = policy_send(port, CMTVCONN_Swap, 0, 0, SOP_SOP0, false, PE_VCS_Send_Swap, 1,
                                 PE_SRC_Ready, 0);
            if (status == STAT_SUCCESS)
            {
                timer_start(&port->timers[POLICY_TIMER], ktSenderResponse);
                port->req_vconn_swap_to_off_as_src = false;
            }
            if (status != STAT_BUSY)
		    {
            	port->msgtx.booltxmsg.vconnswap = false;
		    }
            break;
        case 1:
            if (port->prl_msg_rx)
            {
                protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                                 sizeof(port->policy_pd_data));
                port->prl_msg_rx = false;
                if (port->policy_pd_header.sop.NumDataObjects == 0)
                {
                    if (port->policy_pd_header.sop.MessageType == CMTAccept)
                    {
                        policy_set_state(port, PE_VCS_Send_Swap, 2);
                        timer_disable(&port->timers[POLICY_TIMER]);
                    }
                    else if (port->policy_pd_header.sop.MessageType == CMTReject
                             || port->policy_pd_header.sop.MessageType == CMTWait)
                    {
                        policy_set_state(port, next_state, 0);
                    }
                    else if (port->policy_pd_header.sop.MessageType == CMTNotSupported)
                    {
                        /* If needed, can force becoming the VConn Source */
                        if (!port->is_vconn_source)
                        {
                            policy_set_vconn(port, true);
                        }
                        policy_set_state(port, next_state, 0);
                    }
                    else
                    {
                        policy_set_state(port, PE_SRC_Send_Soft_Reset, 0);
                    }
                }
                else
                {
                    /* Received data message */
                    policy_set_state(port, PE_SRC_Send_Soft_Reset, 0);
                }
            }
            else if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                policy_set_state(port, next_state, 0);
            }
            break;
        case 2:
            if (port->is_vconn_source)
            {
                timer_start(&port->timers[POLICY_TIMER], ktVCONNSourceOn);
                policy_set_state(port, PE_VCS_Send_Swap, 3);
            }
            else
            {
                /* Apply VConn */
                policy_set_vconn(port, true);

                /* Skip next state and send the PS_RDY msg */
                policy_set_state(port, PE_VCS_Send_Swap, 4);
                port->is_vconn_swap = false;
            }
            break;
        case 3:
            if (port->prl_msg_rx)
            {
                protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                                 sizeof(port->policy_pd_data));
                port->prl_msg_rx = false;
                if (port->policy_pd_header.sop.NumDataObjects == 0)
                {
                    switch (port->policy_pd_header.sop.MessageType)
                    {
                        case CMTPS_RDY:
                            /* Disable VCONN source */
                            policy_set_vconn(port, false);
                            policy_set_state(port, next_state, 0);
                            port->is_vconn_swap = true;
                            break;
                        default:
                            /* Ignore all other commands received */
                            break;
                    }
                }
            }
            else if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                policy_send_source_hardreset(port);
            }
            break;
        case 4:
            status =
                policy_send(port, CMTPS_RDY, 0, 0, SOP_SOP0, false, next_state, 0, PE_SRC_Ready, 0);
            break;
        default:
            policy_set_state(port, PE_ErrorRecovery, 0);
            break;
    }
}

static void policy_state_source_evaluate_vconn_swap(struct port *port)
{
    switch (port->policy_substate)
    {
        case 0:
            if ((port->prl_msg_rx_sop != SOP_SOP0)
                || (port->is_vconn_source && !port->vif->VCONN_Swap_To_Off_Supported))
            {
                policy_send_notsupported(port);
            }
            else if (!port->is_vconn_source && !port->vif->VCONN_Swap_To_On_Supported)
            {
                /* If VCONN_Swap_To_On_Supported is false, send Reject instead of Not Supported */
                policy_send(port, CMTReject, 0, 0, SOP_SOP0, false, PE_SRC_Ready, 0, PE_SRC_Ready,
                            0);
            }
            else
            {
                policy_send(port, CMTAccept, 0, 0, SOP_SOP0, false, PE_VCS_Evaluate_Swap, 1,
                            PE_SRC_Ready, 0);
            }
            break;
        case 1:
            if (port->is_vconn_source)
            {
                timer_start(&port->timers[POLICY_TIMER], ktVCONNSourceOn);
                policy_set_state(port, PE_VCS_Evaluate_Swap, 2);
                port->discv_id_counter = 0;
            }
            else
            {
                policy_set_vconn(port, true);
                port->is_vconn_swap = false;
                timer_start(&port->timers[POLICY_TIMER], ktVCONNOnWait);
                policy_set_state(port, PE_VCS_Evaluate_Swap, 3);
            }
            break;
        case 2:
            if (port->prl_msg_rx)
            {
                protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                                 sizeof(port->policy_pd_data));
                port->prl_msg_rx = false;
                if (port->policy_pd_header.sop.NumDataObjects == 0)
                {
                    switch (port->policy_pd_header.sop.MessageType)
                    {
                        case CMTPS_RDY:
                            policy_set_vconn(port, false);
                            policy_set_state(port, PE_SRC_Ready, 0);
                            port->is_vconn_swap = true;
                            break;
                        default:
                            /* Ignore all other commands received */
                            break;
                    }
                }
            }
            else if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                policy_send_source_hardreset(port);
            }
            break;
        case 3:
            if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                policy_set_state(port, PE_VCS_Evaluate_Swap, 4);
            }
            break;
        case 4:
            policy_send(port, CMTPS_RDY, 0, 0, port->prl_msg_rx_sop, false, PE_SRC_Ready, 0,
                        PE_SRC_Ready, 0);
            break;
        default:
            policy_set_state(port, PE_ErrorRecovery, 0);
            break;
    }
}

static void policy_state_source_send_prswap(struct port *port)
{
#if CONFIG_ENABLED(DRP)
    uint8_t status = 0;

    switch (port->policy_substate)
    {
        case 0:
            status = policy_send(port, CMTPR_Swap, 0, 0, SOP_SOP0, false, PE_PRS_SRC_SNK_Send_Swap,
                                 1, PE_SRC_Ready, 0);
            if (status == STAT_SUCCESS)
            {
                timer_start(&port->timers[POLICY_TIMER], ktSenderResponse);
                port->req_pr_swap_as_src = false;
            }
            break;
        case 1:
            /* Require Accept message to move on or go back to ready state */
            if (port->prl_msg_rx)
            {
                protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                                 sizeof(port->policy_pd_data));
                port->prl_msg_rx = false;
                if (port->policy_pd_header.sop.NumDataObjects == 0)
                {
                    if (port->policy_pd_header.sop.MessageType == CMTAccept)
                    {
                        port->is_pr_swap          = true;
                        port->policy_has_contract = false;
                        pps_enable_monitor(port, false);
                        port->source_is_apdo = false;
                        /* Wait for tSrcTransition before turning off vbus */
                        timer_start(&port->timers[POLICY_TIMER], ktSrcTransition);
                        policy_set_state(port, PE_PRS_SRC_SNK_Send_Swap, 2);
                    }
                    else if (port->policy_pd_header.sop.MessageType == CMTReject
                             || port->policy_pd_header.sop.MessageType == CMTWait
                             || port->policy_pd_header.sop.MessageType == CMTNotSupported)
                    {
                        policy_set_state(port, PE_SRC_Ready, 0);
                        port->is_pr_swap = false;
                    }
                    else
                    {
                        policy_set_state(port, PE_SRC_Send_Soft_Reset, 0);
                    }
                }
                else
                {
                    /* Received data message */
                    policy_set_state(port, PE_SRC_Send_Soft_Reset, 0);
                }
            }
            else if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                policy_set_state(port, PE_SRC_Ready, 0);
                port->is_pr_swap = false;
            }
            break;
        case 2:
            /* Wait for tSrcTransition and then turn off power (and Rd on/Rp off) */
            if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                /* Disable VBUS, set alarm (vSafe0V), and discharge */
                fusbdev_tcpd_set_vbus_alarm(port->tcpd_device, VBUS_VSAFE0_ALARM, 0);
                dev_tcpd_port_vbus_src(port, VBUS_OFF, VBUS_OFF, 0);
                fusbdev_tcpd_set_vbus_disch(port->tcpd_device, VBUS_DISCH_DEFAULT);
                port->source_or_sink = Sink;
                policy_set_state(port, PE_PRS_SRC_SNK_Send_Swap, 3);
            }
            break;
        case 3:
            if (port_vbus_vsafe0v(port))
            {
                /* We've reached vSafe0V */
                fusbdev_tcpd_set_vbus_disch(port->tcpd_device, VBUS_DISCH_OFF);
                fusbdev_tcpd_set_vbus_alarm(port->tcpd_device, 0, 0);
                typec_role_swap_to_sink(port);
#if CONFIG_ENABLED(USB4)
                port->usb_mode = false;
#endif
                port->policy_is_source = false;
#if (CONFIG_ENABLED(EPR))
                port->epr_mode = false;
#endif
                policy_set_port_roles(port);
                policy_set_state(port, PE_PRS_SRC_SNK_Send_Swap, 4);
            }
            else
            {
                fusbdev_tcpd_set_vbus_alarm(port->tcpd_device, VBUS_VSAFE0_ALARM, 0);
            }
            break;
        case 4:
            status = policy_send(port, CMTPS_RDY, 0, 0, SOP_SOP0, false, PE_PRS_SRC_SNK_Send_Swap,
                                 5, PE_SRC_Ready, 0);
            /* Allow time for the supply to fall and then send the PS_RDY message */
            if (status == STAT_SUCCESS)
            {
                timer_start(&port->timers[POLICY_TIMER], ktPSSourceOn);
            }
            else if (status == STAT_ERROR)
            {
                policy_set_state(port, PE_ErrorRecovery, 0);
            }
            break;
        case 5:
            /* Wait to receive a PS_RDY message from the new DFP */
            if (port->prl_msg_rx)
            {
                protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                                 sizeof(port->policy_pd_data));
                port->prl_msg_rx = false;
                if (port->policy_pd_header.sop.NumDataObjects == 0)
                {
                    switch (port->policy_pd_header.sop.MessageType)
                    {
                        case CMTPS_RDY:
                            policy_set_state(port, PE_PRS_SRC_SNK_Send_Swap, 6);
                            timer_start(&port->timers[POLICY_TIMER], ktGoodCRCDelay);
                            break;
                        default:
                            break;
                    }
                }
            }
            else if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                port->is_pr_swap = false;
                /* Note: Compliance testing seems to require BOTH HR and ER here. */
                policy_send_source_hardreset(port);
                policy_set_state(port, PE_ErrorRecovery, 0);
            }
            break;
        case 6:
            if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                policy_set_state(port, PE_SNK_Startup, 0);
                dev_tcpd_port_vbus_snk(port, VBUS_5V, CC_RD_CURRENT(port->cc_term_pd_debounce),
                                       pdoTypeFixed);
            }
            break;
        default:
            policy_set_state(port, PE_ErrorRecovery, 0);
            break;
    }
#endif
}

static void policy_state_source_evaluate_prswap(struct port *port)
{
#if (CONFIG_ENABLED(DRP))
    int status = 0;
    switch (port->policy_substate)
    {
        case 0:
            /* Sending accept or reject */
            if (port->prl_msg_rx_sop != SOP_SOP0 || !port->caps_source[0].FPDOSupply.DualRolePower
                || !port->vif->Accepts_PR_Swap_As_Src)
            {
                /* Send the reject/NS if we are not a DRP */
                policy_send_notsupported(port);
            }
            else if (!port->partner_caps_available
                     || (port->caps_header_received.sop.MessageType != DMTSourceCapabilities))
            {

                status = policy_send(port, CMTWait, 0, 0, port->prl_msg_rx_sop, false,
                		PE_SRC_Ready, 0, PE_SRC_Ready, 0);
				port->msgtx.booltxmsg.get_src_caps = true;
            }
            else if (!port->partner_caps.FPDOSupply.DualRolePower
                     || (port->caps_source[0].FPDOSupply.ExternallyPowered
                         && port->partner_caps.FPDOSupply.SupplyType == pdoTypeFixed
                         && !port->partner_caps.FPDOSupply.ExternallyPowered))
            {
                /* Send the reject if partner is not a DRP or Ext powered */
                status = policy_send(port, CMTReject, 0, 0, port->prl_msg_rx_sop, false,
                                     PE_SRC_Ready, 0, PE_SRC_Ready, 0);
            }
            else
            {
                status = policy_send(port, CMTAccept, 0, 0, port->prl_msg_rx_sop, false,
                                     PE_PRS_SRC_SNK_Evaluate_Swap, 1, PE_SRC_Ready, 0);

                if (status == STAT_SUCCESS)
                {
                    port->is_pr_swap          = true;
                    port->policy_has_contract = false;
                    /* Wait for tSrcTransition before turning off vbus */
                    timer_start(&port->timers[POLICY_TIMER], ktSrcTransition);
                }
            }
            break;
        case 1:
            if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                /* Disable VBUS, set alarm (vSafe0V), and discharge */
                fusbdev_tcpd_set_vbus_alarm(port->tcpd_device, VBUS_VSAFE0_ALARM, 0);
                dev_tcpd_port_vbus_src(port, VBUS_OFF, VBUS_OFF, 0);
                fusbdev_tcpd_set_vbus_disch(port->tcpd_device, VBUS_DISCH_DEFAULT);
                port->source_or_sink = Sink;
                policy_set_state(port, PE_PRS_SRC_SNK_Evaluate_Swap, 2);
            }
            break;
        case 2:
            if (port_vbus_vsafe0v(port))
            {
                /* We've reached vSafe0V */
                fusbdev_tcpd_set_vbus_disch(port->tcpd_device, VBUS_DISCH_OFF);
                fusbdev_tcpd_set_vbus_alarm(port->tcpd_device, 0, 0);
                typec_role_swap_to_sink(port);
                port->policy_is_source = false;
#if (CONFIG_ENABLED(EPR))
                port->epr_mode = false;
#endif
                policy_set_port_roles(port);
                policy_set_state(port, PE_PRS_SRC_SNK_Evaluate_Swap, 3);
            }
            else
            {
                fusbdev_tcpd_set_vbus_alarm(port->tcpd_device, VBUS_VSAFE0_ALARM, 0);
            }
            break;
        case 3:
            status = policy_send(port, CMTPS_RDY, 0, 0, port->prl_msg_rx_sop, false,
                                 PE_PRS_SRC_SNK_Evaluate_Swap, 4, PE_ErrorRecovery, 0);
            if (status == STAT_SUCCESS)
            {
                timer_start(&port->timers[POLICY_TIMER], ktPSSourceOn);
            }
            else if (status == STAT_ERROR)
            {
                policy_set_state(port, PE_ErrorRecovery, 0);
            }
            break;
        case 4:
            /* Wait to receive a PS_RDY message from the new DFP */
            if (port->prl_msg_rx)
            {
                protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                                 sizeof(port->policy_pd_data));
                port->prl_msg_rx = false;
                if (port->policy_pd_header.sop.NumDataObjects == 0)
                {
                    switch (port->policy_pd_header.sop.MessageType)
                    {
                        case CMTPS_RDY:
                            timer_disable(&port->timers[POLICY_TIMER]);
                            policy_set_state(port, PE_PRS_SRC_SNK_Evaluate_Swap, 5);
                            timer_start(&port->timers[POLICY_TIMER], ktGoodCRCDelay);
                            break;
                        default:
                            break;
                    }
                }
            }
            else if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                port->is_pr_swap = false;
                policy_set_state(port, PE_ErrorRecovery, 0);
            }
            break;
        case 5:
            if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                policy_set_state(port, PE_SNK_Startup, 0);
                dev_tcpd_port_vbus_snk(port, VBUS_5V, CC_RD_CURRENT(port->cc_term_pd_debounce),
                                       pdoTypeFixed);
                port->is_pr_swap = false;
            }
            break;
        default:
            policy_set_state(port, PE_ErrorRecovery, 0);
            break;
    }
#else
    policy_send_notsupported(port);
#endif
}

static void policy_state_source_wait_new_capabilities(struct port *port)
{
    /* Setup SoftReset exit as intended if Sink realizes something is wrong */
    /* Still potential for Live lock scenario, but only if the sink is at fault */
    if (port->prl_msg_rx)
    {
        protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                         sizeof(port->policy_pd_data));
        port->prl_msg_rx = false;
        if (port->policy_pd_header.sop.NumDataObjects == 0)
        {
            if (port->policy_pd_header.sop.MessageType == CMTSoftReset)
            {
                policy_set_state(port, PE_SRC_Soft_Reset, 0);
            }
        }
    }
}
#endif
static void policy_state_chunking_not_supported(struct port *port)
{
    PD_HEADER_T header = {0};
    switch (port->policy_substate)
    {
        case 0:
            if (CONFIG_ENABLED(EXTMSG) && (port->vif->Chunking_Implemented_SOP))
            {
                /* Unsupported message, protocol layer handled it already*/
                port->policy_sending_message = true;
                policy_set_state(port, policy_state_send_soft_reset(port), 0);
            }
            else
            {
                header.word = port->policy_pd_data[0].word[0];
                if (header.ext.DataSize > 26)
                {
                    /* Multi-chunk message wait for timer before sending not supported */
                    timer_start(&port->timers[POLICY_TIMER], ktChunkingNotSupported);
                    policy_set_state(port, PE_Chunk_NotSupported, 1);
                }
                else
                {
                    /* Single chunk message send not supported */
                    policy_send_notsupported(port);
                }
            }
            break;
        case 1:
            if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                policy_send_notsupported(port);
            }
            break;
        default:
            policy_set_state(port, PE_ErrorRecovery, 0);
            break;
    }
}
#if CONFIG_ENABLED(SRC)
static void policy_state_source_send_softreset(struct port *port)
{
    uint32_t status = 0;
    switch (port->policy_substate)
    {
    	case 0:
    		protocol_reset(port);
    		policy_set_state(port, PE_SRC_Send_Soft_Reset, 1);
    		break;
        case 1:
            status = policy_send(port, CMTSoftReset, 0, 0, SOP_SOP0, false, PE_SRC_Send_Soft_Reset,
                                 2, PE_SRC_Hard_Reset, 0);
            if (status == STAT_SUCCESS)
            {
                timer_start(&port->timers[POLICY_TIMER], ktSenderResponse);
            }
            break;
        case 2:
            if (port->prl_msg_rx)
            {
                protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                                 sizeof(port->policy_pd_data));
                port->prl_msg_rx = false;
                if ((port->policy_pd_header.sop.NumDataObjects == 0)
                    && (port->policy_pd_header.sop.MessageType == CMTAccept))
                {
                    policy_set_state(port, PE_SRC_Send_Capabilities, 0);
                    timer_disable(&port->timers[POLICY_TIMER]);
                }
            }
            else if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                policy_send_source_hardreset(port);
            }
            break;
        default:
            policy_set_state(port, PE_ErrorRecovery, 0);
            break;
    }
}

static void policy_state_source_softreset(struct port *port)
{
    uint32_t status = 0;
    if (port->policy_substate == 0)
    {
    	protocol_reset(port);
    	port->policy_substate++;
    }
    status = policy_send(port, CMTAccept, 0, 0, SOP_SOP0, false, PE_SRC_Send_Capabilities, 0,
                         PE_SRC_Hard_Reset, 0);
    if (status == STAT_SUCCESS)
    {
    	port->enter_usb_timeout = false;
		
    }
}
#endif
#if (CONFIG_ENABLED(USB4))
static void policy_state_send_enter_usb(struct port *port)
{
    uint32_t            status      = 0;
    unsigned            i           = 0;
    enum policy_state_t state_ready = policy_state_ready(port);
    enum policy_state_t state_soft_reset =
        port->policy_is_source ? PE_SRC_Send_Soft_Reset : PE_SNK_Send_Soft_Reset;
    struct event_enter_usb_response_t resp_evt = {0};
    struct event_enter_usb_request_t  req_evt  = {0};
    enum sop_t tx_sop = 0;
    uint8_t temp_val = 0;
    switch (port->policy_substate)
    {
        case 0:
            /* Check if dfp */
            if (port->policy_is_dfp)
            {
                policy_set_state(port, PE_DEU_Send_Enter_USB, 1);
            }
            else
            {
                policy_set_state(port, state_ready, 0);
            }
            break;
        case 1:
            /* Tell dpm to fill out values */
            req_evt.arg.enter_usb_msg = port->policy_pd_data;
            req_evt.ret.success       = EVENT_STATUS_DEFAULT;
            for (i = 0; i < PD_DATA_OBJECT_LEN; i++)
            {
                req_evt.arg.enter_usb_msg->byte[i] = 0;
            }
            req_evt.arg.sop                              = port->prl_msg_rx_sop;
            req_evt.arg.enter_usb_msg->EUDO.CableCurrent = 2 + (port_max_cable_current(port)/5000);
            event_notify(EVENT_ENTER_USB_REQUEST, port->dev, &req_evt);
            policy_set_state(port, PE_DEU_Send_Enter_USB, 2);
            /* Fall through - no break */
        case 2:
        	temp_val = port->usb_supported_sopx - port->enter_usb_sent_sopx;
        	tx_sop = (temp_val & (1U << SOP_SOP1)) ? SOP_SOP1 : ((temp_val & (1U << SOP_SOP2)) ? SOP_SOP2 : SOP_SOP0);
        	status =
                policy_send(port, DMTEnterUSB, port->policy_pd_data[0].byte, PD_DATA_OBJECT_LEN,
                            tx_sop, false, PE_DEU_Send_Enter_USB, 3, state_ready, 0);
            if (status == STAT_SUCCESS)
            {
                timer_start(&port->timers[POLICY_TIMER], ktSenderResponse);
                port->enter_usb_sent_sopx |= (1U << tx_sop);
            }
            break;
        case 3:
            resp_evt.arg.response = false;
            /* Receive accept or reject */
            if (port->prl_msg_rx)
            {
                protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                                 sizeof(port->policy_pd_data));
                timer_disable(&port->timers[POLICY_TIMER]);
                port->prl_msg_rx = false;
                if (port->policy_pd_header.sop.NumDataObjects == 0 && port->prl_msg_rx_sop == port->prl_msg_tx_sop)
                {
                    /* Received a control message */
                    if (port->policy_pd_header.sop.MessageType == CMTAccept)
                    {
                        resp_evt.arg.response = true;
                        policy_set_state(port, state_ready, 0);
                        port->usb_mode = true;
                    }
                    else if (port->policy_pd_header.sop.MessageType == CMTReject
                             || port->policy_pd_header.sop.MessageType == CMTNotSupported)
                    {
                        port->enter_usb_rejected |= 1U << port->prl_msg_rx_sop;
                    }
                    else if (port->policy_pd_header.sop.MessageType == CMTWait)
                    {
                    	policy_set_state(port, state_ready, 0);
                    	if (port->enter_usb_waited_sopx == 0)
                    	{
                    		port->enter_usb_waited_sopx |= 1U << port->prl_msg_rx_sop;
                    		timer_start(&port->timers[ENTER_USB_WAIT_TIMER], ktEnterUSBWait);
                    	}
                    	else
                    	{
                    		port->enter_usb_rejected |= 1U << port->prl_msg_rx_sop;
							policy_set_state(port, state_soft_reset, 0);
                    	}

                    }
                    else
                    {
                    	port->enter_usb_rejected |= 1U << port->prl_msg_rx_sop;
                        policy_set_state(port, state_soft_reset, 0);
                    }
                }
                /* Inform dpm of success or failure */
                resp_evt.arg.sop     = port->prl_msg_rx_sop;
                resp_evt.ret.success = EVENT_STATUS_DEFAULT;
    #if (CONFIG_ENABLED(EXTENDED_EVENT))
                event_notify(EVENT_ENTER_USB_RESPONSE, port->dev, &resp_evt);
    #endif
                if (resp_evt.ret.success == EVENT_STATUS_DEFAULT)
                {
                	if ((port->usb_supported_sopx - port->enter_usb_sent_sopx) != 0 && port->enter_usb_rejected == 0 &&
                			(port->usb_supported_sopx - port->enter_usb_sent_sopx) <  (7U))
                	{
                		policy_set_state(port, PE_DEU_Send_Enter_USB, 0);
                	}
                	else if (port->policy_state != state_soft_reset)
                	{
                		port->msgtx.booltxmsg.enterusb = false;
                		port->usb_supported_sopx &= VALID_SOPMASK;
                		port->usb_supported_sopx &= VALID_SOPMASK;
                		policy_set_state(port, state_ready, 0);
                	}
                }
            }
            else if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                resp_evt.arg.response = false;
                port->enter_usb_rejected |= 1U << port->prl_msg_rx_sop;
                policy_set_state(port, state_ready, 0);
            }
            break;
        default:
            policy_set_state(port, PE_ErrorRecovery, 0);
            break;
    }
}

static void policy_state_process_enter_usb(struct port *port)
{
    enum policy_state_t state_ready       = policy_state_ready(port);
    struct event_enter_usb_received_t evt = {0};
    int                               status;
    switch (port->policy_substate)
    {
        case 0:
            /* Check if ufp and UFP VDO speed is not 0 */
            if (!port->policy_is_dfp && ((port->vif->UFPVDO_USB_Highest_Speed != 0 && port->vif->Is_USB4_Peripheral) ||
            	(port->vif->USB_Comms_Capable && (port->vif->Product_Type_UFP_SOP != 0 && port->vif->Product_Type_UFP_SOP != 3))))
            {
                policy_set_state(port, PE_UEU_Enter_USB_Received, 1);
            }
            else
            {
                status = policy_send(port, CMTReject, 0, 0, port->prl_msg_rx_sop, false,
                                     state_ready, 0, state_ready, 0);
            }
            break;
        case 1:
            /* Inform dpm of message and get told which response to send back*/
            evt.arg.enter_usb_msg = port->policy_pd_data;
            evt.arg.sop           = port->prl_msg_rx_sop;
            evt.ret.success       = EVENT_STATUS_DEFAULT;
            event_notify(EVENT_ENTER_USB_RECEIVED, port->dev, &evt);
            if (evt.ret.success == EVENT_STATUS_FAILED)
            {
                status = policy_send(port, CMTReject, 0, 0, port->prl_msg_rx_sop, false,
                                     state_ready, 0, state_ready, 0);
            }
            else
            {
                status = policy_send(port, CMTAccept, 0, 0, port->prl_msg_rx_sop, false,
                                     state_ready, 0, state_ready, 0);
                if (status == STAT_SUCCESS)
                {
                    port->usb_mode = true;
                }
            }
            break;
        default:
            policy_set_state(port, PE_ErrorRecovery, 0);
            break;
    }
}
#endif
#if CONFIG_ENABLED(SRC)
static void policy_state_source_give_pps_status(struct port *port)
{
    if (port->policy_substate == 0)
    {
        struct event_pps_status_request_t event = {0};
        event.arg.ppssdb  = (struct ext_pps_stat_t *)port->policy_pd_data[0].byte;
        event.ret.success = EVENT_STATUS_DEFAULT;
        event_notify(EVENT_PPS_STATUS_REQUEST, port->dev, &event);
        policy_set_state(port, PE_SRC_Give_PPS_Status, 1);
        if (event.ret.success != EVENT_STATUS_SUCCESS)
        {
            /* Set unknown */
            event.arg.ppssdb->OutputVoltage = 0xFFFF;
            event.arg.ppssdb->OutputCurrent = 0xFF;
            event.arg.ppssdb->Flags         = 0;
        }
    }

    policy_send(port, EMTPPSStatus, port->policy_pd_data[0].byte, EXT_PPS_STAT_LEN, SOP_SOP0, true,
                PE_SRC_Ready, 0, PE_SRC_Send_Soft_Reset, 0);
}
#endif
#if (CONFIG_ENABLED(SNK))
static void policy_state_sink_get_source_info(struct port *port)
{
    uint32_t status = 0;
    switch (port->policy_substate)
    {
        case 0:
            status = policy_send(port, CMTGetSourceInfo, port->policy_pd_data[0].byte, 0, SOP_SOP0,
                                 false, PE_SNK_Get_Source_Info, 1, PE_SNK_Ready, 0);
            if (status == STAT_SUCCESS)
            {
                timer_start(&port->timers[POLICY_TIMER], ktSenderResponse);
            }
            if (status != STAT_BUSY)
            {
            	port->msgtx.booltxmsg.getsourceinfo = false;
            }
            break;
        case 1:
            if (port->prl_msg_rx)
            {
                protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                                 sizeof(port->policy_pd_data));
                port->prl_msg_rx = false;
                if (port->policy_pd_header.sop.NumDataObjects > 0
                    && port->policy_pd_header.sop.MessageType == DMTSourceInfo)
                {
                    policy_set_state(port, PE_SNK_Ready, 0);
                    timer_disable(&port->timers[POLICY_TIMER]);
                }
                else if (port->policy_pd_header.sop.NumDataObjects == 0 &&
                        (port->policy_pd_header.sop.MessageType == CMTReject ||
     					port->policy_pd_header.sop.MessageType == CMTNotSupported))
				{
                	policy_set_state(port, PE_SNK_Ready, 0);
				}
                else
                {
                    timer_disable(&port->timers[POLICY_TIMER]);
                    policy_set_state(port, PE_SNK_Send_Soft_Reset, 0);
                }
            }
            else if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                policy_set_state(port, PE_SNK_Send_Soft_Reset, 0);
            }
            break;
    }
}
#endif
#if CONFIG_ENABLED(SRC)
static void policy_state_source_give_source_info(struct port *port)
{
    struct event_pd_give_source_info_t evt    = {0};
    uint32_t                           status = 0;
    switch (port->policy_substate)
    {
        case 0:
            evt.arg.info         = &port->policy_pd_data[0];
            evt.ret.success      = EVENT_STATUS_DEFAULT;
            evt.arg.info->object = 0;
            event_notify(EVENT_PD_GIVE_SOURCE_INFO, port->dev, &evt);
            policy_set_state(port, PE_SRC_Give_Source_Info, 1);
            break;
        case 1:
            status = policy_send(port, DMTSourceInfo, port->policy_pd_data[0].byte, 4, SOP_SOP0,
                                 false, PE_SRC_Ready, 0, PE_SRC_Send_Soft_Reset, 0);
            break;
        default:
            break;
    }
}
#endif
static void policy_state_get_revision(struct port *port)
{
    struct pd_msg_t    *rev         = &port->policy_pd_data[0];
    enum policy_state_t state_ready = policy_state_ready(port);
    enum policy_state_t state_soft_reset = policy_state_send_soft_reset(port);
    int status = 0;
    switch (port->policy_substate)
    {
        case 0:
            status = policy_send(port, CMTGetRevision, port->policy_pd_data[0].byte, 0, SOP_SOP0, false,
                        PE_Get_Revision, 1, state_soft_reset, 0);
            if (status == STAT_SUCCESS)
            {
            	timer_start(&port->timers[POLICY_TIMER], ktSenderResponse);
            	port->msgtx.booltxmsg.getrevision = false;
            }
            break;
        case 1:
            if (port->prl_msg_rx)
            {
                protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                                 sizeof(port->policy_pd_data));
                port->prl_msg_rx = false;
                if (port->policy_pd_header.sop.NumDataObjects > 0
                    && port->policy_pd_header.sop.Extended == 0
                    && port->policy_pd_header.sop.MessageType == DMTRevision)
                {
                    /* If we receive a revision message */
                    policy_set_state(port, state_ready, 0);
                }
                else if (port->policy_pd_header.sop.NumDataObjects == 0
                         && port->policy_pd_header.sop.MessageType == CMTNotSupported)
                {
                    policy_set_state(port, state_ready, 0);
                }
                else
                {
                    policy_set_state(port, state_soft_reset, 0);
                }
            }
            else if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                policy_set_state(port, state_ready, 0);
            }
            break;
        default:
            break;
    }
}

static void policy_state_give_revision(struct port *port)
{
    struct event_pd_give_revision_t evt = {0};
    enum policy_state_t state_ready     = policy_state_ready(port);
    enum policy_state_t state_soft_reset = policy_state_send_soft_reset(port);
    switch (port->policy_substate)
    {
        case 0:
            evt.arg.revision         = &port->policy_pd_data[0];
            evt.ret.success          = EVENT_STATUS_DEFAULT;
            evt.arg.revision->object = 0;
            event_notify(EVENT_PD_GIVE_REVISION, port->dev, &evt);
            policy_set_state(port, PE_Give_Revision, 1);
            break;
        case 1:
            policy_send(port, DMTRevision, port->policy_pd_data[0].byte, 4, SOP_SOP0, false,
                        state_ready, 0, state_soft_reset, 0);
            break;
        default:
            break;
    }
}
static void policy_state_give_status(struct port *port)
{
    struct event_pd_status_t evt         = {0};
    int status = 0;
    unsigned                 i           = 0;
    enum policy_state_t      state_ready = policy_state_ready(port);
    enum policy_state_t      state_soft_reset = policy_state_send_soft_reset(port);
    switch (port->policy_substate)
    {
        case 0:
            evt.arg.stat    = (struct ext_status_t *)port->policy_pd_data[0].byte;
            evt.arg.sop     = port->prl_msg_rx_sop;
            evt.ret.success = EVENT_STATUS_DEFAULT;
            for (i = 0; i < EXT_STATUS_LEN; i++)
            {
                evt.arg.stat->bytes[i] = 0;
            }
#if (CONFIG_ENABLED(EXTENDED_EVENT))
            event_notify(EVENT_PD_STATUS, port->dev, &evt);
#endif
            policy_set_state(port, PE_Give_Status, 1);
            /* Fall through - no break */
        case 1:
            status = policy_send(port, EMTStatus, port->policy_pd_data[0].byte, EXT_STATUS_LEN,
                        port->prl_msg_rx_sop, true, state_ready, 1, state_soft_reset, 0);
            break;
        default:
            break;
    }
}

static uint8_t least_significant_bit(uint32_t value)
{
	uint8_t i = 0;
	uint8_t lsb = 32;
	for (i = 0; i < 32; i++)
	{
		if ((1U << i) & value)
		{
			lsb = i;
			break;
		}
	}
	return lsb;
}
#if CONFIG_ENABLED(SRC)
const static uint8_t SOURCE_TX_REQ_ACTION_ARRAY[NumTxMsgs] =
{
	/* VDM_Extended (not supported) */
    PE_SRC_Ready,
	/* Extended Control (special) */
	PE_SRC_Ready,
	/* FWUpdateRequest (not supported) */
	PE_SRC_Ready,
	/* SecurityRequest (not supported) */
	PE_SRC_Ready,
	/* GetManufacturerInfo (not supported) */
	PE_SRC_Ready,
	/* GetBatteryStatus */
	PE_Get_Battery_Status,
	/* GetBatteryCapabilities */
	PE_Get_Battery_Cap,
	/* EPRMode (special), */
	PE_SRC_Ready,
	/* EPRRequest (not supported as source) */
	PE_SRC_Ready,
	/* EnterUSB */
#if CONFIG_ENABLED(USB4)
	PE_DEU_Send_Enter_USB,
#else
	PE_SRC_Ready,
#endif
	/* GetCountryInfo (not supported), */
	PE_SRC_Ready,
	/* Alert */
	PE_Send_Alert,
	/* GetRevision */
	PE_Give_Revision,
	/* GetSourceInfo (not supported TODO: DRP Support if req.) */
	PE_SRC_Ready,
	/* GetSinkCapExt */
	PE_SRC_Get_Sink_Cap_Ext,
	/* GetCountryCodes (not supported), */
	PE_SRC_Ready,
	/* GetPPSStatus (not supported as source), */
	PE_SRC_Ready,
	/* FRSwap (not supported) */
	PE_SRC_Ready,
	/* GetStatus, */
	PE_Get_Status,
	/* GetSrcCapExtended */
#if CONFIG_ENABLED(DRP)
	PE_DR_SRC_Get_Source_Cap_Ext,
#else
	PE_SRC_Ready,
#endif
	/* DataReset */
#if CONFIG_ENABLED(USB4)
	PE_Send_Data_Reset,
#else
	PE_SRC_Ready,
#endif

	/* VDM (special) */
#if CONFIG_ENABLED(VDM) && !CONFIG_ENABLED(VDM_MINIMAL)
	PE_GIVE_VDM,
#else
	PE_SRC_Ready,
#endif
	/* BIST */
	PE_SRC_Ready,
	/* Source Caps (not special as source) */
	PE_SRC_Send_Capabilities,
	/* Sink Caps (not special as source) */
#if CONFIG_ENABLED(DRP)
	PE_DR_SRC_Give_Sink_Cap,
#else
	PE_SRC_Ready,
#endif
	/* Request (not special as source) */
	PE_SRC_Ready,
	/* VconnSwap */
	PE_VCS_Send_Swap,
	/* PRSwap */
#if CONFIG_ENABLED(DRP)
	PE_PRS_SRC_SNK_Send_Swap,
#else
	PE_SRC_Ready,
#endif
	/* DRSwap */
	PE_DRS_DFP_UFP_Send_Swap,
	/* GetSinkCaps (not special), */
	PE_SRC_Get_Sink_Cap,
	/* GetSourceCaps (not special), */
#if CONFIG_ENABLED(DRP)
	PE_DR_SRC_Get_Source_Cap,
#else
	PE_SRC_Ready,
#endif
	/* GoToMin, */
	PE_SRC_Transition_Supply,
};
#endif
#if CONFIG_ENABLED(SNK)
const static uint8_t SINK_TX_REQ_ACTION_ARRAY[NumTxMsgs] =
{
	/* VDM_Extended (not supported) */
	PE_SNK_Ready,
	/* Extended Control (should be special, but for now is not special) */
	PE_SNK_Ready,
	/* FWUpdateRequest - not supported */
	PE_SNK_Ready,
	/* SecurityRequest - not supported */
	PE_SNK_Ready,
	/* GetManufacturerInfo - not supported */
	PE_SNK_Ready,
	/* GetBatteryStatus */
#if CONFIG_ENABLED(DRP)
	PE_Get_Battery_Status,
#else
	PE_SNK_Ready,
#endif
	/* GetBatteryCapabilities */
#if CONFIG_ENABLED(DRP)
	PE_Get_Battery_Cap,
#else
	PE_SNK_Ready,
#endif
	/* EPRMode (should be special, but for now is not special), */
	PE_SNK_Ready,
	/* EPRRequest (may be special - right now it is not) */
	PE_SNK_Ready,
	/* EnterUSB */
#if CONFIG_ENABLED(USB4)
	PE_DEU_Send_Enter_USB,
#else
    PE_SNK_Ready,
#endif
	/* GetCountryInfo (not supported), */
	PE_SNK_Ready,
	/* Alert */
	PE_Send_Alert,
	/* GetRevision */
	PE_Get_Revision,
	/* GetSourceInfo */
	PE_SNK_Get_Source_Info,
	/* GetSinkCapExt */
#if CONFIG_ENABLED(DRP)
	PE_DR_SNK_Get_Sink_Cap_Ext,
#else
    PE_SNK_Ready,
#endif
	/* GetCountryCodes (not supported), */
	PE_SNK_Ready,
	/* GetPPSStatus, */
	PE_SNK_Get_PPS_Status,
	/* FRSwap (not supported) */
	PE_SNK_Ready,
	/* GetStatus, */
	PE_Get_Status,
	/* GetSrcCapExtended */
	PE_SNK_Get_Source_Cap_Ext,
	/* DataReset */
#if CONFIG_ENABLED(USB4)
	PE_Send_Data_Reset,
#else
	PE_SNK_Ready,
#endif
	/* VDM (special - may require extra information + event_notify) */
	PE_GIVE_VDM,
	/* BIST (not supported) */
	PE_SNK_Ready,

    /* Source Caps (not special as source) */
#if CONFIG_ENABLED(DRP)
	PE_DR_SNK_Give_Source_Cap,
#else
	PE_SNK_Ready,
#endif
	/* Sink Caps (not special) */
	PE_SNK_Give_Sink_Cap,
	/* Request (not special - can be special in EPR) */
	PE_SNK_Select_Capability,
	/* VconnSwap (not special - DPM should not violate VIF) */
	PE_VCS_Send_Swap,
	/* PRSwap */
#if CONFIG_ENABLED(DRP)
	PE_PRS_SNK_SRC_Send_Swap,
#else
    PE_SNK_Ready,
#endif
	/* DRSwap */
	PE_DRS_UFP_DFP_Send_Swap,
	/* GetSinkCaps, */
#if CONFIG_ENABLED(DRP)
    PE_DR_SNK_Get_Sink_Cap,
#else
	PE_SNK_Ready,
#endif
	/* GetSourceCaps, */
	PE_SNK_Get_Source_Cap,
    /* GoToMin */
    PE_SNK_Ready,
};
#endif
static bool policy_process_tx_req(struct port *port)
{
    /* Handle only message that requires entering specific states. All
     * other messages can be sent as generic message. */
	bool ret = false;
    bool        is_src = port->policy_is_source;
    enum policy_state_t state = policy_state_ready(port);
    if (port_pd_ver(port, SOP_SOP0) != PD_REV3)
    {
        port->msgtx.bytetxmsg.rev3txmsg = 0;
    }
    if (port->alert_response)
    {
    	port->alert_response = false;
    	uint32_t temp = port->msgtx.value &= 1U << GetBatteryStatus;
    }
    uint8_t ls = least_significant_bit(port->msgtx.value);
#if CONFIG_ENABLED(USB4)
    if (ls > 31 || (!port->policy_is_dfp && ((port->vif->Is_USB4_Peripheral && port->vif->UFPVDO_USB_Highest_Speed != 0) ||
       (port->vif->USB_Comms_Capable && (port->vif->Product_Type_UFP_SOP != 0 && port->vif->Product_Type_UFP_SOP != 3)))  &&
        !timer_expired(&port->timers[ENTER_USB_TIMER])))
	{
    	ret = false;
	}
#else
    if (ls > 31)
    {
    	ret = false;
    }
#endif
    else
    {
#if CONFIG_ENABLED(SRC) && CONFIG_ENABLED(SNK)
        if (port->policy_is_source)
        {
            state = SOURCE_TX_REQ_ACTION_ARRAY[31 - ls];
        }
        else
        {
            state = SINK_TX_REQ_ACTION_ARRAY[31 - ls];
        }
#elif CONFIG_ENABLED(SRC)
    	state = SOURCE_TX_REQ_ACTION_ARRAY[31 - ls];
#elif CONFIG_ENABLED(SNK)
    	state = SINK_TX_REQ_ACTION_ARRAY[31 - ls];
#endif
    	/* means we do something */
    	if (state != policy_state_ready(port))
    	{
    		policy_set_state(port, state, 0);
    		ret = true;
    	}
    	else
    	{
    		/* handle special cases */
    		event_notify(EVENT_TX_REQ, port->dev, 0);
    		if (state != policy_state_ready(port))
    		{
    			policy_set_state(port, state, 0);
    			ret = true;
    		}
    		else
    		{
    			/* clear left shift if not normal message action and not special message action 
    			   this implies no action should be taken */
    			port->msgtx.value &= ~(1U << ls);
    		}
    	}

    }
    return ret;
}
#if CONFIG_ENABLED(EPR)
static void policy_state_read_extended_control(struct port *port)
{
    /*  policy_state__read_extended_control */
    struct ext_control_t msg = {0};
    msg.bytes[0]             = port->policy_pd_data[0].byte[2];
    msg.bytes[1]             = port->policy_pd_data[0].byte[3];
    /*  if (port->policy_is_source) */
#if (CONFIG_ENABLED(SRC))
    if (port->policy_state == PE_SRC_Ready)
    {
#if CONFIG_ENABLED(EPR)
        if (msg.type == EPR_Get_Source_Cap)
        {
            if (port->policy_is_source)
            {
                if (port->epr_mode)
                {
                    policy_set_state(port, PE_SRC_Send_Capabilities, 0);
                }
                else
                {
                    policy_set_state(port, PE_SRC_Give_Source_Cap, 0);
                }
            }
            else
            {
                /* not supported or ignore? */
            }
        }
        else if (msg.type == EPR_KeepAlive)
        {
            /* Send EPR_KeepAlive_Ack */
            policy_set_state(port, PE_SRC_EPR_Keep_Alive, 0);
        }
        else if (msg.type == EPR_Get_Sink_Cap)
        {
#if CONFIG_ENABLED(DRP)
                policy_set_state(port, PE_DR_SRC_Give_Sink_Cap, 0);
#else

#endif
        }
        else
        {
            /* Unexpected message */
            policy_set_state(port, PE_SRC_Send_Soft_Reset, 0);
        }
#endif
    }
#endif
#if (CONFIG_ENABLED(SNK))
    if (port->policy_state == PE_SNK_Ready)
    {
#if CONFIG_ENABLED(EPR)
        if (msg.type == EPR_Get_Source_Cap)
        {
#if CONFIG_ENABLED(DRP)
                policy_set_state(port, PE_DR_SNK_Give_Source_Cap, 0);
#else
                policy_set_state(port, PE_SNK_Send_Not_Supported, 0);
#endif
        }
        else if (msg.type == EPR_Get_Sink_Cap)
        {
            if (!port->policy_is_source)
            {
                policy_set_state(port, PE_SNK_Give_Sink_Cap, 0);
            }
            else
            {
                /* not supported or ignore for now, no DRP products supporting EPR */
            }
        }
        else
        {
            /* Unexpected message */
            policy_set_state(port, PE_SNK_Send_Soft_Reset, 0);
        }
#endif
    }
#endif
}
#endif
#if CONFIG_ENABLED(SRC)
#define NUM_SRC_CONTROL_MESSAGES 25
#define NUM_SRC_DATA_MESSAGES 16
#define NUM_SRC_EXTENDED_MESSAGES 17
#define CONTROL_SRC_INDEX 0
#define DATA_SRC_INDEX (CONTROL_SRC_INDEX + NUM_SRC_CONTROL_MESSAGES)
#define EXTENDED_SRC_INDEX (DATA_SRC_INDEX + NUM_SRC_DATA_MESSAGES)
#define NUM_SRC_SUPPORTED_MESSAGES (NUM_SRC_CONTROL_MESSAGES + NUM_SRC_DATA_MESSAGES + NUM_SRC_EXTENDED_MESSAGES)
const uint8_t SOURCE_MESSAGE_ACTION_ARRAY[NUM_SRC_SUPPORTED_MESSAGES] = {
/* TOTAL 25 Control Messages */
/* NUMBER of NOT_Supported Responses */
/* CMT 0x00 - Reserved */
        PE_SRC_Send_Not_Supported,
/* CMT 0x01 - GoodCRC, */
        PE_SRC_Ready,
/* CMT 0x02 - GoToMin */
        PE_SRC_Send_Not_Supported,
/* CMT 0x03 - Accept */
        PE_SRC_Send_Soft_Reset,
/* CMT 0x04 - Reject */
        PE_SRC_Ready,
/* CMT 0x05 - Ping */
        PE_SRC_Send_Not_Supported,
/* CMT 0x06 - PS_RDY */
        PE_SRC_Send_Not_Supported,
/* CMT 0x07 - Get_Source_Cap - SPECIAL */
        PE_SRC_Send_Capabilities,
/* CMT 0x08 - Get_Sink_Cap */
#if (CONFIG_ENABLED(DRP))
        PE_DR_SRC_Give_Sink_Cap,
#else
        PE_SRC_Send_Not_Supported,
#endif
/* CMT 0x09 - DR_Swap */
        PE_DRS_DFP_UFP_Evaluate_Swap,
/* CMT 0x0A - PR_Swap */
#if (CONFIG_ENABLED(DRP))
        PE_PRS_SRC_SNK_Evaluate_Swap,
#else
        PE_SRC_Send_Not_Supported,
#endif
/* CMT 0x0B - Vconn_Swap */
        PE_VCS_Evaluate_Swap,
/* CMT 0x0C - Wait */
        PE_SRC_Send_Soft_Reset,
/* CMT 0x0D - Soft_Reset */
        PE_SRC_Soft_Reset,
/* CMT 0x0E - Data_Reset */
#if (CONFIG_ENABLED(USB4))
        PE_Evaluate_Data_Reset,
#else
        PE_SRC_Send_Not_Supported,
#endif
/* CMT 0x0F - Data_Reset_Complete */
        PE_SRC_Send_Soft_Reset,
/* CMT 0x10 - Not_Supported */
        PE_SRC_Ready,
/* CMT 0x11 - Get_Source_Cap_Extended */
        PE_SRC_Give_Source_Cap_Ext,
/* CMT 0x12 - Get_Status */
        PE_Give_Status,
/* CMT 0x13 - FR_Swap */
#if (CONFIG_ENABLED(DRP))
        PE_SRC_Send_Not_Supported,
#else
        PE_SRC_Send_Not_Supported,
#endif
/* CMT 0x14 - Get_PPS_Status */
        PE_SRC_Give_PPS_Status,
/* CMT 0x15 - Get_Country_Codes */
        PE_SRC_Send_Not_Supported,
/* CMT 0x16 - Get_Sink_Cap_Extended */
#if CONFIG_ENABLED(DRP)
        PE_DR_SRC_Give_Sink_Cap_Ext,
#else
        PE_SRC_Send_Not_Supported,
#endif
/* CMT 0x17 - Get_Source_Info */
        PE_SRC_Give_Source_Info,
/* CMT 0x18 - Get_Revision */
        PE_Give_Revision,
/* TOTAL 16 Data Message Types */
/* DMT 0x00 - Reserved */
        PE_SRC_Send_Not_Supported,
/* DMT 0x01 - Source_Capabilies, */
        PE_SRC_Ready,
/* DMT 0x02 - Request - SPECIAL */
        PE_SRC_Negotiate_Capability,
/* DMT 0x03 - BIST - SPECIAL */
        PE_SRC_Ready,
/* DMT 0x04 - Sink_Capabilities - SPECIAL */
        PE_SRC_Ready,
/* DMT 0x05 - Battery_Status */
        PE_SRC_Send_Not_Supported,
/* DMT 0x06 - Alert */
        PE_Alert_Received,
/* DMT 0x07 - Get_Country_Info */
        PE_SRC_Send_Not_Supported,
/* DMT 0x08 - Enter_USB */
#if CONFIG_ENABLED(USB4)
        PE_UEU_Enter_USB_Received,
#else
        PE_SRC_Send_Not_Supported,
#endif
/* Idea: End Data messages here and use if statements for 3 special messages */
/* DMT 0x09 - EPR_Request - SPECIAL */
#if CONFIG_ENABLED(EPR)
        PE_SRC_Ready,
#else
        PE_SRC_Send_Not_Supported,
#endif
/* DMT 0x0A - EPR_Mode - SPECIAL */
#if CONFIG_ENABLED(EPR)
        PE_SRC_Ready,
#else
        PE_SRC_Send_Not_Supported,
#endif
/* DMT 0x0B - Source_Info */
        PE_SRC_Send_Not_Supported,
/* DMT 0x0C - Revision */
        PE_SRC_Send_Not_Supported,
/* DMT 0x0D - Reserved */
        PE_SRC_Send_Not_Supported,
/* DMT 0x0E - Reserved */
        PE_SRC_Send_Not_Supported,
/* DMT 0x0F - Vendor_Defined - SPECIAL */
        PE_SRC_Ready,
/* TOTAL 23 Extended MESSAGE types */
/* EMT 0x00 - Reserved */
        PE_SRC_Send_Not_Supported,
/* EMT 0x01 - Source_Capabilities_Extended, */
        PE_SRC_Ready,
/* EMT 0x02 - Status */
        PE_SRC_Send_Not_Supported,
/* EMT 0x03 - Get_Battery_Cap */
        PE_Give_Battery_Cap,
/* EMT 0x04 - Get_Battery_Status */
        PE_Give_Battery_Status,
/* EMT 0x05 - Battery_Capabilities */
        PE_SRC_Send_Not_Supported,
/* EMT 0x06 - Get_Manufacturer_Info */
        PE_Give_Manufacturer_Info,
/* EMT 0x07 - Manufacturer_Info */
        PE_SRC_Send_Not_Supported,
/* EMT 0x08 - Security_Request */
        PE_SRC_Send_Not_Supported,
/* EMT 0x09 - Security_Response */
        PE_SRC_Send_Not_Supported,
/* EMT 0x0A - Firmware_Update_Request */
#if (CONFIG_ENABLED(FW_UPDATE))
        PE_Send_Firmware_Update_Response,
#else
        PE_SRC_Send_Not_Supported,
#endif
/* EMT 0x0B - Firmware_Update_Response */
        PE_SRC_Send_Not_Supported,
/* EMT 0x0C - PPS_Status */
        PE_SRC_Ready,
/* EMT 0x0D - Country_Info */
        PE_SRC_Send_Not_Supported,
/* EMT 0x0E - Country_Code */
        PE_SRC_Send_Not_Supported,
/* EMT 0x0F - Sink_Capabilities_Extended */
        PE_SRC_Ready,
/* EMT 0x10 - Extended_Control - SPECIAL */
#if (CONFIG_ENABLED(EPR))
        PE_SRC_Ready,
#else
		PE_SRC_Send_Not_Supported,
#endif
/* EMT 0x11 - EPR_Source_Capabilities - SPECIAL */
        /* PE_SRC_Send_Not_Supported, */
/* EMT 0x12 - EPR_Sink_Capabilities - SPECIAL */
        /* PE_SRC_Ready, */
/* EMT 0x13 - Reserved */
        /* PE_SRC_Send_Not_Supported, */
/* EMT 0x14 - Reserved */
        /* PE_SRC_Send_Not_Supported, */
/* EMT 0x15 - Reserved */
        /* PE_SRC_Send_Not_Supported, */
/* EMT 0x16 - Vendor_Defined_Extended */
        /* PE_SRC_Send_Not_Supported, */
};
#if CONFIG_ENABLED(USB4)
static void policy_usb4_action_center(struct port * port)
{
	/* dont send if VIF type is not PDUSB Host / Hub */
	if (!port->usb_mode && (port->enter_usb_rejected == 0) &&
		 port->vif->Product_Type_DFP_SOP!= 0 && port->vif->Product_Type_DFP_SOP !=3)
	{

#if USB4_TEST
		/* WAIT for SOP_SOP0 messages to send */
		if ((port->disc_id_sent_sopx & ((1U << SOP_SOP0))) == ((1U << SOP_SOP0) ))
#else
		/* WAIT for both DISC_ID messages to send */
		if ((port->disc_id_sent_sopx & ((1U << SOP_SOP0) | (1U << SOP_SOP1))) == ((1U << SOP_SOP0) | (1U << SOP_SOP1)))
#endif
		{
#if USB4_TEST
			/* if SOP0 doesn't support USB4, just leave */
			if ((port->usb_supported_sopx & ((1U << SOP_SOP0) | (1U << SOP_SOP1))) == ((1U << SOP_SOP0)))
#else
			/* if SOP0 and SOP1 doesn't support USB4, just leave */
			if ((port->usb_supported_sopx & ((1U << SOP_SOP0) | (1U << SOP_SOP1))) == ((1U << SOP_SOP0) | (1U << SOP_SOP1)))
#endif
			{
				/* If cable has received TBT3 DISC_MODES OR if cable has not received DISC_SVID ACK despite sending REQ */
				if ((port->disc_modes_received_sopx & (1U << SOP_SOP1)) ||
					 ((port->disc_svid_sent_sopx & (1U << SOP_SOP1)) &&
					  !(port->disc_svid_received_sopx & (1U << SOP_SOP1))))
				{
					if (port->enter_usb_waited_sopx == 0)
					{
						/* prepare to send enterUSB */
						port->msgtx.booltxmsg.enterusb = true;
						/* if passive cable, don't send on SOP1 (clear SOP_SOP1 bit) */
						if (port->cable_vdo_cable_type_ufp == VDM_ID_UFP_SOP_1_PASSIVE_CABLE)
						{
							port->usb_supported_sopx &= ~(1U << SOP_SOP1);
						}
					}
					else if (timer_expired(&port->timers[ENTER_USB_WAIT_TIMER]) && !timer_disabled(&port->timers[ENTER_USB_WAIT_TIMER]))
					{
						timer_disable(&port->timers[ENTER_USB_WAIT_TIMER]);
						/* prepare to re-send enterUSB */
						port->msgtx.booltxmsg.enterusb = true;
					}
				}
			}
			else if (timer_expired(&port->timers[ENTER_USB_WAIT_TIMER])&& !timer_disabled(&port->timers[ENTER_USB_WAIT_TIMER]))
			{
				timer_disable(&port->timers[ENTER_USB_WAIT_TIMER]);
				/* prepare to re-send enterUSB */
				port->msgtx.booltxmsg.enterusb = true;
			}
		}
	}
	if (timer_expired(&port->timers[ENTER_USB_TIMER]) && !timer_disabled(&port->timers[ENTER_USB_TIMER]))
	{
		port->enter_usb_timeout = true;
	}
}
#endif


static void policy_dp_alt_mode_action_center(struct port * port)
{
#if MODAL_OPERATION_SUPPORTED && CONFIG_ENABLED(USB4)
	if (!port->usb_mode && !port->mode_entered && port->enter_usb_timeout && !vdm_is_active(port->dev, 0xFF00) && !port->svid_enabled)
	{
		/* enable SVID driver now that we timed out USB4 */
        /* This will allow SVID to complete. */
		vdm_enable_driver(port->dev, 0xFF00, (1U << SOP_SOP0));
		vdm_list_reset_all(port->dev, 0xFF00, ((1U << SOP_SOP0)), 2);
		port->vdm_auto_state = true;
		port->svid_enabled = true;
		a = 1;
	}
	else
	{
	}
#endif
}

/* Store all automatic actions that a source will do */
static void source_vif_message_requests(struct port *port)
{
	if (port->req_dr_swap_To_ufp_as_src &&
		port->policy_is_dfp &&
	    port->vif->DR_Swap_To_UFP_Supported)
	{
		port->msgtx.booltxmsg.drswap = true;
	}
	if (port->req_vconn_swap_to_off_as_src && port->is_vconn_source
             && port->vif->VCONN_Swap_To_Off_Supported)
	{
		port->msgtx.booltxmsg.vconnswap = true;
	}
	if ((port->current_limited && port->vdm_cbl_present && port->higher_cable_cap))
	{
		port->msgtx.booltxmsg.src_caps = true;
	}
	if (!CONFIG_ENABLED(MINIMAL) && port->partner_caps.object == 0)
	{
		/* note no difference between EPR and nonEPR here */
		port->msgtx.booltxmsg.get_snk_caps = true;
	}
#if CONFIG_ENABLED(DRP)
	if (port->type == TypeC_DRP && port->req_pr_swap_as_src
             && port->partner_caps.FPDOSink.DualRolePower == 1)
	{
	    port->msgtx.booltxmsg.prswap = 1;
	}
#endif
#if CONFIG_ENABLED(USB4)
	policy_usb4_action_center(port);
#endif
#if MODAL_OPERATION_SUPPORTED
	policy_dp_alt_mode_action_center(port);
#endif
}


static void policy_state_source_ready(struct port *port)
{
    PD_HEADER_T msg = {0};
    source_vif_message_requests(port);
    /* Make sure EPR mode terminated before any other behavior */
#if (CONFIG_ENABLED(EPR))
    if (timer_expired(&port->timers[EPR_KEEPALIVE_TIMER]) && port->epr_mode)
    {
        policy_state_invalid(port);
    }
    else if (port->prl_msg_rx)
#else
    if (port->prl_msg_rx)
#endif
    {
        protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                         sizeof(port->policy_pd_data));
        port->prl_msg_rx = false;
        msg.word         = port->policy_pd_header.word;
        uint8_t lowermessageIndex = 0;
        uint8_t uppermessageIndex = NUM_SRC_CONTROL_MESSAGES;
        uint16_t currentStateAction = PE_SRC_Send_Not_Supported;
        if (msg.sop.NumDataObjects != 0)
        {
            if (msg.sop.Extended == 0)
            {
                lowermessageIndex = NUM_SRC_CONTROL_MESSAGES;
                uppermessageIndex = NUM_SRC_CONTROL_MESSAGES + NUM_SRC_DATA_MESSAGES;
            }
            else
            {
                lowermessageIndex = NUM_SRC_CONTROL_MESSAGES + NUM_SRC_DATA_MESSAGES;
                uppermessageIndex = NUM_SRC_SUPPORTED_MESSAGES;
            }
        }
        if (lowermessageIndex + msg.sop.MessageType < uppermessageIndex)
        {
            currentStateAction = SOURCE_MESSAGE_ACTION_ARRAY[lowermessageIndex + msg.sop.MessageType];
        }
        if (!port->vif->Chunking_Implemented_SOP && uppermessageIndex == NUM_SRC_SUPPORTED_MESSAGES)
        {
            currentStateAction = PE_Chunk_NotSupported;
        }
        policy_set_state(port, currentStateAction, 0);
#if CONFIG_ENABLED(EPR)
        if (msg.sop.MessageType == EMTExtendedControl && msg.sop.Extended == 1 && msg.sop.NumDataObjects != 0)
        {
            policy_state_read_extended_control(port);
        }
        else if (msg.sop.MessageType == DMTEPRMode && port_pd_ver(port, SOP_SOP0) == PD_REV3 &&
                 msg.sop.NumDataObjects != 0 && msg.sop.Extended == 0)
        {
            policy_state_read_epr_mode(port);
        }
        else if (port->epr_mode)
        {

            if (msg.sop.NumDataObjects == 0)
            {
                /* CMT 0x07 - Get_Source_Cap - SPECIAL */
                if (msg.sop.MessageType == CMTGetSourceCap)
                {
                    policy_set_state(port, PE_SRC_Give_Source_Cap, 0);
                }
            }
            else if (msg.sop.Extended == 0)
            {
                /* DMT 0x02 - Request - SPECIAL */
                /* DMT 0x01 - Source_Capabilities - SPECIAL */
                /* DMT 0x04 - Sink_Capabilities - SPECIAL */
                if ((msg.sop.MessageType == DMTRequest ||
                    msg.sop.MessageType == DMTSinkCapabilities ||
                    msg.sop.MessageType == DMTSourceCapabilities)
                    && msg.sop.NumDataObjects != 0)
                {
                    policy_state_invalid(port);
                }
                else if (msg.sop.MessageType == DMTEPRequest)
                {
                    policy_set_state(port, PE_SRC_Negotiate_Capability, 0);
                }
                
                /* DMT 0x0A - EPR_Mode - SPECIAL - handled elsewhere */
            }
            /* EMT 0x10 - Extended_Control - SPECIAL - handled elsewhere */
            /* EMT 0x11 - EPR_Source_Capabilities - SPECIAL */
            /* do nothing for now - default behavior fine */
            /* EMT 0x12 - EPR_Sink_Capabilities - SPECIAL */
            /* do nothing for now - default behavior fine */
        }
        else
        {
            /* CMT 0x07 - Get_Source_Cap - SPECIAL - do nothing */
            /* DMT 0x02 - Request - SPECIAL - do nothing */
            /* DMT 0x09 - EPR_Request - SPECIAL */
            if (msg.sop.MessageType == DMTEPRequest && msg.sop.NumDataObjects != 0 &&
                msg.sop.Extended == 0)
            {
                policy_state_invalid(port);
            }
            /* DMT 0x0A - EPR_Mode - SPECIAL - handled elsewhere */
            /* EMT 0x10 - Extended_Control - SPECIAL - handled elsewhere */
            /* EMT 0x11 - EPR_Source_Capabilities - SPECIAL */
            /* do nothing for now - default behavior fine */
            /* EMT 0x12 - EPR_Sink_Capabilities - SPECIAL */
            /* do nothing for now - default behavior fine */
        }
#endif
        if (msg.sop.NumDataObjects != 0 && msg.sop.Extended == 0)
        {
            /* DMT 0x03 - BIST - SPECIAL */
            if (msg.sop.MessageType == DMTBIST)
            {
                policy_process_bist_message(port);
            }
            /* DMT 0x0F - Vendor_Defined - SPECIAL */
#if CONFIG_ENABLED(VDM)
            else if (msg.sop.MessageType == DMTVendorDefined)
            {
                policy_process_vdmrx_engine(port);
            }
#endif
        }
        port->policy_ams_start = false;
        port->policy_ams_active = currentStateAction != PE_SRC_Ready && currentStateAction != PE_SRC_Send_Not_Supported &&
                                  currentStateAction != PE_SRC_Hard_Reset;
    }/* Do not initiate AMS if waiting on extended message to be received */
    else if (!port->prl_msg_rx_pending)
    {
	#if CONFIG_ENABLED(VDM)
		if (port->vdm_check_cbl && policy_check_comm_allowed(port, SOP_MASK_SOP1))
		{
			policy_set_state(port, PE_CBL_Query, 0);
		}
	#endif
		else if (port->current_limited && port->vdm_cbl_present && port->higher_cable_cap)
		{
			/* TODO: Move to DPM.c*/
			/* If port is current limited due to cable, cable with new higher capability is discovered
				 * then re-send the source caps. Set the new higher capability */
			policy_set_state(port, PE_SRC_Send_Capabilities, 0);
			/* Set higher capability to false until new higher capability is discovered */
			port->higher_cable_cap = false;
		}
		else if (!CONFIG_ENABLED(MINIMAL) && port->cbl_rst_state > CBL_RST_DISABLED)
		{
			policy_process_cable_reset(port);
		}
		else if (port->source_is_apdo && timer_expired(&port->timers[PPS_TIMER]))
		{
			/* No PPS re-request within time limit */
			policy_send_source_hardreset(port);
			timer_disable(&port->timers[PPS_TIMER]);
		}
	#if CONFIG_ENABLED(VDM) && !CONFIG_ENABLED(VDM_MINIMAL)
		else if (port->vif->Attempts_Discov_SOP
				 && port->policy_is_dfp && port->vdm_auto_state
				 && timer_expired(&port->timers[POLICY_TIMER])
				 && timer_expired(&port->timers[RP_CHANGE_TIMER])
				 && timer_expired(&port->timers[DISCOVER_IDENTITY_TIMER]))
		{
			timer_disable(&port->timers[RP_CHANGE_TIMER]);
			policy_process_vdm_auto_discv(port);
		}
	#endif
		else if (policy_process_tx_req(port))
		{

		}
		else
		{
			if (timer_expired(&port->timers[POLICY_TIMER]))
			{
				timer_disable(&port->timers[POLICY_TIMER]);
			}
			if (timer_expired(&port->timers[PD_RSP_TIMER]))
			{
				timer_disable(&port->timers[PD_RSP_TIMER]);
			}
			if (timer_expired(&port->timers[RP_CHANGE_TIMER]))
			{
				timer_disable(&port->timers[RP_CHANGE_TIMER]);
			}
			if (timer_expired(&port->timers[DISCOVER_IDENTITY_TIMER]))
			{
				timer_disable(&port->timers[DISCOVER_IDENTITY_TIMER]);
			}
			/* Wait for COMP or RX_HRDRST or RX_STAT */
			dev_tcpd_port_set_sinktx(port, SinkTxOK);
		}
    }
}
#endif
#if (CONFIG_ENABLED(SNK))
static void policy_state_sink_startup(struct port *port)
{
    port->policy_has_contract       = false;
    port->usb_pd_contract.object    = 0;
    port->caps_header_received.word = 0;
    port->sink_selected_voltage     = VBUS_5V;
    port->usb_pd_contract.object    = 0;
    port->partner_caps.object       = 0;
    port->is_pr_swap                = false;
    port->is_hard_reset             = false;
    port->policy_is_source          = false;
    protocol_reset_sop(port, SOP_MASK_SOP0);
    vdm_list_reset_all(port->dev, 0, ((1U << SOP_SOP0) | (1U << SOP_SOP1) | (1U << SOP_SOP2)), 0);
    policy_set_port_roles(port);
    fusbdev_tcpd_pd_enable(port->tcpd_device, true);

    port->vdm_auto_state   = false;
    port->higher_cable_cap = false;

    port->caps_counter      = 0;
    port->policy_ams_active = false;
    port->policy_ams_start  = false;
    timer_disable(&port->timers[RP_CHANGE_TIMER]);
    timer_disable(&port->timers[PD_RSP_TIMER]);
    timer_disable(&port->timers[POLICY_TIMER]);
    timer_disable(&port->timers[PPS_TIMER]);
    policy_set_state(port, PE_SNK_Discovery, 0);

    if (CONFIG_ENABLED(VDM))
    {
        if (port->vif->Attempts_Discov_SOP)
        {
            port->vdm_auto_state = true;
        }
#if MODAL_OPERATION_SUPPORTED
        port->mode_entered = false;
#endif
    }
    policy_reset_message_queue(port);
}

static void policy_state_sink_softreset(struct port *port)
{
    uint32_t status = 0;
    if (port->policy_substate == 0)
    {
    	protocol_reset(port);
    	port->policy_substate++;
    }
    status = policy_send(port, CMTAccept, 0, 0, SOP_SOP0, false, PE_SNK_Wait_For_Capabilities, 0,
                         PE_SNK_Hard_Reset, 0);
    if (status == STAT_SUCCESS)
    {
        port->policy_ams_start = false;
        timer_disable(&port->timers[RP_CHANGE_TIMER]);
        port->policy_ams_active = false;
        timer_start(&port->timers[POLICY_TIMER], ktTypeCSinkWaitCap);
		port->enter_usb_timeout = false;
		timer_start(&port->timers[ENTER_USB_TIMER], ktEnterUSB);
    }
}

static void policy_state_sink_send_softreset(struct port *port)
{
    uint32_t status = 0;
    switch (port->policy_substate)
    {
        case 0:
        	protocol_reset(port);
        	policy_set_state(port, PE_SNK_Send_Soft_Reset, 1);
        	break;
        case 1:
            status = policy_send(port, CMTSoftReset, 0, 0, SOP_SOP0, false, PE_SNK_Send_Soft_Reset,
                                 2, PE_SNK_Hard_Reset, 0);
            if (status == STAT_SUCCESS)
            {
                timer_start(&port->timers[POLICY_TIMER], ktSenderResponse);
            }
            break;
        case 2:
            if (port->prl_msg_rx)
            {
                protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                                 sizeof(port->policy_pd_data));
                port->prl_msg_rx = false;
                if (port->policy_pd_header.sop.NumDataObjects == 0
                    && port->policy_pd_header.sop.MessageType == CMTAccept)
                {
                    timer_disable(&port->timers[POLICY_TIMER]);
                    port->policy_ams_active = false;
                    port->policy_ams_start  = false;
                    timer_disable(&port->timers[RP_CHANGE_TIMER]);
                    policy_set_state(port, PE_SNK_Wait_For_Capabilities, 0);
                    timer_start(&port->timers[POLICY_TIMER], ktTypeCSinkWaitCap);
                    port->enter_usb_timeout = false;
                    timer_start(&port->timers[ENTER_USB_TIMER], ktEnterUSB);
                }
            }
            else if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                policy_send_sink_hardreset(port);
            }
            break;
        default:
            policy_set_state(port, PE_ErrorRecovery, 0);
            break;
    }
}

static void policy_state_sink_transitiondefault(struct port *port)
{
    switch (port->policy_substate)
    {
        case 0:
            port->is_hard_reset       = true;
            port->policy_has_contract = false;
            /* Timeout (Vbus Off) handling required for Type-C only connections */
            timer_start(&port->timers[POLICY_TIMER], ktPSHardResetMax + ktSafe0V);
            if (port->policy_is_dfp)
            {
                port->policy_is_dfp = false;
                policy_set_port_roles(port);
            }
            policy_set_vconn(port, false);
            dev_tcpd_port_vbus_snk(port, VBUS_OFF, VBUS_OFF, 0);
            fusbdev_tcpd_set_vbus_alarm(port->tcpd_device, VBUS_VSAFE0_ALARM, 0);
            policy_set_state(port, PE_SNK_Transition_To_Default, 1);
            break;
        case 1:
            if (port_vbus_vsafe0v(port))
            {
                fusbdev_tcpd_set_vbus_alarm(port->tcpd_device, 0, VBUS_LOW_THRESHOLD(VBUS_5V));
                /* Timeout (VBUS On) required for Type-C only connections */
                timer_start(&port->timers[POLICY_TIMER], ktSrcRecoverMax + ktSrcTurnOn);
                policy_set_state(port, PE_SNK_Transition_To_Default, 2);
            }
            else if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                policy_pd_enable(port, port->policy_is_source);
            }
            else
            {
                fusbdev_tcpd_set_vbus_alarm(port->tcpd_device, VBUS_VSAFE0_ALARM, 0);
            }
            break;
        case 2:
            if (port_vbus_vsafe5v(port))
            {
                /* Re-enable sinking VBus and discharge system */
                dev_tcpd_port_vbus_snk(port, VBUS_5V, CC_RD_CURRENT(port->cc_term_pd_debounce),
                                       pdoTypeFixed);
                fusbdev_tcpd_set_vbus_alarm(port->tcpd_device, VBUS_5V_DISC, 0);
                fusbdev_tcpd_set_vbus_disch(port->tcpd_device, VBUS_DISCH_BLEED);
                policy_pd_enable(port, port->policy_is_source);
                timer_disable(&port->timers[POLICY_TIMER]);
            }
            else if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                /* VBus never recovered */
                policy_set_state(port, PE_ErrorRecovery, 0);
            }
            else
            {
                fusbdev_tcpd_set_vbus_alarm(port->tcpd_device, 0, VBUS_LOW_THRESHOLD(VBUS_5V));
            }
            break;
        default:
            policy_set_state(port, PE_ErrorRecovery, 0);
            break;
    }
}

static void policy_state_sink_discovery(struct port *port)
{
    policy_set_state(port, PE_SNK_Wait_For_Capabilities, 0);
    port->is_hard_reset = false;
    timer_start(&port->timers[POLICY_TIMER], ktTypeCSinkWaitCap);
}

static void policy_state_sink_waitcaps(struct port *port)
{ /* TODO: Add EMTEPRSourceCaps support */
    if (port->prl_msg_rx)
    {
#if CONFIG_ENABLED(POWER_SHARING) || CONFIG_ENABLED(LEGACY_CHARGING)
        event_notify(EVENT_PD_DEVICE, port->dev, 0);
#endif
        protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                         sizeof(port->policy_pd_data));
        port->prl_msg_rx = false;

        if (((port->policy_pd_header.sop.NumDataObjects > 0)
             && (port->policy_pd_header.sop.MessageType == DMTSourceCapabilities)))
        {
            policy_set_partner_cap(port, true);
            policy_set_state(port, PE_SNK_Evaluate_Capability, 0);
            timer_disable(&port->timers[POLICY_TIMER]);
            if (port_pd_ver(port, SOP_SOP0) > port->policy_pd_header.sop.SpecRevision)
            {
                dev_tcpd_port_pd_set_ver(port, SOP_SOP0, port->policy_pd_header.sop.SpecRevision);
            }
#if CONFIG_ENABLED(USB4)
            /* start enter_usb timer for both DFP and UFP */
            timer_start(&port->timers[ENTER_USB_TIMER], ktEnterUSB);
            port->enter_usb_timeout = false;
            if (!port->policy_is_dfp)
			{
			}
            /* if no discover identity sent on SOP', and port is NOT VCONN SOURCE but IS DFP, vconn swap */
			else if (!port->is_vconn_source && !(port->disc_id_sent_sopx & 1U << SOP_SOP1 == 1U << SOP_SOP1))
			{
				port->msgtx.booltxmsg.vconnswap = true;
			}
#endif
        }
#if CONFIG_ENABLED(EPR)
        else if (port->policy_pd_header.sop.Extended && port->epr_mode
                 && port->policy_pd_header.sop.MessageType == EMTEPRSourceCapabilities)
        {
            policy_set_partner_cap(port, true);

            policy_set_state(port, PE_SNK_Evaluate_Capability, 0);
            timer_disable(&port->timers[POLICY_TIMER]);
            if (port_pd_ver(port, SOP_SOP0) > port->policy_pd_header.sop.SpecRevision)
            {
                dev_tcpd_port_pd_set_ver(port, SOP_SOP0, port->policy_pd_header.sop.SpecRevision);
            }
        }
#endif
        else if ((port->policy_pd_header.sop.NumDataObjects == 0)
                 && (port->policy_pd_header.sop.MessageType == CMTSoftReset))
        {
            policy_set_state(port, PE_SNK_Soft_Reset, 0);
        }
    }
    else if (port->policy_has_contract == true && timer_expired(&port->timers[PD_RSP_TIMER])
             && port->hard_reset_counter > HARD_RESET_COUNT)
    {
        timer_disable(&port->timers[PD_RSP_TIMER]);
        policy_set_state(port, PE_ErrorRecovery, 0);
    }
    else if (timer_expired(&port->timers[POLICY_TIMER])
             && (port->hard_reset_counter <= HARD_RESET_COUNT))
    {
        timer_disable(&port->timers[POLICY_TIMER]);
        policy_send_sink_hardreset(port);
    }
    else if (port->policy_has_contract == false && timer_expired(&port->timers[PD_RSP_TIMER])
             && port->hard_reset_counter > HARD_RESET_COUNT)
    {
        timer_disable(&port->timers[PD_RSP_TIMER]);
    }
}

static uint32_t policy_sink_max_power_pdo(struct port * port, struct pd_msg_t source_pdo, struct pd_msg_t * request)
{
	uint32_t power_mw = 0;
	for(uint8_t i = 0; i < port->vif->Num_Snk_PDOs; i++)
	{
		/* don't bother if supply type doesn't match */
		if (port->vif->snk_caps[i].FPDOSink.SupplyType == source_pdo.FPDOSupply.SupplyType)
		{
			if (source_pdo.FPDOSupply.SupplyType == pdoTypeFixed)
			{
				if (port->vif->snk_caps[i].FPDOSink.Voltage == source_pdo.FPDOSupply.Voltage)
				{
					uint32_t max_current = MIN(port->vif->snk_caps[i].FPDOSink.OperationalCurrent, source_pdo.FPDOSupply.MaxCurrent);
					power_mw = (port->vif->snk_caps[i].FPDOSink.Voltage*max_current) / 2;
					/* store current */
					request->FVRDO.OpCurrent = max_current;
					break;
				}
			}
			/* dont bother if APDO Type doesn't match */
			else if (source_pdo.FPDOSupply.SupplyType == pdoTypeAugmented && port->vif->snk_caps[i].APDO.APDOType != source_pdo.PPSAPDO.APDOType)
			{
				/* SPR - PPS Augmented PDO */
				if (source_pdo.APDO.APDOType == 0)
				{
					/* For SPR PPS augmented PDOs, grab highest power:
					 * max_current = MIN(source_pdo_max_current, sink_pdo_max_current)
					 *  max_voltage = MIN(source_pdo_max_voltage, sink_pdo_max_voltage)
					 *  max_power = max_current * max_voltage * 5
					 *  from set of PPS PDOs then iterate through all PDOs
					 *  (more than 1 PPS PDO can be supported by the sink) */
					uint32_t max_voltage = MIN(port->vif->snk_caps[i].PPSAPDO.MaxVoltage, source_pdo.PPSAPDO.MaxVoltage);
					uint32_t max_current = MIN(port->vif->snk_caps[i].PPSAPDO.MaxCurrent, source_pdo.PPSAPDO.MaxCurrent);
					if ((port->vif->snk_caps[i].PPSAPDO.MaxCurrent * max_voltage * 5) > power_mw)
					{
						power_mw = max_current * (max_voltage * 5);
						/* Store voltage and current */
						request->PPSRDO.OpVoltage = max_voltage * 5;
						request->PPSRDO.OpCurrent = max_current;
					}
				}
				/* TODO: Add EPR AVS and SPR AVS PDO support */
			}
		}
	}
	return power_mw;
}

static void policy_state_sink_evaluate_caps(struct port *port)
{
    struct event_pd_snk_eval_src_cap_t snk_eval;
    unsigned long src_power_mw = 0;
    unsigned long src_min_power_mw = 0;/* use for pps */
    unsigned long src_voltage = 0;
    unsigned long src_current = 0;
    unsigned long snk_power_mw = 0;
    unsigned long max_power = 0;
    unsigned long curr_max_pow_voltage = 0;
    unsigned long curr_max_pow_current = 0;
    uint8_t maxIndex = 0;
    uint8_t i = 0;
    snk_eval.arg.num         = port->caps_header_received.sop.NumDataObjects;
    snk_eval.arg.pdo         = &port->caps_received[0];
    snk_eval.ret.index       = 0;
    snk_eval.ret.req.object  = 0;
    snk_eval.ret.type        = pdoTypeFixed;
    snk_eval.ret.success     = EVENT_STATUS_DEFAULT;
    port->hard_reset_counter = 0;

    event_notify(EVENT_PD_SNK_EVAL_SRC_CAP, port->dev, &snk_eval);

    if (snk_eval.ret.success == EVENT_STATUS_DEFAULT)
    {
        /* DPM did not set request object so request default PDO1(5V) */
    	snk_power_mw = port->vif->PD_Min_Power_as_Sink;
        for (i = 0; i < snk_eval.arg.num; i++)
        {
        	src_power_mw = 0;
        	src_min_power_mw = 0;/*use for pps */
            src_voltage = 0;
        	src_current = 0;
        	snk_eval.ret.req.object = 0;
        	/* First, calculate max and min power from source PDOs + VIF min and max power */
        	if (port->caps_received[i].FPDOSupply.SupplyType == pdoTypeFixed)
        	{
        		src_voltage = port->caps_received[i].FPDOSupply.Voltage;
        		src_current = port->caps_received[i].FPDOSupply.MaxCurrent;
				src_power_mw = PD_0P5MW_STEP_TO_MW(src_voltage * src_current);
				src_power_mw = MIN(src_power_mw, port->vif->PD_Power_as_Sink);
				src_voltage *=50;
				src_current *=10;
        	} else if (port->caps_received[i].FPDOSupply.SupplyType == pdoTypeAugmented)
        	{
        		src_voltage = port->caps_received[i].PPSAPDO.MaxVoltage;
				src_current = port->caps_received[i].PPSAPDO.MaxCurrent;
				src_power_mw = src_voltage * src_current*5;
				src_power_mw = MIN(src_power_mw, port->vif->PD_Power_as_Sink);
				src_min_power_mw = port->caps_received[i].PPSAPDO.MinVoltage * src_current * 5;
				src_min_power_mw = MAX(src_min_power_mw, port->vif->PD_Min_Power_as_Sink);
				src_voltage *=100;
				src_current *=50;
        	}
        	/* Now, match PDO with equivalent voltage sink PDO */
        	uint32_t max_sink_pdo_power = policy_sink_max_power_pdo(port, port->caps_received[i], &snk_eval.ret.req);
			/* limit current and voltage from match to minimum current and voltage across */
        	if (port->caps_received[i].FPDOSupply.SupplyType == pdoTypeFixed)
			{
				/* limit current as voltage matches */
        		src_current = MIN(src_current, (snk_eval.ret.req.FVRDO.OpCurrent*10));
			}
			else if (port->caps_received[i].FPDOSupply.SupplyType == pdoTypeAugmented)
			{
				src_current = MIN(src_current, (snk_eval.ret.req.PPSRDO.OpCurrent*50));
				src_voltage = MIN(src_voltage, (snk_eval.ret.req.PPSRDO.OpVoltage*20));
			}
        	src_power_mw = MIN(src_power_mw, (src_current * src_voltage)/1000);
        	if (max_power < src_power_mw || (max_power == src_power_mw && src_voltage > curr_max_pow_voltage))
        	{
        		max_power = src_power_mw;
        		maxIndex = i;
        		curr_max_pow_voltage = src_voltage;
        		curr_max_pow_current = src_current;
        	}
        }
        if (maxIndex == 0)
        {
        	/* means no match of Sink PDO and Source PDOs */
        	/* just request 5V + */
        	src_voltage = port->caps_received[0].FPDOSupply.Voltage;
			src_current = port->caps_received[0].FPDOSupply.MaxCurrent;
			src_power_mw = PD_0P5MW_STEP_TO_MW(src_voltage * src_current);
			src_power_mw = MIN(src_power_mw, port->vif->PD_Power_as_Sink);
			src_voltage *=50;
			src_current *=10;
			if (src_power_mw < snk_power_mw)
			{
				snk_eval.ret.req.FVRDO.CapabilityMismatch = 1;
			}
			if (!snk_eval.ret.req.FVRDO.CapabilityMismatch)
			{
				snk_eval.ret.req.FVRDO.MinMaxCurrent = MIN((2 * port->vif->PD_Power_as_Sink) / (src_voltage/50), port->caps_received[maxIndex].FPDOSupply.MaxCurrent);
			}
			else
			{
				snk_eval.ret.req.FVRDO.MinMaxCurrent = MIN((2 * port->vif->PD_Power_as_Sink) / (src_voltage/50), 300);
			}
			snk_eval.ret.req.FVRDO.ObjectPosition  = maxIndex+1;
			snk_eval.ret.req.FVRDO.GiveBack        = port->vif->GiveBack_May_Be_Set;
			snk_eval.ret.req.FVRDO.NoUSBSuspend    = port->vif->No_USB_Suspend_May_Be_Set;
			snk_eval.ret.req.FVRDO.UnchunkedExtMsg = port->vif->Unchunked_Extended_Messages_Supported;
			snk_eval.ret.req.FVRDO.MinMaxCurrent   = MIN(snk_eval.ret.req.FVRDO.MinMaxCurrent, 300);
			snk_eval.ret.req.FVRDO.OpCurrent       = MIN(src_current/10, snk_eval.ret.req.FVRDO.MinMaxCurrent);
			snk_eval.ret.req.FVRDO.USBCommCapable  = port->vif->USB_Comms_Capable;

        }
        if (port->caps_received[maxIndex].FPDOSupply.SupplyType == pdoTypeFixed)
        {
        	if (max_power < snk_power_mw)
			{
				snk_eval.ret.req.FVRDO.CapabilityMismatch = 1;
			}
			if (!snk_eval.ret.req.FVRDO.CapabilityMismatch)
			{
				snk_eval.ret.req.FVRDO.MinMaxCurrent = MIN((2 * port->vif->PD_Power_as_Sink) / (curr_max_pow_voltage/50), port->caps_received[maxIndex].FPDOSupply.MaxCurrent);
			}
			else
			{
				snk_eval.ret.req.FVRDO.MinMaxCurrent = MIN((2 * port->vif->PD_Power_as_Sink) / (src_voltage/50), 300);
			}
			snk_eval.ret.req.FVRDO.ObjectPosition  = maxIndex+1;
			snk_eval.ret.req.FVRDO.GiveBack        = port->vif->GiveBack_May_Be_Set;
			snk_eval.ret.req.FVRDO.NoUSBSuspend    = port->vif->No_USB_Suspend_May_Be_Set;
			snk_eval.ret.req.FVRDO.UnchunkedExtMsg = port->vif->Unchunked_Extended_Messages_Supported;
			snk_eval.ret.req.FVRDO.OpCurrent       = curr_max_pow_current/10;
			snk_eval.ret.req.FVRDO.MinMaxCurrent = MIN(snk_eval.ret.req.FVRDO.MinMaxCurrent, 300);
			snk_eval.ret.req.FVRDO.USBCommCapable = port->vif->USB_Comms_Capable;
        }
        else if (port->caps_received[maxIndex].FPDOSupply.SupplyType == pdoTypeAugmented)
        {
        	snk_eval.ret.req.PPSRDO.ObjectPosition = maxIndex + 1;
			snk_eval.ret.req.PPSRDO.NoUSBSuspend    = port->vif->No_USB_Suspend_May_Be_Set;
			snk_eval.ret.req.PPSRDO.UnchunkedExtMsg = port->vif->Unchunked_Extended_Messages_Supported;
			snk_eval.ret.req.PPSRDO.USBCommCapable = port->vif->USB_Comms_Capable;
			snk_eval.ret.req.PPSRDO.OpCurrent = curr_max_pow_current/50;
			snk_eval.ret.req.PPSRDO.OpVoltage = curr_max_pow_voltage/20;
			if (max_power < snk_power_mw)
			{
				snk_eval.ret.req.PPSRDO.CapabilityMismatch = 1;
			}
        }


    }

#if CONFIG_ENABLED(EPR)
    snk_eval.ret.req.EPRCAPRDO.EPRCapable = port->vif->EPR_Mode_Capable;
#endif
    port->sink_request[0].object = snk_eval.ret.req.object;
    port->sink_request[1].object = port->caps_received[snk_eval.ret.index].object;
    policy_set_state(port, PE_SNK_Select_Capability, 0);
}

static void policy_state_sink_select_capability(struct port *port)
{
    int        status   = 0;
    enum pdo_t pdo_type = {0};
    int        index    = 0;
    unsigned   voltage  = 0;
    unsigned   current  = 0;
    switch (port->policy_substate)
    {
        case 0:
#if CONFIG_ENABLED(EPR)
            if (port->epr_mode)
            {
                status = policy_send(port, DMTEPRequest, port->sink_request[0].byte, 8, SOP_SOP0,
                                     false, PE_SNK_Select_Capability, 1, PE_SNK_Send_Soft_Reset, 0);
            }
            else
            {
                status = policy_send(port, DMTRequest, port->sink_request[0].byte, 4, SOP_SOP0,
                                     false, PE_SNK_Select_Capability, 1, PE_SNK_Send_Soft_Reset, 0);
            }
#else
            status = policy_send(port, DMTRequest, port->sink_request[0].byte, 4, SOP_SOP0, false,
                                 PE_SNK_Select_Capability, 1, PE_SNK_Send_Soft_Reset, 0);
#endif
            if (status == STAT_SUCCESS)
            {
                timer_start(&port->timers[POLICY_TIMER], ktSenderResponse);
            }
            if (status != STAT_BUSY)
            {
                port->msgtx.booltxmsg.request = false;
            }
            break;
        case 1:
            if (port->prl_msg_rx)
            {
                protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                                 sizeof(port->policy_pd_data));
                port->prl_msg_rx = false;
                timer_disable(&port->timers[POLICY_TIMER]);
                if (port->policy_pd_header.sop.NumDataObjects == 0)
                {
                    if (port->policy_pd_header.sop.MessageType == CMTAccept)
                    {
                        port->usb_pd_contract.object = port->sink_request[0].object;
                        index                     = port->usb_pd_contract.FVRDO.ObjectPosition - 1;
                        pdo_type                  = port->caps_received[index].PDO.SupplyType;
                        port->policy_has_contract = true;
                        port->source_is_apdo      = false;
                        if (pdo_type == pdoTypeAugmented)
                        {
                            port->source_is_apdo = true;
                            /* Set to minimum in case of current fold back */
                            port->sink_selected_voltage = VBUS_5V;
                            port->stored_apdo.object    = port->sink_request[0].object;
                            timer_start(&port->timers[PPS_TIMER], ktPPSRequest);
                            voltage = port->stored_apdo.PPSRDO.OpVoltage;
                            current = port->stored_apdo.PPSRDO.OpCurrent;
                            port->sink_contract_power = voltage * current;
                        }
                        else if (pdo_type == pdoTypeFixed)
                        {
                            voltage = port->caps_received[index].FPDOSupply.Voltage;
                            current = port->sink_request[0].FVRDO.OpCurrent;
                            port->sink_contract_power = (voltage * current) / 2;
                            voltage *= 50;
                            if (port->sink_selected_voltage < voltage)
                            {
                                fusbdev_tcpd_set_vbus_alarm(
                                    port->tcpd_device, VBUS_DISCONNECT(port->sink_selected_voltage),
                                    0);
                            }
                            port->sink_selected_voltage = voltage;
                        }
                        timer_start(&port->timers[POLICY_TIMER], ktPSTransition);
                        policy_set_state(port, PE_SNK_Transition_Sink, 0);
                    }
                    else if (port->policy_pd_header.sop.MessageType == CMTWait
                             || port->policy_pd_header.sop.MessageType == CMTReject)
                    {
#if (CONFIG_ENABLED(EPR))
                        if (!port->epr_mode || !port->epr_try_exit)
                        {
                            if (port->policy_has_contract)
                            {
                                policy_set_state(port, PE_SNK_Ready, 0);
                            }
                            else
                            {
                                policy_set_state(port, PE_SNK_Wait_For_Capabilities, 0);
                                /* Set the counter to avoid a hard reset loop */
                                port->hard_reset_counter = HARD_RESET_COUNT + 1;
                            }
                        }
                        else if (port->policy_pd_header.sop.MessageType == CMTReject)
                        {
                            policy_state_invalid(port);
                            port->epr_try_exit = false;
                            /* Or send EPR Exit and allow source to hard reset */
                        }
#else
                        if (port->policy_has_contract)
                        {
                            policy_set_state(port, PE_SNK_Ready, 0);
                        }
                        else
                        {
                            policy_set_state(port, PE_SNK_Wait_For_Capabilities, 0);
                            /* Set the counter to avoid a hard reset loop */
                            port->hard_reset_counter = HARD_RESET_COUNT + 1;
                        }
#endif
                    }
                    else if (port->policy_pd_header.sop.MessageType == CMTSoftReset)
                    {
                        policy_set_state(port, PE_SNK_Soft_Reset, 0);
                    }
                    else
                    {
                        policy_set_state(port, PE_SNK_Send_Soft_Reset, 0);
                    }
                }
                else
                {
                    if (port->policy_pd_header.sop.MessageType == DMTSourceCapabilities)
                    {
                        policy_set_partner_cap(port, true);
                        policy_set_state(port, PE_SNK_Evaluate_Capability, 0);
                        port->policy_ams_start = false;
                        timer_disable(&port->timers[RP_CHANGE_TIMER]);
#if CONFIG_ENABLED(USB4)
                        /* start enter_usb timer for both DFP and UFP */
                        timer_start(&port->timers[ENTER_USB_TIMER], ktEnterUSB);
                        port->enter_usb_timeout = false;
                        if (!port->policy_is_dfp)
                        {
                        }
            			else if (!port->is_vconn_source && !(port->disc_id_sent_sopx & 1U << SOP_SOP1 == 1U << SOP_SOP1))
            			{
            				port->msgtx.booltxmsg.vconnswap = true;
            			}
#endif
                    }
                    else
                    {
                        policy_set_state(port, PE_SNK_Send_Soft_Reset, 0);
                    }
                }
            }
            else if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                policy_send_sink_hardreset(port);
            }
            break;
        default:
            policy_set_state(port, PE_ErrorRecovery, 0);
            break;
    }
}
static void policy_state_sink_transition_sink(struct port *port)
{
    int        index   = port->usb_pd_contract.FVRDO.ObjectPosition - 1;
    enum pdo_t type    = port->caps_received[index].PDO.SupplyType;
    unsigned   current = 0;
    if (port->prl_msg_rx)
    {
        protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                         sizeof(port->policy_pd_data));
        port->prl_msg_rx = false;
        if (port->policy_pd_header.sop.NumDataObjects == 0)
        {
            /* Disable the PSTransition timer */
            timer_disable(&port->timers[POLICY_TIMER]);
            if (port->policy_pd_header.sop.MessageType == CMTPS_RDY)
            {
                if (type == pdoTypeFixed || type == pdoTypeVariable)
                {
                    current = PD_10MA_STEP_TO_MA(port->sink_request[0].FVRDO.OpCurrent);
                }
                else if (type == pdoTypeBattery)
                {
                    current = (PD_250MW_STEP_TO_MW(port->sink_request[0].BRDO.OpPower)*1000)
                              / port->sink_selected_voltage;
                }
                else if (type == pdoTypeAugmented)
                {
                    current = PD_50MA_STEP_TO_MA(port->sink_request[0].PPSRDO.OpCurrent);
                }
                dev_tcpd_port_vbus_snk(port, port->sink_selected_voltage, current, type);
                fusbdev_tcpd_set_vbus_alarm(port->tcpd_device,
                                            VBUS_DISCONNECT(port->sink_selected_voltage), 0);
                policy_set_state(port, PE_SNK_Ready, 0);
            }
            else if (port->policy_pd_header.sop.MessageType == CMTSoftReset)
            {
                policy_set_state(port, PE_SNK_Soft_Reset, 0);
            }
            else
            {
                policy_send_sink_hardreset(port);
            }
        }
        else
        {
            if (port->policy_pd_header.sop.MessageType == DMTSourceCapabilities)
            {
                policy_set_partner_cap(port, true);
            }
            else
            {
                /* Unexpected data message */
                policy_set_state(port, PE_SNK_Send_Soft_Reset, 0);
            }
        }
    }
    else if (timer_expired(&port->timers[POLICY_TIMER]))
    {
        timer_disable(&port->timers[POLICY_TIMER]);
        policy_send_sink_hardreset(port);
    }
}
#if CONFIG_ENABLED(DRP)
static void policy_state_sink_get_sink_cap(struct port *port)
{
    uint32_t status = 0;
    switch (port->policy_substate)
    {
        case 0:
            status = policy_send(port, CMTGetSinkCap, 0, 0, SOP_SOP0, false, PE_DR_SNK_Get_Sink_Cap,
                                 1, PE_SNK_Ready, 0);
            if (status == STAT_SUCCESS)
            {
                timer_start(&port->timers[POLICY_TIMER], ktSenderResponse);
            }
            if (status != STAT_BUSY)
            {
            	port->msgtx.booltxmsg.get_snk_caps = false;
            }
            break;
        case 1:
            if (port->prl_msg_rx)
            {
                protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                                 sizeof(port->policy_pd_data));
                port->prl_msg_rx = false;
                if (port->policy_pd_header.sop.NumDataObjects > 0
                    && port->policy_pd_header.sop.MessageType == DMTSinkCapabilities)
                {
                    /* Process new sink caps here if necessary */
                    policy_set_state(port, PE_SNK_Ready, 0);
                }
                else if (port->policy_pd_header.sop.NumDataObjects == 0
                         && (port->policy_pd_header.sop.MessageType == CMTReject
                             || port->policy_pd_header.sop.MessageType == CMTNotSupported))
                {
                    policy_set_state(port, PE_SNK_Ready, 0);
                }
                else
                {
                    policy_set_state(port, PE_SNK_Send_Soft_Reset, 0);
                }
            }
            else if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                policy_set_state(port, PE_SNK_Ready, 0);
            }
            break;
        default:
            policy_set_state(port, PE_ErrorRecovery, 0);
            break;
    }
}
#endif
static void policy_state_sink_give_sink_cap(struct port *port)
{
    uint8_t                status = 0;
    unsigned               i      = 0;
    EVENT_PD_GET_SNK_CAP_T evt    = {0};
    switch (port->policy_substate)
    {
        case 0:
            evt.arg.pd_rev  = port_pd_ver(port, SOP_SOP0);
            evt.ret.success = EVENT_STATUS_DEFAULT;
            evt.arg.snk_cap = port->caps_sink;
            /* Copy the PDOs from VIF */
            port->caps_header_sink.sop.NumDataObjects = port->vif->Num_Snk_PDOs;
            for (int i = 0; i < port->vif->Num_Snk_PDOs; i++)
            {
                port->caps_sink[i] = port->vif->snk_caps[i];
            }
#if CONFIG_ENABLED(EPR)
            if (port->epr_mode)
            {
                /* if in EPR mode send all EPR source caps as well */
                for (int i = 7; i < (7 + (port->vif->Num_Snk_EPR_PDOs)); i++)
                {
                    port->caps_sink[i] = port->vif->snk_caps[i];
                }
            }
#endif
            /* Let DPM handle any processing if needed */
            event_notify(EVENT_PD_GET_SNK_CAP, port->dev, &evt);
#if CONFIG_ENABLED(EPR)
            port->epr_capable_pdo = evt.arg.snk_cap[0].FPDOSupply.EPRCapable;
            if (port->epr_mode)
            {
                /*if in epr mode 0 all source caps between k and 7 */
                for (int i = evt.ret.num; i < 7; i++)
                {
                    port->caps_sink[i].object = 0;
                }
                /*if in epr mode add all epr capabilities */
                event_notify(EVENT_PD_GET_SNK_EPR_CAP, port->dev, &evt);
                if (evt.ret.success == EVENT_STATUS_SUCCESS)
                {
                    port->epr_snk_cap_len = evt.ret.num;
                }
            }
#else
            if (evt.ret.success == EVENT_STATUS_SUCCESS)
            {
                port->caps_header_sink.sop.NumDataObjects = evt.ret.num;
            }
#endif
            policy_set_state(port, PE_SNK_Give_Sink_Cap, 1);
            break;
        case 1:
#if CONFIG_ENABLED(EPR)
            if (port->epr_mode)
            {
                policy_send(port, EMTEPRSinkCapabilities, port->caps_sink[1].byte,
                            port->epr_snk_cap_len * 4, SOP_SOP0, true, PE_SNK_Ready, 0,
                            PE_SNK_Ready, 0);
            }
            else
            {
                policy_send(port, DMTSinkCapabilities, port->caps_sink[0].byte,
                            port->caps_header_sink.sop.NumDataObjects * 4, SOP_SOP0, false,
                            PE_SNK_Ready, 0, PE_SNK_Ready, 0);
            }
#else
            policy_send(port, DMTSinkCapabilities, port->caps_sink[0].byte,
                        port->caps_header_sink.sop.NumDataObjects * 4, SOP_SOP0, false,
                        PE_SNK_Ready, 0, PE_SNK_Send_Soft_Reset, 0);
            if (status != STAT_BUSY)
            {
            	port->msgtx.booltxmsg.snk_caps = false;
            }
#endif
    }
}
static void policy_state_sink_get_source_cap(struct port *port)
{
    uint32_t status = 0;
    switch (port->policy_substate)
    {
        case 0:
#if CONFIG_ENABLED(EPR)
            if (!port->epr_mode)
            {
                status = policy_send(port, CMTGetSourceCap, 0, 0, SOP_SOP0, false,
                                     PE_SNK_Get_Source_Cap, 1, PE_SNK_Ready, 0);
                if (status == STAT_SUCCESS)
                {
                    timer_start(&port->timers[POLICY_TIMER], ktSenderResponse);
                }
                if (status != STAT_BUSY)
                {
                    port->msgtx.booltxmsg.get_src_caps = false;
                }
            }
            else
            {
                struct ext_control_t msg = {0};
                msg.type                 = EPR_Get_Source_Cap;
                msg.data                 = 0;
                int status = policy_send(port, EMTExtendedControl, msg.bytes, EXT_CONTROL_LEN,
                                         SOP_SOP0, true, PE_SNK_Get_Source_Cap, 1, PE_SNK_Ready, 0);
                if (status == STAT_SUCCESS)
                {
                    timer_start(&port->timers[POLICY_TIMER], ktSenderResponse);

                }
                if (status != STAT_BUSY)
                {
                    port->msgtx.booltxmsg.get_src_caps = false;
                }
            }
#else
            status = policy_send(port, CMTGetSourceCap, 0, 0, SOP_SOP0, false,
                                 PE_SNK_Get_Source_Cap, 1, PE_SNK_Ready, 0);
            if (status == STAT_SUCCESS)
            {
                timer_start(&port->timers[POLICY_TIMER], ktSenderResponse);
            }
            if (status != STAT_BUSY)
			{
				port->msgtx.booltxmsg.get_src_caps = false;
			}
#endif
            break;
        case 1:
            if (port->prl_msg_rx)
            {
                protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                                 sizeof(port->policy_pd_data));
                port->prl_msg_rx = false;
                if (port->policy_pd_header.sop.NumDataObjects == 0 &&
                   (port->policy_pd_header.sop.MessageType == CMTReject ||
					port->policy_pd_header.sop.MessageType == CMTNotSupported))
                {
                	policy_set_state(port, PE_SNK_Ready, 0);
                }
                else if (port->policy_pd_header.sop.NumDataObjects > 0
                    && (port->policy_pd_header.sop.MessageType == DMTSourceCapabilities)
#if (CONFIG_ENABLED(EPR))
                    && !(port->epr_mode)
#endif
                )
                {
                    policy_set_partner_cap(port, false);
                    timer_disable(&port->timers[POLICY_TIMER]);
                    port->partner_caps_available = true;
                    policy_set_state(port, PE_SNK_Evaluate_Capability, 0);
                }
#if CONFIG_ENABLED(EPR)
                else if (port->policy_pd_header.sop.Extended && port->epr_mode
                         && port->policy_pd_header.sop.MessageType == EMTEPRSourceCapabilities)
                {
                    policy_set_partner_cap(port, false);
                    timer_disable(&port->timers[POLICY_TIMER]);
                    port->partner_caps_available = true;
                    policy_set_state(port, PE_SNK_Evaluate_Capability, 0);
                }
#endif
                else
                {
                    /* No valid source caps message */
                    policy_set_state(port, PE_SNK_Send_Soft_Reset, 0);
                }
            }
            else if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                policy_set_state(port, PE_SNK_Ready, 0);
            }
            break;
        default:
            policy_set_state(port, PE_ErrorRecovery, 0);
            break;
    }
}

static void policy_state_sink_give_sourcecap(struct port *port)
{
#if CONFIG_ENABLED(DRP)
    struct event_pd_get_src_cap_t evt = {0};
    if (port->type == TypeC_DRP)
    {
        if (port->policy_substate == 0)
        {
            evt.arg.pd_rev  = port_pd_ver(port, SOP_SOP0);
            evt.arg.src_cap = port->caps_source;
            evt.arg.max_ma  = port_max_cable_current(port);
            evt.ret.success = EVENT_STATUS_DEFAULT;
            event_notify(EVENT_PD_GET_SRC_CAP, port->dev, &evt);
            if (evt.ret.success == EVENT_STATUS_SUCCESS)
            {
                port->caps_header_source.sop.NumDataObjects = evt.ret.num;
            }
            policy_set_state(port, PE_DR_SNK_Give_Source_Cap, 1);
        }
        else
        {
            policy_send(port, DMTSourceCapabilities, port->caps_source[0].byte,
                        port->caps_header_source.sop.NumDataObjects * 4, SOP_SOP0, false,
                        PE_SNK_Ready, 0, PE_SNK_Ready, 0);
        }
    }
    else
    {
        policy_send_notsupported(port);
    }
#else
    policy_send_notsupported(port);
#endif
}

static void policy_state_give_extended_sink_cap(struct port *port)
{
    enum policy_state_t succ_state = policy_state_ready(port);
    enum policy_state_t fail_state = policy_state_send_soft_reset(port);

    if (port->policy_substate == 0)
    {
        EVENT_PD_EXT_SNK_CAP_T evt = {0};
        unsigned               i   = 0;
        evt.ret.success            = EVENT_STATUS_DEFAULT;
        evt.arg.buf                = (struct ext_sink_cap_t *)port->policy_pd_data;
        for (i = 0; i < EXT_SINK_CAP_LEN; i++)
        {
            evt.arg.buf->bytes[i] = 0;
        }
#if (CONFIG_ENABLED(EXTENDED_EVENT))
        event_notify(EVENT_PD_GET_EXT_SNK_CAP, port->dev, &evt);
#endif
        policy_set_state(port, port->policy_state, 1);
        if (evt.ret.success == EVENT_STATUS_DEFAULT)
        {
            /* send default values */
            evt.arg.buf->VID         = port->vif->Manufacturer_Info_VID_Port;
            evt.arg.buf->PID         = port->vif->Manufacturer_Info_PID_Port;
            evt.arg.buf->XID         = port->vif->XID_SOP;
            evt.arg.buf->SKEDBVersion = 1;
#if CONFIG_ENABLED(DRP)
            evt.arg.buf->SinkMaxPDP   = port->vif->PD_Power_as_Sink / 1000;
            evt.arg.buf->SinkMinPDP   = 5;
            if (!port->policy_is_source)
            {
				evt.arg.buf->SinkOpPDP    = ((port->sink_contract_power/1000) > 5) ? port->sink_contract_power/1000 : 5;
            }
#else
            evt.arg.buf->SinkMaxPDP   = port->vif->PD_Power_as_Sink / 1000;
			evt.arg.buf->SinkOpPDP    = ((port->sink_contract_power/1000) > 5) ? port->sink_contract_power/1000 : 5;
			evt.arg.buf->SinkMinPDP   = 5;
#endif
            evt.arg.buf->BatteryInfo = (port->vif->Num_Fixed_Batteries & 0xf)
                                       | ((port->vif->Num_Swappable_Battery_Slots << 4) & 0xf0);
#if CONFIG_ENABLED(EPR)
            evt.arg.buf->EPRSinkMaxPDP = 0; /* Not supported yet. */
            evt.arg.buf->EPRSinkOpPDP  = 0;
            evt.arg.buf->EPRSinkMinPDP = 0;
#endif
        }
    }
    policy_send(port, EMTSinkCapExtended, port->policy_pd_data[0].byte, EXT_SINK_CAP_LEN, SOP_SOP0,
                true, succ_state, 0, fail_state, 0);
}

static void policy_state_sink_send_drswap(struct port *port)
{
    uint32_t status = 0;
    switch (port->policy_substate)
    {
        case 0:
            status = policy_send(port, CMTDR_Swap, 0, 0, SOP_SOP0, false, PE_DRS_UFP_DFP_Send_Swap,
                                 1, PE_SNK_Ready, 0);
            if (status == STAT_SUCCESS)
            {
                timer_start(&port->timers[POLICY_TIMER], ktSenderResponse);
                port->req_dr_swap_to_dfp_as_sink = false;
            }
            break;
        case 1:
            if (port->prl_msg_rx)
            {
                protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                                 sizeof(port->policy_pd_data));
                port->prl_msg_rx = false;
                if (port->policy_pd_header.sop.NumDataObjects == 0)
                {
                    if (port->policy_pd_header.sop.MessageType == CMTAccept)
                    {
                        port->policy_is_dfp = !port->policy_is_dfp;
                        policy_set_dfp(port, port->policy_is_dfp);
                        policy_set_port_roles(port);
                        policy_set_state(port, PE_SNK_Ready, 0);
#if CONFIG_ENABLED(USB4)
                        /* start enter_usb timer for both DFP and UFP */
                        timer_start(&port->timers[ENTER_USB_TIMER], ktEnterUSB);
                        port->enter_usb_timeout = false;
                        if (!port->policy_is_dfp)
                        {
                        }
            			else if (!port->is_vconn_source && !(port->disc_id_sent_sopx & 1U << SOP_SOP1 == 1U << SOP_SOP1))
            			{
            				port->msgtx.booltxmsg.vconnswap = true;
            			}
#endif
                    }
                    else if (port->policy_pd_header.sop.MessageType == CMTReject
                             || port->policy_pd_header.sop.MessageType == CMTWait
                             || port->policy_pd_header.sop.MessageType == CMTNotSupported)
                    {
                        policy_set_state(port, PE_SNK_Ready, 0);
                    }
                    else
                    {
                        policy_set_state(port, PE_SNK_Send_Soft_Reset, 0);
                    }
                }
                else
                {
                    policy_set_state(port, PE_SNK_Send_Soft_Reset, 0);
                }
            }
            else if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                policy_set_state(port, PE_SNK_Ready, 0);
            }
            break;
        default:
            policy_set_state(port, PE_ErrorRecovery, 0);
            break;
    }
}

static void policy_state_sink_evaluate_drswap(struct port *port)
{
    int status = 0;
#if MODAL_OPERATION_SUPPORTED
    if (port->mode_entered)
    {
        policy_send_sink_hardreset(port);
        return;
    }
#endif
    if (port->prl_msg_rx_sop != SOP_SOP0
        || (port->policy_is_dfp && !port->vif->DR_Swap_To_UFP_Supported)
        || (!port->policy_is_dfp && !port->vif->DR_Swap_To_DFP_Supported))
    {
        policy_send_notsupported(port);
    }
    else
    {
        status =
            policy_send(port, CMTAccept, 0, 0, SOP_SOP0, false, PE_SNK_Ready, 0, PE_SNK_Ready, 0);
        if (status == STAT_SUCCESS)
        {
            port->policy_is_dfp = !port->policy_is_dfp;
            policy_set_dfp(port, port->policy_is_dfp);
#if CONFIG_ENABLED(USB4)
            /* start enter_usb timer for both DFP and UFP */
            timer_start(&port->timers[ENTER_USB_TIMER], ktEnterUSB);
            port->enter_usb_timeout = false;
			if (!port->policy_is_dfp)
			{
			}
			else if (!port->is_vconn_source && !(port->disc_id_sent_sopx & 1U << SOP_SOP1 == 1U << SOP_SOP1))
			{
				port->msgtx.booltxmsg.vconnswap = true;
			}
#endif
        }

    }
}

static void policy_state_sink_send_vconnswap(struct port *port)
{
    int status = 0;
    switch (port->policy_substate)
    {
        case 0:
            status = policy_send(port, CMTVCONN_Swap, 0, 0, SOP_SOP0, false, PE_VCS_Send_Swap, 1,
                                 PE_SNK_Ready, 0);
            if (status == STAT_SUCCESS)
            {
                timer_start(&port->timers[POLICY_TIMER], ktSenderResponse);
                port->req_vconn_swap_to_on_as_sink = false;

            }
            if (status != STAT_BUSY)
            {
            	port->msgtx.booltxmsg.vconnswap = false;
            }
            break;
        case 1:
            if (port->prl_msg_rx)
            {
                protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                                 sizeof(port->policy_pd_data));
                port->prl_msg_rx = false;
                if (port->policy_pd_header.sop.NumDataObjects == 0)
                {
                    if (port->policy_pd_header.sop.MessageType == CMTAccept)
                    {
                        policy_set_state(port, PE_VCS_Send_Swap, 2);
                        timer_disable(&port->timers[POLICY_TIMER]);
                    }
                    else if (port->policy_pd_header.sop.MessageType == CMTReject
                             || port->policy_pd_header.sop.MessageType == CMTWait)
                    {
                        policy_set_state(port, PE_SNK_Ready, 0);
                    }
                    else if (port->policy_pd_header.sop.MessageType == CMTNotSupported)
                    {
                        /* If needed, can force becoming the VConn Source */
                        policy_set_vconn(port, true);
                        policy_set_state(port, PE_SNK_Ready, 0);
                    }
                    else
                    {
                        policy_set_state(port, PE_SNK_Send_Soft_Reset, 0);
                    }
                }
                else
                {
                    /* Received data message */
                    policy_set_state(port, PE_SNK_Send_Soft_Reset, 0);
                }
            }
            else if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                policy_set_state(port, PE_SNK_Ready, 0);
            }
            break;
        case 2:
            if (port->is_vconn_source)
            {
                timer_start(&port->timers[POLICY_TIMER], ktVCONNSourceOn);
                policy_set_state(port, PE_VCS_Send_Swap, 3);
            }
            else
            {
                policy_set_vconn(port, true);
                policy_set_state(port, PE_VCS_Send_Swap, 4);
            }
            break;
        case 3:
            if (port->prl_msg_rx)
            {
                protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                                 sizeof(port->policy_pd_data));
                port->prl_msg_rx = false;
                if (port->policy_pd_header.sop.NumDataObjects == 0)
                {
                    switch (port->policy_pd_header.sop.MessageType)
                    {
                        case CMTPS_RDY:
                            /* Disable VCONN source */
                            policy_set_vconn(port, false);
                            policy_set_state(port, PE_SNK_Ready, 0);
                            break;
                        default:
                            /* Ignore all other commands received */
                            break;
                    }
                }
            }
            else if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                policy_send_sink_hardreset(port);
            }
            break;
        case 4:
            policy_send(port, CMTPS_RDY, 0, 0, SOP_SOP0, false, PE_SNK_Ready, 0, PE_SNK_Ready, 0);
            break;
        default:
            policy_set_state(port, PE_ErrorRecovery, 0);
            break;
    }
}
#if (CONFIG_ENABLED(SNK))
static void policy_state_sink_evaluate_vconnswap(struct port *port)
{
#if CONFIG_ENABLED(EPR)
    enum policy_state_t next_state =
        (port->policy_state_prev == PE_SNK_Epr_Mode_Entry_Wait_For_Response) ?
            PE_SNK_Epr_Mode_Entry_Wait_For_Response :
            PE_SNK_Ready;
#else
enum policy_state_t next_state = PE_SNK_Ready;
#endif
    uint32_t status = 0;
    switch (port->policy_substate)
    {
        case 0:
            if (port->prl_msg_rx_sop != SOP_SOP0
                || (!port->is_vconn_source && !port->vif->VCONN_Swap_To_On_Supported))
            {
                policy_send_notsupported(port);
            }
            else if (port->is_vconn_source && !port->vif->VCONN_Swap_To_Off_Supported)
            {
                /* If VCONN_Swap_To_Off_Supported is false, send Reject instead of Not Supported */
                policy_send(port, CMTReject, 0, 0, SOP_SOP0, false, next_state, 0, next_state, 0);
            }
            else
            {
                status = policy_send(port, CMTAccept, 0, 0, SOP_SOP0, false, PE_VCS_Evaluate_Swap,
                                     1, next_state, 0);
                if (status == STAT_SUCCESS)
                {
                    if (port->is_vconn_source)
                    {
                        timer_start(&port->timers[POLICY_TIMER], ktVCONNSourceOn);
                        port->discv_id_counter = 0;
                    }
                    else
                    {
                        policy_set_vconn(port, true);
                        timer_start(&port->timers[POLICY_TIMER], tVbusTransTime);
                        policy_set_state(port, PE_VCS_Evaluate_Swap, 2);
                    }
                }
            }
            break;
        case 1:
            if (port->prl_msg_rx)
            {
                protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                                 sizeof(port->policy_pd_data));
                port->prl_msg_rx = false;
                if (port->policy_pd_header.sop.NumDataObjects == 0)
                {
                    switch (port->policy_pd_header.sop.MessageType)
                    {
                        case CMTPS_RDY:
                            /* Disable VConn source */
                            policy_set_vconn(port, false);
                            policy_set_state(port, next_state, 0);
                            break;
                        default:
                            break;
                    }
                }
            }
            else if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                policy_send_sink_hardreset(port);
            }
            break;
        case 2:
            if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                dbg_msg("timer disable", 0);
                timer_disable(&port->timers[POLICY_TIMER]);
                policy_set_state(port, PE_VCS_Evaluate_Swap, 3);
            }
            break;
        case 3:
            policy_send(port, CMTPS_RDY, 0, 0, SOP_SOP0, false, next_state, 0, next_state, 0);
            break;
        default:
            policy_set_state(port, PE_ErrorRecovery, 0);
            break;
    }
}
#if CONFIG_ENABLED(DRP)
static void policy_state_sink_sendprswap(struct port *port)
{
    int status = 0;
    switch (port->policy_substate)
    {
        case 0:
            status = policy_send(port, CMTPR_Swap, 0, 0, SOP_SOP0, false, PE_PRS_SNK_SRC_Send_Swap,
                                 1, PE_SNK_Ready, 0);
            if (status == STAT_SUCCESS)
            {
                timer_start(&port->timers[POLICY_TIMER], ktSenderResponse);
                port->req_pr_swap_as_snk = false;
            }
            break;
        case 1:
            if (port->prl_msg_rx)
            {
                protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                                 sizeof(port->policy_pd_data));
                port->prl_msg_rx = false;
                if (port->policy_pd_header.sop.NumDataObjects == 0)
                {
                    if (port->policy_pd_header.sop.MessageType == CMTAccept)
                    {
                        port->is_pr_swap          = true;
                        port->policy_has_contract = false;
                        dev_tcpd_port_vbus_snk(port, VBUS_OFF, VBUS_OFF, 0);
                        timer_start(&port->timers[POLICY_TIMER], ktPSSourceOff);
                        policy_set_state(port, PE_PRS_SNK_SRC_Send_Swap, 2);
                    }
                    else if (port->policy_pd_header.sop.MessageType == CMTReject
                             || port->policy_pd_header.sop.MessageType == CMTWait
                             || port->policy_pd_header.sop.MessageType == CMTNotSupported)
                    {
                        policy_set_state(port, PE_SNK_Ready, 0);
                        port->is_pr_swap = false;
                    }
                    else
                    {
                        policy_set_state(port, PE_SNK_Send_Soft_Reset, 0);
                    }
                }
            }
            else if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                policy_set_state(port, PE_SNK_Ready, 0);
                port->is_pr_swap = false;
            }
            break;
        case 2:
            /* Wait for a PS_RDY message to be received to indicate that the */
            /* original source is no longer supplying VBUS */
            if (port->prl_msg_rx)
            {
                protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                                 sizeof(port->policy_pd_data));
                port->prl_msg_rx = false;
                if (port->policy_pd_header.sop.NumDataObjects == 0)
                {
                    switch (port->policy_pd_header.sop.MessageType)
                    {
                        case CMTPS_RDY:
                            port->policy_is_source = true;
#if (CONFIG_ENABLED(EPR))
                            port->epr_mode = false;
#endif
                            typec_role_swap_to_source(port);
                            policy_set_port_roles(port);
                            timer_disable(&port->timers[POLICY_TIMER]);
                            policy_set_state(port, PE_PRS_SNK_SRC_Send_Swap, 3);
                            fusbdev_tcpd_set_vbus_alarm(port->tcpd_device,
                                                        VBUS_HIGH_THRESHOLD(VBUS_5V),
                                                        VBUS_LOW_THRESHOLD(VBUS_5V));
                            break;
                        default:
                            break;
                    }
                }
            }
            else if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                port->is_pr_swap = false;
                /* Note: Compliance testing seems to require BOTH HR and ER here. */
                policy_send_sink_hardreset(port);
                policy_set_state(port, PE_ErrorRecovery, 0);
            }
            break;
        case 3:
            if (port_vbus_vsafe5v(port))
            {
                status = policy_send(port, CMTPS_RDY, 0, 0, SOP_SOP0, false, PE_SRC_Startup, 0,
                                     PE_ErrorRecovery, 0);
                if (port->is_vconn_source)
                {
                    port->vdm_check_cbl = true;
                }
                if (status == STAT_SUCCESS)
                {
                    timer_start(&port->timers[SWAP_SOURCE_TIMER], ktSwapSourceStart);
                }
            }
            else
            {
                fusbdev_tcpd_set_vbus_alarm(port->tcpd_device, VBUS_HIGH_THRESHOLD(VBUS_5V),
                                            VBUS_LOW_THRESHOLD(VBUS_5V));
            }
            break;
        default:
            policy_set_state(port, PE_ErrorRecovery, 0);
            break;
    }
}
#endif
static void policy_state_sink_evaluate_prswap(struct port *port)
{
#if CONFIG_ENABLED(DRP)
    int status = 0;
    switch (port->policy_substate)
    {
        case 0:
            if (port->prl_msg_rx_sop != SOP_SOP0 || port->vif->Accepts_PR_Swap_As_Snk == false)
            {
                policy_send_notsupported(port);
            }
            else if (port->partner_caps.FPDOSupply.SupplyType == pdoTypeFixed
                     && port->caps_received[0].FPDOSupply.DualRolePower == false)
            {
                status = policy_send(port, CMTReject, 0, 0, port->prl_msg_rx_sop, false,
                                     PE_SNK_Ready, 0, PE_SNK_Ready, 0);
            }
            else
            {
                status = policy_send(port, CMTAccept, 0, 0, port->prl_msg_rx_sop, false,
                                     PE_PRS_SNK_SRC_Evaluate_Swap, 1, PE_SNK_Ready, 0);
                if (status == STAT_SUCCESS)
                {
                    port->is_pr_swap          = true;
                    port->policy_has_contract = false;
                    pps_enable_monitor(port, false);
                    port->source_is_apdo = false;
                    dev_tcpd_port_vbus_snk(port, VBUS_OFF, VBUS_OFF, 0);
                    timer_start(&port->timers[POLICY_TIMER], ktPSSourceOff);
                }
            }
            break;
        case 1:
            if (port->prl_msg_rx)
            {
                protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                                 sizeof(port->policy_pd_data));
                port->prl_msg_rx = false;
                if (port->policy_pd_header.sop.NumDataObjects == 0)
                {
                    switch (port->policy_pd_header.sop.MessageType)
                    {
                        case CMTPS_RDY:
                            timer_disable(&port->timers[POLICY_TIMER]);
                            /* Enable PD to GoodCRC as source */
                            policy_set_pd_source(port, true);
                            port->policy_is_source = true;
#if (CONFIG_ENABLED(EPR))
                            port->epr_mode = false;
#endif
                            /* TODO: Investigate further*/
                            fusbdev_tcpd_set_vbus_alarm(port->tcpd_device, 0,
                                                        VBUS_LOW_THRESHOLD(VBUS_5V));
                            typec_role_swap_to_source(port);
                            policy_set_port_roles(port);
                            timer_start(&port->timers[POLICY_TIMER], 0);
                            policy_set_state(port, PE_PRS_SNK_SRC_Evaluate_Swap, 2);
                            break;
                        default:
                            break;
                    }
                }
            }
            else if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                port_set_error_recovery(port->dev);
            }
            break;
        case 2:
            if (port_vbus_vsafe5v(port))
            {
                status = policy_send(port, CMTPS_RDY, 0, 0, port->prl_msg_rx_sop, false,
                                     PE_SRC_Startup, 0, PE_ErrorRecovery, 0);
                if (status == STAT_SUCCESS)
                {
                    port->is_pr_swap = false;
                    timer_start(&port->timers[SWAP_SOURCE_TIMER], ktSwapSourceStart);
                    if (port->is_vconn_source)
                    {
                        port->vdm_check_cbl = true;
                    }
                }
                else if (status == STAT_ERROR)
                {
                    policy_set_state(port, PE_ErrorRecovery, 0);
                }
            }
            else
            {
                fusbdev_tcpd_set_vbus_alarm(port->tcpd_device, 0, VBUS_LOW_THRESHOLD(VBUS_5V));
            }
            break;
        default:
            policy_set_state(port, PE_ErrorRecovery, 0);
            break;
    }
#else
    policy_send_notsupported(port);
#endif
}

#define NUM_SNK_CONTROL_MESSAGES 25
#define NUM_SNK_DATA_MESSAGES 16
#define NUM_SNK_EXTENDED_MESSAGES 17
#define CONTROL_SNK_INDEX 0
#define DATA_SNK_INDEX (CONTROL_SNK_INDEX + NUM_SNK_CONTROL_MESSAGES)
#define EXTENDED_SNK_INDEX (DATA_SNK_INDEX + NUM_SNK_DATA_MESSAGES)
#define NUM_SNK_SUPPORTED_MESSAGES (NUM_SNK_CONTROL_MESSAGES + NUM_SNK_DATA_MESSAGES + NUM_SNK_EXTENDED_MESSAGES)
const uint8_t SINK_MESSAGE_ACTION_ARRAY[NUM_SNK_SUPPORTED_MESSAGES] = {
/* TOTAL 25 Control Messages */
/* NUMBER of NOT_Supported Responses */
/* CMT 0x00 - Reserved */
        PE_SNK_Send_Not_Supported,
/* CMT 0x01 - GoodCRC, */
        PE_SNK_Ready,
/* CMT 0x02 - GoToMin */
        PE_SNK_Transition_Sink,
/* CMT 0x03 - Accept */
        PE_SNK_Send_Not_Supported,
/* CMT 0x04 - Reject */
        PE_SNK_Ready,
/* CMT 0x05 - Ping */
        PE_SNK_Send_Not_Supported,
/* CMT 0x06 - PS_RDY */
        PE_SNK_Send_Not_Supported,
/* CMT 0x07 - Get_Source_Cap - SPECIAL */
#if (CONFIG_ENABLED(DRP))
        PE_DR_SNK_Give_Source_Cap,
#else
        PE_SNK_Send_Not_Supported,
#endif
/* CMT 0x08 - Get_Sink_Cap */
        PE_SNK_Give_Sink_Cap,
/* CMT 0x09 - DR_Swap */
        PE_DRS_UFP_DFP_Evaluate_Swap,
/* CMT 0x0A - PR_Swap */
#if (CONFIG_ENABLED(DRP))
        PE_PRS_SNK_SRC_Evaluate_Swap,
#else
        PE_SNK_Send_Not_Supported,
#endif
/* CMT 0x0B - Vconn_Swap */
        PE_VCS_Evaluate_Swap,
/* CMT 0x0C - Wait */
        PE_SNK_Ready,
/* CMT 0x0D - Soft_Reset */
        PE_SNK_Soft_Reset,
/* CMT 0x0E - Data_Reset */
#if (CONFIG_ENABLED(USB4))
        PE_Evaluate_Data_Reset,
#else
		PE_SNK_Send_Not_Supported,
#endif
/* CMT 0x0F - Data_Reset_Complete */
        PE_SNK_Send_Soft_Reset,
/* CMT 0x10 - Not_Supported */
        PE_SNK_Ready,
/* CMT 0x11 - Get_Source_Cap_Extended */
#if (CONFIG_ENABLED(DRP))
        PE_DR_SNK_Give_Source_Cap_Ext,
#else
        PE_SNK_Send_Not_Supported,
#endif
/* CMT 0x12 - Get_Status */
        PE_Give_Status,
/* CMT 0x13 - FR_Swap */
#if (CONFIG_ENABLED(DRP))
        PE_SNK_Send_Not_Supported,
#else
        PE_SNK_Send_Not_Supported,
#endif
/* CMT 0x14 - Get_PPS_Status */
        PE_SNK_Send_Not_Supported,
/* CMT 0x15 - Get_Country_Codes */
        PE_SNK_Send_Not_Supported,
/* CMT 0x16 - Get_Sink_Cap_Extended */
        PE_SNK_Give_Sink_Cap_Ext,
/* CMT 0x17 - Get_Source_Info */
        PE_SNK_Send_Not_Supported,
/* CMT 0x18 - Get_Revision */
        PE_Give_Revision,
/* TOTAL 16 Data Message Types */
/* DMT 0x00 - Reserved */
        PE_SNK_Send_Not_Supported,
/* DMT 0x01 - Source_Capabilies, */
        PE_SNK_Evaluate_Capability,
/* DMT 0x02 - Request - SPECIAL */
        PE_SNK_Send_Not_Supported,
/* DMT 0x03 - BIST - SPECIAL */
        PE_SNK_Ready,
/* DMT 0x04 - Sink_Capabilities - SPECIAL */
        PE_SNK_Ready,
/* DMT 0x05 - Battery_Status */
        PE_SNK_Send_Not_Supported,
/* DMT 0x06 - Alert */
        PE_Alert_Received,
/* DMT 0x07 - Get_Country_Info */
        PE_SNK_Send_Not_Supported,
/* DMT 0x08 - Enter_USB */
#if CONFIG_ENABLED(USB4)
        PE_UEU_Enter_USB_Received,
#else
        PE_SNK_Send_Not_Supported,
#endif
/* Idea: End Data messages here and use if statements for 3 special messages */
/* DMT 0x09 - EPR_Request - SPECIAL */
        PE_SNK_Send_Not_Supported,
/* DMT 0x0A - EPR_Mode - SPECIAL */
        PE_SNK_Send_Not_Supported,
/* DMT 0x0B - Source_Info */
        PE_SNK_Send_Not_Supported,
/* DMT 0x0C - Revision */
        PE_SNK_Send_Not_Supported,
/* DMT 0x0D - Reserved */
        PE_SNK_Send_Not_Supported,
/* DMT 0x0E - Reserved */
        PE_SNK_Send_Not_Supported,
/* DMT 0x0F - Vendor_Defined - SPECIAL */
        PE_SNK_Ready,
/* TOTAL 23 Extended MESSAGE types */
/* EMT 0x00 - Reserved */
        PE_SNK_Send_Not_Supported,
/* EMT 0x01 - Source_Capabilities_Extended, */
        PE_SNK_Ready,
/* EMT 0x02 - Status */
        PE_SNK_Send_Not_Supported,
/* EMT 0x03 - Get_Battery_Cap */
        PE_Give_Battery_Cap,
/* EMT 0x04 - Get_Battery_Status */
        PE_Give_Battery_Status,
/* EMT 0x05 - Battery_Capabilities */
        PE_SNK_Send_Not_Supported,
/* EMT 0x06 - Get_Manufacturer_Info */
        PE_Give_Manufacturer_Info,
/* EMT 0x07 - Manufacturer_Info */
        PE_SNK_Send_Not_Supported,
/* EMT 0x08 - Security_Request */
        PE_SNK_Send_Not_Supported,
/* EMT 0x09 - Security_Response */
        PE_SNK_Send_Not_Supported,
/* EMT 0x0A - Firmware_Update_Request */
#if (CONFIG_ENABLED(FW_UPDATE))
        PE_Send_Firmware_Update_Response,
#else
        PE_SNK_Send_Not_Supported,
#endif
/* EMT 0x0B - Firmware_Update_Response */
        PE_SNK_Send_Not_Supported,
/* EMT 0x0C - PPS_Status */
        PE_SNK_Ready,
/* EMT 0x0D - Country_Info */
        PE_SNK_Send_Not_Supported,
/* EMT 0x0E - Country_Code */
        PE_SNK_Send_Not_Supported,
/* EMT 0x0F - Sink_Capabilities_Extended */
        PE_SNK_Ready,
/* EMT 0x10 - Extended_Control - SPECIAL */
        PE_SNK_Send_Not_Supported,
/* EMT 0x11 - EPR_Source_Capabilities - SPECIAL */
        /* PE_SRC_Send_Not_Supported,
/* EMT 0x12 - EPR_Sink_Capabilities - SPECIAL */
        /* PE_SRC_Ready, */
/* EMT 0x13 - Reserved */
        /* PE_SRC_Send_Not_Supported, */
/* EMT 0x14 - Reserved */
        /* PE_SRC_Send_Not_Supported, */
/* EMT 0x15 - Reserved */
        /* PE_SRC_Send_Not_Supported, */
/* EMT 0x16 - Vendor_Defined_Extended */
        /* PE_SRC_Send_Not_Supported, */
};

static void sink_vif_message_requests(struct port * port)
{
	if (port->req_dr_swap_to_dfp_as_sink && !port->policy_is_dfp
			 && port->vif->DR_Swap_To_DFP_Supported)
	{
		port->msgtx.booltxmsg.prswap = true;
	}
#if CONFIG_ENABLED(DRP)
	else if (port->type == TypeC_DRP && port->req_pr_swap_as_snk
			 && port->partner_caps.FPDOSupply.DualRolePower)
	{
		port->msgtx.booltxmsg.drswap = true;
	}
#endif
	else if (port->req_vconn_swap_to_on_as_sink && !port->is_vconn_source
			 && port->vif->VCONN_Swap_To_On_Supported)
	{
		port->msgtx.booltxmsg.vconnswap = true;
	}
	else if (port->renegotiate)
	{
		port->renegotiate = false;
		policy_set_state(port, PE_SNK_Evaluate_Capability, 0);
	}
#if CONFIG_ENABLED(USB4)
	policy_usb4_action_center(port);
#endif
#if MODAL_OPERATION_SUPPORTED
	policy_dp_alt_mode_action_center(port);
#endif
}

static void policy_state_sink_ready(struct port *port)
{
    PD_HEADER_T msg = {0};
    sink_vif_message_requests(port);
#if CONFIG_ENABLED(EPR)
    if (port->epr_mode && timer_expired(&port->timers[EPR_KEEPALIVE_TIMER]))
    {
        EVENT_PD_EPR_KEEP_ALIVE_EXPIRY evt = {0};
        evt.arg.msg.type                   = EPR_KeepAlive;
        event_notify(EVENT_PD_SNK_EPR_KEEP_ALIVE_EXPIRY, port->dev, &evt);
        if (evt.ret.success != EVENT_STATUS_FAILED)
        {
            policy_set_state(port, PE_SNK_EPR_Keep_Alive, 0);
        }
        else
        {
            /* Check if the contract is SPR , if sink request index 0-7 SPR, >7 EPR */
            /* if SPR, just exit */
            /* if EPR, renegotiate the contract and then exit */
            port->negotiate_epr_contract = false;
            port->epr_try_exit           = true;
            if (port->sink_request[0].AVSRDO.ObjectPosition > 7)
            {
                policy_set_state(port, PE_SNK_Get_Source_Cap, 0);
            }
            else if (port->sink_request[0].AVSRDO.ObjectPosition > 0)
            {
                policy_set_state(port, PE_SNK_Send_EPR_Mode_Exit, 0);
            }
        }
    }
    else if (port->prl_msg_rx)
    {
#else
    if (port->prl_msg_rx)
    {
#endif
        protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                         sizeof(port->policy_pd_data));
        port->prl_msg_rx = false;
        msg.word         = port->policy_pd_header.word;
        uint8_t lowermessageIndex = 0;
        uint8_t uppermessageIndex = NUM_SNK_CONTROL_MESSAGES;
        uint16_t currentStateAction = PE_SNK_Send_Not_Supported;
        if (msg.sop.NumDataObjects != 0)
        {
			if (msg.sop.Extended == 0)
			{
				lowermessageIndex = NUM_SNK_CONTROL_MESSAGES;
				uppermessageIndex = NUM_SNK_CONTROL_MESSAGES + NUM_SNK_DATA_MESSAGES;
			}
			else
			{
				lowermessageIndex = NUM_SNK_CONTROL_MESSAGES + NUM_SNK_DATA_MESSAGES;
				uppermessageIndex = NUM_SNK_SUPPORTED_MESSAGES;
			}
        }
        if (lowermessageIndex + msg.sop.MessageType < uppermessageIndex)
        {
            currentStateAction = SINK_MESSAGE_ACTION_ARRAY[lowermessageIndex + msg.sop.MessageType];
        }
        if (!port->vif->Chunking_Implemented_SOP && uppermessageIndex == NUM_SNK_SUPPORTED_MESSAGES)
        {
            currentStateAction = PE_Chunk_NotSupported;
        }
        policy_set_state(port, currentStateAction, 0);
#if CONFIG_ENABLED(EPR)
        if (msg.sop.MessageType == EMTExtendedControl && msg.sop.Extended != 0)
        {
            policy_state_read_extended_control(port);
        }
        else if (msg.sop.MessageType == DMTEPRMode && port_pd_ver(port, SOP_SOP0) == PD_REV3 &&
                msg.sop.Extended == 0 && msg.sop.NumDataObjects !=0)
        {
            policy_state_read_epr_mode(port);
        }
        else if (port->epr_mode)
        {
            /* MT 0x07 - Get_Source_Cap - SPECIAL */
            if (msg.sop.NumDataObjects == 0)
            {
#if CONFIG_ENABLED(DRP)
                if (msg.sop.MessageType == CMTGetSourceCap)
                {
                    policy_set_state(port, PE_DR_SNK_Give_Source_Cap, 0);
                }
#endif
            }
            else if (msg.sop.Extended == 0)
            {
                /* DMT 0x02 - Request - SPECIAL */
                if (msg.sop.MessageType == DMTRequest)
                {
                    policy_state_invalid(port);
                }
                /* DMT 0x04 - Sink_Capabilities - SPECIAL */
                else if (msg.sop.MessageType == DMTSinkCapabilities)
                {
                    policy_state_invalid(port);
                }
            }
            /* DMT 0x0A - EPR_Mode - SPECIAL - handled elsewhere */
            /* EMT 0x10 - Extended_Control - SPECIAL - handled elsewhere */
            /* EMT 0x11 - EPR_Source_Capabilities - SPECIAL */
            /* do nothing for now - default behavior fine */
            /* EMT 0x12 - EPR_Sink_Capabilities - SPECIAL */
            /* do nothing for now - default behavior fine */
        }
        else
        {
            /* CMT 0x07 - Get_Source_Cap - SPECIAL - do nothing */
            /* DMT 0x02 - Request - SPECIAL - do nothing */
            /* DMT 0x09 - EPR_Request - SPECIAL */
            if (msg.sop.MessageType == DMTEPRequest && msg.sop.NumDataObjects != 0 &&
                msg.sop.Extended == 0)
            {
                policy_state_invalid(port);
            }
            /* DMT 0x0A - EPR_Mode - SPECIAL - handled elsewhere */
            /* EMT 0x10 - Extended_Control - SPECIAL - handled elsewhere */
            /* EMT 0x11 - EPR_Source_Capabilities - SPECIAL */
            /* do nothing for now - default behavior fine */
            /* EMT 0x12 - EPR_Sink_Capabilities - SPECIAL */
            /* do nothing for now - default behavior fine */
        }
#else
#endif
        if (msg.sop.Extended && msg.sop.NumDataObjects > 0)
        {
        }
        else if (msg.sop.NumDataObjects > 0)
        {
			/* DMT 0x01 - Source_Caps - SPECIAL */
        	if (msg.sop.MessageType == DMTSourceCapabilities)
        	{
        		policy_set_partner_cap(port, true);
        	}
        	/* DMT 0x03 - BIST - SPECIAL */
			if (msg.sop.MessageType == DMTBIST)
			{
				policy_process_bist_message(port);
			}
			/* DMT 0x0F - Vendor_Defined - SPECIAL */
			else if (msg.sop.MessageType == DMTVendorDefined)
			{
				policy_process_vdmrx_engine(port);
			}

        }
        port->policy_ams_start = false;
        port->policy_ams_active = currentStateAction != PE_SNK_Ready && currentStateAction != PE_SNK_Send_Not_Supported &&
                                  currentStateAction != PE_SNK_Hard_Reset;
    }
    else if ((port_pd_ver(port, SOP_SOP0) == PD_REV2 || dev_tcpd_port_sinktx_state(port) == SinkTxOK) && !port->prl_msg_rx_pending)
    {
        if (port->source_is_apdo && timer_expired(&port->timers[PPS_TIMER]))
        {
            timer_disable(&port->timers[PPS_TIMER]);
            port->sink_request[0].object = port->stored_apdo.object;
            policy_set_state(port, PE_SNK_Select_Capability, 0);
        }
#if (CONFIG_ENABLED(SNK) && CONFIG_ENABLED(EPR))
        else if (port->epr_try_entry == true && port->vif->EPR_Mode_Capable == 1)
        {
            port->epr_try_entry == false;
            policy_set_state(port, PE_SNK_Send_Epr_Mode_Entry, 0);
        }
        else if (port->epr_try_exit == true)
        {
            port->epr_try_exit = false;
            policy_set_state(port, PE_SNK_Send_EPR_Mode_Exit, 0);
        }
#endif
        else if (port->vdm_check_cbl && policy_check_comm_allowed(port, SOP_MASK_SOP1))
        {
            policy_set_state(port, PE_CBL_Query, 0);
            /* Try querying cable only once */
        }
        else if (port->cbl_rst_state > CBL_RST_DISABLED)
        {
            policy_process_cable_reset(port);
        }
        else if (policy_process_tx_req(port))
    	{

        }
        else if (CONFIG_ENABLED(VDM) && port->vif->Attempts_Discov_SOP
            && port->policy_is_dfp && timer_expired(&port->timers[POLICY_TIMER])
            && port->vdm_auto_state)
        {
            timer_disable(&port->timers[POLICY_TIMER]);
            policy_process_vdm_auto_discv(port);
            port->policy_ams_start = false;
        }
    }
    else
    {
        if (timer_expired(&port->timers[POLICY_TIMER]))
        {
            timer_disable(&port->timers[POLICY_TIMER]);
        }
        if (timer_expired(&port->timers[PD_RSP_TIMER]))
        {
            timer_disable(&port->timers[PD_RSP_TIMER]);
        }
        if (timer_expired(&port->timers[DISCOVER_IDENTITY_TIMER]))
        {
            timer_disable(&port->timers[DISCOVER_IDENTITY_TIMER]);
        }
    }
}

#endif
static void policy_state_get_pps_status(struct port *port)
{
    uint32_t status = 0;
    switch (port->policy_substate)
    {
        case 0:
            status = policy_send(port, CMTGetPPSStatus, 0, 0, SOP_SOP0, false,
                                 PE_SNK_Get_PPS_Status, 1, PE_SNK_Ready, 0);
            if (status == STAT_SUCCESS)
            {
                timer_start(&port->timers[POLICY_TIMER], ktSenderResponse);
            }
            if (status == STAT_SUCCESS)
            {
                timer_start(&port->timers[POLICY_TIMER], ktSenderResponse);
                port->msgtx.booltxmsg.getppsstatus = false;
            }
            break;
        case 1:
            if (port->prl_msg_rx)
            {
                protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                                 sizeof(port->policy_pd_data));
                port->prl_msg_rx = false;
                if (port->policy_pd_header.sop.Extended == 1
                    && port->policy_pd_header.sop.MessageType == EMTPPSStatus)
                {
                    policy_set_state(port, PE_SNK_Ready, 0);
#if (CONFIG_ENABLED(EXTENDED_EVENT))
                    event_notify(EVENT_PPS_STATUS_RECIEVED, port->dev, port->policy_pd_data);
#endif
                }
                else if ((port->policy_pd_header.sop.Extended == 0)
                         && (port->policy_pd_header.sop.NumDataObjects == 0)
                         && (port->policy_pd_header.sop.MessageType == CMTNotSupported))
                {
                    policy_set_state(port, PE_SNK_Ready, 0);
                }
                else
                {
                    policy_set_state(port, PE_SNK_Send_Soft_Reset, 0);
                }
            }
            else if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                policy_set_state(port, PE_SNK_Ready, 0);
            }
            break;
    }
}
#endif
static void policy_state_get_status(struct port *port)
{
    uint32_t            status      = 0;
    enum policy_state_t state_ready = policy_state_ready(port);
    enum policy_state_t state_soft_reset = policy_state_send_soft_reset(port);

    switch (port->policy_substate)
    {
        case 0:
            status = policy_send(port, CMTGetStatus, 0, 0, port->prl_msg_rx_sop, false,
                                 PE_Get_Status, 1, state_ready, 0);
            if (status == STAT_SUCCESS)
            {
                timer_start(&port->timers[POLICY_TIMER], ktSenderResponse);
            }
            if (status != STAT_BUSY)
            {
            	port->msgtx.booltxmsg.getstatus = false;
            }
            break;
        default:
            if (port->prl_msg_rx)
            {
                protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                                 sizeof(port->policy_pd_data));
                timer_disable(&port->timers[POLICY_TIMER]);
                port->prl_msg_rx = false;
                if ((port->policy_pd_header.sop.Extended == 1)
                    && (port->policy_pd_header.sop.MessageType == EMTStatus))
                {
                    policy_set_state(port, state_ready, 0);
                    EVENT_PD_STATUS_T evt = {0};
                    evt.arg.stat          = (EXT_STATUS_T *)port->policy_pd_data;
                    evt.arg.sop           = port->prl_msg_rx_sop;
#if (CONFIG_ENABLED(EXTENDED_EVENT))
                    event_notify(EVENT_PD_STATUS, port->dev, &evt);
#endif
                }
                else if ((port->policy_pd_header.sop.Extended == 0)
                         && (port->policy_pd_header.sop.NumDataObjects == 0)
                         && (port->policy_pd_header.sop.MessageType == CMTNotSupported))
                {
                    policy_set_state(port, state_ready, 0);
                }
                else
                {
                    policy_set_state(port, state_soft_reset, 0);
                }
            }
            else if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                policy_set_state(port, state_ready, 0);
            }
            break;
    }
}
static void policy_state_get_battery_cap(struct port *port)
{
    enum policy_state_t state_ready = policy_state_ready(port);
    enum policy_state_t state_soft_reset = policy_state_send_soft_reset(port);
    uint8_t                status  = 0;
    EVENT_PD_GET_BAT_CAP_T evt     = {0};
    EXT_GET_BAT_CAP_T      bat_ref = {0};
    EXT_BAT_CAP_T         *message;
    /* TODO: Implement battery support beyond battery 0 */
    bat_ref.BatteryCapRef = 0;
    evt.arg.success       = false;
    evt.arg.ref           = bat_ref.BatteryCapRef;

    switch (port->policy_substate)
    {
        case 0:
            status = policy_send(port, EMTGetBatteryCap, bat_ref.byte, EXT_GET_BAT_CAP_LEN,
                                 SOP_SOP0, true, PE_Get_Battery_Cap, 1, state_ready, 0);
            if (status == STAT_SUCCESS)
            {
                timer_start(&port->timers[POLICY_TIMER], ktSenderResponse);

            }
            if (status != STAT_BUSY)
            {
                port->msgtx.booltxmsg.getbatterycap = false;
            }
            break;
        case 1:
            if (port->prl_msg_rx == true)
            {
                protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                                 sizeof(port->policy_pd_data));
                timer_disable(&port->timers[POLICY_TIMER]);
                port->prl_msg_rx = false;
                if (port->policy_pd_header.sop.Extended == 1
                    && port->policy_pd_header.sop.NumDataObjects > 0
                    && port->policy_pd_header.sop.MessageType == EMTBatteryCapabilities)
                {
                    message         = (EXT_BAT_CAP_T *)port->policy_pd_data;
                    evt.arg.success = true;
                    evt.arg.cap     = message;

                    policy_set_state(port, state_ready, 0);
                }
                else if (port->policy_pd_header.sop.Extended == 0
                         && port->policy_pd_header.sop.NumDataObjects == 0
                         && port->policy_pd_header.sop.MessageType == CMTNotSupported)
                {
                    policy_set_state(port, state_ready, 0);
                }
                else
                {
                    policy_set_state(port, state_soft_reset, 0);
                }
#if CONFIG_ENABLED(BATTERY_EVENTS)
                event_notify(EVENT_PD_BAT_CAP_RECEIVED, port->dev, &evt);
#endif
            }
            else if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
#if CONFIG_ENABLED(BATTERY_EVENTS)
                event_notify(EVENT_PD_BAT_CAP_RECEIVED, port->dev, &evt);
#endif
                policy_set_state(port, state_ready, 0);
            }
            break;
        default:
            policy_set_state(port, PE_ErrorRecovery, 0);
    }
}
#if (CONFIG_ENABLED(SNK) || CONFIG_ENABLED(DRP))
static void policy_state_get_sourcecap_extended(struct port *port)
{
    uint8_t             status      = STAT_BUSY;
    enum policy_state_t state_ready = policy_state_ready(port);
    enum policy_state_t state_soft_reset = policy_state_send_soft_reset(port);
    switch (port->policy_substate)
    {
        case 0:
            status = policy_send(port, CMTGetSourceCapExt, 0, 0, SOP_SOP0, false,
                                 port->policy_state, 1, state_ready, 0);
            if (status == STAT_SUCCESS)
            {
                timer_start(&port->timers[POLICY_TIMER], ktSenderResponse);
            }
            break;
        case 1:
            if (port->prl_msg_rx == true)
            {
                protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                                 sizeof(port->policy_pd_data));
                timer_disable(&port->timers[POLICY_TIMER]);
                port->prl_msg_rx = false;

                if (port->policy_pd_header.sop.Extended == 1
                    && port->policy_pd_header.sop.NumDataObjects > 0
                    && port->policy_pd_header.sop.MessageType == EMTSourceCapsExtended)
                {
                    /* Unchunked Extended message Source Cap Extended received.
                 *  Process it or inform DPM here. */
                    policy_set_state(port, state_ready, 0);
                }
                else if ((port->policy_pd_header.sop.Extended == 0)
                         && (port->policy_pd_header.sop.NumDataObjects == 0)
                         && (port->policy_pd_header.sop.MessageType == CMTNotSupported))
                {
                    policy_set_state(port, state_ready, 0);
                }
                else
                {
                    policy_set_state(port, state_soft_reset, 0);
                }
            }
            else if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                policy_set_state(port, state_ready, 0);
            }
            break;
        default:
            policy_set_state(port, PE_ErrorRecovery, 0);
    }
}
#endif
static void policy_state_get_sinkcap_extended(struct port *port)
{
    EVENT_PD_EXT_SNK_CAP_T evt   = {0};
    enum policy_state_t    state = policy_state_ready(port);
    enum policy_state_t    state_soft_reset = policy_state_send_soft_reset(port);
    evt.arg.success = false;

    uint8_t status = STAT_BUSY;
    switch (port->policy_substate)
    {
        case 0:
            status = policy_send(port, CMTGetSinkCapExtended, 0, 0, SOP_SOP0, false,
                                 port->policy_state, 1, state, 0);
            if (status == STAT_SUCCESS)
            {
                timer_start(&port->timers[POLICY_TIMER], ktSenderResponse);
            }
            if (status != STAT_BUSY)
            {
                port->msgtx.booltxmsg.getsinkcapext = false;
            }
            break;
        case 1:
            if (port->prl_msg_rx == true)
            {
                protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                                 sizeof(port->policy_pd_data));
                timer_disable(&port->timers[POLICY_TIMER]);
                port->prl_msg_rx = false;

                if ((port->policy_pd_header.sop.Extended == 1)
                    && (port->policy_pd_header.sop.NumDataObjects > 0)
                    && (port->policy_pd_header.sop.MessageType == EMTSinkCapExtended))
                {
                    /* Sink Cap Extended received */

                    evt.arg.buf     = (EXT_SINK_CAP_T *)port->policy_pd_data;
                    evt.arg.success = true;
                    policy_set_state(port, state, 0);
                }
                else if ((port->policy_pd_header.sop.Extended == 0)
                         && (port->policy_pd_header.sop.NumDataObjects == 0)
                         && (port->policy_pd_header.sop.MessageType == CMTNotSupported))
                {
                    /* Not Supported Received*/
                    policy_set_state(port, state, 0);
                }
                else
                {
                    /* Any other unexpected message */
                    policy_set_state(port, state_soft_reset, 0);
                }

                event_notify(EVENT_EXT_SNK_CAP_RECEIVED, port->dev, &evt);
            }
            else if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                event_notify(EVENT_EXT_SNK_CAP_RECEIVED, port->dev, &evt);
                policy_set_state(port, state, 0);
            }
            break;
        default:
            policy_set_state(port, PE_ErrorRecovery, 0);
    }
}

static void policy_state_give_extended_cap(struct port *port)
{
    unsigned            i     = 0;
    enum policy_state_t state = policy_state_ready(port);
    enum policy_state_t state_soft_reset = policy_state_send_soft_reset(port);

    if (port->policy_substate == 0)
    {
        struct event_pd_get_ext_src_cap_t evt = {0};
        evt.ret.success                       = EVENT_STATUS_DEFAULT;
        evt.arg.buf                           = port->policy_pd_data[0].byte;
        for (i = 0; i < EXT_SRC_CAP_LEN; i++)
        {
            evt.arg.buf[i] = 0;
        }
#if (CONFIG_ENABLED(EXTENDED_EVENT))
        event_notify(EVENT_PD_GET_EXT_SRC_CAP, port->dev, &evt);
#endif
        policy_set_state(port, port->policy_state, 1);
        if (evt.ret.success == EVENT_STATUS_DEFAULT)
        {
            struct ext_src_cap_t *cap = (struct ext_src_cap_t *)evt.arg.buf;
            /* Some default values from VIF */
            cap->VID       = port->vif->Manufacturer_Info_VID_Port;
            cap->PID       = port->vif->Manufacturer_Info_PID_Port;
            cap->XID       = port->vif->XID_SOP;
            cap->FWVersion = FWVER_TYPEC(FWVER_PHASE, FWVER_BUILD);
            cap->HWVersion = HWVER;
            cap->PDP       = port->vif->PD_Power_as_Source / 1000;
#if CONFIG_ENABLED(EPR)
            cap->EPRPDP = 140; /* Not supported yet */
#endif
            cap->Battery = (port->vif->Num_Fixed_Batteries & 0xf)
                           | ((port->vif->Num_Swappable_Battery_Slots << 4) & 0xf0);
        }
    }

    policy_send(port, EMTSourceCapsExtended, port->policy_pd_data[0].byte, EXT_SRC_CAP_LEN,
                port->prl_msg_rx_sop, true, state, 0, state_soft_reset, 0);
}

#if CONFIG_ENABLED(USB4)
static void policy_state_dfp_execute_data_reset(struct port *port)
{
    enum policy_state_t state      = policy_state_ready(port);
    enum policy_state_t soft_reset = port->policy_is_source ? PE_SRC_Send_Soft_Reset : PE_SNK_Send_Soft_Reset;
    int status = 0;
    switch (port->policy_substate)
    {
        case 0:
            if (port->is_vconn_source)
            {
                /* turn off vconn, start vconn zero timer */
                policy_set_vconn(port, false);
                timer_start(&port->timers[VCONN_POWER_TIMER], ktVCONNZero);
                /* go to substate 1, no PS_RDY */
                policy_set_state(port, PE_Execute_Data_Reset_DFP, 1);
            }
            else
            {
                /* start discharge timer while waiting for PS_RDY */
                timer_start(&port->timers[VCONN_DISCHARGE_TIMER], ktVCONNSourceDischarge);
                /* go to substate 2, wait for PS_RDY */
                policy_set_state(port, PE_Execute_Data_Reset_DFP, 2);
            }
            break;
        case 1:
            /* after ktVCONNZero expires, start ktVCONNReapplied */
            if (timer_expired(&port->timers[VCONN_POWER_TIMER])
                || (port->vc_term_raw == CC_STAT_SNKOPEN) || (port->vc_term_raw == CC_STAT_UNDEFINED))
            {
                timer_disable(&port->timers[VCONN_POWER_TIMER]);
                timer_start(&port->timers[VCONN_POWER_TIMER], ktVCONNReapplied);
                policy_set_state(port, PE_Execute_Data_Reset_DFP, 3);
            }
            break;
        case 2:
            /* check for PS_RDY message from UFP */
            if (port->prl_msg_rx)
            {
                protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                                 sizeof(port->policy_pd_data));
                port->prl_msg_rx = false;
                if ((port->policy_pd_header.sop.NumDataObjects == 0)
                    && (port->policy_pd_header.sop.MessageType == CMTPS_RDY))
                {
                    timer_disable(&port->timers[VCONN_DISCHARGE_TIMER]);
                    /* start ktVCONNReapplied */
                    timer_start(&port->timers[VCONN_POWER_TIMER], ktVCONNReapplied);
                    policy_set_state(port, PE_Execute_Data_Reset_DFP, 3);
                }
                else
                {
                    policy_set_state(port, PE_ErrorRecovery, 0);
                }
                /* if discharge timer expires, go to error recovery */
            }
            else if (timer_expired(&port->timers[VCONN_DISCHARGE_TIMER]))
            {
                timer_disable(&port->timers[VCONN_DISCHARGE_TIMER]);
                policy_set_state(port, PE_ErrorRecovery, 0);
            }
            break;
        case 3:
            /* after ktVCONNReapplied turn on VCONN */
            if (timer_expired(&port->timers[VCONN_POWER_TIMER]))
            {
            	protocol_reset_sop(port, SOP_MASK_SOP1 | SOP_MASK_SOP2);
                timer_disable(&port->timers[VCONN_POWER_TIMER]);
                policy_set_vconn(port, true);
                timer_start(&port->timers[VCONN_POWER_TIMER], ktVCONNValid);
                policy_set_state(port, PE_Execute_Data_Reset_DFP, 4);
            }
            break;
        case 4:
        	if (port->prl_msg_rx)
        	{
        		policy_set_state(port, PE_ErrorRecovery, 0);
        	}
            /* If ktDataResetFail timer expires, enter error recovery */
        	else if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                policy_set_state(port, PE_ErrorRecovery, 0);
                /* check for VCONN valid timer and Data Reset timer expired to finish data reset */
            }
            else if (timer_expired(&port->timers[VCONN_POWER_TIMER]))
            {
                timer_disable(&port->timers[VCONN_POWER_TIMER]);
                timer_disable(&port->timers[POLICY_TIMER]);
                timer_disable(&port->timers[DATA_RESET_TIMER]);
                /* inform dpm to reconnect usb signals*/
                EVENT_PD_DATA_RESET_T evt = {0};
                evt.ret.success           = EVENT_STATUS_DEFAULT;
                evt.arg.is_dfp            = port->policy_is_dfp;
                event_notify(EVENT_DATA_RESET_EXIT, port->dev, &evt);
                if (evt.ret.success != EVENT_STATUS_FAILED)
                {
                    policy_set_state(port, PE_Execute_Data_Reset_DFP, 5);
                    port->usb_mode = false;
                }
            }
            break;
        case 5:
            /* Send Data Reset Complete message */
            status = policy_send(port, CMTDataResetComplete, 0, 0, SOP_SOP0, false, state, 0, PE_ErrorRecovery, 0);
            if (status == STAT_ABORT || status == STAT_ERROR)
            {
            	policy_set_state(port, PE_ErrorRecovery, 0);
            }
            break;
        default:
            policy_set_state(port, PE_ErrorRecovery, 0);
            break;
    }
}
#endif

#if CONFIG_ENABLED(USB4)

static void policy_state_ufp_execute_data_reset(struct port *port)
{
    int                 status     = 0;
    enum policy_state_t state      = policy_state_ready(port);
    enum policy_state_t soft_reset = port->policy_is_source ? PE_SRC_Send_Soft_Reset : PE_SNK_Send_Soft_Reset;

    switch (port->policy_substate)
    {
        case 0:
        	if (port->is_vconn_source)
            {
                policy_set_vconn(port, false);
                policy_set_state(port, PE_Execute_Data_Reset_UFP, 1);
            }
            else
			{
            	protocol_reset_sop(port, SOP_MASK_SOP1 | SOP_MASK_SOP2);
            	policy_set_state(port, PE_Execute_Data_Reset_UFP, 3);
			}
        	/* Use half the timer for turning off VCONN */
            timer_start(&port->timers[VCONN_POWER_TIMER], ktVCONNZero/2);
            break;
        case 1:
            /* once ktVCONNZero send a PS_RDY */
        	if (port->prl_msg_rx)
			{
				policy_set_state(port, PE_ErrorRecovery, 0);
			}
        	else if (timer_expired(&port->timers[VCONN_POWER_TIMER])
                || (port->vc_term_raw == CC_STAT_SNKOPEN) || (port->vc_term_raw == CC_STAT_UNDEFINED))
            {
        		/* Wait for VCONN to stabilize ktVconnReapplied */
                timer_start(&port->timers[VCONN_POWER_TIMER], ktVCONNReapplied);
                policy_set_state(port, PE_Execute_Data_Reset_UFP, 2);
            }
            break;
        case 2:
        	if (timer_expired(&port->timers[VCONN_POWER_TIMER]))
        	{
        		status = policy_send(port, CMTPS_RDY, 0, 0, SOP_SOP0, false, PE_Execute_Data_Reset_UFP, 3,
        				PE_ErrorRecovery, 0);
				if (status == STAT_ABORT || status == STAT_ERROR)
				{
					policy_set_state(port, PE_ErrorRecovery, 0);
				}
				else if (status == STAT_SUCCESS)
				{
					protocol_reset_sop(port, SOP_MASK_SOP1 | SOP_MASK_SOP2);
				}
        	}
        	else if (port->prl_msg_rx)
			{
				policy_set_state(port, PE_ErrorRecovery, 0);
			}
            break;
        case 3:
            /* wait for Data Reset Complete message */
            if (port->prl_msg_rx)
            {
                protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                                 sizeof(port->policy_pd_data));
                port->prl_msg_rx = false;
                if ((port->policy_pd_header.sop.NumDataObjects == 0)
                    && (port->policy_pd_header.sop.MessageType == CMTDataResetComplete))
                {
                    /* go back to src/snk ready */
                    timer_disable(&port->timers[DATA_RESET_TIMER]);
					timer_start(&port->timers[ENTER_USB_TIMER], ktEnterUSB);
					port->enter_usb_timeout = false;
                    policy_set_state(port, state, 0);
                }
                else
                {
                    policy_set_state(port, PE_ErrorRecovery, 0);
                }
            }
            else if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                policy_set_state(port, PE_ErrorRecovery, 0);
            }
            break;
        default:
            policy_set_state(port, PE_ErrorRecovery, 0);
            break;
    }
}
#endif

#if CONFIG_ENABLED(USB4)
static void policy_state_send_data_reset(struct port *port)
{
    int                 status     = 0;
    enum policy_state_t state      = policy_state_ready(port);
    enum policy_state_t soft_reset = port->policy_is_source ? PE_SRC_Send_Soft_Reset : PE_SNK_Send_Soft_Reset;

    switch (port->policy_substate)
    {
        case 0:
            status = policy_send(port, CMTDataReset, 0, 0, SOP_SOP0, false, PE_Send_Data_Reset, 1,
                                 state, 0);
            if (status == STAT_SUCCESS)
            {
                timer_start(&port->timers[POLICY_TIMER], ktSenderResponse);
            }
            break;
        case 1:
            if (port->prl_msg_rx)
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                                 sizeof(port->policy_pd_data));
                port->prl_msg_rx = false;
                if (port->policy_pd_header.sop.NumDataObjects == 0)
                {
                    if (port->policy_pd_header.sop.MessageType == CMTAccept)
                    {
                        timer_start(&port->timers[DATA_RESET_TIMER], ktDataReset);
                        /* inform dpm */
                        EVENT_PD_DATA_RESET_T evt = {0};
                        evt.ret.success           = EVENT_STATUS_DEFAULT;
                        evt.arg.is_dfp            = port->policy_is_dfp;
#if (CONFIG_ENABLED(EXTENDED_EVENT))
                        event_notify(EVENT_DATA_RESET_ENTER, port->dev, &evt);
#endif
                        if (evt.ret.success != EVENT_STATUS_FAILED)
                        {
                            /* exit all alternate modes */
                            vdm_list_reset_all(port->dev, 0xFF00, ((1U << SOP_SOP0) | (1U << SOP_SOP1) | (1U << SOP_SOP2)), 0);
                            if (port->policy_is_dfp)
                            {
                                policy_set_state(port, PE_Execute_Data_Reset_DFP, 0);
                                timer_start(&port->timers[POLICY_TIMER], ktDataResetFail);
                            }
                            else
                            {
                                policy_set_state(port, PE_Execute_Data_Reset_UFP, 0);
                                timer_start(&port->timers[POLICY_TIMER], ktDataResetFailUFP);
                            }
                        }
                    }
                    else if (port->policy_pd_header.sop.MessageType == CMTNotSupported)
                    {
                        policy_set_state(port, state, 0);
                    }
                    else
                    {
                        /* any other control message */
                        policy_set_state(port, soft_reset, 0);
                    }
                }
                else
                {
                    /* any other message */
                	timer_disable(&port->timers[POLICY_TIMER]);
                    policy_set_state(port, PE_ErrorRecovery, 0);
                }
            }
            else if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                policy_set_state(port, PE_ErrorRecovery, 0);
            }
            break;
        default:
            policy_set_state(port, PE_ErrorRecovery, 0);
            break;
    }
}
#endif

#if CONFIG_ENABLED(USB4)
static void policy_state_evaluate_data_reset(struct port *port)
{
    int                   status = 0;
    EVENT_PD_DATA_RESET_T evt    = {0};
    enum policy_state_t   state  = policy_state_ready(port);

    switch (port->policy_substate)
    {
        case 0:
            if (port_pd_ver(port, SOP_SOP0) != PD_REV3)
            {
                /* If PD2 mode, send Not Supported message */
                policy_send_notsupported(port);
            }
            else
            {
                status = policy_send(port, CMTAccept, 0, 0, SOP_SOP0, false, PE_Evaluate_Data_Reset,
                                     1, state, 0);
                if (status == STAT_SUCCESS)
                {
                    timer_start(&port->timers[DATA_RESET_TIMER], ktDataReset);
                }
            }
            break;
        case 1:
            /* inform dpm */
            evt.ret.success = EVENT_STATUS_DEFAULT;
            evt.arg.is_dfp  = port->policy_is_dfp;
#if (CONFIG_ENABLED(EXTENDED_EVENT))
            event_notify(EVENT_DATA_RESET_ENTER, port->dev, &evt);
#endif
            if (evt.ret.success != EVENT_STATUS_FAILED)
            {
                /* exit all alternate modes */
                vdm_list_reset_all(port->dev, 0xFF00, ((1U << SOP_SOP0) | (1U << SOP_SOP1) | (1U << SOP_SOP2)), 0);
                if (port->policy_is_dfp)
                {
                    policy_set_state(port, PE_Execute_Data_Reset_DFP, 0);
                    timer_start(&port->timers[POLICY_TIMER], ktDataResetFail);
                }
                else
                {
                    policy_set_state(port, PE_Execute_Data_Reset_UFP, 0);
                    timer_start(&port->timers[POLICY_TIMER], ktDataResetFailUFP);
                }
            }
            break;
        default:
            policy_set_state(port, PE_ErrorRecovery, 0);
            break;
    }
}
#endif

static void policy_state_give_manufacture_info(struct port *port)
{
    EVENT_PD_GET_MAN_INFO_T evt         = {0};
    uint8_t                 i           = 0;
    uint8_t                *buf         = port->policy_pd_data[0].byte;
    bool                    invalid_ref = true;
    char                    ns[]        = "Not Supported";
    struct ext_mfi_t       *manf        = (struct ext_mfi_t *)buf;
    enum policy_state_t     state_ready = policy_state_ready(port);
    if (port->policy_substate == 0)
    {
        if (port->vif->Manufacturer_Info_Supported_Port)
        {
            /* copy message from get_manufacture_info */
            struct ext_get_mfi_t manf_ref = {0};
            for (i = 0; i < EXT_GET_MFI_LEN; i++)
            {
                manf_ref.byte[i] = port->policy_pd_data[0].byte[i + 2];
            }
            evt.arg.buf     = buf;
            evt.ret.success = EVENT_STATUS_DEFAULT;
#if (CONFIG_ENABLED(EXTENDED_EVENT))
            event_notify(EVENT_PD_GET_MAN_INFO, port->dev, &evt);
#endif
            policy_set_state(port, PE_Give_Manufacturer_Info, 1);
            if (evt.ret.success == EVENT_STATUS_DEFAULT)
            {
                port->policy_pd_header.word = 0;
                /* MFI target is port*/
                if (!manf_ref.Target)
                {
                    manf->PID                   = port->vif->Manufacturer_Info_PID_Port;
                    manf->VID                   = port->vif->Manufacturer_Info_VID_Port;
                    manf->str[0]                = '\0';
                    port->policy_pd_header.word = (2 * sizeof(uint16_t)) + 1;
                }
                else
                {
                    /*MFI target is battery*/
                    if (manf_ref.Target < 2)
                    {
                        if (manf_ref.Reference < 4
                            && manf_ref.Reference < port->vif->Num_Fixed_Batteries)
                        {
                            /* Fixed battery 0..3 */
                            manf->PID   = port->vif->Manufacturer_Info_PID_Port;
                            manf->VID   = port->vif->Manufacturer_Info_VID_Port;
                            invalid_ref = false;
                        }
                        else if (manf_ref.Reference > 4 && manf_ref.Reference < 8
                                 && (manf_ref.Reference - 4)
                                        < port->vif->Num_Swappable_Battery_Slots)
                        {
                            /* 4..7 Hot Swappable Battery Slots */
                            manf->PID   = port->vif->Manufacturer_Info_PID_Port;
                            manf->VID   = port->vif->Manufacturer_Info_VID_Port;
                            invalid_ref = false;
                        }
                        else
                        {
                            /* invalid reference, send as is if target is port */
                            manf->PID = 0;
                            manf->VID = 0xFFFF;
                        }
                    }
                    else
                    {
                        /* invalid reference */
                        manf->PID = 0;
                        manf->VID = 0xFFFF;
                    }
                    port->policy_pd_header.word = (2 * sizeof(uint16_t));
                    if (invalid_ref == true)
                    { /* send not supported message */
                        for (i = 0; i < sizeof(ns); i++)
                        {
                            manf->str[i] = ns[i];
                        }
                        port->policy_pd_header.word += sizeof(ns);
                    }
                }
            }
            else
            {
                port->policy_pd_header.word = evt.ret.len;
            }
        }
        else
        {
            policy_send_notsupported(port);
        }
    }

    if (port->policy_substate > 0)
    {
        policy_send(port, EMTManufacturerInfo, port->policy_pd_data[0].byte,
                    port->policy_pd_header.word, port->prl_msg_rx_sop, true, state_ready, 0,
                    state_ready, 0);
    }
}

static void policy_state_give_battery_status(struct port *port)
{
    enum policy_state_t state_ready        = policy_state_ready(port);
    struct event_pd_get_bat_stat_t evt     = {0};
    struct pd_msg_t                batStat = {0};
    struct ext_get_bat_stat_t      batRef  = {0};

    if (port->vif->Num_Fixed_Batteries > 0 || port->vif->Num_Swappable_Battery_Slots > 0)
    {
        if (port->policy_substate == 0)
        {
            batStat.object   = 0;
            batRef.byte[0]   = port->policy_pd_data[0].byte[2];
            evt.arg.ref      = batRef.BatteryStatRef;
            evt.ret.bat_stat = &batStat;
            evt.ret.success  = EVENT_STATUS_DEFAULT;
#if CONFIG_ENABLED(BATTERY_EVENTS)
            event_notify(EVENT_PD_GET_BAT_STAT, port->dev, &evt);
#endif
            if (evt.ret.success == EVENT_STATUS_DEFAULT)
            {
                /* Default behavior is to just report charging and unkown */
                if (batRef.BatteryStatRef < 4
                    && batRef.BatteryStatRef < port->vif->Num_Fixed_Batteries)
                {
                    /* Fixed battery 0..3 */
                    batStat.BSDO.BatteryPresent  = 1;
                    batStat.BSDO.BatteryCapacity = 0xFFFF;
                }
                else if (batRef.BatteryStatRef < 8 && batRef.BatteryStatRef > 3
                         && (batRef.BatteryStatRef - 4) < port->vif->Num_Swappable_Battery_Slots)
                {
                    /* 4..7 Hot Swappable Battery Slots. Requires */
                    batStat.BSDO.BatteryPresent  = 0x1;
                    batStat.BSDO.BatteryCapacity = 0xFFFF;
                }
                else
                {
                    /* Invalid battery reference */
                    batStat.BSDO.InvalidReference = 0x1;
                    batStat.BSDO.BatteryCapacity  = 0xFFFF;
                }
            }
            port->policy_pd_data[0].object = batStat.object;
            policy_set_state(port, PE_Give_Battery_Status, 1);
        }
        policy_send(port, DMTBatteryStatus, port->policy_pd_data[0].byte, sizeof(struct pd_msg_t),
                    SOP_SOP0, false, state_ready, 0, state_ready, 0);
    }
    else
    {
        policy_send_notsupported(port);
    }
}

static void policy_state_get_battery_status(struct port *port)
{
    uint8_t                 status = 0;
    EVENT_PD_GET_BAT_STAT_T evt    = {0};
    evt.arg.success                = false;

    enum policy_state_t state      = policy_state_ready(port);
    enum policy_state_t soft_reset = policy_state_send_soft_reset(port);

    struct ext_get_bat_stat_t *bat_ref = (struct ext_get_bat_stat_t *)port->policy_pd_data;

    switch (port->policy_substate)
    {
        case 0:
            bat_ref->BatteryStatRef = 0;
            /* find first bit set */
            for (uint8_t i = 0; i < 8; i++)
            {
                if (port->bat_stat_ref & (1 << i))
                {
                    /* clear bit */
                    port->bat_stat_ref &= ~(1 << i);
                    bat_ref->BatteryStatRef = i;
                    break;
                }
            }
            policy_set_state(port, PE_Get_Battery_Status, 1);
            break;
        case 1:
            status = policy_send(port, EMTGetBatteryStatus, port->policy_pd_data[0].byte,
                                 EXT_GET_BAT_STAT_LEN, SOP_SOP0, true, PE_Get_Battery_Status, 2,
                                 state, 0);
            if (status == STAT_SUCCESS)
            {
                timer_start(&port->timers[POLICY_TIMER], ktSenderResponse);
            }
            if (status != STAT_BUSY)
            {
                port->msgtx.booltxmsg.getbatterystatus = false;
            }
            break;
        case 2:
            if (port->prl_msg_rx)
            {
                protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                                 sizeof(port->policy_pd_data));
                timer_disable(&port->timers[POLICY_TIMER]);
                port->prl_msg_rx = false;
                if ((port->policy_pd_header.sop.NumDataObjects > 0)
                    && (port->policy_pd_header.sop.Extended == 0)
                    && (port->policy_pd_header.sop.MessageType == DMTBatteryStatus))
                {
                    /* copy BSDO object into bat_stat */
                    evt.ret.bat_stat = &port->policy_pd_data[0];
                    evt.arg.success  = true;
                    policy_set_state(port, PE_Get_Battery_Status, 3);
                }
                else if ((port->policy_pd_header.sop.NumDataObjects == 0)
                         && (port->policy_pd_header.sop.MessageType == CMTNotSupported))
                {
                    policy_set_state(port, state, 0);
                }
                else
                {
                    policy_set_state(port, soft_reset, 0);
                }
#if CONFIG_ENABLED(BATTERY_EVENTS)
                event_notify(EVENT_PD_BAT_STAT_RECEIVED, port->dev, &evt);
#endif
            }
            else if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
#if CONFIG_ENABLED(BATTERY_EVENTS)
                event_notify(EVENT_PD_BAT_STAT_RECEIVED, port->dev, &evt);
#endif
                policy_set_state(port, PE_Get_Battery_Status, 3);
            }
            break;
        case 3:
            /* check if there's more batteries */
            if (port->bat_stat_ref != 0)
            {
                port->policy_ams_start  = true;
                port->policy_ams_active = false;
                policy_set_state(port, PE_Get_Battery_Status, 0);
            }
            else
            {
                /* if alert type had status change, get status*/
                if (port->msgtx.booltxmsg.getstatus)
                {
                    port->msgtx.booltxmsg.getstatus        = false;
                    port->policy_ams_start  = true;
                    port->policy_ams_active = false;
                    policy_set_state(port, PE_Get_Status, 0);
                }
                else
                {
                    policy_set_state(port, state, 0);
                }
            }
            break;
        default:
            policy_set_state(port, PE_ErrorRecovery, 0);
    }
}

static void policy_state_give_battery_capabilites(struct port *port)
{
    enum policy_state_t state_ready = policy_state_ready(port);
    enum policy_state_t failed_state = policy_state_send_soft_reset(port);
    struct event_pd_get_bat_cap_t evt    = {0};
    struct ext_get_bat_cap_t      batRef = {0};
    struct ext_bat_cap_t         *batCap = (struct ext_bat_cap_t *)port->policy_pd_data[0].byte;

    if (port->vif->Num_Fixed_Batteries > 0 || port->vif->Num_Swappable_Battery_Slots > 0)
    {
        if (port->policy_substate == 0)
        {
            batRef.byte[0]  = port->policy_pd_data[0].byte[2];
            evt.arg.ref     = batRef.BatteryCapRef;
            evt.arg.cap     = batCap;
            evt.ret.success = EVENT_STATUS_DEFAULT;
#if CONFIG_ENABLED(BATTERY_EVENTS)
            event_notify(EVENT_PD_GET_BAT_CAP, port->dev, &evt);
#endif
            policy_set_state(port, PE_Give_Battery_Cap, 1);
            if (evt.ret.success == EVENT_STATUS_DEFAULT)
            {
                batCap->VID         = port->vif->Manufacturer_Info_VID_Port;
                batCap->PID         = port->vif->Manufacturer_Info_PID_Port;
                batCap->DesignCap   = 0xFFFF;
                batCap->LastFullCap = 0xFFFF;
                if (batRef.BatteryCapRef < 4
                    && batRef.BatteryCapRef < port->vif->Num_Fixed_Batteries)
                {
                    /* 0..3 Fixed batteries */
                    batCap->Type = 0;
                }
                else if (batRef.BatteryCapRef < 8 && batRef.BatteryCapRef > 3
                         && (batRef.BatteryCapRef - 4) < port->vif->Num_Swappable_Battery_Slots)
                {
                    /* 4..7 Hot Swappable Battery Slots. Requires external
                     * mechanisms to detect if present so just default to a valid
                     * reference. */
                    batCap->Type = 0;
                }
                else
                {
                    /* Invalid battery  */
                    batCap->Type = 1;
                }
            }
        }
        policy_send(port, EMTBatteryCapabilities, port->policy_pd_data[0].byte, EXT_BAT_CAP_LEN,
                    SOP_SOP0, true, state_ready, 0, failed_state, 0);
    }
    else
    {
        /* Send a Not Supported message if there are no batteries */
        policy_send_notsupported(port);
    }
}

static void policy_state_give_alert(struct port *port)
{
    uint8_t             status = 0;
    unsigned            i      = 0;
    EVENT_PD_ALERT_T    evt    = {0};
    enum policy_state_t state  = policy_state_ready(port);

    if (port->policy_substate == 0)
    {
        evt.arg.alert_msg = port->policy_pd_data;
        evt.ret.success   = EVENT_STATUS_DEFAULT;
        /* set everything to 0 */
        for (i = 0; i < PD_DATA_OBJECT_LEN; i++)
        {
            evt.arg.alert_msg->byte[i] = 0;
        }
/* Get dpm to fill out alert request */
#if (CONFIG_ENABLED(EXTENDED_EVENT))
        event_notify(EVENT_PD_GET_ALERT_REQ, port->dev, &evt);
#endif
        policy_set_state(port, PE_Send_Alert, 1);
    }

    status = policy_send(port, DMTAlert, port->policy_pd_data[0].byte, PD_DATA_OBJECT_LEN, SOP_SOP0,
                         false, state, 0, state, 0);
    if (status == STAT_SUCCESS) {
        port->pps_mode_change = false;
    }
    if (status != STAT_BUSY)
    {
        port->msgtx.booltxmsg.alert = false;
    }
}

static void policy_state_process_alert(struct port *port)
{
    EVENT_PD_ALERT_T    evt        = {0};
    uint32_t            alert_type = 0;
    enum policy_state_t state      = policy_state_ready(port);

    /* to determine if there is a change in status */
    port->msgtx.booltxmsg.getstatus = false;
    /* alert dpm of new alert message */
    evt.arg.alert_msg = port->policy_pd_data;
#if (CONFIG_ENABLED(EXTENDED_EVENT))
    event_notify(EVENT_PD_ALERT_RECEIVED, port->dev, &evt);
#endif

    alert_type = evt.arg.alert_msg->ALERT.Type;
    port->alert_response = false;
    /* if there is no event, go back to src/snk ready */
    if (alert_type == 0)
    {
    	policy_set_state(port, state, 0);
    }
    else if (alert_type & ALERT_TYPE_BATTERY)
    {
    	port->alert_response = true;
        /* Determine which batteries to request status for */
        struct ext_get_bat_stat_t batRef;
        port->msgtx.booltxmsg.getbatterystatus = true;
        batRef.BatteryStatRef = (evt.arg.alert_msg->ALERT.FixedBat & 0xfU)
                                | ((evt.arg.alert_msg->ALERT.HSwapBat << 4) & 0xf0U);
        /* write batRef into port variable to pass to get battery status */
        port->bat_stat_ref = batRef.BatteryStatRef;
        /* clear battery status bit in alert_type */
        alert_type &= ~ALERT_TYPE_BATTERY;

        /* check if any other status bit is set */
        if (alert_type != 0)
        {
            /* set request status bool to true */
            policy_set_state(port, PE_Get_Status, 0);
        }
        else
        {
            policy_set_state(port, state, 0);
        }
    }
    else
    {
    	policy_set_state(port, PE_Get_Status, 0);
    }

}

/////////////////////////////////////////////////////////////////////////
// Common States
/////////////////////////////////////////////////////////////////////////
static void policy_state_error_recovery(struct port *port)
{
    policy_pd_disable(port);
    EVENT_ERROR_RECOVERY_T evt = {0};
    event_notify(EVENT_ERROR_RECOVERY, port->dev, &evt);
}

static void policy_state_not_supported(struct port *port)
{
    /* TODO Inform DPM if needed. */
    policy_set_state(port, policy_state_ready(port), 0);
}

/////////////////////////////////////////////////////////////////////////
// Utility functions
/////////////////////////////////////////////////////////////////////////
static void policy_state_send_hardreset(struct port *port)
{
    uint32_t            status = 0;
    enum policy_state_t next_state = policy_state_transition_to_default(port);
    switch (port->policy_substate)
    {
        case 0:
            status = policy_send(port, CMTHardreset, 0, 0, SOP_SOP0, false, next_state, 0,
                                 PE_ErrorRecovery, 0);
            if (status == STAT_SUCCESS)
            {
                port->hard_reset_counter++;
                timer_disable(&port->timers[PD_RSP_TIMER]);
                timer_start(&port->timers[POLICY_TIMER], ktPSHardReset - ktPSHardResetOverhead);
            }
#if CONFIG_ENABLED(FAULT_PROTECTION)
            if (port->uvp_active)
            {
            	/* Don't wait for hardreset if we debounce UVP */
            	policy_set_state(port, next_state, 0);
            	timer_disable(&port->timers[PD_RSP_TIMER]);

            }
#endif
            break;
        default:
            policy_set_state(port, PE_ErrorRecovery, 0);
            break;
    }
}

static void policy_state_send_cblrst(struct port *port)
{
    uint32_t            status     = 0;
    enum policy_state_t next_state = policy_state_ready(port);
    switch (port->policy_substate)
    {
        case 0:
            status = policy_send(port, CMTHardreset, 0, 0, SOP_SOP1, false, next_state, 0,
                                 next_state, 0);
            if (status == STAT_SUCCESS)
            {
                protocol_reset_sop(port, SOP_MASK_SOP1 | SOP_MASK_SOP2);
                /*reset PD VDM driver here, effectively exiting entered mode*/
                vdm_list_reset_all(port->dev, 0xFF00, ((1U << SOP_SOP0) | (1U << SOP_SOP1) | (1U << SOP_SOP2)), 0);
            }
            break;
        default:
            policy_set_state(port, next_state, 0);
            break;
    }
}

static void policy_state_notsupported(struct port *port)
{
    enum policy_state_t next_state = policy_state_ready(port);
    enum policy_state_t fail_state = policy_state_send_soft_reset(port);

    if (((port->prl_msg_rx_sop == SOP_SOP0) && port_pd_ver(port, SOP_SOP0) != PD_REV3)
        || ((port->prl_msg_rx_sop == SOP_SOP0) && port_pd_ver(port, SOP_SOP0) != PD_REV3))
    {
        policy_send(port, CMTReject, 0, 0, port->prl_msg_rx_sop, false, next_state, 0, fail_state,
                    0);
    }
    else
    {
        policy_send(port, CMTNotSupported, 0, 0, port->prl_msg_rx_sop, false, next_state, 0,
                    fail_state, 0);
    }
}

static void policy_state_send_cable_softreset(struct port *port)
{
    uint32_t            status     = 0;
    enum policy_state_t next_state = policy_state_ready(port);
    switch (port->policy_substate)
    {
        case 0:
            status = policy_send(port, CMTSoftReset, 0, 0, SOP_SOP0, false,
                                 PE_DFP_CBL_Send_Soft_Reset, 1, next_state, 0);
            if (status == STAT_SUCCESS)
            {
                timer_start(&port->timers[POLICY_TIMER], ktSenderResponse);
            }
            break;
        case 1:
            if (port->prl_msg_rx)
            {
                protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                                 sizeof(port->policy_pd_data));
                port->prl_msg_rx = false;
                if (port->policy_pd_header.sop.NumDataObjects == 0
                    && port->policy_pd_header.sop.MessageType == CMTAccept)
                {
                    policy_set_state(port, next_state, 0);
                    port->policy_has_contract       = false;
                    port->usb_pd_contract.object    = 0;
                    port->caps_header_received.word = 0;
                    protocol_reset_sop(port, SOP_MASK_SOP1 | SOP_MASK_SOP2);
                }
                else
                {
                    /* Unexpected message */
                    policy_set_state(port, PE_DFP_CBL_Send_Cable_Reset, 0);
                }
            }
            else if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                policy_set_state(port, PE_DFP_CBL_Send_Cable_Reset, 0);
            }
            break;
        default:
            policy_set_state(port, PE_ErrorRecovery, 0);
            break;
    }
}

static void policy_state_send_cable_hardreset(struct port *port)
{
    port->cbl_rst_state = CBL_RST_DISABLED;
    policy_state_send_cblrst(port);
}

static void policy_state_bist_carriermode(struct port *port)
{
    /* Prevent sleep during BIST */
    port->event = true;

    switch (port->policy_substate)
    {
        case 0:
            /* Direct the protocol engine to start CM2 signaling  */
            fusbdev_tcpd_set_bist_mode(port->tcpd_device, BIST_MODE_TX);
            timer_start(&port->timers[POLICY_TIMER], ktBISTContMode);
            policy_set_state(port, PE_BIST_Carrier_Mode, 1);
            break;
        case 1:
            /* Wait few milliseconds for Bist to disable */
            if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                fusbdev_tcpd_set_bist_mode(port->tcpd_device, BIST_MODE_DISABLE);
                timer_start(&port->timers[POLICY_TIMER], ktGoodCRCDelay);
                policy_set_state(port, PE_BIST_Carrier_Mode, 2);
            }
            break;
        case 2:
            if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                fusbdev_tcpd_pd_enable(port->tcpd_device, true);
                policy_set_state(port, policy_state_transition_to_default(port), 0);
            }
            break;
        default:
            policy_set_state(port, PE_ErrorRecovery, 0);
            break;
    }
}

static void policy_state_bist_test_data(struct port *port)
{
    /* Prevent sleep during BIST */
    port->event = true;

    /* Nothing needed here.  Wait for detach or reset to end this mode. */
}

void policy_state_invalid(struct port *port)
{
    /* reset if we get to an invalid state */
#if (CONFIG_ENABLED(DRP))
    if (port->policy_is_source)
    {
        policy_send_source_hardreset(port);
    }
    else
    {
        policy_send_sink_hardreset(port);
    }
#elif (CONFIG_ENABLED(SRC))
    policy_send_source_hardreset(port);
#elif (CONFIG_ENABLED(SNK))
    policy_send_sink_hardreset(port);
#endif
}

static void policy_state_send_generic_cmd(struct port *port)
{
    enum policy_state_t state = policy_state_ready(port);
    policy_send(port, port->policy_pd_header.sop.MessageType, port->policy_pd_data[0].byte, 0,
                port->prl_msg_tx_sop, false, state, 0, state, 0);
}

static void policy_state_send_generic_data(struct port *port)
{
    enum policy_state_t next_state = policy_state_ready(port);
    bool                ext        = port->policy_pd_header.sop.Extended ? true : false;
    uint32_t            status     = 0;
    switch (port->policy_substate)
    {
        case 0:
            status = policy_send(port, port->policy_pd_header.sop.MessageType,
                                 port->policy_pd_data[0].byte,
                                 port->policy_pd_header.sop.NumDataObjects * 4,
                                 port->prl_msg_tx_sop, ext, PE_Send_Generic_Data, 1, next_state, 0);
            if (status == STAT_SUCCESS)
            {
                timer_start(&port->timers[POLICY_TIMER], ktSenderResponse);
            }
            break;
        case 1:
            if (port->prl_msg_rx)
            {
                protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                                 sizeof(port->policy_pd_data));
                port->prl_msg_rx = false;
            }
            else if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                policy_set_state(port, next_state, 0);
            }
            break;
        default:
            policy_set_state(port, next_state, 0);
            break;
    }
}

static void policy_state_get_fw_id_resp(struct port *port)
{
    POLICY_STATE_T           state_ready   = policy_state_ready(port);
    EVENT_PD_GET_FW_ID_T     evt           = {0};
    GET_FW_ID_RESP_PAYLOAD_T fw_id_resp    = {0};
    FW_UPDATE_HEADER_T      *fw_update_req = (FW_UPDATE_HEADER_T *)&port->policy_pd_data[0].byte[2];

    evt.arg.ver        = fw_update_req->ProtocolVersion;
    evt.arg.command    = fw_update_req->MessageType;
    evt.arg.fw_id_resp = &fw_id_resp;
    evt.ret.success    = EVENT_STATUS_DEFAULT;
    event_notify(EVENT_PD_GET_FW_ID, port->dev, &evt);
    if (evt.ret.success == EVENT_STATUS_DEFAULT)
    {
        fw_id_resp.ProtocolVersion = PDFU_PROTOCOL_V1;
        fw_id_resp.MessageType     = RESP_GET_FW_ID;
        fw_id_resp.Status          = errUNKNOWN;
    }
    policy_send(port, EMTFWUpdateResponse, fw_id_resp.byte, PDFU_GET_FW_ID_RESP_SIZE, SOP_SOP0,
                true, state_ready, 0, state_ready, 0);
}

#if CONFIG_ENABLED(FW_UPDATE)
static void policy_state_initiate_fw_update(struct port *port)
{
    int                          status      = 0;
    POLICY_STATE_T               state_ready = policy_state_ready(port);
    EVENT_PD_FW_INITIATE_T       evt         = {0};
    PDFU_INITIATE_RESP_PAYLOAD_T fw_init_resp = {0};
    PDFU_INITIATE_REQ_PAYLOAD_T *fw_update_req =
        (PDFU_INITIATE_REQ_PAYLOAD_T *)&port->policy_pd_data[0].byte[2];

    evt.arg.fw_init_req  = fw_update_req;
    evt.arg.fw_init_resp = &fw_init_resp;
    evt.ret.success      = EVENT_STATUS_DEFAULT;
    event_notify(EVENT_PD_FW_INITIATE, port->dev, &evt);
    if (evt.ret.success == EVENT_STATUS_DEFAULT)
    {
        fw_init_resp.ProtocolVersion = PDFU_PROTOCOL_V1;
        fw_init_resp.MessageType     = RESP_GET_FW_ID;
        fw_init_resp.Status          = errUNKNOWN;
    }
    status = policy_send(port, EMTFWUpdateResponse, fw_init_resp.byte, 7, SOP_SOP0, true,
                         state_ready, 0, state_ready, 0);
    if (status == STAT_SUCCESS && evt.ret.success == EVENT_STATUS_SUCCESS)
    {
        event_notify(EVENT_PD_INITIATE_RESP_SENT, port->dev, 0);
    }
}
#endif

static void policy_state_fw_update_resp(struct port *port)
{
    FW_UPDATE_HEADER_T *fw_update_req = (FW_UPDATE_HEADER_T *)&port->policy_pd_data[0].byte[2];

    switch (fw_update_req->MessageType)
    {
        case REQ_GET_FW_ID:
            policy_state_get_fw_id_resp(port);
            break;
#if CONFIG_ENABLED(FW_UPDATE)
        case REQ_PDFU_INITIATE:
            policy_state_initiate_fw_update(port);
            break;
#endif
        case REQ_VENDOR_SPECIFIC:
            break;
        default:
            policy_set_state(port, PE_Chunk_NotSupported, 0);
            break;
    }
}

const void (*policy_state_run[NUM_PE_STATES])(struct port *port) = {
#if (CONFIG_ENABLED(SRC))
        policy_state_source_startup,
        policy_state_source_ready,
        policy_state_source_softreset,
        policy_state_source_send_softreset,
        policy_state_send_hardreset,
        policy_state_source_transition_default,
        policy_state_notsupported,
        policy_state_not_supported,
        policy_state_source_discovery,
        policy_state_source_send_capabilities,
        policy_state_source_give_source_capabilities,//10
        policy_state_source_negotiate_capability,/* index 11 */
        policy_state_source_transition_supply,
        policy_state_source_capability_response,
        policy_state_source_get_sink_cap,
        policy_state_source_wait_new_capabilities,
        policy_state_source_send_ping,
        policy_state_get_sinkcap_extended,
        policy_state_give_extended_cap,
        policy_state_source_give_pps_status,
        policy_state_source_give_source_info,//20
#if (CONFIG_ENABLED(EPR))
        policy_state_source_evaluate_epr_mode_entry,/* index 21 */
        policy_state_source_epr_mode_entry_ack,
        policy_state_source_epr_mode_entry_failed,
        policy_state_source_epr_mode_discover_cable,
        policy_state_source_epr_mode_evaluate_cable_epr,
        policy_state_source_epr_mode_entry_succeeded,
        policy_state_source_epr_keep_alive,
        policy_state_source_send_epr_mode_exit,
#endif
#endif
#if (CONFIG_ENABLED(SNK))
        policy_state_sink_startup,
        policy_state_sink_ready,
        policy_state_sink_softreset,/* index 30 */
        policy_state_sink_send_softreset,
        policy_state_send_hardreset,
        policy_state_sink_transitiondefault,
        policy_state_notsupported,
        policy_state_not_supported,
        policy_state_sink_discovery,
        policy_state_sink_waitcaps,
        policy_state_sink_evaluate_caps,
        policy_state_sink_select_capability,
        policy_state_sink_transition_sink,
        policy_state_sink_give_sink_cap,/* index 40 */
        policy_state_sink_get_source_cap,
        policy_state_give_extended_sink_cap,
        policy_state_get_sourcecap_extended,
        policy_state_get_pps_status,
        policy_state_sink_get_source_info,
#if CONFIG_ENABLED(EPR)
        policy_state_sink_epr_mode_entry,
        policy_state_sink_epr_mode_entry_wait_for_response,
        policy_state_sink_epr_keep_alive,
        policy_state_sink_send_epr_exit,
        policy_state_invalid,/* exit mode received unimplemented, index 50 */
#endif
#endif
#if (CONFIG_ENABLED(DRP))
        policy_state_source_evaluate_prswap,
        policy_state_source_send_prswap,
        policy_state_sink_evaluate_prswap,
        policy_state_sink_sendprswap,
        policy_state_source_get_sourcecap,
        policy_state_source_give_sink_cap,
        policy_state_sink_get_sink_cap,
        policy_state_sink_give_sourcecap,
        policy_state_get_sourcecap_extended,
        policy_state_give_extended_cap,/* index 60 */
        policy_state_get_sinkcap_extended,
        policy_state_give_extended_sink_cap,
#endif
#if (CONFIG_ENABLED(VDM))
        policy_state_source_cable_query,
#if !CONFIG_ENABLED(VDM_MINIMAL)
        policy_state_give_vdm,
#endif
#endif
#if (CONFIG_ENABLED(USB4))
        policy_state_send_enter_usb,
        policy_state_process_enter_usb,
        policy_state_send_data_reset,
        policy_state_evaluate_data_reset,
        policy_state_dfp_execute_data_reset,
        policy_state_ufp_execute_data_reset,/* index 70 */
#endif
        policy_state_get_status,
        policy_state_give_status,
        policy_state_chunking_not_supported,
        policy_state_get_battery_cap,
        policy_state_give_battery_capabilites,
        policy_state_get_battery_status,
        policy_state_give_battery_status,
        policy_state_invalid, /* get_manufacturer_info not supported */
        policy_state_give_manufacture_info,
        policy_state_invalid, /* get country codes not supported, index 80 */
        policy_state_invalid, /* give country codes not supported */
        policy_state_invalid, /* STATE(PE_Get_Country_Info) */
        policy_state_invalid, /* STATE(PE_Give_Country_Info) */
        policy_state_give_alert,
        policy_state_process_alert,
        policy_state_invalid,/* Security req not supported */
        policy_state_invalid,/* security response not supported */
        policy_state_invalid,/* security response received not supported */
        policy_state_invalid, /* fw update req not supported */
        policy_state_fw_update_resp,/* firmware update response supported, index 90 */
        policy_state_invalid, /* firmware update response received not supported */
        policy_state_send_drswap,
        policy_state_evaluate_drswap,
        policy_state_send_drswap,
        policy_state_evaluate_drswap,
        policy_state_send_vconn_swap,
        policy_state_evaluate_vconn_swap,
        policy_state_invalid,/* cbl soft reset not supported */
        policy_state_invalid,/* cbl hard reset not supported */
        policy_state_send_cable_softreset,/* index 100 */
        policy_state_send_cable_softreset,
        policy_state_send_cable_hardreset,
        policy_state_bist_carriermode,
        policy_state_bist_test_data,
        policy_state_error_recovery,
        policy_state_get_revision,
        policy_state_give_revision,
        policy_state_send_generic_cmd,
        policy_state_send_generic_data,
        policy_state_disabled
    };

void policy_engine(struct port *port)
{
    uint32_t abc = port->policy_state;
    policy_state_run[port->policy_state](port);
}
