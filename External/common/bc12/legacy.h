/*******************************************************************************
 * @file   legacy.h
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

 * \defgroup BC1_2
 * @{
*******************************************************************************/
#ifndef CORE_LEGACY_H_
#define CORE_LEGACY_H_

#include "port_bc1p2.h"
/**
 * Legacy BC1.2 charger detection
 */
#ifdef __cplusplus
extern "C"
{
#endif

    /**
 * @brief Initialize the BC1.2 state machine and maintain detached state
 * @param BC1.2 State machine
 * @param @ref BC1P2_MODE_T object
 */
    void bc_sm_init(struct legacy_state *, enum bc1p2_mode);

    /**
 * @brief Enable and start the BC1.2 detection.
 * Call this once BC1.2 can start
 * @param BC1.2 object
 */
    void bc_sm_enable(struct legacy_state *);

    /**
 * @@brief Disable the BC1.2 state machine and detach
 * @param
 */
    void bc_sm_disable(struct legacy_state *);

#ifdef __cplusplus
}
#endif

#endif /** @} CORE_LEGACY_H_ */
