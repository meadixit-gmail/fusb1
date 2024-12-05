/*******************************************************************************
 * @file     vdm_vendor.c
 * @author   USB PD Firmware Team
 * @brief
 * @addtogroup App
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
 *
 * @tableofcontents
 *
 * @{
 ******************************************************************************/

#include "vdm.h"
#include "vif_types.h"
#include "port_tcpd.h"

#if CONFIG_ENABLED(VDM) && !CONFIG_ENABLED(VDM_MINIMAL)

#define VENDOR_SID 0x1057

enum
{
    VENDOR_STATE_DISABLED,
    VENDOR_STATE_REQ_MODE,
    VENDOR_STATE_ENTER_MODE,
    VENDOR_STATE_EXIT_MODE,
};
/**
 *  driver objects for two ports
 */
struct vendor_state
{
    uint32_t   id;
    bool       enabled;
    bool       modeen;
    uint32_t   state;
    VDM_OBJ_T *obj;
} VENDOR_STATE[2];

static void vendor_reset(struct port_tcpd *const, uint16_t);

/**
 * @brief handle the port partner mode response
 * @param arg partner Mode response
 */
static void vendor_process_partner_mode(VDM_SM_ARG_T *arg)
{
    PD_MSG_T *rspvdo = (PD_MSG_T *)arg->buf.vdo;
    PD_MSG_T  header;
    header.object               = arg->buf.vdo[0];
    struct vendor_state *pState = &VENDOR_STATE[arg->dev->port_tcpd_id];
    arg->buf.count              = 0;
    if (header.SVDM.HEADER.CommandType == VDM_CMD_TYPE_REQ)
    {
        rspvdo->object              = 0;
        rspvdo->SVDM.HEADER.SVID    = pState->obj->svid;
        rspvdo->SVDM.HEADER.VersionMajor = VDM_STRUCTURED_VER(arg->pdrev);
        rspvdo->SVDM.HEADER.VDMType     = 1;
        rspvdo->SVDM.HEADER.ObjPos      = 0;
        rspvdo->SVDM.HEADER.CommandType = VDM_CMD_TYPE_ACK;
        rspvdo->SVDM.HEADER.Command     = VDM_CMD_DISCOVER_MODES;
        rspvdo->SVDM.HEADER.VersionMinor = VDM_STRUCTURED_MAX_VER_MINOR(arg->pdrev);
        rspvdo++;
        arg->buf.count++;
        /* MODE */
        rspvdo->object = 0x1;
        arg->buf.count++;
        arg->tx_msg = true;
    }
    else if (header.SVDM.HEADER.CommandType == VDM_CMD_TYPE_ACK)
    {
        pState->state = VENDOR_STATE_ENTER_MODE;
    }
    else
    {
        pState->enabled = false;
    }
}

/**
 * @brief Handle partner enter mode response
 * @param arg response to enter mode command
 */
static void vendor_process_partner_enter_mode(VDM_SM_ARG_T *arg)
{
    struct vendor_state *pState = &VENDOR_STATE[arg->dev->port_tcpd_id];
    PD_MSG_T            *rspvdo = (PD_MSG_T *)arg->buf.vdo;
    PD_MSG_T             header;
    header.object  = arg->buf.vdo[0];
    arg->buf.count = 0;
    if (header.SVDM.HEADER.CommandType == VDM_CMD_TYPE_REQ)
    {
        if (header.SVDM.HEADER.ObjPos == 1)
        {
            pState->modeen = true;
        }
        rspvdo->object              = 0;
        rspvdo->SVDM.HEADER.SVID    = pState->obj->svid;
        rspvdo->SVDM.HEADER.VersionMajor = VDM_STRUCTURED_VER(arg->pdrev);
        rspvdo->SVDM.HEADER.VDMType = 1;
        rspvdo->SVDM.HEADER.ObjPos  = header.SVDM.HEADER.ObjPos;
        if (rspvdo->SVDM.HEADER.ObjPos == 1)
        {
            rspvdo->SVDM.HEADER.CommandType = VDM_CMD_TYPE_ACK;
        }
        else
        {
            rspvdo->SVDM.HEADER.CommandType = VDM_CMD_TYPE_NAK;
        }
        rspvdo->SVDM.HEADER.Command = VDM_CMD_ENTER_MODE;
        rspvdo->SVDM.HEADER.VersionMinor = VDM_STRUCTURED_MAX_VER_MINOR(arg->pdrev);
        arg->buf.count++;
        arg->tx_msg = true;
    }
    else if (header.SVDM.HEADER.CommandType == VDM_CMD_TYPE_ACK)
    {
        pState->state   = VENDOR_STATE_DISABLED;
        pState->enabled = false;
        pState->modeen  = true;
    }
    else
    {
        pState->enabled = false;
        pState->state   = VENDOR_STATE_DISABLED;
    }
}

/**
 * @brief Handle partner exit mode response
 * @param arg Partner exit mode response
 */
static void vendor_process_partner_exit_mode(VDM_SM_ARG_T *arg)
{
    struct vendor_state *pState = &VENDOR_STATE[arg->dev->port_tcpd_id];
    PD_MSG_T            *rspvdo = (PD_MSG_T *)arg->buf.vdo;
    PD_MSG_T             header;
    header.object  = arg->buf.vdo[0];
    arg->buf.count = 0;
    if (header.SVDM.HEADER.CommandType == VDM_CMD_TYPE_REQ)
    {
        rspvdo->object              = 0;
        rspvdo->SVDM.HEADER.SVID    = pState->obj->svid;
        rspvdo->SVDM.HEADER.VersionMajor = VDM_STRUCTURED_VER(arg->pdrev);
        ;
        rspvdo->SVDM.HEADER.VDMType = 1;
        rspvdo->SVDM.HEADER.ObjPos  = header.SVDM.HEADER.ObjPos;
        if (rspvdo->SVDM.HEADER.ObjPos == 1 && pState->modeen)
        {
            rspvdo->SVDM.HEADER.CommandType = VDM_CMD_TYPE_ACK;
        }
        else
        {
            rspvdo->SVDM.HEADER.CommandType = VDM_CMD_TYPE_NAK;
        }
        pState->modeen              = false;
        rspvdo->SVDM.HEADER.Command = VDM_CMD_EXIT_MODE;
        rspvdo->SVDM.HEADER.VersionMinor = VDM_STRUCTURED_MAX_VER_MINOR(arg->pdrev);
        arg->buf.count++;
        arg->tx_msg = true;
    }
    else if (header.SVDM.HEADER.CommandType == VDM_CMD_TYPE_ACK)
    {
        pState->state   = VENDOR_STATE_DISABLED;
        pState->modeen  = false;
        pState->enabled = false;
    }
    else
    {
        pState->enabled = false;
    }
}

/**
 * @brief Default handler for message that are not valid
 * @param arg Contains buffer to write NAK message
 */
static void vendor_partner_nak(VDM_SM_ARG_T *arg)
{
    PD_MSG_T *vdo = (PD_MSG_T *)arg->buf.vdo;
    PD_MSG_T  header;
    header.object = arg->buf.vdo[0];
    if (header.SVDM.HEADER.CommandType == VDM_CMD_TYPE_REQ)
    {
        vdo->object                  = 0;
        vdo->SVDM.HEADER.SVID        = VENDOR_STATE[arg->dev->port_tcpd_id].obj->svid;
        vdo->SVDM.HEADER.VersionMajor = VDM_STRUCTURED_VER(arg->pdrev);
        vdo->SVDM.HEADER.VDMType     = 1;
        vdo->SVDM.HEADER.ObjPos      = header.SVDM.HEADER.ObjPos;
        vdo->SVDM.HEADER.CommandType = VDM_CMD_TYPE_NAK;
        vdo->SVDM.HEADER.Command     = header.SVDM.HEADER.Command;
        vdo->SVDM.HEADER.VersionMinor = VDM_STRUCTURED_MAX_VER_MINOR(arg->pdrev);
        arg->buf.count               = 1;
    }
}

/**
 * @brief Handle incoming messages from port partner
 * @param arg Argument contains messages and data
 */
static void vendor_process_partner(VDM_SM_ARG_T *arg)
{
    PD_MSG_T header;
    header.object = arg->buf.vdo[0];
    switch (header.SVDM.HEADER.Command)
    {
        case VDM_CMD_DISCOVER_MODES:
            vendor_process_partner_mode(arg);
            break;
        case VDM_CMD_ENTER_MODE:
            vendor_process_partner_enter_mode(arg);
            break;
        case VDM_CMD_EXIT_MODE:
            vendor_process_partner_exit_mode(arg);
            break;
        default:
            vendor_partner_nak(arg);
            break;
    }
}

/**
 * @brief Create DP Request mode message
 * @param arg
 */
static void vdm_dp_request_modes(VDM_SM_ARG_T *arg)
{
    PD_MSG_T vdm;
    vdm.object                  = 0;
    vdm.SVDM.HEADER.SVID        = VENDOR_STATE[arg->dev->port_tcpd_id].obj->svid;
    vdm.SVDM.HEADER.VDMType     = VDM_STRUCTURED;
    vdm.SVDM.HEADER.VersionMajor = VDM_STRUCTURED_VER(arg->pdrev);
    vdm.SVDM.HEADER.ObjPos      = 0;
    vdm.SVDM.HEADER.CommandType = VDM_CMD_TYPE_REQ;
    vdm.SVDM.HEADER.Command     = VDM_CMD_DISCOVER_MODES;
    vdm.SVDM.HEADER.VersionMinor = VDM_STRUCTURED_MAX_VER_MINOR(arg->pdrev);
    arg->buf.vdo[0]             = vdm.object;
    arg->buf.count              = 1;
}

/**
 * @brief Create DP Enter mode request message
 * @param arg Contains buffer for output message
 */
static void vdm_dp_request_enter_mode(VDM_SM_ARG_T *arg)
{
    PD_MSG_T vdm;
    vdm.object                  = 0;
    vdm.SVDM.HEADER.SVID        = VENDOR_STATE[arg->dev->port_tcpd_id].obj->svid;
    vdm.SVDM.HEADER.VDMType     = VDM_STRUCTURED;
    vdm.SVDM.HEADER.VersionMajor = VDM_STRUCTURED_VER(arg->pdrev);
    vdm.SVDM.HEADER.ObjPos      = 0x1;
    vdm.SVDM.HEADER.CommandType = VDM_CMD_TYPE_REQ;
    vdm.SVDM.HEADER.Command     = VDM_CMD_ENTER_MODE;
    vdm.SVDM.HEADER.VersionMinor = VDM_STRUCTURED_MAX_VER_MINOR(arg->pdrev);
    arg->buf.vdo[0]             = vdm.object;
    arg->buf.count              = 1;
}

/**
 * @brief Create DP Exit mode request message
 * @param arg Contains buffer for output message
 */
static void vdm_dp_request_exit_mode(VDM_SM_ARG_T *arg)
{
    PD_MSG_T vdm;
    vdm.object                  = 0;
    vdm.SVDM.HEADER.SVID        = VENDOR_STATE[arg->dev->port_tcpd_id].obj->svid;
    vdm.SVDM.HEADER.VDMType     = VDM_STRUCTURED;
    vdm.SVDM.HEADER.VersionMajor = VDM_STRUCTURED_VER(arg->pdrev);
    vdm.SVDM.HEADER.ObjPos      = 0x1;
    vdm.SVDM.HEADER.CommandType = VDM_CMD_TYPE_REQ;
    vdm.SVDM.HEADER.Command     = VDM_CMD_EXIT_MODE;
    vdm.SVDM.HEADER.VersionMinor = VDM_STRUCTURED_MAX_VER_MINOR(arg->pdrev);
    arg->buf.vdo[0]             = vdm.object;
    arg->buf.count              = 1;
}

/**
 * @brief State Machine entry for VDM driver
 * @param arg Argument to state machine contains message and buffer
 */
static void vendor_sm(VDM_SM_ARG_T *arg)
{
    struct vendor_state *vdm = &VENDOR_STATE[arg->dev->port_tcpd_id];
    struct port * port = arg->dev->port_tcpd_priv;
    switch (vdm->state)
    {
        case VENDOR_STATE_REQ_MODE:
            vdm_dp_request_modes(arg);
            port->vdm_tx_svid = VENDOR_SID;
            break;
        case VENDOR_STATE_ENTER_MODE:
            vdm_dp_request_enter_mode(arg);
            port->vdm_tx_svid = VENDOR_SID;
            break;
        case VENDOR_STATE_EXIT_MODE:
            vdm_dp_request_exit_mode(arg);
            port->vdm_tx_svid = VENDOR_SID;
            break;
        default:
            vdm->enabled = false;
            vdm->state   = VENDOR_STATE_DISABLED;
            port->vdm_tx_svid = 0;
            break;
    }
}

/**
 * @brief Check the active state of the VDM Driver
 * @param portid
 * @return true if active otherwise false
 */
static bool vendor_active(struct port_tcpd const *dev)
{
    return VENDOR_STATE[dev->port_tcpd_id].enabled;
}

/**
 * @brief Set VDM active. When partner advertises VDM with DP_SID port VDM
 * driver is activated. Otherwise it is deactivated.
 * @param port Port which detect DP_SID in partner.
 * @param active Set active state if true
 */
static void vendor_set_active(struct port_tcpd const *dev, bool active)
{
    VENDOR_STATE[dev->port_tcpd_id].enabled = active;
}

/**
 * DP Driver driver callback handlers
 */
static SVDM_HANDLER_T svdmh = {
    .vdm_process = vendor_process_partner,
    .sm          = vendor_sm,
};

/**
 * Policy VDM driver object
 */
static VDM_OBJ_T vendor_vdm_obj = {
    .structured = true,
    .svid       = VENDOR_SID,
    .svdmh      = &svdmh,
    .active     = vendor_active,
    .set_active = vendor_set_active,
    .reset      = vendor_reset,
	.sopmask = (1 << SOP_SOP0) | (1U << SOP_SOP1) | (1U << SOP_SOP2),
};

/**
 * POLCIY VDM list object
 */
VDM_ITEM_T vendor_driv = {
    .portmask = 0,
    .vdm      = &vendor_vdm_obj,
    .next     = 0,
};

/**
 * @brief Function to reset the DP driver state
 * @param port The port VDM driver to reset
 */
static void vendor_reset(struct port_tcpd *const dev, uint16_t state_reset_enum)
{
    struct vendor_state *vdm = &VENDOR_STATE[dev->port_tcpd_id];
    vdm->id                  = dev->port_tcpd_id;
    vdm->enabled             = false;
    if (state_reset_enum == 0)
    {
    	vdm->state           = VENDOR_STATE_REQ_MODE;
    }
    else
    {
    	vdm->state           = state_reset_enum;
    }
    vdm->obj                 = &vendor_vdm_obj;
    vdm->modeen              = false;
}

#endif /* CONFIG_VDM */

/** @} */
