/*******************************************************************************
 * @file     pdps_ocp.c
 * @author   USB PD Firmware Team
 *
 * @copyright @parblock
 * Copyright &copy; 2021 ON Semiconductor &reg;. All rights reserved.
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
#include "port_bc1p2.h"
#include "pdps_ocp.h"
#include "dpm.h"
#include "power_sharing.h"
#include "logging.h"
#include "FUSB15xxx_hal.h"
#include "batt_ntc_monitor.h"
#include "legacy.h"
#include "fusbdev_hal.h"
#include "timer.h"
#include "dev_tcpd.h"
extern volatile bool gSleepInt;
void                 pdps_ocp_init(void *ocp_dev)
{
    struct pdps_ocp *ocp_dev_p = (struct pdps_ocp *)ocp_dev;
    ocp_dev_p->ocp_count       = 0;
    ocp_dev_p->ocp_state       = PDPS_OCP_DISABLED;
    gSleepInt                  = true;
    timer_disable(&ocp_dev_p->pdps_ocp_timer);
}

void pdps_ocp_reset(void *ocp_dev, void *ps_dev, void *p_bc1p2, void *p_tcpd)
{
    struct pdps_ocp          *ocp_dev_p = (struct pdps_ocp *)ocp_dev;
    struct power_sharing_dev *ps_dev_p  = (struct power_sharing_dev *)ps_dev;
    struct port_bc1p2        *p_bc1p2_p = (struct port_bc1p2 *)p_bc1p2;
    struct port_tcpd         *p_tcpd_p  = (struct port_tcpd *)p_tcpd;
    /* Disable Type-C State Machine */
    port_disable(p_tcpd_p);
    /* BC1.2 */
#if (CONFIG_ENABLED(LEGACY_CHARGING))
    port_bc1p2_sm_disable(p_bc1p2_p->port_bc1p2_priv);
#endif
    struct port *port = (struct port *)p_tcpd_p->port_tcpd_priv;
    /* Perform Error Recovery Flow */
    port_set_error_recovery(p_tcpd_p);

    ocp_dev_p->ocp_state = PDPS_OCP_RECOVERY;
    gSleepInt            = true;
}

void pdps_ocp_disable_port(void *ocp_dev, void *ps_dev, void *p_bc1p2, void *p_tcpd)
{
    struct pdps_ocp          *ocp_dev_p = (struct pdps_ocp *)ocp_dev;
    struct power_sharing_dev *ps_dev_p  = (struct power_sharing_dev *)ps_dev;
    struct port_bc1p2        *p_bc1p2_p = (struct port_bc1p2 *)p_bc1p2;
    struct port_tcpd         *p_tcpd_p  = (struct port_tcpd *)p_tcpd;
    /* Disable Type-C State Machine */
    port_disable(p_tcpd_p);
    /* BC1.2 */
#if (CONFIG_ENABLED(LEGACY_CHARGING))
    port_bc1p2_sm_disable(p_bc1p2_p->port_bc1p2_priv);
#endif

    struct port *port = (struct port *)p_tcpd_p->port_tcpd_priv;
    /* Disable VBUS - Set Rp */
    port_set_error_recovery(p_tcpd_p);
    ocp_dev_p->ocp_state = PDPS_OCP_WAIT_DETACH;
    gSleepInt            = true;
}

static void pdps_ocp_recovery(void *ocp_dev, void *ps_dev, void *p_tcpd)
{
    struct power_sharing_dev *ps_dev_p  = (struct power_sharing_dev *)ps_dev;
    struct pdps_ocp          *ocp_dev_p = (struct pdps_ocp *)ocp_dev;
    struct port_tcpd         *p_tcpd_p  = (struct port_tcpd *)p_tcpd;
    struct port              *port      = (struct port *)p_tcpd_p->port_tcpd_priv;
    if (timer_expired(&port->timers[TC_TIMER]))
    {
        /* Set unattached flow */
        port_set_unattached(p_tcpd_p);
        /* Blanking period for Rp to account for DRP toggle */
        timer_start(&ocp_dev_p->pdps_ocp_timer, PDPS_OCP_ATTACH_MAX);

        ocp_dev_p->ocp_state = PDPS_OCP_ATTACH;
        gSleepInt            = true;
    }
}

static void pdps_ocp_attach(void *ocp_dev, void *ps_dev, void *p_tcpd)
{
    struct pdps_ocp          *ocp_dev_p = (struct pdps_ocp *)ocp_dev;
    struct power_sharing_dev *ps_dev_p  = (struct power_sharing_dev *)ps_dev;
    struct port_tcpd         *p_tcpd_p  = (struct port_tcpd *)p_tcpd;
    struct port              *port      = (struct port *)p_tcpd_p->port_tcpd_priv;
    if (timer_expired(&ocp_dev_p->pdps_ocp_timer))
    {
        port_enable((struct port_tcpd *)p_tcpd);

        /* Wait for attached_wait_src */
        if (port->tc_state == AttachWaitSource)
        {
            if (ps_dev_p->shared_capacity > 15000)
            {
                ocp_dev_p->ocp_state = PDPS_OCP_WAIT_SOURCE;
            }
            else
            {
                ocp_dev_p->ocp_state = PDPS_OCP_WAIT_POWER;
            }
            gSleepInt = true;
        }
    }
}

static void pdps_ocp_wait_source(void *ocp_dev, void *ps_dev, void *p_bc1p2, void *p_tcpd)
{
    struct pdps_ocp          *ocp_dev_p = (struct pdps_ocp *)ocp_dev;
    struct power_sharing_dev *ps_dev_p  = (struct power_sharing_dev *)ps_dev;
    struct port_bc1p2        *p_bc1p2_p = (struct port_bc1p2 *)p_bc1p2;
    struct port_tcpd         *p_tcpd_p  = (struct port_tcpd *)p_tcpd;
    struct port              *port      = (struct port *)p_tcpd_p->port_tcpd_priv;
    if (timer_expired(&ocp_dev_p->pdps_ocp_timer) && (port->tc_state == AttachedSource))
    {
#if (CONFIG_ENABLED(LEGACY_CHARGING))
        port_bc1p2_sm_enable(p_bc1p2_p->port_bc1p2_priv);
#endif
        timer_disable(&ocp_dev_p->pdps_ocp_timer);
        ocp_dev_p->ocp_state = PDPS_OCP_DISABLED;
        gSleepInt            = true;
    }
}

static void pdps_ocp_wait_detach(void *ocp_dev, void *ps_dev, void *p_tcpd)
{
    struct pdps_ocp          *ocp_dev_p = (struct pdps_ocp *)ocp_dev;
    struct power_sharing_dev *ps_dev_p  = (struct power_sharing_dev *)ps_dev;
    struct port_tcpd         *p_tcpd_p  = (struct port_tcpd *)p_tcpd;
    struct port              *port      = (struct port *)p_tcpd_p->port_tcpd_priv;
    /* On Src.Open, set Unattached and enable port */
    enum cc_t orientation = port_get_cc_orientation(p_tcpd);
    if (fusbdev_tcpd_cc_stat(port->dev->tcpd_device, orientation) == CC_STAT_SRCOPEN)
    {
        if (ps_dev_p->shared_capacity > 15000)
        {
            port_set_unattached(p_tcpd_p);
        }
        ocp_dev_p->ocp_state = PDPS_OCP_DISABLED;
        gSleepInt            = true;
    }
}

static void pdps_ocp_wait_power(void *ocp_dev, void *ps_dev, void *p_tcpd)
{
    struct pdps_ocp          *ocp_dev_p = (struct pdps_ocp *)ocp_dev;
    struct port_tcpd         *p_tcpd_p  = (struct port_tcpd *)p_tcpd;
    struct power_sharing_dev *ps_dev_p  = (struct power_sharing_dev *)ps_dev;
    struct port              *port      = (struct port *)p_tcpd_p->port_tcpd_priv;
    /* On Src.Open, set Unattached and enable port */
    CC_T pin = port->cc_pin;
    if (ps_dev_p->shared_capacity > 15000)
    {
        ocp_dev_p->ocp_state = PDPS_OCP_ATTACH;
    }
    else if (fusbdev_tcpd_cc_stat(port->dev->tcpd_device, pin) == CC_STAT_SRCOPEN)
    {
        gSleepInt = true;
        if (ps_dev_p->shared_capacity > 0)
        {
            dev_tcpd_port_vbus_src(port, 0, 0, false);
            port_set_unattached(p_tcpd_p);
            port->tc_enabled     = true;
            ocp_dev_p->ocp_state = PDPS_OCP_DISABLED;
        }
        else
        {
            ocp_dev_p->ocp_state = PDPS_OCP_DISABLED;
        }
    }
}

void pdps_ocp_sm(void *ocp_dev, void *ps_dev, void *p_bc1p2, void * port_dev)
{
    struct pdps_ocp          *ocp_dev_p = (struct pdps_ocp *)ocp_dev;
    struct power_sharing_dev *ps_dev_p  = (struct power_sharing_dev *)ps_dev;
    struct port_bc1p2        *p_bc1p2_p = (struct port_bc1p2 *)p_bc1p2;
    struct port_tcpd *p_tcpd_p = (struct port_tcpd *)port_dev;
    switch (ocp_dev_p->ocp_state)
    {
        case PDPS_OCP_DISABLED:
            break;
        case PDPS_OCP_RECOVERY:
            pdps_ocp_recovery(ocp_dev_p, ps_dev_p, p_tcpd_p);
            break;
        case PDPS_OCP_ATTACH:
            pdps_ocp_attach(ocp_dev_p, ps_dev_p, p_tcpd_p);
            break;
        case PDPS_OCP_WAIT_SOURCE:
            pdps_ocp_wait_source(ocp_dev_p, ps_dev_p, p_bc1p2_p, p_tcpd_p);
            break;
        case PDPS_OCP_WAIT_DETACH:
            pdps_ocp_wait_detach(ocp_dev_p, ps_dev_p, p_tcpd_p);
            break;
        case PDPS_OCP_WAIT_POWER:
            pdps_ocp_wait_power(ocp_dev_p, ps_dev_p, p_tcpd_p);
            break;
        default:
            break;
    }
}

uint32_t pdps_ocp_min_timer(void *ocp_dev)
{
    struct pdps_ocp *ocp_dev_p = (struct pdps_ocp *)ocp_dev;
    uint32_t         r;

    if (!timer_disabled(&ocp_dev_p->pdps_ocp_timer))
    {
        r = timer_remaining(&ocp_dev_p->pdps_ocp_timer);
    }
    else
    {
        r = ~0UL;
    }

    return r;
}
