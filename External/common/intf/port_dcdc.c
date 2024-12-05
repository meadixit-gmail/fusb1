/*******************************************************************************
 * @file     port_dcdc.c
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
#include "port.h"
#include "dc_dc_board.h"
#include "port_dcdc.h"
#if CONFIG_ENABLED(DCDC)

static struct dc_dc_obj DCDC_obj   = {0};
static struct port_dcdc PORTS_DCDC = {0};

struct port_dcdc *dpm_dcdc_init(void *dpm)
{
    struct dc_dc_obj *dcdc_obj = &DCDC_obj;
    struct port_dcdc *dev      = &PORTS_DCDC;

    /*save dpm_info*/
    dev->dpm_priv = dpm;
    /*save port dcdc device*/
    dcdc_obj->dev = dev;
    /*save port dcdc info*/
    dev->port_dcdc_priv = dcdc_obj;
    return dev;
}

void port_dcdc_dev_init(void *dcdc_priv, struct dc_dc_master *DCDC_master,
                        struct dc_dc_data *DCDC_data)
{
    port_dcdc_master_init((struct dc_dc_obj *)dcdc_priv, DCDC_master, DCDC_data);
}

void port_dc_dc_set(void *dcdc_priv, int tcpd_id, uint32_t mv, uint32_t ma)
{
    struct dc_dc_obj *dcdc_obj = (struct dc_dc_obj *)dcdc_priv;
    dc_dc_set(dcdc_obj->dcdc_data[tcpd_id], mv, ma);
}

#if (DEVICE_TYPE == FUSB15200 || DEVICE_TYPE == FUSB15201 || DEVICE_TYPE == FUSB15201P)
void port_dc_dc_delayed_disable(void *dcdc_priv, int tcpd_id)
{
    struct dc_dc_obj *dcdc_obj = (struct dc_dc_obj *)dcdc_priv;
    dc_dc_delayed_disable(dcdc_obj->dcdc_data[tcpd_id]);
}

void port_dc_dc_enable(void *dcdc_priv, int tcpd_id, bool en)
{
    struct dc_dc_obj *dcdc_obj = (struct dc_dc_obj *)dcdc_priv;
    dc_dc_enable(dcdc_obj->dcdc_data[tcpd_id], en);
}

void port_dc_dc_delayed_vbus_write(void *dcdc_priv, int tcpd_id, uint32_t mv, uint32_t ma)
{
    struct dc_dc_obj *dcdc_obj = (struct dc_dc_obj *)dcdc_priv;
    dc_dc_delayed_vbus_write(dcdc_obj->dcdc_data[tcpd_id], mv, ma);
}

void port_process_dc_dc(void *dcdc_priv)
{
    struct dc_dc_obj *dcdc_obj = (struct dc_dc_obj *)dcdc_priv;
    port_processing_dc_dc(dcdc_obj);
}

#endif
/*TODO: to be used when properly ported*/
#if 0
uint32_t port_dcdc_next_schedule(struct dc_dc_data *obj)

{
	unsigned long min = ~0;
	struct ticktimer_t *t;
	unsigned long r, i;

	for (i = 0; i < DC_DC_TIMER_COUNT; i++) {
		t = &obj->dc_dc_timers[i];
		if (!timer_disabled(t)) {
			r = timer_remaining(t);
			if (r < min) {
				min = r;
			}
		}
	}
	return min;
}
#endif

#endif
