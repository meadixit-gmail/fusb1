
/*
 * Copyright (c) 2020 ON Semiconductor. All rights reserved.
 *
 * This software and/or documentation is licensed by ON Semiconductor under
 * limited terms and conditions. The terms and conditions pertaining to the
 * software and/or documentation are available at
 *
 * http://www.onsemi.com/site/pdf/ONSEMI_T&C.pdf
 *
 * ON Semiconductor; Standard Terms and Conditions of Sale,
 * Section 8 Software.
 *
 *  DO NOT USE THIS SOFTWARE AND/OR DOCUMENTATION UNLESS YOU HAVE CAREFULLY
 * READ AND YOU AGREE TO THE LIMITED TERMS AND CONDITIONS. BY USING THIS
 * SOFTWARE AND/OR DOCUMENTATION, YOU AGREE TO THE LIMITED TERMS
 * AND CONDITIONS.
 */

#ifndef FSCPM_DPM_H_
#define FSCPM_DPM_H_

#include "port.h"
#include "observer.h"
#if (!CONFIG_ENABLED(ALT_BATT_NTC))
#include "batt_ntc_monitor.h"
#else
#include "pdps_batt_ntc_monitor.h"
#endif
#include "power_sharing.h"
#include "timer.h"
#if (DEVICE_TYPE == FUSB15101)
#include "fusb15101_sm.h"

#define DEFERRED_SLEEP_TIME_MS  30U

#endif

#if CONFIG_ENABLED(CUSTOM_APPLICATION)
#include "pdps_ocp.h"
#endif

#if CONFIG_ENABLED(VDM)
#include "vdm.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#define POWER_SHARING 1

#define FW_VERSION_1 0x7ab7
#define FW_VERSION_2 0x6a2e
#define FW_VERSION_3 0x3f34
#define FW_VERSION_4 0x19e2

#define FW_VERSION_SMALL                                                                           \
    ((FW_VERSION_4 & 0xF) | ((FW_VERSION_3 & 0x7) << 4) | ((FW_VERSION_2 & 1) << 7))
#define HIGH_BYTE(x) ((x) >> 8)
#define LOW_BYTE(x)  ((x)&0xFF)

#define HIGH_TEMP_POWER 15000 /* Reduced power for high temp */

#define NUM_WAKEUP_TIMERS (TCPD_TIMER + LEGACY_TIMER + DCDC_TIMER + BNM_TIMER)

    //typedef struct POWER_RESERVE_T DPM_PS_T;
    /*DPM struct to hold common information for both port e.g.: abstraction driver*/
    typedef struct dpm_t
    {
        struct dpm_info  *dpm_info[TYPE_C_PORT_COUNT];
        struct port_tcpd *port_tcpd[TYPE_C_PORT_COUNT];
#if CONFIG_ENABLED(DCDC)
        struct port_dcdc *port_dcdc;
#endif
#if CONFIG_ENABLED(POWER_SHARING) || CONFIG_ENABLED(ALT_BATT_NTC) || CONFIG_ENABLED(BATT_NTC)
        struct power_sharing_dev ps_dev;
#endif
#if CONFIG_ENABLED(LEGACY_CHARGING) || CONFIG_ENABLED(CUSTOM_APPLICATION)
        struct port_bc1p2 *port_bc1p2[TYPE_C_PORT_COUNT];
#endif
        struct fusb_driver_t *fusb_driv;
#if (DEVICE_TYPE == FUSB15201 || DEVICE_TYPE == FUSB15201P)
        struct batt_ntc_monitor monitor;
#endif
    } DPM_T;

    /* Struct to store dpm information*/
    typedef struct dpm_info
    {
        int        port_id;
        bool       pps_active;
        uint32_t   port_pdp;
        PPS_MODE_T pps_mode;
        bool       pd_dev_attached;
#if CONFIG_ENABLED(CUSTOM_APPLICATION)
        struct pdps_ocp ocp_dev;
        TICKTIMER_T pps_query_timer;
#endif
        DPM_T *dpm;
    } DPM_INFO_T;
    void dpm_init(void);
    void dpm_reinit(void);
#if (DEVICE_TYPE == FUSB15101)
    bool fusb15101_set_vbus_source(struct fusb_tcpd_device *dev, unsigned mv, unsigned ma,
                                   PDO_T type);
#endif

    void dpm_vbus_monitor_set_pps_cl(struct port_tcpd *dev, uint32_t vbus_sink_selected);
    
    void dpm_vbus_monitor_set_pps_cv(struct port_tcpd *dev, uint32_t vbus_sink_selected);
    
    
extern struct dpm_t           DPM;
#ifdef __cplusplus
}
#endif

#endif /** @} FSCPM_DPM_H_ */
