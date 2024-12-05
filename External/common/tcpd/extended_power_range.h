/*******************************************************************************
 * @file     policy.h
 * @author   USB PD Firmware Team
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
 ******************************************************************************/
/*
 * extended_power_range.h
 *
 * Defines functionality for the Policy Engine state machine.
 */
#ifndef _EXTENDED_POWER_RANGE_H_

#include "port_tcpd.h"

void        policy_state_sink_epr_mode_entry(struct port *port);
void        policy_state_sink_epr_mode_entry_wait_for_response(struct port *port);
void        policy_state_sink_epr_keep_alive(struct port *port);
void        policy_state_sink_send_epr_exit(struct port *port);
void        policy_state_read_epr_mode(struct port *port);
void        policy_state_sink_epr_mode_entry(struct port *port);
extern void policy_state_invalid(struct port *port);
extern int  policy_send(struct port *port, unsigned msg, uint8_t *data, unsigned long len,
                        enum sop_t sop, bool ext, int succ_state, int succ_idx, int fail_state,
                        int fail_idx);
void        policy_state_source_evaluate_epr_mode_entry(struct port *port);
void policy_state_source_epr_mode_entry_ack(struct port *port);
void policy_state_source_epr_mode_entry_failed(struct port *port);
void policy_state_source_epr_mode_discover_cable(struct port *port);
void policy_state_source_epr_mode_evaluate_cable_epr(struct port *port);
void policy_state_source_epr_mode_entry_succeeded(struct port *port);
void policy_state_source_epr_keep_alive(struct port *port);
void policy_state_source_send_epr_mode_exit(struct port *port);


#endif /* _EXTENDED_POWER_RANGE_H_ */
