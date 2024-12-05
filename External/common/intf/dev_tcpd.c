/*******************************************************************************
 * @file     dev_tcpd.c
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
#include "pd_types.h"
#include "vif_types.h"
#include "port.h"
#include "port_tcpd.h"
#include "dev_tcpd.h"
#include "observer.h"
#include "vdm.h"

/***********************************************************************************
 * DPM Interfaces to fusbdev driver
 **********************************************************************************/
enum sink_tx_state_t dev_tcpd_port_sinktx_state(struct port *port)
{
    enum cc_rp_t rp = fusbdev_tcpd_pullup_value(port->tcpd_device, port->cc_pin);
    if (rp == CC_RP_3p0A)
	{
		return SinkTxOK;
	}
    else
    {
    	return SinkTxNG;
    }
}

bool dev_tcpd_port_set_sinktx(struct port *port, enum sink_tx_state_t state)
{
    bool success = false;
    if (port_pd_ver(port, SOP_SOP0) > PD_REV2)
    {
        if (state == SinkTxOK)
        {
            fusbdev_tcpd_set_rp(port->tcpd_device, port->cc_pin, CC_RP_3p0A);
            success = true;
        }
        else if (state == SinkTxNG)
        {
            fusbdev_tcpd_set_rp(port->tcpd_device, port->cc_pin, CC_RP_1p5A);
            success = true;
        }
    }
    return success;
}

bool dev_tcpd_port_set_vconn(struct port *port, bool enable)
{
    bool              vconn_enabled = false;
    enum cc_t         vcon;
    EVENT_VCONN_REQ_T vconn_req;
    vcon                    = (port->cc_pin == CC1) ? CC2 : CC1;
    vconn_req.arg.enable    = enable;
    vconn_req.arg.vconn_pin = vcon;
    vconn_req.ret.success   = EVENT_STATUS_DEFAULT;

    /* Enable/disable the termination */
    if (enable)
    {
        fusbdev_tcpd_set_term(port->tcpd_device, vcon, CC_TERM_OPEN);
    }
    else
    {
        fusbdev_tcpd_set_term(port->tcpd_device, vcon, CC_TERM_RD);
    }
#if CONFIG_ENABLED(EXTENDED_EVENTS)
    event_notify(EVENT_VCONN_REQ, port->dev, &vconn_req);
#endif
    if (vconn_req.ret.success == EVENT_STATUS_DEFAULT)
    {
        /* DPM does not override so default behavior */
        fusbdev_tcpd_vconn_enable(port->tcpd_device, enable, HAL_VCONN_OCP_10mA);
        vconn_enabled = true;
    }
    else if (vconn_req.ret.success == EVENT_STATUS_SUCCESS)
    {
        /* DPM override */
        vconn_enabled = true;
    }

    port->is_vconn_source = enable;
    return vconn_enabled;
}

/***********************************************
 * Policy public functions
 ************************************************/

void dev_tcpd_port_pd_set_sop_ver(struct port *port, enum pd_rev_t rev)
{
    port->pd_rev[SOP_SOP0] = rev;
    fusbdev_tcpd_pd_rev(port->tcpd_device, rev, SOP_SOP0);
}

void dev_tcpd_port_pd_set_cable_ver(struct port *port, enum pd_rev_t rev)
{
    port->pd_rev[SOP_SOP1] = rev;
    port->pd_rev[SOP_SOP2] = rev;
}

void dev_tcpd_port_pd_set_ver(struct port *port, enum sop_t sop, enum pd_rev_t rev)
{
    enum pd_rev_t pdrev = port->vif->PD_Specification_Revision;
    if (rev < pdrev)
    {
        pdrev = rev;
    }
    if (sop == SOP_SOP0)
    {
        dev_tcpd_port_pd_set_sop_ver(port, pdrev);
        if (pdrev < port_pd_ver(port, SOP_SOP1))
        {
            dev_tcpd_port_pd_set_cable_ver(port, pdrev);
        }
    }
    else
    {
        dev_tcpd_port_pd_set_cable_ver(port, pdrev);
    }
}

bool dev_tcpd_port_vbus_src(struct port *port, unsigned mv, unsigned ma, enum pdo_t type)
{
    bool             vbus_on = false;
    EVENT_VBUS_REQ_T vbus_req;
    vbus_req.arg.mv      = mv;
    vbus_req.arg.ma      = ma;
    vbus_req.arg.type    = type;
    vbus_req.ret.success = EVENT_STATUS_DEFAULT;
    event_notify(EVENT_VBUS_REQ, port->dev, &vbus_req);
#if (DEVICE_TYPE == FUSB15200 || DEVICE_TYPE == FUSB15101)
	vbus_on = fusbdev_tcpd_set_vbus_source(port->tcpd_device, mv, ma, type);
#else
	vbus_on = true;
#endif

    if (vbus_on)
    {
        fusbdev_tcpd_set_pd_source(port->tcpd_device, true);
#if (DEVICE_TYPE == FUSB15101)
        if (mv != 0)
        {
        	/* Avoid changing resolution during fault handling  */
			if (vbus_req.arg.type == pdoTypeAugmented)
			{
				fusbdev_tcpd_set_vbus_res(port->tcpd_device, 21000);
			}
			else
			{
				fusbdev_tcpd_set_vbus_res(port->tcpd_device, mv);
			}
        }
#else
        fusbdev_tcpd_set_vbus_res(port->tcpd_device, mv);
#endif
    }

    return vbus_on;
}

bool dev_tcpd_port_vbus_snk(struct port *port, unsigned mv, unsigned ma, enum pdo_t type)
{
    bool              snk = false;
    EVENT_VBUS_SINK_T evt = {0};
    evt.arg.ma            = ma;
    evt.arg.mv            = mv;
    evt.arg.type          = type;
    evt.ret.success       = EVENT_STATUS_DEFAULT;
#if CONFIG_ENABLED(SNK)
    event_notify(EVENT_VBUS_SINK, port->dev, &evt);
#endif
    if (evt.ret.success == EVENT_STATUS_DEFAULT)
    {
        snk = fusbdev_tcpd_set_vbus_sink(port->tcpd_device, mv, ma, type);
    }
    else if (evt.ret.success == EVENT_STATUS_SUCCESS)
    {
        snk = true;
    }

    if (snk)
    {
        fusbdev_tcpd_set_pd_source(port->tcpd_device, false);
#if (DEVICE_TYPE != FUSB15101)
        fusbdev_tcpd_set_vbus_res(port->tcpd_device, mv);
#endif
    }
    return snk;
}

void dev_tcpd_port_vbus_init(struct port *port, bool en)
{
    if (en)
    {
        fusbdev_tcpd_vbus_init(port->tcpd_device);
        fusbdev_tcpd_vbus_events(port->tcpd_device, true);
    }
    else
    {
        fusbdev_tcpd_vbus_deinit(port->tcpd_device);
        fusbdev_tcpd_vbus_events(port->tcpd_device, false);
    }
}

unsigned port_max_cable_current(struct port *port)
{
    unsigned max;
    if (port->cable_vdo_cable_current == VDM_CABLE_MAX_CURRENT_5A)
    {
        /* 5A */
        max = 5000;
    }
    else
    {
        /* Default to 3A for any other value */
        max = 3000;
    }
    return max;
}

unsigned port_max_cable_voltage(struct port *port)
{
    unsigned max = 20000; // 20V default

    if (port->cable_vdo_cable_voltage == VDM_CABLE_MAX_VOLTAGE_20V)
    {
        max = 20000;
    }
    else if (port->cable_vdo_cable_voltage == VDM_CABLE_MAX_VOLTAGE_30V)
    {
        max = 30000;
    }
    else if (port->cable_vdo_cable_voltage == VDM_CABLE_MAX_VOLTAGE_40V)
    {
        max = 40000;
    }
    else if (port->cable_vdo_cable_voltage == VDM_CABLE_MAX_VOLTAGE_50V)
    {
        max = 50000;
    }

    return max;
}
