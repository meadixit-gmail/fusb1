/*******************************************************************************
 * @file     port_dcdc.h
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
#ifndef __PORT_DCDC_H__
#define __PORT_DCDC_H__

#include "pd_types.h"

struct dc_dc_obj
{
    struct port_dcdc          *dev;
    struct dc_dc_master const *dcdc_master[I2C_PORT_COUNT];
    struct dc_dc_data         *dcdc_data[TYPE_C_PORT_COUNT]; //10 slaves for now?
};

struct port_dcdc *dpm_dcdc_init(void *dpm);
void              port_dcdc_dev_init(void *dcdc_priv, struct dc_dc_master *DCDC_master,
                                     struct dc_dc_data *DCDC_data);
void              port_dc_dc_set(void *dcdc_priv, int tcpd_id, uint32_t mv, uint32_t ma);
void              port_dc_dc_delayed_disable(void *dcdc_priv, int tcpd_id);
void              port_dc_dc_enable(void *dcdc_priv, int tcpd_id, bool en);
void port_dc_dc_delayed_vbus_write(void *dcdc_priv, int tcpd_id, uint32_t mv, uint32_t ma);
void port_process_dc_dc(void *dcdc_priv);

#endif //__PORT_DCDC_H__
