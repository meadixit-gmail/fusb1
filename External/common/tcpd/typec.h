/*******************************************************************************
 * @file     typec.h
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
 * Defines the Type-C state machine functions
 */
#ifndef _TYPEC_H_
#define _TYPEC_H_

#include "dev_tcpd.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
 * @brief
 */
    void typec_reset(struct port *);

    /**
 *
 * @param
 */
    void typec_role_swap_to_sink(struct port *);

    /**
 *
 * @param
 */
    void typec_role_swap_to_source(struct port *);

    /**
 * @brief
 */
    void typec_set_attach_wait_source(struct port *);

    /**
 * @brief
 */
    void typec_set_attached_source(struct port *);

    /**
 * @brief
 */
    void typec_set_unattached_wait_source(struct port *);

    /**
 * @brief
 */
    void typec_set_unoriented_debug_acc_source(struct port *);

    /**
 * @brief
 */
    void typec_set_oriented_debug_acc_source(struct port *);

    /**
 * @brief
 */
    void typec_set_attach_wait_sink(struct port *);

    /**
 * @brief
 */
    void typec_set_attached_sink(struct port *);

    /**
 * @brief
 */
    void typec_set_try_sink(struct port *);

    /**
 * @brief
 */
    void typec_set_debug_acc_sink(struct port *);

    /**
 * @brief
 */
    void typec_set_try_wait_sink(struct port *);

    /**
 * @brief
 */
    void typec_set_try_source(struct port *);

    /**
 * @brief
 */
    void typec_set_try_wait_source(struct port *);

    /**
 * @brief
 */
    void typec_set_attach_wait_acc(struct port *);

    /**
 * @brief
 */
    void typec_set_audio_acc(struct port *);

    /**
 * @brief
 */
    void typec_set_powered_acc(struct port *);

    /**
 * @brief
 */
    void typec_set_unsupported_acc(struct port *);

    /**
 * @brief
 */
    void typec_set_disabled(struct port *);

    /**
 * @brief
 */
    void typec_set_error_recovery(struct port *);

    /**
 * @brief
 */
    void typec_set_unattached(struct port *);

    /**
 * @brief
 */
    void typec_set_illegal_cable(struct port *);

    /**
 * @fn typec_sm
 * @brief Type-C and PD state machine handler
 * @param @ref Port object
 */

    void typec_sm(struct port *);

#ifdef __cplusplus
}
#endif

#endif /* _TYPEC_H_ */
