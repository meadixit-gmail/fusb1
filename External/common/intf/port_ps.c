/*******************************************************************************
 * @file     port_psc.c
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
#include "port.h"
#include "power_sharing.h"

#if CONFIG_ENABLED(POWER_SHARING)

void port_ps_dev_init(struct power_sharing_dev *ps_dev)
{
    assert(ps_dev != 0);
    port_ps_init(ps_dev);
}
#if !CONFIG_ENABLED(CONST_EVENTS)
void port_ps_dev_register_callbacks(void)
{
    port_ps_register_callbacks();
}
#endif
void port_ps_dev_processing(struct power_sharing_dev *ps_dev)
{
    port_ps_processing(ps_dev);
}

void port_ps_notify_tc_attached(int port_tcpd_id, void *ctx, struct power_sharing_dev *ps_dev)
{
    ps_event_tc_attached(ps_dev->port_tcpd[port_tcpd_id], ctx);
}

void port_ps_notify_tc_detached(int port_tcpd_id, void *ctx, struct power_sharing_dev *ps_dev)
{
    ps_event_tc_detached(ps_dev->port_tcpd[port_tcpd_id], ctx);
}

void port_ps_notify_pd_device(int port_tcpd_id, void *ctx, struct power_sharing_dev *ps_dev)
{
    ps_event_pd_device(ps_dev->port_tcpd[port_tcpd_id], ctx);
}

#endif //CONFIG_ENABLED(POWER_SHARING)
