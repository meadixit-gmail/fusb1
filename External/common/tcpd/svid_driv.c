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

/* Define states */
enum vdm_state_t
{
    STATE_DISABLED,
    STATE_DISCOVER_ID,
    STATE_DISCOVER_SVID,
};

/* Define state data for each state */
struct vdm_state
{
    enum vdm_state_t   state;
    struct vdm_item_t *obj;
    bool               active;
} VDM_STATE[2];

static void vdm_pd_reset(struct port_tcpd *const dev, uint16_t state_reset_enum);

static void vdm_pd_process_partner_discv_id(struct vdm_sm_arg_t *arg)
{
    struct pd_msg_t *rspvdo           = (struct pd_msg_t *)arg->buf.vdo;
    struct pd_msg_t  header           = {0};
    uint32_t         product_type_dfp = 0;
    uint32_t         product_type_ufp = 0;
    header.object                     = arg->buf.vdo[0];
    struct port * port = arg->dev->port_tcpd_priv;
    arg->buf.count                    = 0;
    if (header.SVDM.HEADER.CommandType == VDM_CMD_TYPE_REQ)
    {
        if (arg->sop == SOP_SOP0)
        {
            rspvdo->object                  = 0;
            rspvdo->SVDM.HEADER.SVID        = SID;
            rspvdo->SVDM.HEADER.VersionMajor = VDM_STRUCTURED_VER(arg->pdrev);
            rspvdo->SVDM.HEADER.VDMType     = 1;
            rspvdo->SVDM.HEADER.ObjPos      = 0;
            rspvdo->SVDM.HEADER.CommandType = VDM_CMD_TYPE_ACK;
            rspvdo->SVDM.HEADER.Command     = VDM_CMD_DISCOVER_IDENTITY;
            rspvdo->SVDM.HEADER.VersionMinor = VDM_STRUCTURED_MAX_VER_MINOR(arg->pdrev);
            rspvdo++;
            arg->buf.count++;

            /* ID Header VDO */
            rspvdo->object              = 0;
            rspvdo->SVDM.ID.usb_vid     = arg->vif->USB_VID_SOP;
            rspvdo->SVDM.ID.type_dfp    = arg->vif->Product_Type_DFP_SOP;
            rspvdo->SVDM.ID.modal_op    = arg->vif->Modal_Operation_Supported_SOP;
            rspvdo->SVDM.ID.conn_type   = arg->vif->ID_Header_Connector_Type;
            rspvdo->SVDM.ID.type_ufp    = arg->vif->Product_Type_UFP_SOP;
            rspvdo->SVDM.ID.device_data = arg->vif->Data_Capable_as_USB_Device_SOP;
            rspvdo->SVDM.ID.host_data   = arg->vif->Data_Capable_as_USB_Host_SOP;
            if (arg->pdrev < PD_REV3)
            {
                /* Reserved fields before PD_REV3 */
                if (rspvdo->SVDM.ID.type_ufp == VDM_ID_UFP_SOP_PSD
                    || rspvdo->SVDM.ID.type_ufp == VDM_ID_UFP_SOP_1_VPD)
                {
                    rspvdo->SVDM.ID.type_ufp = 0;
                }
                rspvdo->SVDM.ID.type_dfp  = 0;
                rspvdo->SVDM.ID.conn_type = 0;
            }
            product_type_dfp = rspvdo->SVDM.ID.type_dfp;
            product_type_ufp = rspvdo->SVDM.ID.type_ufp;
            rspvdo++;
            arg->buf.count++;

            /* XID */
            rspvdo->object            = 0;
            rspvdo->SVDM.CERTSTAT.xid = arg->vif->XID_SOP;
            rspvdo++;
            arg->buf.count++;

            /* Product */
            rspvdo->object                   = 0;
            rspvdo->SVDM.PROD.usb_product_id = arg->vif->PID_SOP;
            rspvdo->SVDM.PROD.bcd_device     = arg->vif->bcdDevice_SOP;
            rspvdo++;
            arg->buf.count++;
            arg->tx_msg = true;

            if (arg->pdrev > PD_REV2)
            {
                /* VDOs sent by UFP product types */
                switch (product_type_ufp)
                {
                    case VDM_ID_UFP_SOP_HUB:
                    case VDM_ID_UFP_SOP_PERIPHERAL:
                        /* UFP Hub or UFP Peripheral - UFP VDO */
                        rspvdo->object                           = 0;
                        rspvdo->SVDM.UFPVDO.ufp_vdo_version      = UFP_VDO_VERSION_1_3;
                        rspvdo->SVDM.UFPVDO.usb_highest_speed    = arg->vif->UFPVDO_USB_Highest_Speed;
                        rspvdo->SVDM.UFPVDO.alt_mode_tbt3        = 0;
                        rspvdo->SVDM.UFPVDO.alt_mode_no_reconfig = 0;
                        rspvdo->SVDM.UFPVDO.alt_mode_reconfig = arg->vif->DisplayPort_Enabled;

                        if (!rspvdo->SVDM.UFPVDO.alt_mode_tbt3
                            && !rspvdo->SVDM.UFPVDO.alt_mode_no_reconfig
                            && !rspvdo->SVDM.UFPVDO.alt_mode_reconfig)
                        {
                            rspvdo->SVDM.UFPVDO.vbus_required  = 1;
                            rspvdo->SVDM.UFPVDO.vconn_required = 0;
                            rspvdo->SVDM.UFPVDO.vconn_power    = 0;
                        }
                        else if (rspvdo->SVDM.UFPVDO.alt_mode_reconfig)
                        {
                            rspvdo->SVDM.UFPVDO.vbus_required  = 0;
                            rspvdo->SVDM.UFPVDO.vconn_required = 1;
                            rspvdo->SVDM.UFPVDO.vconn_power    = 1;
                        }

                        if (rspvdo->SVDM.UFPVDO.usb_highest_speed >= 2)
						{
							rspvdo->SVDM.UFPVDO.device_cap_usb_4 = 1;
						}
                        if (rspvdo->SVDM.UFPVDO.usb_highest_speed >= 1)
						{
							rspvdo->SVDM.UFPVDO.device_cap_usb_3_2 = 1;
						}
                        if (rspvdo->SVDM.UFPVDO.usb_highest_speed >= 0)
                        {
                            rspvdo->SVDM.UFPVDO.device_cap_usb_2 = 1;
                        }
                        rspvdo->SVDM.UFPVDO.device_cap_usb_billboard = 0;
                        rspvdo++;
                        arg->buf.count++;
                        break;
                    case VDM_ID_UFP_SOP_PSD:
                    	/* UFP PSD implementation (if needed) */
                    	break;
                    case VDM_ID_UFP_AMA:
                        /* UFP AMA implementation (if needed) */
                        break;
                    default:
                        /* Undefined UFP (0), Active Cable (4), UFP VDP (6)
					 * do not return product VDOs */
                        break;
                }
                /* IF DRD make sure to put space between UFP and DFP vdo as required */
                if (product_type_dfp && product_type_ufp && product_type_ufp != VDM_ID_UFP_SOP_PSD)
                {
                	rspvdo->object = 0;
                	rspvdo++;
                	arg->buf.count++;
                }

                /* VDOs sent by DFP product types */
                switch (product_type_dfp)
                {
                    case VDM_ID_DFP_HUB:
                    case VDM_ID_DFP_HOST:
                    case VDM_ID_DFP_POWER_BRICK:
                        rspvdo->object                      = 0;
                        rspvdo->SVDM.DFPVDO.dfp_vdo_version = DFP_VDO_VERSION_1_2;
                        rspvdo->SVDM.DFPVDO.port_number     = arg->dev->port_tcpd_id;
                        if (arg->vif->USB_Comms_Capable && product_type_dfp == VDM_ID_DFP_HOST)
                        {
                            rspvdo->SVDM.DFPVDO.host_cap_usb_2 =
                                arg->vif->DFPVDO_USB2_0_Capable;
                        }
                        else
                        {
                            rspvdo->SVDM.DFPVDO.host_cap_usb_2 = 0;
                        }
                        rspvdo->SVDM.DFPVDO.host_cap_usb_3_2 = arg->vif->DFPVDO_USB3_2_Capable;
                        rspvdo->SVDM.DFPVDO.host_cap_usb_4   = arg->vif->DFPVDO_USB4_Capable;
                        rspvdo->SVDM.DFPVDO.connector_type   = 0;
                        rspvdo++;
                        arg->buf.count++;
                        break;
                    default:
                        /* Undefined DFP (0) and AMC DFP (4)
					 * do not return product VDOs */
                        break;
                }
            }
        }
    }
    else if (header.SVDM.HEADER.CommandType == VDM_CMD_TYPE_ACK)
    {
        if (arg->sop == SOP_SOP0 && arg->vif->Modal_Operation_Supported_SOP)
        {
            //delay SVID driver until USB4 entry signals complete
#if CONFIG_ENABLED(USB4)
            VDM_STATE[arg->dev->port_tcpd_id].active = false;
            VDM_STATE[arg->dev->port_tcpd_id].state = STATE_DISABLED;
            /* Enable driver so cable_svid can run */
            port->vdm_auto_state = true;
#else
            VDM_STATE[arg->dev->port_tcpd_id].state = STATE_DISCOVER_SVID;
#endif

        }
        else
        {
            VDM_STATE[arg->dev->port_tcpd_id].active = false;
        }
    }
    else
    {
        /* NAK or invalid command Type */
        if (arg->sop == SOP_SOP0)
        {
            VDM_STATE[arg->dev->port_tcpd_id].active = false;
            VDM_STATE[arg->dev->port_tcpd_id].state = STATE_DISABLED;
        }
    }
}

static void vdm_pd_process_partner_disc_svid(struct vdm_sm_arg_t *arg)
{
    struct vdm_item_t *p         = VDM_STATE[arg->dev->port_tcpd_id].obj->next;
    struct port * port = arg->dev->port_tcpd_priv;
    struct pd_msg_t    vdo       = {0};
    int                i         = 0;
    unsigned long      portmask  = 0x1U << arg->dev->port_tcpd_id;
    struct pd_msg_t    header    = {0};
    int                vdo_count = arg->buf.count;
    header.object                = arg->buf.vdo[0];
    arg->buf.count               = 0;
    if (header.SVDM.HEADER.CommandType == VDM_CMD_TYPE_REQ)
    {
        arg->buf.count = 0;
        if (arg->sop == SOP_SOP0)
        {
            /* Copy svids */
            i          = 0;
            vdo.object = 0;
            while (p && i < 12)
            {
                if ((p->portmask & portmask) && (p->vdm->sopmask & (1U << SOP_SOP0)))
                {
                    /* Check odd and even */
                    if (i & 0x1)
                    {
                        vdo.SVDM.SVIDO.svid_low = p->vdm->svid;
                    }
                    else
                    {
                        /* Writing to new 32-bit VDO */
                        vdo.SVDM.SVIDO.svid_high = p->vdm->svid;
                        /* Add zero if svids are odd */
                        vdo.SVDM.SVIDO.svid_low = 0;
                        arg->buf.count++;
                    }
                    arg->buf.vdo[1 + i / 2] = vdo.object;
                    i++;
                }
                if (p->next == VDM_STATE[arg->dev->port_tcpd_id].obj)
                {
                    /* Next is the current driver so we have reached the end */
                    if (i > 1 && (i & 0x1) == 0)
                    {
                        /* If even add two additional zero svids per spec */
                        arg->buf.vdo[i] = 0;
                        arg->buf.count++;
                    }
                    break;
                }
                else
                {
                    p = p->next;
                }
            }
            vdo.object              = 0;
            vdo.SVDM.HEADER.SVID    = SID;
            vdo.SVDM.HEADER.VersionMajor = VDM_STRUCTURED_VER(arg->pdrev);
            vdo.SVDM.HEADER.VDMType = 1;
            vdo.SVDM.HEADER.ObjPos  = 0;
            vdo.SVDM.HEADER.Command = VDM_CMD_DISCOVER_SVIDS;
            vdo.SVDM.HEADER.VersionMinor = VDM_STRUCTURED_MAX_VER_MINOR(arg->pdrev);
            if (i == 0)
            {
                vdo.SVDM.HEADER.CommandType = VDM_CMD_TYPE_NAK;
                arg->buf.vdo[0]             = vdo.object;
            }
            else
            {
                vdo.SVDM.HEADER.CommandType = VDM_CMD_TYPE_ACK;
            }
            arg->buf.vdo[0] = vdo.object;
            arg->buf.count++;
            arg->tx_msg = true;
        }
    }
    else if (header.SVDM.HEADER.CommandType == VDM_CMD_TYPE_ACK)
    {
        if (arg->sop == SOP_SOP0)
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
                        if (vdo.SVDM.SVIDO.svid_low == p->vdm->svid)
                        {
                            p->vdm->set_active(arg->dev, true);
                            port->vdm_auto_state = true;
                        }
                        if (vdo.SVDM.SVIDO.svid_high == p->vdm->svid)
                        {
                            p->vdm->set_active(arg->dev, true);
                            port->vdm_auto_state = true;
                        }
                    }
                }
                if (p->next == VDM_STATE[arg->dev->port_tcpd_id].obj)
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
                VDM_STATE[arg->dev->port_tcpd_id].state  = STATE_DISABLED;
                VDM_STATE[arg->dev->port_tcpd_id].active = false;
            }
        }
    }
    else
    {
        if (arg->sop == SOP_SOP0)
        {
            VDM_STATE[arg->dev->port_tcpd_id].active = false;
        }
    }
}

static void vdm_pd_partner_nak(struct vdm_sm_arg_t *arg)
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

static void vdm_pd_process_partner(struct vdm_sm_arg_t *arg)
{
    struct pd_msg_t header;
    struct port_tcpd *dev = arg->dev;
    arg->vif = dev->vif;
    header.object = arg->buf.vdo[0];
    uint8_t cmd   = header.SVDM.HEADER.Command;
    if (cmd == VDM_CMD_DISCOVER_IDENTITY)
    {
        vdm_pd_process_partner_discv_id(arg);
    }
    else if (arg->vif->Modal_Operation_Supported_SOP && cmd == VDM_CMD_DISCOVER_SVIDS)
    {
        vdm_pd_process_partner_disc_svid(arg);
    }
    else if (cmd == VDM_CMD_ATTENTION)
    { /* Ignore */
    }
    else
    {
        /* NAK remaining */
        vdm_pd_partner_nak(arg);
    }
}

static void vdm_pd_request_discv_id(struct vdm_sm_arg_t *arg)
{
    struct pd_msg_t vdm         = {0};
    vdm.object                  = 0;
    vdm.SVDM.HEADER.SVID        = SID;
    vdm.SVDM.HEADER.VDMType     = VDM_STRUCTURED;
    vdm.SVDM.HEADER.VersionMajor = VDM_STRUCTURED_VER(arg->pdrev);
    vdm.SVDM.HEADER.ObjPos      = 0;
    vdm.SVDM.HEADER.CommandType = VDM_CMD_TYPE_REQ;
    vdm.SVDM.HEADER.Command     = VDM_CMD_DISCOVER_IDENTITY;
    vdm.SVDM.HEADER.VersionMinor = VDM_STRUCTURED_MAX_VER_MINOR(arg->pdrev);
    arg->buf.vdo[0]             = vdm.object;
    arg->buf.count              = 1;
    arg->sop                    = SOP_SOP0;
    arg->tx_msg                 = true;
}

static void vdm_pd_request_discv_svid(struct vdm_sm_arg_t *arg)
{
    struct pd_msg_t vdm         = {0};
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
    arg->sop                    = SOP_SOP0;
    arg->tx_msg                 = true;
}

static void vdm_pd_sm(struct vdm_sm_arg_t *arg)
{
    struct vdm_state *vdm = &VDM_STATE[arg->dev->port_tcpd_id];
    struct port * port = arg->dev->port_tcpd_priv;
    switch (vdm->state)
    {
        case STATE_DISCOVER_ID:
            vdm_pd_request_discv_id(arg);
            port->vdm_tx_svid = SID;
            break;
        case STATE_DISCOVER_SVID:
            vdm_pd_request_discv_svid(arg);
            port->vdm_tx_svid = SID;
            break;
        default:
            vdm->state  = STATE_DISABLED;
            vdm->active = false;
            port->vdm_tx_svid = 0;
            break;
    }
}

static bool vdm_pd_active(struct port_tcpd const *dev)
{
    return VDM_STATE[dev->port_tcpd_id].active;
}

static void vdm_pd_set_active(struct port_tcpd const *dev, bool active)
{
    VDM_STATE[dev->port_tcpd_id].active = active;
}

static const struct svdm_handler_t svdmh = {
    .vdm_process = vdm_pd_process_partner,
    .sm          = vdm_pd_sm,
};

static const struct vdm_obj_t pd_vdm_obj = {
    .structured = true,
    .svid       = SID,
    .svdmh      = &svdmh,
    .active     = vdm_pd_active,
    .set_active = vdm_pd_set_active,
    .reset      = vdm_pd_reset,
	.sopmask = (1U << SOP_SOP0),
};

struct vdm_item_t vdm_pd_driv = {
    .portmask = 0,
    .vdm      = &pd_vdm_obj,
    .next     = 0,
};

static void vdm_pd_reset(struct port_tcpd *const dev, uint16_t state_reset_enum)
{
    struct vdm_state *vdm = &VDM_STATE[dev->port_tcpd_id];
    vdm->active           = true;
    if (state_reset_enum == 0)
    {
        vdm->state        = STATE_DISCOVER_ID;
    }
    else
    {
        vdm->state        = state_reset_enum;
    }
    vdm->obj              = &vdm_pd_driv;
}

#endif /* CONFIG_VDM */
