#ifndef GET_CAPACITY_H
#define GET_CAPACITY_H
#include "pdps_batt_ntc_monitor.h"
#include "port.h"
#include "vif_types.h"
uint32_t get_capacity(struct port_tcpd *p_tcpd, enum batt_range b_r, enum temp_range t_r);

#endif
