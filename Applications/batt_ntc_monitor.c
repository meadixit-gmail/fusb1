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
#include "batt_ntc_monitor.h"
#include "port.h"
#include "dpm.h"
#include "dev_tcpd.h"
#include "fusb15xxx.h"

#if CONFIG_ENABLED(BATTERY_NTC)
void bnm_batt_ntc_init(void *dpm_priv, struct batt_ntc_monitor *monitor,
                       compensate_ports_cb compensate_cb)
{
    HAL_PORTCFG_T portCfg = {.an  = PORT_ANALOG,
                             .alt = PORT_ALTERNATE,
                             .pu  = PORT_PULLUP_DISABLE,
                             .pd  = PORT_PULLDOWN_DISABLE};

    struct dpm_t *dpm = (struct dpm_t *)dpm_priv;

    /* Init Temp Monitor - Port 1*/
    dev_port_configure(dpm->fusb_driv, PORT20, &portCfg, HAL_GPIO_SHARED);
    dev_port_ntc_enable(dpm->fusb_driv, HAL_NTC_A, true);
    dev_adc_ch_set_low_alarm_interrupt(dpm->fusb_driv, TEMP1_ADC_CH, true);
    dev_adc_ch_set_high_alarm_interrupt(dpm->fusb_driv, TEMP1_ADC_CH, true);
    dev_adc_channel_enable(dpm->fusb_driv, TEMP1_ADC_CH);
    dev_adc_ch_set_alarm(dpm->fusb_driv, TEMP1_ADC_CH, TEMP_125C, 0);

    monitor->p1_temperature_range           = TEMP_RANGE_L;
    monitor->p1_temperature_range_debounced = TEMP_RANGE_L;

    /* Init Battery Monitor - Port 2*/
    dev_port_configure(dpm->fusb_driv, PORT15, &portCfg, HAL_GPIO_SHARED);
    dev_port_ntc_enable(dpm->fusb_driv, HAL_NTC_B, true);
    dev_adc_ch_set_low_alarm_interrupt(dpm->fusb_driv, TEMP2_ADC_CH, true);
    dev_adc_ch_set_high_alarm_interrupt(dpm->fusb_driv, TEMP2_ADC_CH, true);
    dev_adc_channel_enable(dpm->fusb_driv, TEMP2_ADC_CH);
    dev_adc_ch_set_alarm(dpm->fusb_driv, TEMP2_ADC_CH, TEMP_125C, 0);

    monitor->p2_temperature_range           = TEMP_RANGE_L;
    monitor->p2_temperature_range_debounced = TEMP_RANGE_L;

    monitor->compensate_cb = compensate_cb;
    monitor->dpm_priv      = dpm_priv;
}

void bnm_monitor_batt_ntc(struct batt_ntc_monitor *monitor)
{
    bool          update_power = false;
    struct dpm_t *dpm          = (struct dpm_t *)monitor->dpm_priv;

    HAL_ADC_CHx_STAT_T temp1_stat = dev_adc_channel_status(dpm->fusb_driv, TEMP1_ADC_CH);
    HAL_ADC_CHx_STAT_T temp2_stat = dev_adc_channel_status(dpm->fusb_driv, TEMP2_ADC_CH);

    /* Port 1 Temp */
    if (temp1_stat.alarmh || temp1_stat.alarml)
    {
        timer_start(&monitor->p1_temp_debounce_timer, TEMP_DEBOUNCE);

        switch (monitor->p1_temperature_range)
        {
            case TEMP_RANGE_H:
                if (temp1_stat.alarmh)
                {
                    monitor->p1_temperature_range = TEMP_RANGE_L;
                    dev_adc_ch_set_alarm(dpm->fusb_driv, TEMP1_ADC_CH, TEMP_125C, 0);
                }
                else
                {
                    monitor->p1_temperature_range = TEMP_RANGE_CRIT;
                    dev_adc_ch_set_alarm(dpm->fusb_driv, TEMP1_ADC_CH, TEMP_115C, 0);
                }
                break;
            case TEMP_RANGE_L:
                monitor->p1_temperature_range = TEMP_RANGE_H;
                dev_adc_ch_set_alarm(dpm->fusb_driv, TEMP1_ADC_CH, TEMP_135C, TEMP_105C);
                break;
            case TEMP_RANGE_CRIT:
                monitor->p1_temperature_range = TEMP_RANGE_H;
                dev_adc_ch_set_alarm(dpm->fusb_driv, TEMP1_ADC_CH, TEMP_135C, TEMP_105C);
                break;
            default:
                break;
        }

        dev_adc_ch_set_low_alarm_interrupt(dpm->fusb_driv, TEMP1_ADC_CH, true);
        dev_adc_ch_set_high_alarm_interrupt(dpm->fusb_driv, TEMP1_ADC_CH, true);
    }

    if (timer_expired(&monitor->p1_temp_debounce_timer)
        && !timer_disabled(&monitor->p1_temp_debounce_timer))
    {
        timer_disable(&monitor->p1_temp_debounce_timer);

        monitor->p1_temperature_range_debounced = monitor->p1_temperature_range;
        update_power                            = true;
    }

    /* Port 2 Temp */
    if (temp2_stat.alarmh || temp2_stat.alarml)
    {
        timer_start(&monitor->p2_temp_debounce_timer, TEMP_DEBOUNCE);

        switch (monitor->p2_temperature_range)
        {
            case TEMP_RANGE_H:
                if (temp2_stat.alarmh)
                {
                    monitor->p2_temperature_range = TEMP_RANGE_L;
                    dev_adc_ch_set_alarm(dpm->fusb_driv, TEMP2_ADC_CH, TEMP_125C, 0);
                }
                else
                {
                    monitor->p2_temperature_range = TEMP_RANGE_CRIT;
                    dev_adc_ch_set_alarm(dpm->fusb_driv, TEMP2_ADC_CH, 0, TEMP_115C);
                }
                break;
            case TEMP_RANGE_L:
                monitor->p2_temperature_range = TEMP_RANGE_H;
                dev_adc_ch_set_alarm(dpm->fusb_driv, TEMP2_ADC_CH, TEMP_135C, TEMP_105C);
                break;
            case TEMP_RANGE_CRIT:
                monitor->p2_temperature_range = TEMP_RANGE_H;
                dev_adc_ch_set_alarm(dpm->fusb_driv, TEMP2_ADC_CH, TEMP_135C, TEMP_105C);
                break;
            default:
                break;
        }

        dev_adc_ch_set_low_alarm_interrupt(dpm->fusb_driv, TEMP2_ADC_CH, true);
        dev_adc_ch_set_high_alarm_interrupt(dpm->fusb_driv, TEMP2_ADC_CH, true);
    }

    if (timer_expired(&monitor->p2_temp_debounce_timer)
        && !timer_disabled(&monitor->p2_temp_debounce_timer))
    {
        timer_disable(&monitor->p2_temp_debounce_timer);

        monitor->p2_temperature_range_debounced = monitor->p2_temperature_range;
        update_power                            = true;
    }

    if (monitor->compensate_cb && update_power)
    {
        monitor->compensate_cb(monitor);
    }
}

uint32_t bnm_min_timer(struct batt_ntc_monitor *monitor)
{
    uint32_t min = ~0;
    uint32_t time;

    if (!timer_disabled(&monitor->p1_temp_debounce_timer))
    {
        time = timer_remaining(&monitor->p1_temp_debounce_timer);
        if (time < min)
        {
            min = time;
        }
    }

    if (!timer_disabled(&monitor->p2_temp_debounce_timer))
    {
        time = timer_remaining(&monitor->p2_temp_debounce_timer);
        if (time < min)
        {
            min = time;
        }
    }

    return min;
}

#else
void bnm_batt_ntc_init(void *dpm_priv, struct batt_ntc_monitor *monitor,
                       compensate_ports_cb compensate_cb)
{
}

void bnm_monitor_batt_ntc(struct batt_ntc_monitor *monitor)
{
}

uint32_t bnm_min_timer(struct batt_ntc_monitor *monitor)
{
    return ~0;
}
#endif
