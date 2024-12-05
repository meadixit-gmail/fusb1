/*******************************************************************************
 * @file   legacy_types.h
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
#ifndef LEGACY_TYPES_H
#define LEGACY_TYPES_H
#include "platform.h"

/**
 * BC1.2 timer values in milliseconds
 */
#define TVDxSRC_ON    40UL
#define TSVLD_CON_PWD 1000UL
#define TDETACH       10UL
#define TEXIT_MODE    10UL
#define TDCD_TIMEOUT  300UL
#define TDCD_DBNC     10UL
#define TBC12         1750UL

/**
 * BC1.2 voltage references in millivolts
 */
#define V150mV     150UL
#define VDIV       2750UL
#define VDIV_L     2650UL
#define VDIV_H     2850UL
#define VLGC_L     800UL
#define VLGC_H     2000UL
#define VDAT_REF_L 250UL
#define VDAT_REF_H 400UL
#define VLGC_LOW_L 0UL
#define VLGC_LOW_H 800UL
#define VLGC_HI_L  2000UL
#define VLGC_HI_H  3600UL

/**
 * BC1.2 states.
 * @deprecated These are information only.
 */
typedef enum bc1p2_state
{
    BC1P2_STATE_DISABLED = 0,

    BC1P2_STATE_CSM_ENABLE,
    BC1P2_STATE_CSM_PRIMARY_DETECTION,
    BC1P2_STATE_CSM_SECONDARY_DETECTION,
    BC1P2_STATE_CSM_SDP_ATTACHED,
    BC1P2_STATE_CSM_DCP_ATTACHED,
    BC1P2_STATE_CSM_CDP_ATTACHED,
    BC1P2_STATE_CSM_PROPRIETARY,

    BC1P2_STATE_CDP_PRIMARY1,
    BC1P2_STATE_CDP_PRIMARY2,
    BC1P2_STATE_CDP_ATTACHED,

    BC1P2_STATE_DCP_ADV_DIV,
    BC1P2_STATE_DCP_ADV_DCP,
} BC1P2_STATE_Enum;

/**
 * BC1.2 mode used to initialize the state machine
 */
typedef enum bc1p2_mode
{
    BC1P2_MODE_CONSUMER,
    BC1P2_MODE_PROVIDER_CDP,
    BC1P2_MODE_PROVIDER_DCP,
} BC1P2_MODE_Enum;

typedef enum bc1p2_timer
{
    BC1P2_TIMER0 = 0,
    BC1P2_TIMER1,
    BC1P2_TIMER2,
    BC1P2_TIMER_COUNT,
} BC1P2_TIMER_Enum;

#endif
