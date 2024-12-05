/*******************************************************************************
 * @file   dp_driv.c
 * @author USB Firmware Team
 * @brief  Display Port driver
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
*******************************************************************************/

#include "vdm.h"
#include "vif_types.h"
#include "dev_tcpd.h"

#if CONFIG_ENABLED(VDM) && !CONFIG_ENABLED(VDM_MINIMAL) && MODAL_OPERATION_SUPPORTED

#define VDM_CMD_DP_STATUS 0x10
#define VDM_CMD_DP_CONFIG 0x11

/**
 * Display port pin mask
 */
enum
{
    DP_PIN_ASSIGN_NS = 0,  /**< PIN ASSIGNMENT None  */
    DP_PIN_ASSIGN_A  = 1,  /**< PIN ASSIGNMENT A   */
    DP_PIN_ASSIGN_B  = 2,  /**< PIN ASSIGNMENT B   */
    DP_PIN_ASSIGN_C  = 4,  /**< PIN ASSIGNMENT C   */
    DP_PIN_ASSIGN_D  = 8,  /**< PIN ASSIGNMENT D   */
    DP_PIN_ASSIGN_E  = 16, /**< PIN ASSIGNMENT E   */
    DP_PIN_ASSIGN_F  = 32, /**< PIN ASSIGNMENT F   */
};

/**
 * Display port signalling
 */
enum
{
    DP_SIGNALING_DP1p3   = 1, /**< DP_SIGNALING_DP1p3   */
    DP_SIGNALING_USBGen2 = 2, /**< DP_SIGNALING_USBGen2 */
};

/**
 * Display port Source mode
 */
enum dp_mode_t
{
    DP_MODE_USB  = 0, /**< Only USB  */
    DP_MODE_DFP  = 1, /**< DP Source  */
    DP_MODE_UFP  = 2, /**< DP Sink  */
    DP_MODE_BOTH = 3, /**< DP Source and Sink */
};

/**
 * Display port capability field
 */
struct dp_cap_t
{
    union
    {
        uint32_t object;
        uint8_t  byte[4];
        struct
        {
            uint32_t UfpCapable       :1;
            uint32_t DfpCapable       :1;
            uint32_t Signaling        :4;
            uint32_t Receptacle       :1;
            uint32_t USB2p0NotUsed    :1;
            uint32_t DfpPinAssignment :8;
            uint32_t UfpPinAssignment :8;
            uint32_t Rsvd0            :8;
        };
    };
};
STATIC_ASSERT(sizeof(struct dp_cap_t) == 4, dp_cap_size_must_be_32_bit);

/**
 * DP status information
 */
struct dp_stat_t
{
    union
    {
        uint32_t object;
        uint8_t  byte[4];
        struct
        {
            uint32_t DfpConnected           :1;
            uint32_t UfPConnected           :1;
            uint32_t PowerLow               :1;
            uint32_t Enabled                :1;
            uint32_t MultiFunctionPreferred :1;
            uint32_t UsbConfigRequest       :1;
            uint32_t ExitDpModeRequest      :1;
            uint32_t HpdState               :1;
            uint32_t IrqHpd                 :1;
            uint32_t Rsvd0                  :23;
        };
    };
};
STATIC_ASSERT(sizeof(struct dp_stat_t) == 4, dp_status_size_must_be_32_bit);

/**
 * DP Config message for port partner
 */
struct dp_config_t
{
    union
    {
        uint32_t object;
        uint8_t  byte[4];
        struct
        {
            uint32_t Dfp       :1;
            uint32_t Ufp       :1;
            uint32_t Signaling :4;
            uint32_t           :2;
            uint32_t PinAssign :8;
        };
    };
};
STATIC_ASSERT(sizeof(struct dp_config_t) == 4, dp_config_size_must_be_32_bit);

/**
 * DP driver internal states
 */
enum dp_state_t
{
    STATE_DISABLED,
    STATE_DP_MODES,
    STATE_DP_ENTER_MODE,
    STATE_DP_EXIT_MODE,
    STATE_DP_STATUS,
    STATE_DP_CONFIG
} DP_STATE_T;

/**
 *  DP driver objects for two ports
 */
struct vdm_state
{
    uint32_t                id;
    enum dp_state_t         state;
    bool                    enabled;
    bool                    modeen;
    bool                    dpen;
    const struct vdm_obj_t *obj;
    struct dp_cap_t         cap;
    struct dp_stat_t        status;

    /* Internal object to track partner states */
    struct
    {
        struct dp_cap_t    cap;
        struct dp_config_t config;
        struct dp_stat_t   status;
        uint32_t           mode_idx;
        bool               cap_match;
        bool               modeen;
    } partner;
} DP_VDM_STATE[2];

static void vdm_dp_reset(struct port_tcpd *const dev, uint16_t state_reset_enum);

/**
 * @brief Call to check if Port can be DFP_D
 * @param partner capabilities for partner #struct dp_cap_t
 * @param port capabilities for port #struct dp_cap_t
 * @return true if port and partner capabilities match.
 */
static bool vdm_dp_evaluate_partner_cap(uint32_t partner, uint32_t port)
{
    bool            result = false;
    struct dp_cap_t port_cap;
    struct dp_cap_t partner_cap;

    port_cap.object    = port;
    partner_cap.object = partner;

    if (((port_cap.UfpCapable & partner_cap.DfpCapable)
         || (port_cap.DfpCapable & partner_cap.UfpCapable))
        && (port_cap.Signaling & partner_cap.Signaling))
    {
        result = true;
    }

    return result;
}

/**
 * @brief handle the port partner mode response
 * @param arg partner Mode response
 */
static void vdm_dp_process_partner_mode(struct vdm_sm_arg_t *arg)
{
    uint32_t         i          = 0;
    struct pd_msg_t *rspvdo     = (struct pd_msg_t *)arg->buf.vdo;
    struct pd_msg_t  header     = {0};
    header.object               = arg->buf.vdo[0];
    struct vdm_state *state_obj = &DP_VDM_STATE[arg->dev->port_tcpd_id];
    uint32_t          vdo_count = arg->buf.count;
    arg->buf.count              = 0;
    if (header.SVDM.HEADER.CommandType == VDM_CMD_TYPE_REQ)
    {
        rspvdo->object                  = 0;
        rspvdo->SVDM.HEADER.SVID        = SVID_VID_DP;
        rspvdo->SVDM.HEADER.VersionMajor = VDM_STRUCTURED_VER(arg->pdrev);
        rspvdo->SVDM.HEADER.VDMType     = 1;
        rspvdo->SVDM.HEADER.ObjPos      = 0;
        rspvdo->SVDM.HEADER.CommandType = VDM_CMD_TYPE_ACK;
        rspvdo->SVDM.HEADER.Command     = VDM_CMD_DISCOVER_MODES;
        rspvdo->SVDM.HEADER.VersionMinor = VDM_STRUCTURED_MAX_VER_MINOR(arg->pdrev);
        rspvdo++;
        arg->buf.count++;
        rspvdo->object = state_obj->cap.object;
        arg->buf.count++;
        arg->tx_msg = true;
    }
    else if (header.SVDM.HEADER.CommandType == VDM_CMD_TYPE_ACK)
    {
        state_obj->partner.cap_match = false;
        state_obj->partner.mode_idx  = 0;
        for (i = 1; i < vdo_count; i++)
        {
            if (vdm_dp_evaluate_partner_cap(arg->buf.vdo[i], state_obj->cap.object))
            {
                state_obj->partner.cap_match  = true;
                state_obj->partner.mode_idx   = i;
                state_obj->partner.cap.object = arg->buf.vdo[i];
            }
        }
        if (state_obj->partner.cap_match)
        {
            state_obj->state = STATE_DP_ENTER_MODE;
        }
        else
        {
            state_obj->enabled = false;
        }
    }
    else
    {
        state_obj->enabled = false;
    }
}

/**
 * @brief Handle partner enter mode response
 * @param arg response to enter mode command
 */
static void vdm_dp_process_partner_enter_mode(struct vdm_sm_arg_t *arg)
{
    struct vdm_state *pState = &DP_VDM_STATE[arg->dev->port_tcpd_id];
    struct pd_msg_t  *rspvdo = (struct pd_msg_t *)arg->buf.vdo;
    struct pd_msg_t   header = {0};
    header.object            = arg->buf.vdo[0];
    arg->buf.count           = 0;
    if (header.SVDM.HEADER.CommandType == VDM_CMD_TYPE_REQ)
    {
        rspvdo->object              = 0;
        rspvdo->SVDM.HEADER.SVID    = SVID_VID_DP;
        rspvdo->SVDM.HEADER.VersionMajor = VDM_STRUCTURED_VER(arg->pdrev);
        rspvdo->SVDM.HEADER.VDMType = 1;
        rspvdo->SVDM.HEADER.ObjPos  = header.SVDM.HEADER.ObjPos;
        if (header.SVDM.HEADER.ObjPos == 1)
        {
            rspvdo->SVDM.HEADER.CommandType = VDM_CMD_TYPE_ACK;
            pState->modeen                  = true;
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
        pState->state          = STATE_DP_STATUS;
        pState->partner.modeen = true;
    }
    else
    {
        pState->enabled = false;
    }
}

/**
 * @brief Handle partner exit mode response
 * @param arg Partner exit mode response
 */
static void vdm_dp_process_partner_exit_mode(struct vdm_sm_arg_t *arg)
{
    struct vdm_state *pState = &DP_VDM_STATE[arg->dev->port_tcpd_id];
    struct pd_msg_t  *rspvdo = (struct pd_msg_t *)arg->buf.vdo;
    struct pd_msg_t   header = {0};
    header.object            = arg->buf.vdo[0];
    arg->buf.count           = 0;
    if (header.SVDM.HEADER.CommandType == VDM_CMD_TYPE_REQ)
    {
        rspvdo->object              = 0;
        rspvdo->SVDM.HEADER.SVID    = SVID_VID_DP;
        rspvdo->SVDM.HEADER.VersionMajor = VDM_STRUCTURED_VER(arg->pdrev);
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
        pState->state          = STATE_DISABLED;
        pState->partner.modeen = false;
        pState->enabled        = false;
    }
    else
    {
        pState->enabled = false;
    }
}

/**
 * @brief Change the current display port mode for port
 * @param VDM driver object
 * @param mode Mode to enable
 */
static void vdm_dp_change_mode(struct vdm_state *vdm, enum dp_mode_t mode)
{
    if (mode == DP_MODE_DFP)
    {
        vdm->status.DfpConnected = 1;
        vdm->status.UfPConnected = 0;
    }
    if (mode == DP_MODE_UFP)
    {
        vdm->status.DfpConnected = 0;
        vdm->status.UfPConnected = 1;
    }
    if (mode == DP_MODE_BOTH)
    {
        vdm->status.DfpConnected = 1;
        vdm->status.UfPConnected = 1;
    }
    if (mode == DP_MODE_USB)
    {
        /* Just enable usb no DP */
        vdm->status.DfpConnected = 0;
        vdm->status.UfPConnected = 0;
    }
}

/**
 * @brief function checks the port and partner capabilities and current status
 * to pick a configuration the the partner can support.
 * @param port_cap  Capabilities of port.
 * @param partner_cap Capabilities of partner.
 * @param port_status Status port port.
 * @param partner_status Status of partner.
 * @return
 */
static struct dp_config_t vdm_dp_select_config(uint32_t port_cap, uint32_t partner_cap,
                                               uint32_t port_status, uint32_t partner_status)
{
    uint32_t pin    = 0;
    uint32_t signal = 0;

    struct dp_config_t part_config  = {0};
    struct dp_stat_t   dp_port_stat = {0};
    struct dp_stat_t   dp_part_stat = {0};
    struct dp_cap_t    dp_port_cap  = {0};
    struct dp_cap_t    dp_part_cap  = {0};

    dp_port_cap.object  = port_cap;
    dp_part_cap.object  = partner_cap;
    dp_port_stat.object = port_status;
    dp_part_stat.object = partner_status;
    part_config.object  = 0;

    signal = dp_port_cap.Signaling & dp_part_cap.Signaling;

    /* Pick signaling scheme */
    if (signal & DP_SIGNALING_DP1p3)
    {
        part_config.Signaling = DP_SIGNALING_DP1p3;
    }
    else if (signal & DP_SIGNALING_USBGen2)
    {
        part_config.Signaling = DP_SIGNALING_USBGen2;
    }

    /* Pick a mode */
    if (dp_port_cap.DfpCapable & dp_part_cap.UfpCapable & dp_port_stat.DfpConnected
        & dp_part_stat.UfPConnected)
    {
        part_config.Ufp = 1;
        if (dp_part_cap.Receptacle == 0)
        {
            pin = dp_port_cap.DfpPinAssignment & dp_part_cap.DfpPinAssignment;
        }
        else
        {
            pin = dp_port_cap.DfpPinAssignment & dp_part_cap.UfpPinAssignment;
        }
    }
    else if (dp_port_cap.UfpCapable & dp_part_cap.DfpCapable & dp_port_stat.UfPConnected
             & dp_part_stat.DfpConnected)
    {
        part_config.Dfp = 1;
        if (dp_part_cap.Receptacle == 0)
        {
            pin = dp_port_cap.UfpPinAssignment & dp_part_cap.UfpPinAssignment;
        }
        else
        {
            pin = dp_port_cap.UfpPinAssignment & dp_part_cap.DfpPinAssignment;
        }
    }

    /* If pin is not matched the USB mode only */
    if (pin & DP_PIN_ASSIGN_A)
    {
        part_config.PinAssign = DP_PIN_ASSIGN_A;
    }
    else if (pin & DP_PIN_ASSIGN_B)
    {
        part_config.PinAssign = DP_PIN_ASSIGN_B;
    }
    else if (pin & DP_PIN_ASSIGN_C)
    {
        part_config.PinAssign = DP_PIN_ASSIGN_C;
    }
    else if (pin & DP_PIN_ASSIGN_D)
    {
        part_config.PinAssign = DP_PIN_ASSIGN_D;
    }
    else if (pin & DP_PIN_ASSIGN_E)
    {
        part_config.PinAssign = DP_PIN_ASSIGN_E;
    }
    else if (pin & DP_PIN_ASSIGN_F)
    {
        part_config.PinAssign = DP_PIN_ASSIGN_F;
    }

    return part_config;
}

/**
 * @brief Handle status update from partner.
 * @param arg Partner status update.
 */
static void vdm_dp_process_partner_status(struct vdm_sm_arg_t *arg)
{
    struct vdm_state *pState = &DP_VDM_STATE[arg->dev->port_tcpd_id];
    struct pd_msg_t  *rspvdo = (struct pd_msg_t *)arg->buf.vdo;
    struct pd_msg_t   header = {0};
    header.object            = arg->buf.vdo[0];
    arg->buf.count           = 0;
    if (header.SVDM.HEADER.CommandType == VDM_CMD_TYPE_REQ)
    {
        rspvdo->object              = 0;
        rspvdo->SVDM.HEADER.SVID    = SVID_VID_DP;
        rspvdo->SVDM.HEADER.VersionMajor = VDM_STRUCTURED_VER(arg->pdrev);
        rspvdo->SVDM.HEADER.VersionMinor = VDM_MAX_VERSION_MINOR[arg->dev->port_tcpd_id];
        ;
        rspvdo->SVDM.HEADER.VDMType     = 1;
        rspvdo->SVDM.HEADER.ObjPos      = 0;
        rspvdo->SVDM.HEADER.CommandType = VDM_CMD_TYPE_ACK;
        rspvdo->SVDM.HEADER.Command     = VDM_CMD_DP_STATUS;
        rspvdo++;
        arg->buf.count++;

        rspvdo->object = pState->status.object;
        arg->buf.count++;
        arg->tx_msg = true;
    }
    else if (header.SVDM.HEADER.CommandType == VDM_CMD_TYPE_ACK)
    {
        pState->partner.status.object = arg->buf.vdo[1];
        if (pState->status.UfPConnected && pState->status.DfpConnected
            && pState->partner.status.UfPConnected && pState->partner.status.DfpConnected)
        {
            vdm_dp_change_mode(pState, DP_MODE_UFP);
        }
        else
        {
            if (pState->partner.status.UfPConnected || pState->partner.status.DfpConnected)
            {
                pState->partner.config =
                    vdm_dp_select_config(pState->cap.object, pState->partner.cap.object,
                                         pState->status.object, pState->partner.status.object);
                pState->state = STATE_DP_CONFIG;
            }
            else
            {
                pState->enabled = false;
            }
        }
    }
}

/**
 * @brief Handle DP Config response from port partner.
 * @param arg DP Config response.
 */
static void vdm_dp_process_partner_config(struct vdm_sm_arg_t *arg)
{
    struct vdm_state *pState = &DP_VDM_STATE[arg->dev->port_tcpd_id];
    struct pd_msg_t  *vdo    = (struct pd_msg_t *)arg->buf.vdo;
    struct pd_msg_t   header = {0};
    header.object            = arg->buf.vdo[0];
    arg->buf.count           = 0;
    if (header.SVDM.HEADER.CommandType == VDM_CMD_TYPE_REQ)
    {
        vdo->object                  = 0;
        vdo->SVDM.HEADER.SVID        = SVID_VID_DP;
        vdo->SVDM.HEADER.VersionMajor = VDM_STRUCTURED_VER(arg->pdrev);
        vdo->SVDM.HEADER.VersionMinor = VDM_MAX_VERSION_MINOR[arg->dev->port_tcpd_id];
        vdo->SVDM.HEADER.VDMType     = 1;
        vdo->SVDM.HEADER.ObjPos      = 0;
        vdo->SVDM.HEADER.CommandType = VDM_CMD_TYPE_ACK;
        vdo->SVDM.HEADER.Command     = VDM_CMD_DP_CONFIG;
        vdo++;
        arg->buf.count++;

        vdo->object = pState->partner.config.object;
        arg->buf.count++;
        pState->dpen = true;
        arg->tx_msg  = true;
        /* TODO Enable dp */
    }
    else if (header.SVDM.HEADER.CommandType == VDM_CMD_TYPE_ACK)
    {
        /* TODO enable display port */
        pState->dpen    = true;
        pState->enabled = false;
    }
}

/**
 * @brief Handle Attention message for DP from port partner.
 * @param arg DP Attention message
 */
static void vdm_dp_process_partner_attention(struct vdm_sm_arg_t *arg)
{
    struct vdm_state *pState = &DP_VDM_STATE[arg->dev->port_tcpd_id];
    struct pd_msg_t   header = {0};
    header.object            = arg->buf.vdo[0];

    arg->buf.count = 0;
    if (header.SVDM.HEADER.CommandType == VDM_CMD_TYPE_REQ)
    {
        if (pState->state == STATE_DP_STATUS || pState->state == STATE_DP_CONFIG)
        {
            pState->enabled = true;
        }
        else
        {
            /* TODO Process HPD and IRQ changes  */
        }
    }
    else if (header.SVDM.HEADER.CommandType == VDM_CMD_TYPE_ACK)
    {
    }
}

/**
 * @brief Default handler for message that are not valid
 * @param arg Contains buffer to write NAK message
 */
static void vdm_dp_partner_nak(struct vdm_sm_arg_t *arg)
{
    struct pd_msg_t *vdo    = (struct pd_msg_t *)arg->buf.vdo;
    struct pd_msg_t  header = {0};
    header.object           = arg->buf.vdo[0];
    if (header.SVDM.HEADER.CommandType == VDM_CMD_TYPE_REQ)
    {
        vdo->object                  = 0;
        vdo->SVDM.HEADER.SVID        = SVID_VID_DP;
        vdo->SVDM.HEADER.VersionMajor = VDM_STRUCTURED_VER(arg->pdrev);
        vdo->SVDM.HEADER.VDMType     = 1;
        vdo->SVDM.HEADER.ObjPos      = header.SVDM.HEADER.ObjPos;
        vdo->SVDM.HEADER.CommandType = VDM_CMD_TYPE_NAK;
        vdo->SVDM.HEADER.Command     = header.SVDM.HEADER.Command;
        if (vdo->SVDM.HEADER.VersionMinor == vdo->SVDM.HEADER.Command <=15)
        {
        	vdo->SVDM.HEADER.VersionMinor = VDM_STRUCTURED_MAX_VER_MINOR(arg->pdrev);
        }
        arg->buf.count               = 1;
        arg->tx_msg                  = true;
    }
}

/**
 * @brief Handle incoming messages from port partner
 * @param arg Argument contains messages and data
 */
static void vdm_dp_process_partner(struct vdm_sm_arg_t *arg)
{
    struct pd_msg_t header = {0};
    header.object          = arg->buf.vdo[0];
    uint8_t cmd            = header.SVDM.HEADER.Command;

    if (arg->vif->Modal_Operation_Supported_SOP)
    {
        if (cmd == VDM_CMD_DISCOVER_MODES)
        {
            vdm_dp_process_partner_mode(arg);
        }
        else if (cmd == VDM_CMD_ENTER_MODE)
        {
            vdm_dp_process_partner_enter_mode(arg);
        }
        else if (cmd == VDM_CMD_EXIT_MODE)
        {
            vdm_dp_process_partner_exit_mode(arg);
        }
        else if (cmd == VDM_CMD_DP_STATUS)
        {
            vdm_dp_process_partner_status(arg);
        }
        else if (cmd == VDM_CMD_DP_CONFIG)
        {
            vdm_dp_process_partner_config(arg);
        }
        else if (cmd == VDM_CMD_ATTENTION)
        {
            vdm_dp_process_partner_attention(arg);
        }
        else
        {
            vdm_dp_partner_nak(arg);
        }
    }
    else
    {
        if (cmd != VDM_CMD_ATTENTION)
        {
            vdm_dp_partner_nak(arg);
        }
        else
        {
            /* Ignore */
            arg->buf.count = 0;
        }
    }
}

/**
 * @brief Create DP Request mode message
 * @param arg
 */
static void vdm_dp_request_modes(struct vdm_sm_arg_t *arg)
{
    struct pd_msg_t vdm = {0};
    ;
    vdm.object                  = 0;
    vdm.SVDM.HEADER.SVID        = SVID_VID_DP;
    vdm.SVDM.HEADER.VDMType     = VDM_STRUCTURED;
    vdm.SVDM.HEADER.VersionMajor = VDM_STRUCTURED_VER(arg->pdrev);
    vdm.SVDM.HEADER.ObjPos      = 0;
    vdm.SVDM.HEADER.CommandType = VDM_CMD_TYPE_REQ;
    vdm.SVDM.HEADER.Command     = VDM_CMD_DISCOVER_MODES;
    vdm.SVDM.HEADER.VersionMinor = VDM_STRUCTURED_MAX_VER_MINOR(arg->pdrev);
    arg->buf.vdo[0]             = vdm.object;
    arg->buf.count              = 1;
    arg->tx_msg                 = true;
}

/**
 * @brief Create DP Enter mode request message
 * @param arg Contains buffer for output message
 */
static void vdm_dp_request_enter_mode(struct vdm_sm_arg_t *arg)
{
    struct pd_msg_t vdm = {0};
    ;
    vdm.object                  = 0;
    vdm.SVDM.HEADER.SVID        = SVID_VID_DP;
    vdm.SVDM.HEADER.VDMType     = VDM_STRUCTURED;
    vdm.SVDM.HEADER.VersionMajor = VDM_STRUCTURED_VER(arg->pdrev);
    vdm.SVDM.HEADER.ObjPos      = DP_VDM_STATE[arg->dev->port_tcpd_id].partner.mode_idx;
    vdm.SVDM.HEADER.CommandType = VDM_CMD_TYPE_REQ;
    vdm.SVDM.HEADER.Command     = VDM_CMD_ENTER_MODE;
    vdm.SVDM.HEADER.VersionMinor = VDM_STRUCTURED_MAX_VER_MINOR(arg->pdrev);
    arg->buf.vdo[0]             = vdm.object;
    arg->buf.count              = 1;
    arg->tx_msg                 = true;
}

/**
 * @brief Create DP Exit mode request message
 * @param arg Contains buffer for output message
 */
static void vdm_dp_request_exit_mode(struct vdm_sm_arg_t *arg)
{
    struct pd_msg_t vdm = {0};
    ;
    vdm.object                  = 0;
    vdm.SVDM.HEADER.SVID        = SVID_VID_DP;
    vdm.SVDM.HEADER.VDMType     = VDM_STRUCTURED;
    vdm.SVDM.HEADER.VersionMajor = VDM_STRUCTURED_VER(arg->pdrev);
    vdm.SVDM.HEADER.ObjPos      = DP_VDM_STATE[arg->dev->port_tcpd_id].partner.mode_idx;
    vdm.SVDM.HEADER.CommandType = VDM_CMD_TYPE_REQ;
    vdm.SVDM.HEADER.Command     = VDM_CMD_EXIT_MODE;
    vdm.SVDM.HEADER.VersionMinor = VDM_STRUCTURED_MAX_VER_MINOR(arg->pdrev);
    arg->buf.vdo[0]             = vdm.object;
    arg->buf.count              = 1;
    arg->tx_msg                 = true;
}

/**
 * @brief Create DP Status request message
 * @param arg Contains buffer for output message
 */
static void vdm_dp_request_status(struct vdm_sm_arg_t *arg)
{
    struct pd_msg_t vdm         = {0};
    vdm.object                  = 0;
    vdm.SVDM.HEADER.SVID        = SVID_VID_DP;
    vdm.SVDM.HEADER.VDMType     = VDM_STRUCTURED;
    vdm.SVDM.HEADER.VersionMajor = VDM_STRUCTURED_VER(arg->pdrev);
    vdm.SVDM.HEADER.VersionMinor = VDM_MAX_VERSION_MINOR[arg->dev->port_tcpd_id];
    vdm.SVDM.HEADER.ObjPos      = DP_VDM_STATE[arg->dev->port_tcpd_id].partner.mode_idx;
    vdm.SVDM.HEADER.CommandType = VDM_CMD_TYPE_REQ;
    vdm.SVDM.HEADER.Command     = VDM_CMD_DP_STATUS;
    arg->buf.vdo[0]             = vdm.object;
    arg->buf.vdo[1]             = DP_VDM_STATE[arg->dev->port_tcpd_id].status.object;
    arg->buf.count              = 2;
    arg->tx_msg                 = true;
}

/**
 * @brief Create DP Config Request Message
 * @param arg Contains buffer for output message
 */
static void vdm_dp_request_config(struct vdm_sm_arg_t *arg)
{
    struct pd_msg_t vdm         = {0};
    vdm.object                  = 0;
    vdm.SVDM.HEADER.SVID        = SVID_VID_DP;
    vdm.SVDM.HEADER.VDMType     = VDM_STRUCTURED;
    vdm.SVDM.HEADER.VersionMajor = VDM_STRUCTURED_VER(arg->pdrev);
    vdm.SVDM.HEADER.VersionMinor = VDM_MAX_VERSION_MINOR[arg->dev->port_tcpd_id];
    vdm.SVDM.HEADER.ObjPos      = DP_VDM_STATE[arg->dev->port_tcpd_id].partner.mode_idx;
    vdm.SVDM.HEADER.CommandType = VDM_CMD_TYPE_REQ;
    vdm.SVDM.HEADER.Command     = VDM_CMD_DP_CONFIG;
    arg->buf.vdo[0]             = vdm.object;
    arg->buf.vdo[1]             = DP_VDM_STATE[arg->dev->port_tcpd_id].partner.config.object;
    arg->buf.count              = 2;
    arg->tx_msg                 = true;
}

/**
 * @brief State Machine entry for VDM driver
 * @param arg Argument to state machine contains message and buffer
 */
static void vdm_dp_sm(struct vdm_sm_arg_t *arg)
{
    struct vdm_state *pState = &DP_VDM_STATE[arg->dev->port_tcpd_id];
    struct port * port = arg->dev->port_tcpd_priv;
    switch (pState->state)
    {
        case STATE_DP_MODES:
            vdm_dp_request_modes(arg);
            port->vdm_tx_svid = SVID_VID_DP;
            break;
        case STATE_DP_ENTER_MODE:
            if (pState->partner.cap_match && pState->partner.mode_idx > 0)
            {
                vdm_dp_request_enter_mode(arg);
                port->vdm_tx_svid = SVID_VID_DP;
            }
            else
            {
                pState->enabled = false;
                pState->state   = STATE_DISABLED;
                port->vdm_tx_svid = 0;
            }
            break;
        case STATE_DP_EXIT_MODE:
            vdm_dp_request_exit_mode(arg);
            port->vdm_tx_svid = SVID_VID_DP;
            break;
        case STATE_DP_STATUS:
            vdm_dp_request_status(arg);
            port->vdm_tx_svid = SVID_VID_DP;
            break;
        case STATE_DP_CONFIG:
            vdm_dp_request_config(arg);
            port->vdm_tx_svid = SVID_VID_DP;
            break;
        default:
            pState->enabled = false;
            pState->state   = STATE_DISABLED;
            port->vdm_tx_svid = 0;
            break;
    }
}

/**
 * @brief Check the active state of the VDM Driver
 * @param portid
 * @return true if active otherwise false
 */
static bool vdm_dp_active(struct port_tcpd const *dev)
{
    return DP_VDM_STATE[dev->port_tcpd_id].enabled;
}

/**
 * @brief Set VDM active. When partner advertises VDM with SVID_VID_DP port VDM
 * driver is activated. Otherwise it is deactivated.
 * @param port Port which detect SVID_VID_DP in partner.
 * @param active Set active state if true
 */
static void vdm_dp_set_active(struct port_tcpd const *dev, bool active)
{
    DP_VDM_STATE[dev->port_tcpd_id].enabled = active;
}

/**
 * DP Driver driver callback handlers
 */
static SVDM_HANDLER_T svdmh = {
    .vdm_process = vdm_dp_process_partner,
    .sm          = vdm_dp_sm,
};

/**
 * Policy VDM driver object
 */
static VDM_OBJ_T dp_vdm_obj = {
    .structured = true,
    .svid       = SVID_VID_DP,
    .svdmh      = &svdmh,
    .active     = vdm_dp_active,
    .set_active = vdm_dp_set_active,
    .reset      = vdm_dp_reset,
	.sopmask = SOP_SOP0 | SOP_SOP1 | SOP_SOP2,
};

/**
 * POLCIY VDM list object
 */
VDM_ITEM_T vdm_dp_driv = {
    .portmask = 0,
    .vdm      = &dp_vdm_obj,
    .next     = 0,
};

/**
 * @brief Function to reset the DP driver state
 * @param port The port VDM driver to reset
 */
static void vdm_dp_reset(struct port_tcpd *const dev, uint16_t state_reset_enum)
{
    struct vdm_state *pState = &DP_VDM_STATE[dev->port_tcpd_id];
    struct port * port = dev->port_tcpd_priv;
    pState->id               = dev->port_tcpd_id;
    if (state_reset_enum == 0)
    {
        pState->state        = STATE_DP_MODES;
    }
    else
    {
        pState->state        = state_reset_enum;
    }
    pState->enabled          = false;
    pState->obj              = &dp_vdm_obj;
    pState->modeen           = false;
    pState->dpen             = false;

    pState->cap.object        = 0;
    pState->cap.DfpCapable    = 1;
    pState->cap.UfpCapable    = 1;
    pState->cap.Signaling     = 1;
    pState->cap.Receptacle    = 1;
    pState->cap.USB2p0NotUsed = 1;
    /* DP spec calls for Dfp and Ufp pins to be swapped if receptacle = 0.
     * i.e Plug describes the capabilities of the Receptacle it supports */
    pState->cap.DfpPinAssignment = DP_PIN_ASSIGN_C | DP_PIN_ASSIGN_D | DP_PIN_ASSIGN_E;
    pState->cap.UfpPinAssignment = DP_PIN_ASSIGN_C | DP_PIN_ASSIGN_D | DP_PIN_ASSIGN_E;

    pState->status.object                 = 0;
    pState->status.DfpConnected           = 1;
    pState->status.UfPConnected           = 1;
    pState->status.PowerLow               = 0;
    pState->status.Enabled                = 1;
    pState->status.MultiFunctionPreferred = 0;
    pState->status.UsbConfigRequest       = 0;
    pState->status.ExitDpModeRequest      = 0;
    pState->status.HpdState               = 0;
    pState->status.IrqHpd                 = 0;

    pState->partner.cap_match = false;
    pState->partner.mode_idx  = 0;
}

#endif /* CONFIG_VDM */
