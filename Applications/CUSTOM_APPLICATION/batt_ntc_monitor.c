/*******************************************************************************
 * @file     batt_ntc_monitor.c
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
#include "pdps_batt_ntc_monitor.h"
#include "port.h"
#include "fusb15xxx.h"
#include "legacy_types.h"
#include "pd_types.h"
#include "fusbdev_hal.h"
struct batt_ntc_monitor *globalMonitor;
void                     bnm_batt_ntc_init(void *dpm_priv, struct batt_ntc_monitor *monitor,
                                           compensate_ports_cb compensate_cb)
{
    HAL_PORTCFG_T portCfg = {.an  = PORT_ANALOG,
                             .alt = PORT_ALTERNATE,
                             .pu  = PORT_PULLUP_DISABLE,
                             .pd  = PORT_PULLDOWN_DISABLE};
    globalMonitor         = monitor;
    monitor->dpm_priv     = dpm_priv;
    /* Init Battery Monitor */
#if (DEVICE_TYPE == FUSB15201)
    PORT_DRIVER.Configure(PORT15, &portCfg, HAL_GPIO_SHARED);
#elif DEVICE_TYPE == FUSB15201P
    PORT_DRIVER.Configure(PORT3, &portCfg, HAL_GPIO_B);
#endif
    ADC_DRIVER.ChannelSetLowAlarmInterrupt(BATT_ADC_CH, true);
    ADC_DRIVER.ChannelSetHighAlarmInterrupt(BATT_ADC_CH, true);
    ADC_DRIVER.ChannelEnable(BATT_ADC_CH, true);
    ADC_DRIVER.ChannelSetAlarm(BATT_ADC_CH, 0, BATT_5_5V + BATT_HYSTERESIS_500mV);

    monitor->battery_range           = BATT_RANGE_CRIT;
    monitor->battery_range_debounced = BATT_RANGE_CRIT;

    monitor->compensate_cb = compensate_cb;

#if (DEVICE_TYPE == FUSB15201)
    /* Init Temp Monitor */
    PORT_DRIVER.Configure(PORT20, &portCfg, HAL_GPIO_SHARED);
#elif DEVICE_TYPE == FUSB15201P
    PORT_DRIVER.Configure(PORT7, &portCfg, HAL_GPIO_B);
#endif
    NTC_DRIVER.Enable(NTC_Port_A_B_Interface, HAL_NTC_A, true);
    ADC_DRIVER.ChannelSetLowAlarmInterrupt(TEMP_ADC_CH, true);
    ADC_DRIVER.ChannelSetHighAlarmInterrupt(TEMP_ADC_CH, true);
    ADC_DRIVER.ChannelEnable(TEMP_ADC_CH, true);
    ADC_DRIVER.ChannelSetAlarm(TEMP_ADC_CH, 0, TEMP_105C);

    monitor->temperature_range           = TEMP_RANGE_CRIT;
    monitor->temperature_range_debounced = TEMP_RANGE_CRIT;

    NVIC_SetPriority(ADC_IRQn, 1);
    NVIC_EnableIRQ(ADC_IRQn);
    ADC_DRIVER.Start();
}

void bnm_monitor_batt_ntc(struct batt_ntc_monitor *monitor)
{
    bool update_power = false;
    /* Battery */
#if (CONFIG_ENABLED(CUSTOM_APPLICATION_001))
    if (monitor->battery_low_alarm || monitor->battery_high_alarm)
    {
        timer_start(&monitor->batt_debounce_timer, BATT_DEBOUNCE);
        switch (monitor->battery_range)
        {
            /* Means transitioning from 3, use 3_2 */
            case BATT_RANGE_H:
                monitor->battery_range = BATT_RANGE_M;
                ADC_DRIVER.ChannelSetAlarm(BATT_ADC_CH, BATT_9_0V,
                                           BATT_11_5V + BATT_HYSTERESIS_750mV);
                break;
            case BATT_RANGE_M:
                if (monitor->battery_high_alarm)
                {
                    monitor->battery_range = BATT_RANGE_H;
                    ADC_DRIVER.ChannelSetAlarm(BATT_ADC_CH, BATT_11_5V, 0);
                }
                else
                {
                    monitor->battery_range = BATT_RANGE_L;
                    ADC_DRIVER.ChannelSetAlarm(BATT_ADC_CH, BATT_5_5V,
                                               BATT_9_0V + BATT_HYSTERESIS_500mV);
                }
                break;
            case BATT_RANGE_L:
                if (monitor->battery_high_alarm)
                {
                    monitor->battery_range = BATT_RANGE_M;
                    ADC_DRIVER.ChannelSetAlarm(BATT_ADC_CH, BATT_9_0V,
                                               BATT_11_5V + BATT_HYSTERESIS_750mV);
                }
                else
                {
                    monitor->battery_range = BATT_RANGE_CRIT;
                    ADC_DRIVER.ChannelSetAlarm(BATT_ADC_CH, 0, BATT_5_5V + BATT_HYSTERESIS_500mV);
                }
                break;
            case BATT_RANGE_CRIT:
                monitor->battery_range = BATT_RANGE_L;
                ADC_DRIVER.ChannelSetAlarm(BATT_ADC_CH, BATT_5_5V,
                                           BATT_9_0V + BATT_HYSTERESIS_500mV);
                break;
            default:
                break;
        }
        monitor->battery_low_alarm  = false;
        monitor->battery_high_alarm = false;
        ADC_DRIVER.ChannelSetLowAlarmInterrupt(BATT_ADC_CH, true);
        ADC_DRIVER.ChannelSetHighAlarmInterrupt(BATT_ADC_CH, true);
    }

    if (timer_expired(&monitor->batt_debounce_timer)
        && !timer_disabled(&monitor->batt_debounce_timer))
    {
        timer_disable(&monitor->batt_debounce_timer);

        monitor->battery_range_debounced = monitor->battery_range;
        update_power                     = true;
    }

    /* Temperature */
    if (monitor->temperature_high_alarm || monitor->temperature_low_alarm)
    {
        timer_start(&monitor->temp_debounce_timer, TEMP_DEBOUNCE);
        switch (monitor->temperature_range)
        {
            case TEMP_RANGE_H:
                if (monitor->temperature_high_alarm)
                {
                    monitor->temperature_range = TEMP_RANGE_M;
                    ADC_DRIVER.ChannelSetAlarm(TEMP_ADC_CH, TEMP_120C, TEMP_95C);
                }
                else
                {
                    monitor->temperature_range = TEMP_RANGE_CRIT;
                    ADC_DRIVER.ChannelSetAlarm(TEMP_ADC_CH, 0, TEMP_105C);
                }
                break;
            case TEMP_RANGE_M:
                if (monitor->temperature_high_alarm)
                {
                    monitor->temperature_range = TEMP_RANGE_L;
                    ADC_DRIVER.ChannelSetAlarm(TEMP_ADC_CH, TEMP_115C, 0);
                }
                else
                {
                    monitor->temperature_range = TEMP_RANGE_H;
                    ADC_DRIVER.ChannelSetAlarm(TEMP_ADC_CH, TEMP_125C, TEMP_100C);
                }
                break;
            case TEMP_RANGE_L:
                monitor->temperature_range = TEMP_RANGE_M;
                ADC_DRIVER.ChannelSetAlarm(TEMP_ADC_CH, TEMP_120C, TEMP_95C);
                break;
            case TEMP_RANGE_CRIT:
                monitor->temperature_range = TEMP_RANGE_H;
                ADC_DRIVER.ChannelSetAlarm(
                    TEMP_ADC_CH, TEMP_125C,
                    TEMP_100C); //(current temp->high temp threshold, low temp threshold)You are basically setting your thresholds for the new LOW range
                //so for the new LOW range 110 will be our trigger for HIGH and 0 means there is no low to low threshold
                break;
            default:
                break;
        }

        monitor->temperature_low_alarm  = false;
        monitor->temperature_high_alarm = false;
        ADC_DRIVER.ChannelSetLowAlarmInterrupt(TEMP_ADC_CH, true);
        ADC_DRIVER.ChannelSetHighAlarmInterrupt(TEMP_ADC_CH, true);
    }

    if (timer_expired(&monitor->temp_debounce_timer)
        && !timer_disabled(&monitor->temp_debounce_timer))
    {
        timer_disable(&monitor->temp_debounce_timer);

        monitor->temperature_range_debounced = monitor->temperature_range;
        update_power                         = true;
    }
#elif (CONFIG_ENABLED(CUSTOM_APPLICATION_002))
    if (monitor->battery_low_alarm || monitor->battery_high_alarm)
    {
        timer_start(&monitor->batt_debounce_timer, 500);

        switch (monitor->battery_range)
        {
            case BATT_RANGE_H:
                monitor->battery_range = BATT_RANGE_M;
                ADC_DRIVER.ChannelSetAlarm(BATT_ADC_CH, BATT_9_0V,
                                           BATT_11_5V + BATT_HYSTERESIS_750mV);
                break;
            case BATT_RANGE_M:
                if (monitor->battery_high_alarm)
                {
                    monitor->battery_range = BATT_RANGE_H;
                    ADC_DRIVER.ChannelSetAlarm(BATT_ADC_CH, BATT_11_5V, 0);
                }
                else
                {
                    monitor->battery_range = BATT_RANGE_L;
                    ADC_DRIVER.ChannelSetAlarm(BATT_ADC_CH, BATT_5_5V,
                                               BATT_9_0V + BATT_HYSTERESIS_500mV);
                }
                break;
            case BATT_RANGE_L:
                if (monitor->battery_high_alarm)
                {
                    monitor->battery_range = BATT_RANGE_M;
                    ADC_DRIVER.ChannelSetAlarm(BATT_ADC_CH, BATT_9_0V,
                                               BATT_11_5V + BATT_HYSTERESIS_750mV);
                }
                else
                {
                    monitor->battery_range = BATT_RANGE_CRIT;
                    ADC_DRIVER.ChannelSetAlarm(BATT_ADC_CH, 0, BATT_5_5V + BATT_HYSTERESIS_500mV);
                }
                break;
            case BATT_RANGE_CRIT:
                monitor->battery_range = BATT_RANGE_L;
                ADC_DRIVER.ChannelSetAlarm(BATT_ADC_CH, BATT_5_5V,
                                           BATT_9_0V + BATT_HYSTERESIS_500mV);
                break;
            default:
                break;
        }
        monitor->battery_low_alarm  = false;
        monitor->battery_high_alarm = false;
        ADC_DRIVER.ChannelSetLowAlarmInterrupt(BATT_ADC_CH, true);
        ADC_DRIVER.ChannelSetHighAlarmInterrupt(BATT_ADC_CH, true);
    }

    if (timer_expired(&monitor->batt_debounce_timer)
        && !timer_disabled(&monitor->batt_debounce_timer))
    {
        timer_disable(&monitor->batt_debounce_timer);

        monitor->battery_range_debounced = monitor->battery_range;
        update_power                     = true;
    }

    // TODO: Translate temp to ADC V
    // TODO: Undefined behavior > 135C?

    if (monitor->temperature_high_alarm || monitor->temperature_low_alarm)
    {
        timer_start(&monitor->temp_debounce_timer, 500);

        switch (monitor->temperature_range)
        {
            case TEMP_RANGE_H:
                if (monitor->temperature_high_alarm)
                {
                    monitor->temperature_range = TEMP_RANGE_M;
                    ADC_DRIVER.ChannelSetAlarm(TEMP_ADC_CH, TEMP_120C, TEMP_95C);
                }
                else
                {
                    monitor->temperature_range = TEMP_RANGE_CRIT;
                    ADC_DRIVER.ChannelSetAlarm(TEMP_ADC_CH, 0, TEMP_105C);
                }
                break;
            case TEMP_RANGE_M:
                if (monitor->temperature_high_alarm)
                {
                    monitor->temperature_range = TEMP_RANGE_L;
                    ADC_DRIVER.ChannelSetAlarm(TEMP_ADC_CH, TEMP_115C, 0);
                }
                else
                {
                    monitor->temperature_range = TEMP_RANGE_H;
                    ADC_DRIVER.ChannelSetAlarm(TEMP_ADC_CH, TEMP_125C, TEMP_100C);
                }
                break;
            case TEMP_RANGE_L:
                monitor->temperature_range = TEMP_RANGE_M;
                ADC_DRIVER.ChannelSetAlarm(TEMP_ADC_CH, TEMP_120C, TEMP_95C);
                break;
            case TEMP_RANGE_CRIT:
                monitor->temperature_range = TEMP_RANGE_H;
                ADC_DRIVER.ChannelSetAlarm(
                    TEMP_ADC_CH, TEMP_125C,
                    TEMP_100C); //(current temp->high temp threshold, low temp threshold)You are basically setting your thresholds for the new LOW range
                //so for the new LOW range 110 will be our trigger for HIGH and 0 means there is no low to low threshold
                break;
            default:
                break;
        }

        monitor->temperature_low_alarm  = false;
        monitor->temperature_high_alarm = false;
        ADC_DRIVER.ChannelSetLowAlarmInterrupt(TEMP_ADC_CH, true);
        ADC_DRIVER.ChannelSetHighAlarmInterrupt(TEMP_ADC_CH, true);
    }

    if (timer_expired(&monitor->temp_debounce_timer)
        && !timer_disabled(&monitor->temp_debounce_timer))
    {
        timer_disable(&monitor->temp_debounce_timer);

        monitor->temperature_range_debounced = monitor->temperature_range;
        update_power                         = true;
    }
#endif
    if (monitor->compensate_cb && update_power)
    {
        monitor->compensate_cb(monitor);
    }
}

uint32_t bnm_min_timer(struct batt_ntc_monitor *monitor)
{
    uint32_t min = ~0;
    uint32_t time;

    if (!timer_disabled(&monitor->batt_debounce_timer))
    {
        time = timer_remaining(&monitor->batt_debounce_timer);
        if (time < min)
        {
            min = time;
        }
    }

    if (!timer_disabled(&monitor->temp_debounce_timer))
    {
        time = timer_remaining(&monitor->temp_debounce_timer);
        if (time < min)
        {
            min = time;
        }
    }

    return min;
}
