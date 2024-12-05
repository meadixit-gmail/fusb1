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
#if (!CONFIG_ALT_BATT_NTC)
#ifndef _BATT_NTC_MONITOR_H_
#define _BATT_NTC_MONITOR_H_

#include "timer.h"

#ifdef __cplusplus
extern "C"
{
#endif
#define NUM_TYPEC_PORTS 2
#define BATT_DEBOUNCE   500 /* ms */
#define TEMP1_ADC_CH    ADC_CH3
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
#define TEMP2_ADC_CH ADC_CH9
#elif (DEVICE_TYPE == FUSB15201P)
#define TEMP2_ADC_CH ADC_CH7
#endif
#define TEMP_DEBOUNCE 500 /* ms */

/* NTC Panasonic ERTJ0ER104 || 20k */
#define TEMP_135C 104
#define TEMP_125C 134
#define TEMP_115C 173
#define TEMP_105C 224

    enum temp_range
    {
        TEMP_RANGE_H,
        TEMP_RANGE_L,
        TEMP_RANGE_CRIT
    };

    struct batt_ntc_monitor;

    typedef void (*compensate_ports_cb)(struct batt_ntc_monitor *monitor);

    struct batt_ntc_monitor
    {
        void               *dpm_priv;
        struct ticktimer_t  p1_temp_debounce_timer;
        struct ticktimer_t  p2_temp_debounce_timer;
        enum temp_range     p1_temperature_range;
        enum temp_range     p1_temperature_range_debounced;
        enum temp_range     p2_temperature_range;
        enum temp_range     p2_temperature_range_debounced;
        compensate_ports_cb compensate_cb;
    };

    void     bnm_batt_ntc_init(void *, struct batt_ntc_monitor *monitor,
                               compensate_ports_cb compensate_cb);
    void     bnm_monitor_batt_ntc(struct batt_ntc_monitor *monitor);
    uint32_t bnm_min_timer(struct batt_ntc_monitor *monitor);

#ifdef __cplusplus
}
#endif
#endif /* _BATT_NTC_MONITOR_H_ */

#endif
