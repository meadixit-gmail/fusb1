/*******************************************************************************
 * @file     port_tcpd.c
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
#include "dev_tcpd.h"
#include "typec.h"
#include "policy.h"
#include "protocol.h"

static struct port      PORTS[TYPE_C_PORT_COUNT];
static struct port_tcpd PORTS_TCPD[TYPE_C_PORT_COUNT];

struct port_tcpd *dpm_port_init(void *dpm_info, int port_id, void *vif)
{
    struct port      *port = &PORTS[port_id];
    struct port_tcpd *dev  = &PORTS_TCPD[port_id];

    /*save dpm_info*/
    dev->dpm_info_priv = dpm_info;
    /*save port id*/
    dev->port_tcpd_id = port_id;
    /*save vif*/
    dev->vif = (struct port_vif_t *)vif;
    /*save port tcpd device*/
    port->dev = dev;
    /*save port info*/
    dev->port_tcpd_priv = port;
    return dev;
}

/* Initialize
 *
 * Initializes the port data object's members */
void port_tcpd_init(void *port_priv)
{
    struct port *port     = (struct port *)port_priv;
    struct port_tcpd *dev = port->dev;
    port->vif = dev->vif;
    port->tcpd_device = dev->tcpd_device;
    port->is_vconn_source = false;
    typec_reset(port);
    policy_reset(port);
    protocol_reset(port);
    port->type = dev->vif->Type_C_State_Machine;
    typec_set_unattached(port);
}

unsigned long port_tcpd_next_schedule(struct port_tcpd *const dev)
{
    struct port  *port = (struct port *)dev->port_tcpd_priv;
    unsigned long min  = ~0;

    for (unsigned i = 0; i < NUM_TIMERS; i++)
    {
        struct ticktimer_t *t = &port->timers[i];
        if (!timer_disabled(t))
        {
            unsigned long r = timer_remaining(&port->timers[i]);
            if (r < min)
            {
                min = r;
            }
        }
    }

    return min;
}

void port_tcpd_pd_enable(struct port_tcpd *dev)
{
    struct port *port = (struct port *)dev->port_tcpd_priv;
    policy_pd_enable(port, true);
}

void port_tcpd_pd_disable(struct port_tcpd *dev)
{
    struct port *port = (struct port *)dev->port_tcpd_priv;
    policy_pd_disable(port);
}
#if (CONFIG_ENABLED(SRC))
void port_tcpd_set_attached_source(struct port_tcpd *dev)
{
    struct port *port = (struct port *)dev->port_tcpd_priv;
    typec_set_attached_source(port);
}

void port_tcpd_set_unattached_wait_source(struct port_tcpd *dev)
{
    struct port *port = (struct port *)dev->port_tcpd_priv;
    typec_set_unattached_wait_source(port);
}
#endif

void port_send_hard_reset(struct port_tcpd *dev)
{
    struct port *port = (struct port *)dev->port_tcpd_priv;
    policy_send_source_hardreset(port);
}

void port_set_error_recovery(struct port_tcpd *dev)
{
    struct port *port = (struct port *)dev->port_tcpd_priv;
    typec_set_error_recovery(port);
}

void port_set_disabled(struct port_tcpd *dev)
{
    struct port *port = (struct port *)dev->port_tcpd_priv;
    typec_set_disabled(port);
}

void port_set_unattached(struct port_tcpd *dev)
{
    struct port *port = (struct port *)dev->port_tcpd_priv;
    typec_set_unattached(port);
}

enum cc_t port_get_cc_orientation(struct port_tcpd *dev)
{
    struct port *port = (struct port *)dev->port_tcpd_priv;
    return port->cc_pin;
}

bool port_yield(struct port_tcpd *dev)
{
    struct port *port = (struct port *)dev->port_tcpd_priv;
    return !(port->event && port->tc_enabled);
}

bool port_tc_enabled(struct port_tcpd *dev)
{
    struct port *port = (struct port *)dev->port_tcpd_priv;
    return port->tc_enabled;
}

bool port_sending_message(struct port_tcpd *dev)
{
    struct port *port = (struct port *)dev->port_tcpd_priv;
    return port->policy_sending_message;
}

void port_disable(struct port_tcpd *dev)
{
    struct port *port = (struct port *)dev->port_tcpd_priv;
    port->tc_enabled  = false;
}

void port_enable(struct port_tcpd *dev)
{
    struct port *port = (struct port *)dev->port_tcpd_priv;
    port->tc_enabled  = true;
}

enum tc_state_t port_tc_state(struct port_tcpd *dev)
{
    struct port *port = (struct port *)dev->port_tcpd_priv;
    return port->tc_state;
}

void port_sm(struct port_tcpd *const dev)
{
    struct port *port = (struct port *)dev->port_tcpd_priv;
    typec_sm(port);
}

void port_sm_exec(struct port_tcpd *dev)
{
    struct port *port = (struct port *)dev->port_tcpd_priv;
    do
    {
        typec_sm(port);
    } while (port->event);
}

void port_cable_reset(struct port_tcpd *dev)
{
    struct port *port = (struct port *)dev->port_tcpd_priv;
    if (port->cbl_rst_state == CBL_RST_DISABLED)
    {
        port->cbl_rst_state = CBL_RST_START;
    }
}

uint32_t port_sink_selected_voltage(struct port_tcpd *dev)
{
    struct port *port = (struct port *)dev->port_tcpd_priv;
    return (port->sink_selected_voltage);
}

bool port_policy_has_contract(struct port_tcpd *dev)
{
    struct port *port = (struct port *)dev->port_tcpd_priv;
    return port->policy_has_contract;
}

void port_set_contract_valid(struct port_tcpd *dev, bool valid)
{
    struct port *port       = (struct port *)dev->port_tcpd_priv;
    port->is_contract_valid = valid;
}

void port_source_set_rp_current(struct port_tcpd *dev, enum cc_rp_t current)
{
    struct port *port = (struct port *)dev->port_tcpd_priv;
    port->src_current = current;
    fusbdev_tcpd_set_rp(dev->tcpd_device, CC1 | CC2, current);
}

void port_transmit_source_caps(struct port_tcpd *dev)
{
    struct port *port   = (struct port *)dev->port_tcpd_priv;
    port->msgtx.booltxmsg.get_src_caps = true;
    port->event         = true;
}

void port_transmit_get_battery_capability(struct port_tcpd *dev)
{
    struct port *port = (struct port *)dev->port_tcpd_priv;
    port->msgtx.booltxmsg.getbatterycap = true;
    port->event       = true;
}

void port_transmit_get_battery_status(struct port_tcpd *dev)
{
    struct port *port  = (struct port *)dev->port_tcpd_priv;
    port->msgtx.booltxmsg.getbatterystatus = true;
    port->event        = true;
}
