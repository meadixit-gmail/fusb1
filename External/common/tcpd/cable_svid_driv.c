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
/* Define states */
enum cable_svid_state_t
{
    STATE_DISABLED,
    STATE_CABLE_DISCOVER_SVID,
};

/* Define state data for each state */
struct cable_svid_state
{
    enum cable_svid_state_t   state;
    struct vdm_item_t *obj;
    bool               active;
} CABLE_SVID_STATE[2];

static void vdm_cable_svid_reset(struct port_tcpd *const dev, uint16_t state_reset_enum);

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

static void vdm_cable_request_discv_svid(struct vdm_sm_arg_t *arg)
{
    struct pd_msg_t vdm         = {0};
    struct port * port = arg->dev->port_tcpd_priv;
    port->disc_svid_sent_sopx |= (1U << SOP_SOP1);
    vdm.object                  = 0;
    vdm.SVDM.HEADER.SVID        = SID;
    vdm.SVDM.HEADER.VDMType     = VDM_STRUCTURED;
    vdm.SVDM.HEADER.VersionMajor = VDM_STRUCTURED_VER(arg->pdrev);
    vdm.SVDM.HEADER.ObjPos      = 0;
    vdm.SVDM.HEADER.CommandType = VDM_CMD_TYPE_REQ;
    vdm.SVDM.HEADER.Command     = VDM_CMD_DISCOVER_SVIDS;
    vdm.SVDM.HEADER.VersionMinor = VDM_STRUCTURED_MAX_VER_MINOR(arg->pdrev);
    arg->buf.vdo[0]             = vdm.object;
    arg->buf.count              = 1;
    arg->sop                    = SOP_SOP1;
    arg->tx_msg                 = true;
}

static void vdm_cable_process_partner_discv_svid(struct vdm_sm_arg_t *arg)
{
    struct vdm_item_t *p         = CABLE_SVID_STATE[arg->dev->port_tcpd_id].obj->next;
    struct port * port = arg->dev->port_tcpd_priv;
    struct pd_msg_t    vdo       = {0};
    int                i         = 0;
    unsigned long      portmask  = 0x1U << arg->dev->port_tcpd_id;
    struct pd_msg_t    header    = {0};
    int                vdo_count = arg->buf.count;
    header.object                = arg->buf.vdo[0];
    arg->buf.count               = 0;
    if (header.SVDM.HEADER.CommandType == VDM_CMD_TYPE_ACK)
    {
        if (arg->sop == SOP_SOP1)
        {
			while (p)
			{
				if (p->portmask & portmask)
				{
					for (i = 1; i < vdo_count; i++)
					{
						vdo.object = arg->buf.vdo[i];
						/* Check lower 16 bit */
						/* If driver activates, re-enable VDM auto state */
						if (vdo.SVDM.SVIDO.svid_low == p->vdm->svid && (p->vdm->sopmask & (1U << SOP_SOP1)))
						{
							p->vdm->set_active(arg->dev, true);
							port->vdm_auto_state = true;
						}
						if (vdo.SVDM.SVIDO.svid_high == p->vdm->svid && (p->vdm->sopmask & (1U << SOP_SOP1)))
						{
							p->vdm->set_active(arg->dev, true);
							port->vdm_auto_state = true;
						}
					}
				}
				if (p->next == CABLE_SVID_STATE[arg->dev->port_tcpd_id].obj)
				{
					break;
				}
				else
				{
					p = p->next;
				}
			}
			if (i == 7 && vdo.SVDM.SVIDO.svid_low != 0)
			{
				/* Not all SVIDs received yet */
			}
			else
			{
				CABLE_SVID_STATE[arg->dev->port_tcpd_id].state  = STATE_DISABLED;
				CABLE_SVID_STATE[arg->dev->port_tcpd_id].active = false;
			}
	}

    }
    else
    {
        if (arg->sop == SOP_SOP1)
        {
        	CABLE_SVID_STATE[arg->dev->port_tcpd_id].active = false;
        	CABLE_SVID_STATE[arg->dev->port_tcpd_id].state  = STATE_DISABLED;
        }
    }
}

static void vdm_cable_svid_sm(struct vdm_sm_arg_t *arg)
{
    struct cable_svid_state *vdm = &CABLE_SVID_STATE[arg->dev->port_tcpd_id];
    struct port * port = arg->dev->port_tcpd_priv;
    switch (vdm->state)
    {
        case STATE_CABLE_DISCOVER_SVID:
            vdm_cable_request_discv_svid(arg);
            port->vdm_tx_svid = SID;
            break;
        default:
            vdm->state  = STATE_DISABLED;
            vdm->active = false;
            port->vdm_tx_svid = 0;
            break;
    }
}

static bool vdm_cable_svid_active(struct port_tcpd const *dev)
{
    return CABLE_SVID_STATE[dev->port_tcpd_id].active;
}

static void vdm_cable_svid_set_active(struct port_tcpd const *dev, bool active)
{
    CABLE_SVID_STATE[dev->port_tcpd_id].active = active;
}

static void vdm_cable_svid_process_partner(struct vdm_sm_arg_t *arg)
{
    uint32_t         i          = 0;
    struct pd_msg_t *rspvdo     = (struct pd_msg_t *)arg->buf.vdo;
    struct pd_msg_t  header     = {0};
    header.object               = arg->buf.vdo[0];
    struct cable_svid_state *state_obj = &CABLE_SVID_STATE[arg->dev->port_tcpd_id];
    uint32_t          vdo_count = arg->buf.count;
    uint8_t cmd            = header.SVDM.HEADER.Command;
	if (cmd == VDM_CMD_DISCOVER_SVIDS)
	{
		vdm_cable_process_partner_discv_svid(arg);
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
    .vdm_process = vdm_cable_svid_process_partner,
    .sm          = vdm_cable_svid_sm,
};

static const struct vdm_obj_t pd_vdm_cable_svid_obj = {
    .structured = true,
    .svid       = SID,
    .svdmh      = &svdmh,
    .active     = vdm_cable_svid_active,
    .set_active = vdm_cable_svid_set_active,
    .reset      = vdm_cable_svid_reset,
	.sopmask = (1U << SOP_SOP1),
};

struct vdm_item_t vdm_cable_svid_driv = {
    .portmask = 0,
    .vdm      = &pd_vdm_cable_svid_obj,
    .next     = 0,
};

static void vdm_cable_svid_reset(struct port_tcpd *const dev, uint16_t state_reset_enum)
{
    struct cable_svid_state *vdm = &CABLE_SVID_STATE[dev->port_tcpd_id];
    vdm->active           = false;
    if (state_reset_enum == 0)
    {
        vdm->state        = STATE_CABLE_DISCOVER_SVID;
    }
    else
    {
        vdm->state        = state_reset_enum;
    }
    vdm->obj              = &vdm_cable_svid_driv;
}

#endif /* CONFIG_VDM */
