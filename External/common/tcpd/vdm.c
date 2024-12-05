/*******************************************************************************
 * @file     vdm.c
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
/* **************************************************************************
 *  vdm.cpp
 *
 *  An example implementation of the VDM functionality API.
 * ************************************************************************** */
#include "policy.h"
#include "vdm.h"

/**
 * @brief a circular linked list
 */

struct vdm_obj_list_t
{
    struct vdm_item_t *head;
};

static struct vdm_obj_list_t vdm_list = {0};
uint8_t VDM_MAX_VERSION_MINOR[TYPE_C_PORT_COUNT] = {0};
void vdm_list_register(struct port_tcpd *const dev, VDM_ITEM_T *item)
{
    struct vdm_item_t *p = vdm_list.head;
    if (p == 0)
    {
        /* Add first element */
        vdm_list.head = item;
    }
    else
    {
        while (p && p->next != vdm_list.head)
        {
            p = p->next;
        }
        if (p)
        {
            p->next = item;
        }
    }
    /* circular list */
    item->portmask |= 1U << dev->port_tcpd_id;
    item->next = vdm_list.head;
}

struct vdm_obj_t const *vdm_list_find_svid(struct port_tcpd *const dev, uint16_t svid, uint8_t sop)
{
    struct vdm_item_t *p    = vdm_list.head;
    unsigned           mask = 1U << dev->port_tcpd_id;
    while (p)
    {
        if (p->vdm->svid == svid && (p->portmask & mask) && (p->vdm->sopmask & sop))
        {
            return p->vdm;
        }
        if (p->next == vdm_list.head)
        {
            break;
        }
        else
        {
            p = p->next;
        }
    }
    return 0;
}

void vdm_list_reset_all(struct port_tcpd *const dev, uint16_t svid, uint8_t sop, uint16_t state_reset_enum)
{
    unsigned           portmask = 1U << dev->port_tcpd_id;
    struct port * port = dev->port_tcpd_priv;
    struct vdm_item_t *p        = vdm_list.head;
    if (svid == 0)
    {
    	VDM_MAX_VERSION_MINOR[dev->port_tcpd_id] = VDM_STRUCTURED_MAX_VER_MINOR(port->pd_rev[SOP_SOP0]);
    }
    while (p)
    {
        if (p->portmask & portmask)
        {
            if (!svid)
            {
                p->vdm->reset(dev, state_reset_enum);
            }
            else
            {
                if (p->vdm->svid == svid && (sop & p->vdm->sopmask))
                {
                    /*TODO: support for SOP1 and SOP2 for cable plugs will be added in JIRA FFUSB15200-319
					 until then, we reset when SVID=0XFF00 without identifying if communication used is SOP1/SOP2 and
					 if it has entered active mode*/
                    p->vdm->reset(dev, state_reset_enum);
                }
            }
        }
        if (p->next == vdm_list.head)
        {
            break;
        }
        else
        {
            p = p->next;
        }
    }
}

bool vdm_list_run_active(struct vdm_sm_arg_t *arg)
{
    bool               done     = true;
    struct vdm_item_t *obj      = vdm_list.head;
    unsigned           portmask = 1U << arg->dev->port_tcpd_id;
    struct port * port = arg->dev->port_tcpd_priv;
    while (obj)
    {
        if (obj->vdm->active(arg->dev) && (portmask & obj->portmask) && (obj->vdm->sopmask & (arg->sop)))
        {
            done = false;
            if (obj->vdm->structured)
            {
                obj->vdm->svdmh->sm(arg);
            }
            else
            {
                obj->vdm->uvdmh->sm(arg);
            }
            port->vdm_tx_svid = obj->vdm->svid;
            break;
        }
        else
        {
            if (obj->next == vdm_list.head)
            {
                break;
            }
            else
            {
                obj = obj->next;
            }
        }
    }
    return done;
}

void vdm_disable_driver(struct port_tcpd const *dev, uint16_t svid, uint8_t sop)
{
    struct vdm_item_t *obj      = vdm_list.head;
    unsigned           portmask = 1U << dev->port_tcpd_id;
    while (obj)
    {
        if ((obj->portmask & portmask) && obj->vdm->svid == svid && (sop & obj->vdm->sopmask))
        {
            obj->vdm->set_active(dev, false);
            break;
        }
        if (obj->next == vdm_list.head)
        {
            break;
        }
        else
        {
        	obj = obj->next;
        }
    }
}

void vdm_enable_driver(struct port_tcpd const *dev, uint16_t svid, uint8_t sop)
{
    struct vdm_item_t *obj      = vdm_list.head;
    unsigned           portmask = 1U << dev->port_tcpd_id;
    while (obj)
    {
        if ((obj->portmask & portmask) && obj->vdm->svid == svid && (obj->vdm->sopmask & sop))
        {
            obj->vdm->set_active(dev, true);
            break;
        }
        if (obj->next == vdm_list.head)
        {
            break;
        }
        else
        {
        	obj = obj->next;
        }
    }
}

void vdm_disable_all(struct port_tcpd const *dev)
{
    struct vdm_item_t *obj      = vdm_list.head;
    unsigned           portmask = 1U << dev->port_tcpd_id;
    while (obj)
    {
        if (obj->portmask & portmask)
        {
            obj->vdm->set_active(dev, false);
        }
        if (obj->next == vdm_list.head)
        {
            break;
        }
        else
		{
			obj = obj->next;
		}
    }
}

bool vdm_is_active(struct port_tcpd const *dev, uint16_t svid)
{
    struct vdm_item_t *obj      = vdm_list.head;
    unsigned           portmask = 1U << dev->port_tcpd_id;
    bool ret = false;
	while (obj)
	{
		if (obj->vdm->active(dev) && (portmask & obj->portmask))
		{
			ret = true;
			break;
		}
		else
		{
			if (obj->next == vdm_list.head)
			{
				break;
			}
			else
			{
				obj = obj->next;
			}
		}
	}
	return ret;
}

void update_svdm_version_minor(struct port_tcpd const *dev, uint8_t svdm_command)
{
	struct port * port = dev->port_tcpd_priv;
	VDM_MAX_VERSION_MINOR[dev->port_tcpd_id] = 1;
}
