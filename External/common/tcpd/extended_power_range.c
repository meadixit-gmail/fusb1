/*******************************************************************************
 * @file     extended_power_range.c
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
 * extended_power_range.c
 *
 * Implements the extended power range (EPR) functions
 */
#include "extended_power_range.h"
#include "dev_tcpd.h"
#include "vif_types.h"
#include "policy.h"
#include "typec.h"
#include "protocol.h"
#include "vdm.h"
#include "observer.h"
#include "logging.h"
#include <stdlib.h>

#if CONFIG_ENABLED(EPR)
#if CONFIG_ENABLED(SNK)
void policy_state_sink_epr_mode_entry(struct port *port)
{
    PD_MSG_T tx_epr_mode = {0};
    PD_MSG_T rx_epr_mode = {0};
    uint32_t status      = 0;

    tx_epr_mode.object = 0;
    rx_epr_mode.object = port->prl_rx_data[0].object;

    switch (port->policy_substate)
    {
        case 0:
            /* Enter */
            tx_epr_mode.EPRMDO.action = EprModeEnter;
            tx_epr_mode.EPRMDO.data   = port->vif->PD_Power_as_Sink / 1000;
            status = policy_send(port, DMTEPRMode, tx_epr_mode.byte, 4, SOP_SOP0, false,
                                 PE_SNK_Send_Epr_Mode_Entry, 1, PE_SNK_Ready, 0);
            if (status == STAT_SUCCESS)
            {
                timer_start(&port->timers[POLICY_TIMER], ktSenderResponse);
                timer_start(&port->timers[SINK_EPR_ENTER_TIMER], tEnterEPR);
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
                    && port->policy_pd_header.sop.MessageType == DMTEPRMode
                    && port->policy_pd_data[0].EPRMDO.action == EprModeEnterAck)
                {
#if (CONFIG_ENABLED(SNK))
                    port->epr_try_entry = 0;
#endif

                    policy_set_state(port, PE_SNK_Epr_Mode_Entry_Wait_For_Response, 0);
                }
                else
                {
                    timer_disable(&port->timers[POLICY_TIMER]);
                    policy_set_state(port, PE_SNK_Send_Soft_Reset, 0);
                }
            }
            else if (timer_expired(&port->timers[POLICY_TIMER])
                     || timer_expired(&port->timers[SINK_EPR_ENTER_TIMER]))
            {
                timer_disable(&port->timers[POLICY_TIMER]);
                policy_set_state(port, PE_SNK_Send_Soft_Reset, 0);
            }
            break;
        default:
            break;
    }
}

void policy_state_sink_epr_mode_entry_wait_for_response(struct port *port)
{
    PD_MSG_T rx_epr_mode = {0};
    switch (port->policy_substate)
    {
        case 0:
            if (port->prl_msg_rx)
            {
                protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
                                 sizeof(port->policy_pd_data));

                port->prl_msg_rx   = false;
                rx_epr_mode.object = port->prl_rx_data[0].object;
                if (port->policy_pd_header.sop.NumDataObjects > 0)
                {
                    if (port->policy_pd_header.sop.Extended == 0
                        && port->policy_pd_header.sop.MessageType == DMTEPRMode
                        && rx_epr_mode.EPRMDO.action == EprModeEnterSucc)
                    {
                        port->epr_mode               = true;
                        port->negotiate_epr_contract = true;
                        port->msgtx.booltxmsg.get_src_caps = true;
                        policy_set_state(port, PE_SNK_Wait_For_Capabilities, 0);
                        timer_start(&port->timers[POLICY_TIMER], ktTypeCSinkWaitCap);
                        timer_disable(&port->timers[SINK_EPR_ENTER_TIMER]);
                    }
                }
                else if (port->policy_pd_header.sop.MessageType == CMTVCONN_Swap)
                {
                    policy_set_state(port, PE_VCS_Evaluate_Swap, 0);
                }
                else
                {
                    timer_disable(&port->timers[POLICY_TIMER]);
                    policy_set_state(port, PE_SNK_Send_Soft_Reset, 0);
                }
            }
            else if (timer_expired(&port->timers[SINK_EPR_ENTER_TIMER]))
            {
                timer_disable(&port->timers[SINK_EPR_ENTER_TIMER]);
                policy_set_state(port, PE_SNK_Send_Soft_Reset, 0);
            }
            break;
    }
}
void policy_state_sink_epr_keep_alive(struct port *port)
{
    EXT_CONTROL_T msg = {0};
    msg.type          = EPR_KeepAlive;
    msg.data          = 0;
    uint32_t status   = 0;
    switch (port->policy_substate)
    {
        case 0:
            /*TODO: Add ACK support*/
            status = policy_send(port, EMTExtendedControl, msg.bytes, 2, SOP_SOP0, true,
                                 PE_SNK_EPR_Keep_Alive, 1, PE_SNK_Ready, 0);
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
                if (port->policy_pd_header.sop.MessageType == EMTExtendedControl &&
                	port->policy_pd_header.sop.Extended == 1 &&
                    port->policy_pd_data[0].EXTCTRLDO.type == EPR_KeepAlive_Ack)
                {
                    timer_start(&port->timers[POLICY_TIMER], ktEPRSinkKeepAlive);
                    policy_set_state(port, PE_SNK_Ready, 0);
                }
                else
                {
                    port->epr_mode = false;
                    policy_set_state(port, PE_SNK_Hard_Reset, 0);
                }
            }
            if (timer_expired(&port->timers[POLICY_TIMER]))
            {
                policy_set_state(port, PE_SNK_Hard_Reset, 0);
            }
            break;
        default:
            break;
    }
}

void policy_state_sink_send_epr_exit(struct port *port)
{
	PD_MSG_T msg      = {0};
    msg.EPRMDO.action = EprModeExit;
    msg.EPRMDO.data   = 0;
    uint32_t status   = 0;

    switch(port->policy_substate)
    {
         case 0:
             status = policy_send(port, DMTEPRMode, msg.byte, 4, SOP_SOP0, false,
                                  PE_SNK_Wait_For_Capabilities, 0, PE_SNK_Ready, 0);
             if(status == STAT_SUCCESS)
             {
                 port->epr_mode = false;
             }
             break;
         default:
             break;
    }
}
#endif

void policy_state_read_epr_mode(struct port *port)
{
    struct pd_msg_t *msg = &port->policy_pd_data[0];
#if CONFIG_ENABLED(SRC)
    if (port->policy_state == PE_SRC_Ready)
    {
        if (msg->EPRMDO.action == EprModeEnter)
        {
            policy_set_state(port, PE_SRC_Evaluate_EPR_Mode_Entry, 0);
        }
        else if (msg->EPRMDO.action == EprModeExit && port->epr_mode)
        {
            port->epr_mode = false;
            if (port->sink_request[0].AVSRDO.ObjectPosition >= 7)
            {
                /* violation (to exit must be in SPR contract) */
                policy_state_invalid(port);
            }
            else
            {
                /* exit epr mode */
                port->epr_mode = false;
                policy_set_state(port, PE_SRC_Send_Capabilities, 0);
            }
        }
        else
        {
            /* Unexpected message */
            policy_set_state(port, PE_SRC_Send_Soft_Reset, 0);
        }
    }
#endif
#if CONFIG_ENABLED(SNK)
    if (port->policy_state == PE_SNK_Ready)
    {
        if (msg->EPRMDO.action == EprModeExit && port->epr_mode)
        {
            port->epr_mode = false;
            if (port->sink_request[0].AVSRDO.ObjectPosition > 7)
            {
                /* violation (to exit must be in SPR contract) */
                policy_state_invalid(port);
            }
            else
            {
                /* exit epr mode */
                port->epr_mode = false;
                policy_set_state(port, PE_SNK_Wait_For_Capabilities, 0);
            }
        }
        else
        {
            /* Unexpected message */
            policy_set_state(port, PE_SNK_Send_Soft_Reset, 0);
        }
    }
#endif
}
#if CONFIG_ENABLED(SRC)
void policy_state_source_evaluate_epr_mode_entry(struct port *port)
{
    EVENT_EPR_MODE_ENTRY_T evt = {0};
    struct pd_msg_t       *msg = &port->epr_mode_message;
    msg->EPRMDO.data           = EPREnterFailUnknown;
    msg->EPRMDO.action         = EprModeEnterFail;
    switch (port->policy_substate)
    {
        case 0:
            /*2.a - check most recent rdo */
            if (port->epr_capable_rdo)
            {
                /*2.b - check most recent 5v fixed pdo */
                if (port->epr_capable_pdo)
                {
                    event_notify(EVENT_EPR_MODE_ENTRY, port->dev, &evt);
                    if (evt.ret.success == EVENT_STATUS_SUCCESS)
                    {
                        /*Send EPR MODE ACK */
                        msg->EPRMDO.action = EprModeEnterAck;
                    }
                    else
                    {
                        msg->EPRMDO.data = EPREnterFailEPRNotAvailable;
                    }
                }
                else
                {
                    /*2b failure - Send EPR_Mode with Action 4 and Data 5 */
                    msg->EPRMDO.data = EPREnterFailNotEPRCapablePDO;
                }
            }
            else
            {
                /*2a failure - Send EPR_Mode with Action 4 and Data 3*/
                msg->EPRMDO.data = EPREnterFailNotEPRCapableRDO;
            }
            if (msg->EPRMDO.action == EprModeEnterFail)
            {
                policy_set_state(port, PE_SRC_EPR_Mode_Entry_Failed, 0);
            }
            else
            {
                policy_set_state(port, PE_SRC_EPR_Mode_Entry_ACK, 0);
            }
            break;
        default:
            break;
    }
}

void policy_state_source_epr_mode_entry_ack(struct port *port)
{
    int status = 0;
    /*Step 2d - send an EPR message with action field set to 2*/
    switch (port->policy_substate)
    {
        case 0:
            status = policy_send(port, DMTEPRMode, port->epr_mode_message.byte, 4, SOP_SOP0, false,
                                 PE_SRC_EPR_Mode_Entry_ACK, 1, PE_SRC_Send_Soft_Reset, 0);
            break;
        case 1:
        	if (port->prl_msg_rx)
        	{
        		policy_set_state(port, PE_SRC_Send_Soft_Reset, 0);
        	}
        	else if (port->captive_cable)
            {
                /* Step 4 - Skip Vconn swap and Discover Identity if captive cable */
                policy_set_state(port, PE_SRC_EPR_Mode_Entry_Succeeded, 0);
            }
            else
            {
                /* Step 5 - If Vconn source, discover cable, else vconn swap then discover cable */
                if (port->is_vconn_source)
                {
                    /* do not vconn_swap, go discover cable */
                    policy_set_state(port, PE_SRC_EPR_Mode_Discover_Cable, 0);
                }
                else
                {
                    /* vconn swap */
                    policy_set_state(port, PE_VCS_Send_Swap, 0);
                }
            }
            break;
        default:
            break;
    }
}

void policy_state_source_epr_mode_entry_failed(struct port *port)
{
    int status = 0;
    switch (port->policy_substate)
    {
        case 0:
            status = policy_send(port, DMTEPRMode, port->epr_mode_message.byte, 4, SOP_SOP0, false,
                                 PE_SRC_Ready, 0, PE_SRC_Send_Soft_Reset, 0);
            break;
        default:
            break;
    }
}
void policy_state_source_epr_mode_discover_cable(struct port *port)
{
    /* Step 5a, check if we are vconn source */
    if (port->is_vconn_source)
    {
        /* Step 6 - Discover Identity on SOP' to find cable is EPR capable*/
        policy_set_state(port, PE_CBL_Query, 0);
    }
    else
    {
        /*Step 5 a i) - Abort entry*/
        port->epr_mode_message.EPRMDO.action = EprModeEnterFail;
        port->epr_mode_message.EPRMDO.data   = EPREnterFailVConn;
        policy_set_state(port, PE_SRC_EPR_Mode_Entry_Failed, 0);
    }
}

void policy_state_source_epr_mode_evaluate_cable_epr(struct port *port)
{
    /* Make sure can take 50V, 5V on cable, and marked as EPR capable */
    if (port->epr_mode_message.EPRMDO.action == EprModeEnterSucc
        && port->cable_vdo_cable_current == 2 && port->cable_vdo_cable_voltage == 3
        && port->epr_capable_cable
    )
    {
        policy_set_state(port, PE_SRC_EPR_Mode_Entry_Succeeded, 0);
    }
    else if (port->prl_msg_rx)
    {
    	protocol_receive(port, &port->policy_pd_header, port->policy_pd_data,
    	                                 sizeof(port->policy_pd_data));
    	port->prl_msg_rx = false;
    	policy_set_state(port, PE_SRC_Send_Soft_Reset, 0);
    }
    else
    {
        port->epr_mode_message.EPRMDO.action = EprModeEnterFail;
        port->epr_mode_message.EPRMDO.data   = EPREnterFailCable;
        policy_set_state(port, PE_SRC_EPR_Mode_Entry_Failed, 0);
    }
}
void policy_state_source_epr_mode_entry_succeeded(struct port *port)
{
    /* Assume message will be successful (preset epr_mode = true) */
    port->epr_mode = true;
    /* Note get_snk_caps is shared with EPR Get Snk Caps in process_tx_req() */
    port->msgtx.booltxmsg.get_snk_caps = true;
    int status = policy_send(port, DMTEPRMode, port->epr_mode_message.byte, 4, SOP_SOP0, false,
                             PE_SRC_Send_Capabilities, 0, PE_SRC_Send_Soft_Reset, 0);
    if (status == STAT_ABORT || status == STAT_ERROR)
    {
        /* Since state has changed from SRC_Ready, need to disable epr_mode and disable EPR timer */
        port->epr_mode = false;
        timer_disable(&port->timers[EPR_KEEPALIVE_TIMER]);
    }
}
void policy_state_source_epr_keep_alive(struct port *port)
{
    struct ext_control_t msg = {0};
    msg.type                 = EPR_KeepAlive_Ack;
    msg.data                 = 0;
    int status = policy_send(port, EMTExtendedControl, msg.bytes, EXT_CONTROL_LEN, SOP_SOP0, true,
                             PE_SRC_Ready, 0, PE_SRC_Ready, 0);
}

void policy_state_source_send_epr_mode_exit(struct port *port)
{
    PD_MSG_T msg      = {0};
    msg.EPRMDO.action = EprModeExit;
    msg.EPRMDO.data   = 0;
    uint32_t status   = 0;

    switch(port->policy_substate)
    {
         case 0:
             status = policy_send(port, DMTEPRMode, msg.byte, 4, SOP_SOP0, false,
                                  PE_SRC_Send_Capabilities, 0, PE_SRC_Ready, 0);
             if(status == STAT_SUCCESS)
             {
                 port->epr_mode = false;
             }
             break;
         default:
             break;
    }
}
#endif
#endif

