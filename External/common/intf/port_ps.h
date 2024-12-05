/*******************************************************************************
 * @file     port_ps.h
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
#ifndef __PORT_PS_H__
#define __PORT_PS_H__

#include "pd_types.h"
#include "power_sharing.h"
#include "fusbdev_hal.h"

//is this needed?
struct power_sharing
{
    struct port_ps           *dev;
    struct port_tcpd         *tcpd_dev;
    struct power_sharing_dev *ps_dev;
};

struct port_ps *dpm_ps_init(void *dpm_info, int port_id);
void            port_ps_dev_init(struct power_sharing_dev *ps_dev);
void            port_ps_dev_register_callbacks(void);
void            port_ps_dev_processing(struct power_sharing_dev *ps_dev);
void            port_ps_notify_tc_attached(int, void *, struct power_sharing_dev *);
void            port_ps_notify_tc_detached(int, void *, struct power_sharing_dev *);
void            port_ps_notify_pd_device(int, void *, struct power_sharing_dev *);

#endif //__PORT_PS_H__
