/*******************************************************************************
 * @file     batt_ntc_monitor.h
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
#ifndef _BATT_NTC_MONITOR_H_
#define _BATT_NTC_MONITOR_H_

#include "timer.h"
#include "pdps_ocp.h"
#ifdef __cplusplus
extern "C"
{
#endif

#define NUM_TYPEC_PORTS 2

/* Battery */
#if (DEVICE_TYPE == FUSB15201)
#define BATT_ADC_CH ADC_CH9
#elif (DEVICE_TYPE == FUSB15201P)
#define BATT_ADC_CH ADC_CH7
#endif
#define BATT_11_5V            388
#define BATT_9_0V             304
#define BATT_5_5V             185
#define BATT_HYSTERESIS_500mV 17
#define BATT_HYSTERESIS_750mV 25
#define BATT_DEBOUNCE         500 /* ms */

/* NTC */
#if (DEVICE_TYPE == FUSB15201)
#define TEMP_ADC_CH ADC_CH3
#elif (DEVICE_TYPE == FUSB15201P)
#define TEMP_ADC_CH ADC_CH3
#endif
#define TEMP_135C     25
#define TEMP_130C     28
#define TEMP_125C     32
#define TEMP_120C     36
#define TEMP_115C     40
#define TEMP_110C     45
#define TEMP_105C     51
#define TEMP_100C     58
#define TEMP_95C      66
#define TEMP_90C      76
#define TEMP_DEBOUNCE 50 /* ms */

    enum batt_range
    {
        BATT_RANGE_H,
        BATT_RANGE_M,
        BATT_RANGE_L,
        BATT_RANGE_CRIT
    };

    enum temp_range
    {
        TEMP_RANGE_H,
        TEMP_RANGE_M,
        TEMP_RANGE_L,
        TEMP_RANGE_CRIT
    };

    struct batt_ntc_monitor;

    typedef void (*compensate_ports_cb)(struct batt_ntc_monitor *monitor);

    struct batt_ntc_monitor
    {
        void               *dpm_priv;
        struct ticktimer_t  temp_debounce_timer;
        struct ticktimer_t  batt_debounce_timer;
        enum temp_range     temperature_range;
        enum temp_range     temperature_range_debounced;
        enum batt_range     battery_range;
        enum batt_range     battery_range_debounced;
        compensate_ports_cb compensate_cb;
        bool                left_crit_range;
        bool                battery_low_alarm;
        bool                battery_high_alarm;
        bool                temperature_low_alarm;
        bool                temperature_high_alarm;
    };
    extern struct batt_ntc_monitor *globalMonitor;
    void                            bnm_batt_ntc_init(void *dpm, struct batt_ntc_monitor *monitor,
                                                      compensate_ports_cb compensate_cb);
    void                            bnm_monitor_batt_ntc(struct batt_ntc_monitor *monitor);
    uint32_t                        bnm_min_timer(struct batt_ntc_monitor *monitor);

#ifdef __cplusplus
}
#endif
#endif /* _BATT_NTC_MONITOR_H_ */
