/**
 * @file   fusb18xxx_hal.h
 * @author USB Firmware Team
 * @brief  Defines functionality for the Device Policy Manager state machine.
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
 *
 * @addtogroup HAL
 * @{
 * *******************************************************************************/
#ifndef __FUSB18XXX_HAL_H__
#define __FUSB18XXX_HAL_H__

#include "port.h"

#ifdef FUSB18XXX_DEVICE
bool fusb18xxx_tcpd_rx_hardreset(HAL_USBPD_T *tcpd);
void fusb18xxx_tcpd_tx_hardreset(HAL_USBPD_T *tcpd, SOP_T sop);
void fusb18xxx_tcpd_set_rp(HAL_USBPD_T *tcpd, int mask, CC_RP_T rp);
void fusb18xxx_tcpd_set_orient(HAL_USBPD_T *tcpd, CC_T cc);
void fusb18xxx_tcpd_set_cc(HAL_USBPD_T *tcpd, int mask, CC_TERM_T term, CC_RP_T rp, DRP_MODE_T drp);
CC_STAT_T  fusb18xxx_tcpd_cc_stat(HAL_USBPD_T *tcpd, CC_T cc);
void       fusb18xxx_tcpd_cc_events_enable(HAL_USBPD_T *tcpd, bool en);
void       fusb18xxx_tcpd_set_drp(HAL_USBPD_T *tcpd, DRP_MODE_T drp);
DRP_STAT_T fusb18xxx_tcpd_drp_stat(HAL_USBPD_T *tcpd);
void       fusb18xxx_tcpd_vbus_deinit(HAL_USBPD_T *tcpd);
void       fusb18xxx_tcpd_vbus_deinit(HAL_USBPD_T *tcpd);
void       fusb18xxx_tcpd_vbus_events(HAL_USBPD_T *tcpd, bool en);
bool       fusb18xxx_tcpd_set_vbus_source(HAL_USBPD_T *tcpd, unsigned mv, unsigned ma, PDO_T type);
bool       fusb18xxxx_tcpd_set_vbus_sink(HAL_USBPD_T *tcpd, unsigned mv, unsigned ma, PDO_T type);
unsigned   fusb18xxx_tcpd_vbus_value(HAL_USBPD_T *tcpd);
void       fusb18xxx_tcpd_set_vbus_res(HAL_USBPD_T *tcpd, unsigned max);
void       fusb18xxx_tcpd_set_vbus_alarm(HAL_USBPD_T *tcpd, unsigned low, unsigned high);
void       fusb18xxx_tcpd_set_vbus_disch(HAL_USBPD_T *tcpd, VBUS_DISCH_T disch);
CC_RP_T    fusb18xxx_tcpd_pullup_value(HAL_USBPD_T *tcpd, CC_T cc);
void       fusb18xxx_tcpd_set_term(HAL_USBPD_T *tcpd, int mask, CC_TERM_T term);
void       fusb18xxx_tcpd_vconn_enable(HAL_USBPD_T *tcpd, bool en);
void       fusb18xxx_tcpd_pd_enable(HAL_USBPD_T *tcpd, bool en);
void       fusb18xxx_tcpd_pd_sop_enable(HAL_USBPD_T *tcpd, unsigned mask);
void       fusb18xxx_tcpd_pd_rev(HAL_USBPD_T *tcpd, PD_REV_T rev, SOP_T sop);
void       fusb18xxx_tcpd_set_pd_dfp(HAL_USBPD_T *tcpd, bool dfp_en);
void       fusb18xxx_tcpd_set_pd_source(HAL_USBPD_T *tcpd, bool src_en);
void       fusb18xxx_tcpd_set_bist_mode(HAL_USBPD_T *tcpd, BIST_MODE_T mode);
PORT_TX_STAT_T fusb18xxx_tcpd_port_tx_status(HAL_USBPD_T *tcpd);
bool  fusb18xxx_tcpd_pd_tx(HAL_USBPD_T *tcpd, uint8_t const *buf, unsigned long len, SOP_T sop,
                           int retries);
bool  fusb18xxx_tcpd_pd_check_rx(HAL_USBPD_T *tcpd);
SOP_T fusb18xxx_tcpd_pd_rx(HAL_USBPD_T *tcpd, uint8_t *buf, unsigned long len);

typedef enum fusb18xxx_type
{
    fusb18101_dev = 0x1201,
    fusb18200_dev,
    fusb18201_dev,
    fusb18201p_dev,
} FUSB18DEV_TYPE_T;

void         fusb18xxx_init(enum fusb18xxx_type dev_id);
HAL_USBPD_T *fusb18xxx_device_tcpd_init(int dev_id, bool en);
#endif

#endif //__FUSB18XXX_HAL_H__
