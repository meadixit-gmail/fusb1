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
 * policy.h
 *
 * Defines functionality for the Policy Engine state machine.
 */
#ifndef _POLICY_H_
#define _POLICY_H_

#include "dev_tcpd.h"

#if CONFIG_ENABLED(LOG)
static const char *const policy_string[] = {
#define STATE_STR(str) #str,
    CREATE_POLICY_STATES(STATE_STR)};

#define policy_set_state(port, state, sub)                                                         \
    do                                                                                             \
    {                                                                                              \
        dbg_msg("%d:%s(%d)", port->dev->port_tcpd_id, policy_string[state], sub);                  \
        policy_set_state_func(port, state, sub);                                                   \
    } while (0)
#else
#define policy_set_state(port, state, sub) policy_set_state_func(port, state, sub)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    /**
 * @brief Set different policy states
 */
   void policy_set_state_func(struct port *port, enum policy_state_t state, uint32_t sub);

    /**
 * @brief Initialize the policy variables to default
 */
    void policy_reset(struct port *);

    /**
 * Public functions available from policy
 */
    void policy_engine(struct port *);

    /**
 * @brief enable PD as source or sink
 * @param[in] port object
 * @param[in] true if port is source
 */
    void policy_pd_enable(struct port *, bool);

    /**
 * @brief disable PD
 */
    void policy_pd_disable(struct port *);

    /**
 * @brief Called when protocol detects hard reset
 */
    void policy_receive_hardreset(struct port *);

    void policy_send_source_hardreset(struct port *port);

#ifdef __cplusplus
}
#endif

#endif /* _POLICY_H_ */
