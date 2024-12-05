#include "get_capacity.h"
uint32_t get_capacity(struct port_tcpd *p_tcpd, enum batt_range b_r, enum temp_range t_r)
{
    uint32_t new_capacity = 0;
#if CONFIG_ENABLED(CUSTOM_APPLICATION_002)
    if (b_r == BATT_RANGE_H)
    {
        if (t_r == TEMP_RANGE_L)
        {
            new_capacity = p_tcpd->vif->Product_Total_Source_Power_mW;
        }
        else
        {
            new_capacity = 45000;
        }
    }
    else if (b_r == BATT_RANGE_M)
    {
        new_capacity = 30000;
    }
    else if (b_r != BATT_RANGE_CRIT && t_r != TEMP_RANGE_CRIT)
    {
        new_capacity = 15000;
    }
#elif CONFIG_ENABLED(CUSTOM_APPLICATION_001)
    /* Set shared power based on temp and batt */
    if (b_r == BATT_RANGE_H)
    {
        if (t_r == TEMP_RANGE_L)
        {
            new_capacity = p_tcpd->vif->Product_Total_Source_Power_mW;
        }
        else
        {
            new_capacity = 45000;
        }
    }
    else if (b_r == BATT_RANGE_M)
    {
        if (t_r == TEMP_RANGE_H)
        {
            new_capacity = 30000;
        }
        else
        {
            new_capacity = 45000;
        }
    }
    else
    {
        new_capacity = 30000;
    }
#endif
    if (b_r == BATT_RANGE_CRIT || t_r == TEMP_RANGE_CRIT)
    {
        new_capacity = 0;
    }
    return new_capacity;
}
