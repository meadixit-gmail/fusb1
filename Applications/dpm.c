/*******************************************************************************
 * @file   dpm.c
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
#include "fusb15xxx.h"
#include "dev_tcpd.h"
#include "fusbdev_hal.h"
#include "timer.h"
#include "vif_types.h"
#include "vdm.h"
#include "power_sharing.h"
#include "dpm.h"
#include "dc_dc_board.h"
extern volatile bool gSleepInt;
#include "legacy_types.h"
#include "port_bc1p2.h"
#include "port_dcdc.h"
#if (CONFIG_ENABLED(POWER_SHARING))
#include "port_ps.h"
#endif

#if CONFIG_ENABLED(CUSTOM_APPLICATION_INCLUDE)
#include "custom_application.h"

HAL_I2C_T    *i2c_master[I2C_PORT_COUNT];
volatile bool i2c_porta_interrupt;
volatile bool i2c_portb_interrupt;
#endif
#if CONFIG_ALT_BATT_NTC
#include "custom_application_monitor.h"
#endif
#if (CONFIG_ENABLED(CUSTOM_APPLICATION_001)) || (CONFIG_ENABLED(CUSTOM_APPLICATION_002))
#include "get_capacity.h"
#define PPS_QUERY_TIME 50
#endif

const int              device_id                           = FUSB_DEVICE_TYPE;
static struct dpm_info fusb_dpm_objects[TYPE_C_PORT_COUNT] = {0};
struct dpm_t           DPM                                 = {0};

/* VIF information */
#if (DEVICE_TYPE != FUSB15101)
extern PORT_VIF_T port1_vif;
extern PORT_VIF_T port2_vif;
#else
extern PORT_VIF_T port_vif;
#endif
#if DEVICE_TYPE != FUSB15101
PORT_VIF_T *dpm_vif[TYPE_C_PORT_COUNT] = {&port1_vif, &port2_vif};
#else
PORT_VIF_T       *dpm_vif[TYPE_C_PORT_COUNT] = {&port_vif};
#endif
/* VDM SVID drivers */
extern VDM_ITEM_T vdm_pd_driv;
extern VDM_ITEM_T vdm_cable_svid_driv;
extern VDM_ITEM_T vdm_cable_tbt3_driv;
extern VDM_ITEM_T vdm_dp_driv;
extern VDM_ITEM_T vendor_driv;

/*Other extern function*/
extern void dpm_register_callbacks();

#if CONFIG_ENABLED(DCDC)
/*DCDC CFG*/
extern DCDC_MASTER_T DCDC_master[];
extern DCDC_DATA_T   DCDC_data[];
#endif

uint8_t num_attached_ports(DPM_T *dpm)
{
    uint8_t num_ports = 0;
    for (uint8_t i = 0; i < TYPE_C_PORT_COUNT; i++)
    {
        enum tc_state_t state = port_tc_state(dpm->port_tcpd[i]);
#if (CONFIG_ENABLED(DRP))
        if (state == AttachedSource || state == AttachedSink)
        {
            num_ports++;
        }
#elif CONFIG_ENABLED(SRC)
        if (state == AttachedSource)
        {
        	num_ports++;
        }
#else
        if (state == AttachedSink)
        {
        	num_ports++;
        }
#endif
    }
    return num_ports;
}

void dpm_vbus_monitor_set_pps_cv(struct port_tcpd *dev, uint32_t vbus_sink_selected)
{
    struct dpm_info *dpm = (struct dpm_info *)dev->dpm_info_priv;
    fusbdev_tcpd_set_vbus_alarm(dev->tcpd_device, PERCENT(vbus_sink_selected, CL_PERCENT), 0);
    dpm->pps_mode = PPS_MODE_CV;
}

void dpm_vbus_monitor_set_pps_cl(struct port_tcpd *dev, uint32_t vbus_sink_selected)
{
    struct dpm_info *dpm = (struct dpm_info *)dev->dpm_info_priv;
    fusbdev_tcpd_set_vbus_alarm(dev->tcpd_device, PERCENT(PPS_MIN_V, UV_PERCENT),
                                PERCENT(vbus_sink_selected, CV_PERCENT));
    dpm->pps_mode = PPS_MODE_CL;
}

#if (DEVICE_TYPE == FUSB15200 || DEVICE_TYPE == FUSB15201 || DEVICE_TYPE == FUSB15201P)
#if (!CONFIG_ENABLED(POWER_SHARING))
/* Function is meant for assured power */
static void dpm_update_port_pdp(struct dpm_info *dpm_info, uint32_t new_pdp)
{
    if (dpm_info->port_pdp != new_pdp)
    {
        dpm_info->port_pdp = new_pdp;
        port_set_contract_valid(dpm_info->dpm->port_tcpd[dpm_info->port_id], false);
        port_transmit_source_caps(dpm_info->dpm->port_tcpd[dpm_info->port_id]);
    }
}
#endif

#if (!CONFIG_ENABLED(CUSTOM_APPLICATION_INCLUDE)) && (!CONFIG_ENABLED(CUSTOM_APPLICATION_001))                           \
    && (!CONFIG_ENABLED(CUSTOM_APPLICATION_002))
static void dpm_compensate_power_profile(struct batt_ntc_monitor *monitor)
{
    uint32_t new_capacity;
    uint8_t  i;
    DPM_T   *dpm = (struct dpm_t *)monitor->dpm_priv;
#if (CONFIG_ENABLED(POWER_SHARING))
    POWER_RESERVE_T *ps = &dpm->ps_dev;
#endif
    /** Port 1 **/
    /* Disable / Enable on critical temp */
    if (monitor->p1_temperature_range_debounced == TEMP_RANGE_CRIT)
    {
        port_set_disabled(((struct dpm_t *)monitor->dpm_priv)->port_tcpd[HAL_USBPD_CH_A]);
    }
    else
    {
        if (port_tc_state(((struct dpm_t *)monitor->dpm_priv)->port_tcpd[HAL_USBPD_CH_A])
            == Disabled)
        {
            port_set_unattached(((struct dpm_t *)monitor->dpm_priv)->port_tcpd[HAL_USBPD_CH_A]);
        }
    }

    /* Set PDP based on temp */
    if (monitor->p1_temperature_range_debounced == TEMP_RANGE_L)
    {
        new_capacity =
            ((struct dpm_t *)monitor->dpm_priv)->port_tcpd[HAL_USBPD_CH_A]->vif->PD_Power_as_Source;
    }
    else if (monitor->p1_temperature_range_debounced == TEMP_RANGE_H)
    {
        new_capacity = HIGH_TEMP_POWER;
    }
#if (!CONFIG_ENABLED(POWER_SHARING))
    dpm_update_port_pdp(((struct dpm_t *)monitor->dpm_priv)->dpm_info[HAL_USBPD_CH_A],
                        new_capacity);
#else
    ps_update_shared_capacity(ps, new_capacity);
#endif
    /** Port 2 **/
    /* Disable / Enable on critical temp */
    if (monitor->p2_temperature_range_debounced == TEMP_RANGE_CRIT)
    {
        port_set_disabled(((struct dpm_t *)monitor->dpm_priv)->port_tcpd[HAL_USBPD_CH_B]);
    }
    else
    {
        if (port_tc_state(((struct dpm_t *)monitor->dpm_priv)->port_tcpd[HAL_USBPD_CH_B])
            == Disabled)
        {
            port_set_unattached(((struct dpm_t *)monitor->dpm_priv)->port_tcpd[HAL_USBPD_CH_B]);
        }
    }

    /* Set PDP based on temp */
    if (monitor->p2_temperature_range_debounced == TEMP_RANGE_L)
    {
        new_capacity =
            ((struct dpm_t *)monitor->dpm_priv)->port_tcpd[HAL_USBPD_CH_B]->vif->PD_Power_as_Source;
    }
    else if (monitor->p2_temperature_range_debounced == TEMP_RANGE_H)
    {
        new_capacity = HIGH_TEMP_POWER;
    }
#if (!CONFIG_ENABLED(POWER_SHARING))
    dpm_update_port_pdp(((struct dpm_t *)monitor->dpm_priv)->dpm_info[HAL_USBPD_CH_B],
                        new_capacity);
#else
    ps_update_shared_capacity(ps, new_capacity);
#endif
}
#else
#if (CONFIG_ENABLED(CUSTOM_APPLICATION_002) || CONFIG_ENABLED(CUSTOM_APPLICATION_001))

static void dpm_update_source_caps(struct power_sharing_dev *ps, uint32_t new_capacity)
{
    for (int port_id = 0; port_id < NUM_TYPEC_PORTS; port_id++)
    {
        int index = 0;
        for (uint32_t i = 0; i < ps->port_tcpd[port_id]->vif->Num_Src_PDOs; i++)
        {
            if (new_capacity == 30000 && i == 3)
            {
                index++;
            }
            else if (new_capacity != 30000 && i == 4)
            {
                index++;
            }
            ps->port[port_id]->caps_source[i] = ps->port_tcpd[port_id]->vif->src_caps[index];
            index++;
        }
    }
}

static void dpm_compensate_power_profile(struct batt_ntc_monitor *monitor)
{
    uint32_t         new_capacity = 0;
    uint8_t          i;
    DPM_T           *dpm          = monitor->dpm_priv;
    POWER_RESERVE_T *ps           = &dpm->ps_dev;
    uint8_t          num_attached = num_attached_ports(dpm);
    new_capacity = get_capacity(dpm->port_tcpd[0], monitor->battery_range_debounced,
                                monitor->temperature_range_debounced);
    if (monitor->battery_range_debounced == BATT_RANGE_CRIT
        || monitor->temperature_range_debounced == TEMP_RANGE_CRIT)
    {
        ps->shared_capacity = 0;
        for (i = 0; i < NUM_TYPEC_PORTS; i++)
        {
#if (CONFIG_ENABLED(CUSTOM_APPLICATION_INCLUDE))
            dpm->dpm_info[i]->ocp_dev.ocp_state = PDPS_OCP_DISABLED;
#endif
            port_set_disabled(ps->port_tcpd[i]);
        }
    }
    else
    {
        if (ps->shared_capacity == 0 && monitor->battery_range_debounced != BATT_RANGE_CRIT
            && monitor->battery_range_debounced != TEMP_RANGE_CRIT)
        {
            monitor->left_crit_range = false;
            for (uint8_t i = 0; i < NUM_TYPEC_PORTS; i++)
            {
                struct port *port = ps->port_tcpd[i]->port_tcpd_priv;
#if (CONFIG_ENABLED(CUSTOM_APPLICATION_001))
                port->src_current = CC_RP_3p0A;
#elif (CONFIG_ENABLED(CUSTOM_APPLICATION_002))
                if (new_capacity >= 30000)
                {
                    port->src_current = CC_RP_3p0A;
                }
                else
                {
                    port->src_current = CC_RP_1p5A;
                }
#endif
#if (CONFIG_ENABLED(CUSTOM_APPLICATION_INCLUDE))
                pdps_ocp_reset(&dpm->dpm_info[i]->ocp_dev, dpm->port_tcpd[i]->tcpd_device,
                               dpm->port_bc1p2[i], dpm->port_tcpd[i]);
#else
                port_set_error_recovery(ps->port_tcpd[i]);
#endif
            }
        }
        else
        {
            for (i = 0; i < NUM_TYPEC_PORTS; i++)
            {
                if (port_tc_state(ps->port_tcpd[i]) == Disabled)
                {
#if (CONFIG_ENABLED(CUSTOM_APPLICATION_INCLUDE))
                    pdps_ocp_reset(&dpm->dpm_info[i]->ocp_dev, dpm->port_tcpd[i]->tcpd_device,
                                   dpm->port_bc1p2[i], dpm->port_tcpd[i]);
#else
                    port_set_error_recovery(ps->port_tcpd[i]);
#endif
                }
            }
        }
    }

    if (new_capacity == ps->shared_capacity)
    {
        return;
    }
    if (new_capacity == 15000)
    {
        for (i = 0; i < NUM_TYPEC_PORTS; i++)
        {
            struct port *port = ps->port_tcpd[i]->port_tcpd_priv;
            port_tcpd_pd_disable(ps->port_tcpd[i]);
            //no setting RP current for GM
            port->src_current = CC_RP_1p5A;
#if (CONFIG_ENABLED(CUSTOM_APPLICATION_INCLUDE))
            pdps_ocp_reset(&dpm->dpm_info[i]->ocp_dev, &dpm->ps_dev, dpm->port_bc1p2[i],
                           dpm->port_tcpd[i]);
#else
            port_set_error_recovery(ps->port_tcpd[i]);
#endif
            //port_tcpd_pd_disable(ps->port_tcpd[i]);

            port->is_contract_valid = false;
        }
    }
    /* Set current PD contract to invalid if going down in power */
    if (new_capacity < ps->shared_capacity)
    {
        for (i = 0; i < NUM_TYPEC_PORTS; i++)
        {
            struct port *port = ps->port_tcpd[i]->port_tcpd_priv;
            if (port->pd_active)
            {
                port->is_contract_valid = false;
                ps->available_power += ps->sink_cap[i].pdo_power;
                ps->sink_cap[i].pdo_power = 15000;
                ps->available_power -= ps->sink_cap[i].pdo_power;
            }
        }
    }
    else if (new_capacity > 15000 && ps->shared_capacity <= 15000)
    {
        /* Reset available power to shared capacity - num_ports x pdo_power*/
        ps->available_power = ps->shared_capacity - NUM_TYPEC_PORTS * 9000;
        for (i = 0; i < NUM_TYPEC_PORTS; i++)
        {
            struct port *port = ps->port_tcpd[i]->port_tcpd_priv;
            //reset RP to correct value - PDPS device is Source device
            port->src_current = CC_RP_3p0A;
            if (port_tc_state(ps->port_tcpd[i]) == AttachedSource)
            {
#if (CONFIG_ENABLED(CUSTOM_APPLICATION_INCLUDE))
                pdps_ocp_reset(&dpm->dpm_info[i]->ocp_dev, dpm->port_tcpd[i]->tcpd_device,
                               dpm->port_bc1p2[i], dpm->port_tcpd[i]);
                dpm->dpm_info[i]->ocp_dev.ocp_state = PDPS_OCP_DISABLED;
#else
                port_set_error_recovery(ps->port_tcpd[i]);
#endif
            }
#if (CONFIG_ENABLED(CUSTOM_APPLICATION_INCLUDE))
            pdps_ocp_reset(&dpm->dpm_info[i]->ocp_dev, &dpm->ps_dev, dpm->port_bc1p2[i],
                           dpm->port_tcpd[i]);
#else
            port_set_error_recovery(ps->port_tcpd[i]);
#endif
            port->is_contract_valid   = false;
            ps->sink_cap[i].pdo_power = 9000;
        }
    }
#if (CONFIG_ENABLED(POWER_SHARING))
    /* Recalculate power profile */
    ps->available_power += new_capacity - ps->shared_capacity;
    ps_update_shared_capacity(ps, new_capacity);
    if (new_capacity > 15000)
    {
        ps->pdp_rating = MIN(ps->port_tcpd[0]->vif->PD_Power_as_Source, new_capacity - 9000);
    }
#else
    /* Need  */
    ps->shared_capacity = new_capacity;
    for (i = 0; i < NUM_TYPEC_PORTS; i++)
    {
        dpm_update_port_pdp(dpm->dpm_info[i], (new_capacity / 2000) * 1000);
    }
#endif
    gSleepInt = true;
}

#endif
#endif
#endif
#if CONFIG_ENABLED(CUSTOM_APPLICATION_INCLUDE)

#endif
/* Initializes the dpm tcpd port data object's members */
void dpm_init(void)
{
    struct dpm_info *dpm_info;
    struct dpm_t    *dpm = &DPM;

#if CONFIG_ENABLED(POWER_SHARING) || CONFIG_ENABLED(CUSTOM_APPLICATION)                                          \
    || CONFIG_ENABLED(CUSTOM_APPLICATION_001) || CONFIG_ENABLED(CUSTOM_APPLICATION_002)
    struct power_sharing_dev *ps_dev = (struct power_sharing_dev *)&DPM.ps_dev;
#endif

#if (DEVICE_TYPE == FUSB15101)
    ps_init();
#endif
    /*initialize TCPD info*/
    for (int i = 0; i < TYPE_C_PORT_COUNT; i++)
    {
        dpm_info = &fusb_dpm_objects[i];
        dpm->port_tcpd[i] =
            (struct port_tcpd *)dpm_port_init((void *)dpm_info, i, (void *)dpm_vif[i]);
        dpm->port_tcpd[i]->tcpd_device = device_tcpd_init(i, device_id, true, false);
#if (DEVICE_TYPE == FUSB15101)
        dpm->port_tcpd[i]->tcpd_device->fusb15101_state = fusb15101_state_init();
        dpm->port_tcpd[i]->tcpd_device->fusb15101_state->port_dev = dpm->port_tcpd[i];
#endif
        port_tcpd_init(dpm->port_tcpd[i]->port_tcpd_priv);
#if (CONFIG_ENABLED(SLEEP))
        device_sleep_init(dpm->port_tcpd[i]->tcpd_device);
#endif
        dpm_info->port_pdp = dpm->port_tcpd[i]->vif->PD_Power_as_Source;
#if CONFIG_ENABLED(VDM) && !CONFIG_ENABLED(VDM_MINIMAL)
#if (RESPONDS_TO_DISCOV_SOP || ATTEMPTS_DISCOV_SOP || ATTEMPTS_DISCVID_SOP_P_FIRST)
            /* Register vdm driver needed for supporting vdms*/
            vdm_list_register(dpm->port_tcpd[i], &vdm_pd_driv);
#endif
#if CONFIG_ENABLED(USB4)
            /* Simple check for if we are not USB4 peripheral */
            if (!dpm->port_tcpd[i]->vif->Is_USB4_Peripheral)
            {
				vdm_list_register(dpm->port_tcpd[i], &vdm_cable_svid_driv);
				vdm_list_register(dpm->port_tcpd[i], &vdm_cable_tbt3_driv);
            }
#endif
#if (MODAL_OPERATION_SUPPORTED)
            /* Register vdm driver for display port */
            vdm_list_register(dpm->port_tcpd[i], &vdm_dp_driv);
#endif
            /* Register vdm driver for vendor specific usage */
            vdm_list_register(dpm->port_tcpd[i], &vendor_driv);
#endif
#if CONFIG_ENABLED(LEGACY_CHARGING)
        dpm->port_bc1p2[i]               = dpm_legacy_init(dpm->port_tcpd[i]->dpm_info_priv, i);
        dpm->port_bc1p2[i]->bc1p2_device = device_bc1p2_init(i, device_id, true);
        port_bc1p2_init(dpm->port_bc1p2[i]->port_bc1p2_priv, BC1P2_MODE_PROVIDER_DCP);
#endif

#if CONFIG_ENABLED(POWER_SHARING) || CONFIG_ENABLED(ALT_BATT_NTC)
        ps_dev->port_tcpd[i] = dpm->port_tcpd[i];
        ps_dev->port[i]      = dpm->port_tcpd[i]->port_tcpd_priv;
#endif
#if CONFIG_ENABLED(CUSTOM_APPLICATION_INCLUDE)
        pdps_ocp_init(&dpm_info->ocp_dev);
        dpm_ocp_init();
#endif
        dpm_info->dpm     = dpm;
        dpm_info->port_id = i;
        dpm->dpm_info[i]  = dpm_info;
    }

#if CONFIG_ENABLED(POWER_SHARING)
    port_ps_dev_init(ps_dev);
#if !CONFIG_ENABLED(CONST_EVENTS)
    port_ps_dev_register_callbacks();
#endif
#if (CONFIG_ENABLED(BATT_NTC) || CONFIG_ENABLED(ALT_BATT_NTC))
    //make sure to init power to 0 to make sure things are initialized properly.
    ps_dev->shared_capacity = 0;
#endif
#endif

#if CONFIG_ENABLED(DCDC)
    dpm->port_dcdc              = dpm_dcdc_init(dpm);
    dpm->port_dcdc->dcdc_device = device_dcdc_init(device_id, true);
    port_dcdc_dev_init(dpm->port_dcdc->port_dcdc_priv, &DCDC_master[0], &DCDC_data[0]);
#if (DEVICE_TYPE == FUSB15200)
    fusbdev_set_gpio_pin(HAL_GPIO16, HAL_GPIO_HIGH, HAL_GPIO_SHARED);
#endif
#endif
#if !CONFIG_ENABLED(CONST_EVENTS)
    dpm_register_callbacks();
#endif
#if ((DEVICE_TYPE == FUSB15201) || (DEVICE_TYPE == FUSB15201P)) && CONFIG_ENABLED(ALT_BATT_NTC)
    bnm_batt_ntc_init((void *)dpm, &dpm->monitor, dpm_compensate_power_profile);
#endif

#if CONFIG_ENABLED(CUSTOM_APPLICATION)
    custom_application_dpm_init();
#endif
}

void dpm_reinit(void)
{
	timer_init(HAL_SYSTICK_Count, 10, 1);
#if CONFIG_ENABLED(CUSTOM_APPLICATION)
    ADC_DRIVER.Configure(ADC_Interface_ADC_CTRL_adc_loop_Msk
                         | ADC_Interface_ADC_CTRL_adc_sleep_op_Msk);
#else
    ADC_DRIVER.Configure(ADC_Interface_ADC_CTRL_adc_loop_Msk);
#endif
    /* enable averaging ADC */
    ADC_Interface->ADC_AVG = ADC_Interface_ADC_AVG_adc_avg_Msk;
	struct dpm_info *dpm_info;
	    struct dpm_t    *dpm = &DPM;

	#if CONFIG_ENABLED(POWER_SHARING) || CONFIG_ENABLED(CUSTOM_APPLICATION)                                          \
	    || CONFIG_ENABLED(CUSTOM_APPLICATION_001) || CONFIG_ENABLED(CUSTOM_APPLICATION_002)
	    struct power_sharing_dev *ps_dev = (struct power_sharing_dev *)&DPM.ps_dev;
	#endif

	#if (DEVICE_TYPE == FUSB15101)
	    ps_init();
	#endif
	    /*initialize TCPD info*/
	    for (int i = 0; i < TYPE_C_PORT_COUNT; i++)
	    {
	        dpm_info = &fusb_dpm_objects[i];
	        dpm->port_tcpd[i] =
	            (struct port_tcpd *)dpm_port_init((void *)dpm_info, i, (void *)dpm_vif[i]);
	        dpm->port_tcpd[i]->tcpd_device = device_tcpd_init(i, device_id, true, true);
	#if (DEVICE_TYPE == FUSB15101)
	        fusb15101_state_init(dpm->port_tcpd[i]->tcpd_device->fusb15101_state);
	        dpm->port_tcpd[i]->tcpd_device->fusb15101_state->port_dev = dpm->port_tcpd[i];
	#endif
	        port_tcpd_init(dpm->port_tcpd[i]->port_tcpd_priv);
	#if (CONFIG_ENABLED(SLEEP))
	        device_sleep_init(dpm->port_tcpd[i]->tcpd_device);
	#endif
	        dpm_info->port_pdp = dpm->port_tcpd[i]->vif->PD_Power_as_Source;
	        if (CONFIG_VDM)
	        {
	#if (RESPONDS_TO_DISCOV_SOP || ATTEMPTS_DISCOV_SOP || ATTEMPTS_DISCVID_SOP_P_FIRST)
	            /* Register vdm driver needed for supporting vdms*/
	            vdm_list_register(dpm->port_tcpd[i], &vdm_pd_driv);
	#endif
	#if (MODAL_OPERATION_SUPPORTED)
	            /* Register vdm driver for display port */
	            vdm_list_register(dpm->port_tcpd[i], &vdm_dp_driv);
	#endif
	            /* Register vdm driver for vendor specific usage */
	            vdm_list_register(dpm->port_tcpd[i], &vendor_driv);
	        }
	#if CONFIG_ENABLED(LEGACY_CHARGING)
	        dpm->port_bc1p2[i]               = dpm_legacy_init(dpm->port_tcpd[i]->dpm_info_priv, i);
	        dpm->port_bc1p2[i]->bc1p2_device = device_bc1p2_init(i, device_id, true);
	        port_bc1p2_init(dpm->port_bc1p2[i]->port_bc1p2_priv, BC1P2_MODE_PROVIDER_DCP);
	#endif

	#if CONFIG_ENABLED(POWER_SHARING) || CONFIG_ENABLED(ALT_BATT_NTC)
	        ps_dev->port_tcpd[i] = dpm->port_tcpd[i];
	        ps_dev->port[i]      = dpm->port_tcpd[i]->port_tcpd_priv;
	#endif
	#if CONFIG_ENABLED(CUSTOM_APPLICATION)
	        pdps_ocp_init(&dpm_info->ocp_dev);
	        dpm_ocp_init();
	#endif
	        dpm_info->dpm     = dpm;
	        dpm_info->port_id = i;
	        dpm->dpm_info[i]  = dpm_info;
	    }

	#if CONFIG_ENABLED(POWER_SHARING)
	    port_ps_dev_init(ps_dev);
#if !CONFIG_ENABLED(CONST_EVENTS)
	    port_ps_dev_register_callbacks();
#endif
	#if (CONFIG_ENABLED(BATT_NTC) || CONFIG_ENABLED(ALT_BATT_NTC))
	    //make sure to init power to 0 to make sure things are initialized properly.
	    ps_dev->shared_capacity = 0;
	#endif
	#endif

	#if CONFIG_ENABLED(DCDC)
	    dpm->port_dcdc              = dpm_dcdc_init(dpm);
	    dpm->port_dcdc->dcdc_device = device_dcdc_init(device_id, true);
	    port_dcdc_dev_init(dpm->port_dcdc->port_dcdc_priv, &DCDC_master[0], &DCDC_data[0]);
	#if (DEVICE_TYPE == FUSB15200)
	    fusbdev_set_gpio_pin(HAL_GPIO16, HAL_GPIO_HIGH, HAL_GPIO_SHARED);
	#endif
	#endif
	    dpm_register_callbacks();
	#if ((DEVICE_TYPE == FUSB15201) || (DEVICE_TYPE == FUSB15201P)) && CONFIG_ENABLED(ALT_BATT_NTC)
	    bnm_batt_ntc_init((void *)dpm, &dpm->monitor, dpm_compensate_power_profile);
	#endif

	#if CONFIG_ENABLED(CUSTOM_APPLICATION)
	    __enable_irq();
	    PDPS_DRIVER pdps_driver;
	#if (DEVICE_TYPE == FUSB15201)
	    pdps_driver.i2cNum = 3; /*POWERSUPPLY I2C */
	#elif (DEVICE_TYPE == FUSB15201P)
	    pdps_driver.i2cNum       = 1; /*POWERSUPPLY I2C */
	#endif
	    pdps_driver.numPorts = 2; /*Number of ports */
	    pdpsInit(&pdps_driver);
	#endif
}

void dpm_device_init(void)
{
    DPM.fusb_driv = 0;
    /*get abstraction driver for DPM specific access*/
    DPM.fusb_driv = device_init(device_id);
}
#if CONFIG_ENABLED(BATT_NTC) || CONFIG_ENABLED(ALT_BATT_NTC)
void dpm_mitigation_processing(struct batt_ntc_monitor *monitor)
{
    struct dpm_t *dpm = monitor->dpm_priv;
#if (CONFIG_ENABLED(ALT_BATT_NTC))
    if (monitor->battery_range_debounced == BATT_RANGE_CRIT
        || monitor->temperature_range_debounced == TEMP_RANGE_CRIT)
    {
        for (int i = 0; i < NUM_TYPEC_PORTS; i++)
        {
            struct port *port = dpm->port_tcpd[i]->port_tcpd_priv;
            port->tc_enabled  = false;
        }
    }
    else if (dpm->ps_dev.shared_capacity == 15000)
    {
        for (int i = 0; i < NUM_TYPEC_PORTS; i++)
        {
            struct port *port = dpm->port_tcpd[i]->port_tcpd_priv;
            port->tc_enabled  = true;
            port->src_current = CC_RP_1p5A;
            port_tcpd_pd_disable(dpm->port_tcpd[i]);
        }
    }
    else
    {
        for (int i = 0; i < NUM_TYPEC_PORTS; i++)
        {
            struct port *port = dpm->port_tcpd[i]->port_tcpd_priv;
            port->tc_enabled  = true;
            port->src_current = CC_RP_3p0A;
        }
    }
#else
    if (monitor->p1_temperature_range_debounced == TEMP_RANGE_CRIT
        || monitor->p2_temperature_range_debounced == TEMP_RANGE_CRIT)
    {
        for (int i = 0; i < NUM_TYPEC_PORTS; i++)
        {
            struct port *port = dpm->port_tcpd[i]->port_tcpd_priv;
            //port->src_current = CC_RP_1p5A;
            port->tc_enabled = false;
        }
    }
    else if (dpm->ps_dev.shared_capacity == 15000 || dpm->dpm_info[0]->port_pdp == 7500)
    {
        for (int i = 0; i < NUM_TYPEC_PORTS; i++)
        {
            struct port *port = dpm->port_tcpd[i]->port_tcpd_priv;
            port->src_current = CC_RP_1p5A;
            port->tc_enabled  = true;
            port_tcpd_pd_disable(dpm->port_tcpd[i]);
        }
    }
    else
    {
        for (int i = 0; i < NUM_TYPEC_PORTS; i++)
        {
            struct port *port = dpm->port_tcpd[i]->port_tcpd_priv;
            port->src_current = CC_RP_3p0A;
            port->tc_enabled  = true;
        }
    }
#endif
}
#endif
#if CONFIG_ENABLED(SRC)
void dpm_pps_monitor(struct port_tcpd *dev)
{
    struct dpm_info *dpm_info     = (struct dpm_info *)dev->dpm_info_priv;
    struct port     *port    = dev->port_tcpd_priv;
    struct dpm_t* dpm = dpm_info->dpm;
#if (TYPE_C_PORT_COUNT == 2)
    struct port * otherPort = dpm->port_tcpd[(dev->port_tcpd_id+1)%NUM_TYPEC_PORTS]->port_tcpd_priv;
#endif
    if (dpm_info->pps_active)
    {
        uint32_t vbus               = fusbdev_tcpd_vbus_value(dev->tcpd_device);
        uint32_t vbus_sink_selected = port_sink_selected_voltage(dev);
        if (vbus <= PERCENT(PPS_MIN_V, UV_PERCENT)
			&& (dpm_info->pps_mode == PPS_MODE_CL))
		{
#if (DEVICE_TYPE == FUSB15101)
        	if (!dev->tcpd_device->fusb15101_state->UVPDetected)
        	{
        		timer_start(&port->timers[UVP_DBNC_TIMER], ktUVPDebounce);
        	}
        	dev->tcpd_device->fusb15101_state->UVPDetected = true;
        	if (timer_expired(&port->timers[UVP_DBNC_TIMER]))
        	{
        		port->uvp_active = true;
        		port_send_hard_reset(dev);
        	}
#else
			port_set_error_recovery(dev);
#endif
		}
#if (DEVICE_TYPE == FUSB15101)
        else
        {
        	dev->tcpd_device->fusb15101_state->UVPDetected = false;
        }
#endif
#if (TYPE_C_PORT_COUNT == 2)
        if (!(port->policy_sending_message || otherPort->policy_sending_message) &&
        		 !(port->policy_state == PE_SRC_Transition_Supply || otherPort->policy_state == PE_SRC_Transition_Supply))
#else
        if (!(port->policy_sending_message) && !(port->policy_state == PE_SRC_Transition_Supply))
#endif
        {
#if (CONFIG_ENABLED(CUSTOM_APPLICATION))
			bool old_pps_mode = dpm_info->pps_mode;
			pdpsReadFaultStatus();
			pdpsReadFaultStatus();
			MPQ4260_STATUS_FAULTS fault;
			fault = pdpsGetFaultStatus(dev->port_tcpd_id);
			if (fault.IOUT_OC_FAULT)
			{
				/* means PPS is active and fault triggered, switch to cc mode */
				dpm_vbus_monitor_set_pps_cl(dev, vbus_sink_selected);
			}
			else if (old_pps_mode == PPS_MODE_CL)
			{
				/* means fault is not triggered, swap to CV mode */
				dpm_vbus_monitor_set_pps_cv(dev, vbus_sink_selected);
			}
			if (old_pps_mode != dpm_info->pps_mode)
			{
				port->msgtx.booltxmsg.alert = true;
			}
			//handled in ocp_processing
#elif (DEVICE_TYPE == FUSB15101)
			HAL_CC_CV_CCSTAT_T cc_status = CC_CV_DRIVER.CCStatus();
			/*check if vbus is below CL limit, re-enable the low threshold alarm to PPS shutdown range*/
			if (cc_status.cc_mode)
			{
				dpm_vbus_monitor_set_pps_cl(dev, vbus_sink_selected);

				/*check if vbus is still above CV limit, re-enable low threshold alarm to CL limit*/
			}
			else
			{
				dpm_vbus_monitor_set_pps_cv(dev, vbus_sink_selected);
			}
#else
			/*check if vbus is below CL limit, re-enable the low threshold alarm to PPS shutdown range*/
			if (vbus <= PERCENT(vbus_sink_selected, CL_PERCENT))
			{
				dpm_vbus_monitor_set_pps_cl(dev, vbus_sink_selected);

				/*check if vbus is still above CV limit, re-enable low threshold alarm to CL limit*/
			}
			else if (vbus >= PERCENT(vbus_sink_selected, CV_PERCENT))
			{
			    dpm_vbus_monitor_set_pps_cv(dev, vbus_sink_selected);
			}
#endif
        }
    }
	else
	{
		bool old_pps_mode = dpm_info->pps_mode;
		if (old_pps_mode == PPS_MODE_CL)
		{
		    dpm_vbus_monitor_set_pps_cv(dev, port->sink_selected_voltage);
		}
	}
}
#endif
void dpm_typec_fault_detection(struct port_tcpd *port_tcpd)
{
    if (fusbdev_tcpd_cc_check_error(port_tcpd->tcpd_device))
    {
        port_set_disabled(port_tcpd);
        /*TODO: need to take out the typeC parameters here*/
        fusbdev_tcpd_set_cc(port_tcpd->tcpd_device, CC1 | CC2, CC_TERM_RD, CC_RP_NOCHANGE,
                            DRP_MODE_NOCHANGE);
        fusbdev_tcpd_initialize(port_tcpd->tcpd_device, true, false);
    }
}
#if DEVICE_TYPE == FUSB15101
void dpm_min_voltage_monitor(struct port_tcpd *dev)
{
	struct port * port = (struct port *)dev->port_tcpd_priv;
	if (port->tc_state == AttachedSource && port->policy_state != PE_SRC_Startup && port->policy_state != PE_SRC_Transition_To_Default)
	{
        uint32_t vbus               = fusbdev_tcpd_vbus_pps_value(dev->tcpd_device);
        if (vbus <= PERCENT(4000, 100))
        {
        	dev->tcpd_device->fusb15101_state->UVPDetected = true;
        	dev->tcpd_device->fusb15101_state->curr_state.current_fault_state = FAULT_STATE_FAULT;
        	timer_disable(&(dev->tcpd_device->fusb15101_state->timers[FAULT_TIMER]));
        }
	}
}
#endif
void dpm_exec()
{
    struct dpm_t *dpm   = &DPM;
    bool          yield = true;

    do
    {
        yield = true;
#if (DEVICE_TYPE == FUSB15101)
        vbus_sm();
        fault_sm();
#if CONFIG_ENABLED(MOIST_DETECTION)
        moist_sm();
#endif
#endif
#if CONFIG_ENABLED(DCDC)
#if DEVICE_TYPE == FUSB15200 || (DEVICE_TYPE == FUSB15201) || (DEVICE_TYPE == FUSB15201P)
        port_process_dc_dc(dpm->port_dcdc->port_dcdc_priv);

#endif
#endif

#if (CONFIG_ENABLED(BATT_NTC) || CONFIG_ENABLED(ALT_BATT_NTC))
#if DEVICE_TYPE == FUSB15200 || (DEVICE_TYPE == FUSB15201) || (DEVICE_TYPE == FUSB15201P)
        bnm_monitor_batt_ntc(&dpm->monitor);
        dpm_mitigation_processing(&dpm->monitor);
#if (CONFIG_ENABLED(CUSTOM_APPLICATION))
        dpm_ocp_processing();
#endif
#endif
#endif
#if CONFIG_ENABLED(POWER_SHARING)
        port_ps_dev_processing(&dpm->ps_dev);
#endif
        for (int i = 0; i < TYPE_C_PORT_COUNT; i++)
        {
            dpm_typec_fault_detection(dpm->port_tcpd[i]);
#if CONFIG_ENABLED(CUSTOM_APPLICATION)
            pdps_ocp_sm(&dpm->dpm_info[i]->ocp_dev, &dpm->ps_dev, dpm->port_bc1p2[i], dpm->port_tcpd[i]);
#endif
            port_sm(dpm->port_tcpd[i]);
            if (!port_yield(dpm->port_tcpd[i]))
            {
                yield = false;
            }
#if CONFIG_ENABLED(SRC)
            dpm_pps_monitor(dpm->port_tcpd[i]);
#if DEVICE_TYPE == FUSB15101
            dpm_min_voltage_monitor(dpm->port_tcpd[i]);
#endif
#endif
#if CONFIG_ENABLED(LEGACY_CHARGING)
            if (!dpm->dpm_info[i]->pd_dev_attached)
            {
                bc_sm(dpm->port_bc1p2[i]->port_bc1p2_priv);
                if (!bc_yield(dpm->port_bc1p2[i]->port_bc1p2_priv))
                {
                    yield = false;
                }
            }
#endif
        }
#if (CONFIG_ENABLED(AUTO_YIELD))
        yield = true;
#endif
    } while (!yield);
}

unsigned long dpm_next_wakeup()
{
    /* Compare different timers here and find the timer that
     * expires earliest. */
    struct dpm_t *dpm = &DPM;
    unsigned long min = ~0;
    unsigned int  i   = 0;
    unsigned int  r[NUM_WAKEUP_TIMERS];
    for (int x = 0; x < TCPD_TIMER; x++)
    {
        r[i++] = port_tcpd_next_schedule(dpm->port_tcpd[x]);
    }
#if CONFIG_ENABLED(LEGACY_CHARGING)
    for (int x = 0; x < LEGACY_TIMER; x++)
    {
        r[i++] = port_bc1p2_next_schedule(dpm->port_bc1p2[x]);
    }
#endif

#if (DEVICE_TYPE == FUSB15201) || (DEVICE_TYPE == FUSB15201P)
#if CONFIG_ENABLED(DCDC)
    /*TODO: to be enabled and replaced with properly ported function call port_dcdc_next_schedule*/
    r[i++] = dc_dc_min_timer(&DCDC_data[0]);
    r[i++] = dc_dc_min_timer(&DCDC_data[1]);
#endif
#if (CONFIG_ENABLED(BATT_NTC)) || ((CONFIG_ENABLED(ALT_BATT_NTC)))
    r[i++] = bnm_min_timer(&dpm->monitor);
#endif
#endif

    for (i = 0; i < NUM_WAKEUP_TIMERS; i++)
    {
        if (r[i] < min)
        {
            min = r[i];
        }
    }
    return min;
}

#if (DEVICE_TYPE == FUSB15101)
/* Called by PD engine */
bool fusb15101_set_vbus_source(struct fusb_tcpd_device *dev, unsigned mv, unsigned ma, PDO_T type)
{
    bool ret;

    switch (type)
    {
        case pdoTypeFixed:
            ret = fusb15101_set_vbus(mv, ADJUSTED_CURRENT(ma), CC_CV_Control_CVCC_CTRL1_OCP_MODE_CCLM,
                                     CC_CV_Control_CVCC_CTRL1_CDC_GAIN_0MV_PER_AMP,
                                     DAC_Interface_CV_DAC_CTRL_CCDAC_REFP_120P,
                                     CC_CV_Control_CVCC_CTRL1_CVUVP_REFP_90_PERCENT);
            break;
        case pdoTypeAugmented:
            ret = fusb15101_set_vbus(mv, ADJUSTED_CURRENT(ma), CC_CV_Control_CVCC_CTRL1_OCP_MODE_CCLM,
                                     CC_CV_Control_CVCC_CTRL1_CDC_GAIN_0MV_PER_AMP,
                                     DAC_Interface_CV_DAC_CTRL_CCDAC_REFP_100P,
                                     CC_CV_Control_CVCC_CTRL1_CVUVP_REFP_FIXED);
            break;
        default:
            ret = fusb15101_set_vbus(mv, ADJUSTED_CURRENT(ma), CC_CV_Control_CVCC_CTRL1_OCP_MODE_CCLM,
                                     CC_CV_Control_CVCC_CTRL1_CDC_GAIN_0MV_PER_AMP,
                                     DAC_Interface_CV_DAC_CTRL_CCDAC_REFP_120P,
                                     CC_CV_Control_CVCC_CTRL1_CVUVP_REFP_90_PERCENT);
            break;
    }

    return ret;
}

#endif
