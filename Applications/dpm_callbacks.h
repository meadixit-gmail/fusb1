/*******************************************************************************
 * @file   dpm_callbacks.h
 * @author USB Firmware Team
 * @brief  Implements the Device Policy Manager state machine functions.
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
#include "dev_tcpd.h"
#include "vif_types.h"
#if (CONFIG_ENABLED(POWER_SHARING))
#include "port_ps.h"
#endif
#include "port_dcdc.h"
#include "port_bc1p2.h"
#include "port.h"
#include "dpm_util.h"
#include "dpm.h"
#if CONFIG_ENABLED(CUSTOM_APPLICATION)
#include "port_ps.h"
#include "pdps_ocp.h"
#include "pd_powersupply.h"
#endif
#if (DEVICE_TYPE == FUSB15201) || (DEVICE_TYPE == FUSB15201P)
#include "batt_ntc_monitor.h"
#endif





void dpm_event_vbus_req_cb(struct port_tcpd *dev, void *ctx);