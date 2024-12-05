/*******************************************************************************
 * @file     svid_driv.c
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
#include "vdm.h"
#include "vif_types.h"
#include "port_tcpd.h"

#if CONFIG_ENABLED(VDM) && !CONFIG_ENABLED(VDM_MINIMAL)
#define SID 0xFF00

#define TBT3SVID 0x8087
#define TBT3ALTMODEID 0x0001
/* Define states */
enum cable_tbt3_state_t
{
    STATE_DISABLED,
    STATE_CABLE_DISCOVER_MODES,
};

/* Define state data for each state */
struct cable_tbt3_state
{
    enum cable_tbt3_state_t   state;
    struct vdm_item_t *obj;
    bool               active;
} CABLE_TBT3_STATE[2];

static void vdm_cable_tbt3_reset(struct port_tcpd *const dev, uint16_t state_reset_enum);

static void vdm_cable_partner_nak(struct vdm_sm_arg_t *arg)
{
    struct pd_msg_t  header = {0};
    struct pd_msg_t *vdo    = (struct pd_msg_t *)arg->buf.vdo;
    header.object           = arg->buf.vdo[0];

    if (header.SVDM.HEADER.CommandType == VDM_CMD_TYPE_REQ)
    {
        vdo->object                  = 0;
        vdo->SVDM.HEADER.SVID        = SID;
        vdo->SVDM.HEADER.VersionMajor = VDM_STRUCTURED_VER(arg->pdrev);
        vdo->SVDM.HEADER.VDMType     = 1;
        vdo->SVDM.HEADER.ObjPos      = header.SVDM.HEADER.ObjPos;
        vdo->SVDM.HEADER.CommandType = VDM_CMD_TYPE_NAK;
        vdo->SVDM.HEADER.Command     = header.SVDM.HEADER.Command;
        vdo->SVDM.HEADER.VersionMinor = VDM_STRUCTURED_MAX_VER_MINOR(arg->pdrev);
        arg->buf.count               = 1;
        arg->tx_msg                  = true;
    }
}

static void vdm_cable_request_discv_modes(struct vdm_sm_arg_t *arg)
{
    struct pd_msg_t vdm         = {0};
    struct port * port = arg->dev->port_tcpd_priv;
    port->disc_modes_sent_sopx |= (1U << SOP_SOP1);
    vdm.object                  = 0;
    vdm.SVDM.HEADER.SVID        = TBT3SVID;
    vdm.SVDM.HEADER.VDMType     = VDM_STRUCTURED;
    vdm.SVDM.HEADER.VersionMajor = VDM_STRUCTURED_VER(arg->pdrev);
    vdm.SVDM.HEADER.ObjPos      = 0;
    vdm.SVDM.HEADER.CommandType = VDM_CMD_TYPE_REQ;
    vdm.SVDM.HEADER.Command     = VDM_CMD_DISCOVER_MODES;
    vdm.SVDM.HEADER.VersionMinor = VDM_STRUCTURED_MAX_VER_MINOR(arg->pdrev);
    arg->buf.vdo[0]             = vdm.object;
    arg->buf.count              = 1;
    arg->sop                    = SOP_SOP1;
    arg->tx_msg                 = true;
}

static void vdm_cable_process_partner_discv_mode(struct vdm_sm_arg_t *arg)
{
    uint32_t         i          = 0;
    struct pd_msg_t *rspvdo     = (struct pd_msg_t *)arg->buf.vdo;
    struct pd_msg_t  header     = {0};
    struct port * port = arg->dev->port_tcpd_priv;
    port->disc_modes_received_sopx |= (1U << SOP_SOP1);
    header.object               = arg->buf.vdo[0];
    struct cable_tbt3_state *state_obj = &CABLE_TBT3_STATE[arg->dev->port_tcpd_id];
    uint32_t          vdo_count = arg->buf.count;
    uint8_t cmd            = header.SVDM.HEADER.Command;
	if (cmd == VDM_CMD_DISCOVER_MODES)
	{
		if (header.SVDM.HEADER.CommandType == VDM_CMD_TYPE_ACK)
		{
			/* make sure valid disc. mode response */
			for (uint8_t i = 1; i < arg->buf.count; i++)
			{
				/* Alt mode ID must be correct */
				if (rspvdo[i].SVDM.TBT3MODEVDO.AlternateMode == TBT3ALTMODEID)
				{
					/* Now alt mode ID is correct, correct speed for USB4 purposes */
					port->tbt3vdo.object = rspvdo[i].object;
					if (port->cable_vdo_cable_highest_usb_speed < port->tbt3vdo.SVDM.TBT3MODEVDO.CableSpeed &&
						port->cable_vdo_cable_type_ufp == VDM_ID_UFP_SOP_1_PASSIVE_CABLE &&
						port->cable_vdo_cable_highest_usb_speed == 2)
					{
						port->cable_vdo_cable_highest_usb_speed = port->tbt3vdo.SVDM.TBT3MODEVDO.CableSpeed;
					}

				}
			}
		}
	}
	CABLE_TBT3_STATE[arg->dev->port_tcpd_id].state  = STATE_DISABLED;
	CABLE_TBT3_STATE[arg->dev->port_tcpd_id].active  = false;
}

static void vdm_cable_tbt3_sm(struct vdm_sm_arg_t *arg)
{
    struct cable_tbt3_state *vdm = &CABLE_TBT3_STATE[arg->dev->port_tcpd_id];
    struct port * port = arg->dev->port_tcpd_priv;
    switch (vdm->state)
    {
        case STATE_CABLE_DISCOVER_MODES:
            vdm_cable_request_discv_modes(arg);
            port->vdm_tx_svid = TBT3SVID;
            break;
        default:
            vdm->state  = STATE_DISABLED;
            vdm->active = false;
            port->vdm_tx_svid = 0;
            break;
    }
}

static bool vdm_cable_tbt3_active(struct port_tcpd const *dev)
{
    return CABLE_TBT3_STATE[dev->port_tcpd_id].active;
}

static void vdm_cable_tbt3_set_active(struct port_tcpd const *dev, bool active)
{
    CABLE_TBT3_STATE[dev->port_tcpd_id].active = active;
}

static void vdm_cable_tbt3_process_partner(struct vdm_sm_arg_t *arg)
{
    uint32_t         i          = 0;
    struct pd_msg_t *rspvdo     = (struct pd_msg_t *)arg->buf.vdo;
    struct pd_msg_t  header     = {0};
    header.object               = arg->buf.vdo[0];
    struct cable_tbt3_state *state_obj = &CABLE_TBT3_STATE[arg->dev->port_tcpd_id];
    uint32_t          vdo_count = arg->buf.count;
    uint8_t cmd            = header.SVDM.HEADER.Command;
    if (cmd == VDM_CMD_DISCOVER_MODES)
	{
		vdm_cable_process_partner_discv_mode(arg);
	}
	else if (cmd == VDM_CMD_ATTENTION)
	{
		//ignore
	}
	else
	{
		vdm_cable_partner_nak(arg);
	}
}

static const struct svdm_handler_t svdmh = {
    .vdm_process = vdm_cable_tbt3_process_partner,
    .sm          = vdm_cable_tbt3_sm,
};

static const struct vdm_obj_t pd_vdm_cable_tbt3_obj = {
    .structured = true,
    .svid       = TBT3SVID,
    .svdmh      = &svdmh,
    .active     = vdm_cable_tbt3_active,
    .set_active = vdm_cable_tbt3_set_active,
    .reset      = vdm_cable_tbt3_reset,
	.sopmask = (1U << SOP_SOP1),
};

struct vdm_item_t vdm_cable_tbt3_driv = {
    .portmask = 0,
    .vdm      = &pd_vdm_cable_tbt3_obj,
    .next     = 0,
};

static void vdm_cable_tbt3_reset(struct port_tcpd *const dev, uint16_t state_reset_enum)
{
    struct cable_tbt3_state *vdm = &CABLE_TBT3_STATE[dev->port_tcpd_id];
    vdm->active           = false;
    if (state_reset_enum == 0)
    {
        vdm->state        = STATE_CABLE_DISCOVER_MODES;
    }
    else
    {
        vdm->state        = state_reset_enum;
    }
    vdm->obj              = &vdm_cable_tbt3_driv;
}

#endif /* CONFIG_VDM */
