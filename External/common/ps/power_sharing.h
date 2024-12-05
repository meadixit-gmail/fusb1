/*******************************************************************************
 * @file     power_sharing.h
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

#ifndef _POWER_SHARING_H_
#define _POWER_SHARING_H_

#include "timer.h"
#include "pd_types.h"
#include "fusbdev_hal.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define BATT_STATUS_TIME 60000
#define PS_SCALE         1024

#define MIN_POWER_3A   15000
#define MIN_POWER_1_5A 7500

#define DEFAULT_BATT_CAP 108 /* 10.8 Wh or ~2800mAh Battery */

#define MAX(X, Y) (X) > (Y) ? (X) : (Y)

    enum CHARGE_STATUS_T
    {
        BATTERY_CHARGING    = 0,
        BATTERY_DISCHARGING = 1,
        BATTERY_IDLE        = 2,
    };

    struct BATTERY_T
    {
        uint32_t capacity;
        uint32_t soc;
        uint32_t charge_status;
    };

    struct SINK_CAP_T
    {
        uint32_t min_power;        // sink min req
        uint32_t op_power;         // sink operating req
        uint32_t max_power;        // sink max req
        uint32_t port_power;       // max offered pdp to port
        uint32_t advertised_power; // power advertised on port

        uint32_t pdo_power; //actual power reserved from shared_cap

        struct BATTERY_T battery[8];

        uint8_t  giveback;
        uint8_t  mismatch;
        uint32_t reserve;
        uint8_t  renegotiate;
        uint8_t  redistributing;
        uint8_t  mismatch_handling;
        uint8_t  pd_capable;

        TICKTIMER_T status_request_timer;
    };

    typedef struct power_sharing_dev
    {
        uint32_t          shared_capacity;
        uint32_t          pdp_rating;
        uint32_t          available_power; // shared_cap - all sink[x].pdo_power
        struct SINK_CAP_T sink_cap[TYPE_C_PORT_COUNT];

        uint32_t cv_thresh;   // %, e.g. 60 = 60%
        uint32_t default_cap; // assume pd2.0 sink has this battery capacity
        uint32_t default_soc; // assume pd2.0 sink has this soc

        uint32_t bist_shared_test_mode;
        struct port *port[TYPE_C_PORT_COUNT];
        struct port_tcpd *port_tcpd[TYPE_C_PORT_COUNT];
    } POWER_RESERVE_T;

    void port_ps_init(struct power_sharing_dev *power);
    void port_ps_register_callbacks(void);
    void port_ps_processing(struct power_sharing_dev *ps_dev);
    void ps_event_tc_attached(struct port_tcpd *dev, void *ctx);
    void ps_event_tc_detached(struct port_tcpd *dev, void *ctx);

    /* Returns smallest running timer used for resending Get Battery Status */
    uint32_t ps_min_timer(struct power_sharing_dev *ps_dev);

    /* Processes resending on Get Battery Status - called from main loop */
    void ps_processing(struct power_sharing_dev *ps_dev);
    /* Adjusts total system power for power sharing */
    void ps_update_shared_capacity(struct power_sharing_dev *ps_dev, uint32_t new_capacity);
    void ps_event_pd_get_src_cap(struct port_tcpd *dev, void *ctx);
    void ps_event_pd_req_cb(struct port_tcpd *dev, void *ctx);
    void ps_event_pd_new_contract(struct port_tcpd *dev, void *ctx);
    void ps_event_bist_shared_test_mode(struct port_tcpd *dev, void *ctx);
    void ps_event_pd_snk_cap(struct port_tcpd *dev, void *ctx);
    void ps_event_pd_snk_cap_ext(struct port_tcpd *dev, void *ctx);
    void ps_event_battery_cap(struct port_tcpd *dev, void *ctx);
    void ps_event_battery_status(struct port_tcpd *dev, void *ctx);
    void ps_event_pd_device(struct port_tcpd *dev, void *ctx);
uint32_t ps_max_contract_power(struct power_sharing_dev *ps_dev, uint8_t port_id);

#ifdef __cplusplus
}
#endif

#endif /** @} _POWER_SHARING_H_ */
