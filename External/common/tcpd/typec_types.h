/*******************************************************************************
 * @file     typec_types.h
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
#ifndef _TYPEC_TYPES_H_
#define _TYPEC_TYPES_H_

#include "platform.h"
#define IF(cond, foo...) IF_IMPL(cond, foo)
#define IF_IMPL(cond, foo...) IF_ ## cond (foo)
#define IF_0(foo...)
#define IF_1(foo...) foo

#if (CONFIG_ENABLED(SRC))
#define SRC_STATES 1
#else
#define SRC_STATES 0
#endif

#if (CONFIG_ENABLED(SNK))
#define SNK_STATES 1
#else
#define SNK_STATES 0
#endif

#if (CONFIG_ENABLED(DRP))
#define DRP_STATES 1
#else
#define DRP_STATES 0
#endif

/** Define various VBUS thresholds and conversion macros */
#define VBUS_SCALE(x) (x * 1000) /* mv scale */
#define VBUS_OFF      VBUS_SCALE(0U)
#define VBUS_0V       VBUS_SCALE(0U)
#define VBUS_3V       VBUS_SCALE(3U)
#define VBUS_5V       VBUS_SCALE(5U)
#define VBUS_9V       VBUS_SCALE(9U)
#define VBUS_12V      VBUS_SCALE(12U)
#define VBUS_15V      VBUS_SCALE(15U)
#define VBUS_20V      VBUS_SCALE(20U)

/** TODO: Raise this thresholds higher */
#define VBUS_SINK_LOW_THRESHOLD   4300U
#define VBUS_5V_DISC              3670U
#define VBUS_VSAFE0               800U
#define VBUS_VSAFE0_ALARM         700U
#define VBUS_VOLTAGE(v)           VBUS_SCALE(v)
#define VBUS_DISCONNECT(mv)       (mv - (mv / 5))  //< 20% below
#define VBUS_LOW_THRESHOLD(mv)    (mv - (mv / 20)) //<  5% below
#define VBUS_HIGH_THRESHOLD(mv)   (mv + (mv / 20)) //<  5% above
#define VBUS_10PCT_HIGH_THRESHOLD(mv) (mv + (mv / 10)) //>  10% above
#define VBUS_VSAFE0_THRESHOLD(mv) (mv - (mv / 10)) //<  10% below
#define ADJUSTED_CURRENT(ma)      (ma > 50 ? (((ma * 106) / 100) - 50) : ma)
#define VBUS_MAX_VOLTAGE          VBUS_HIGH_THRESHOLD(VBUS_20V)

#define VBUS_PPS_DISC(mv)		((mv * 95) / 100) //< Scaled by 95%
#define VBUS_PD_FIXED_DISC(mv)	(((mv * 90) / 100) - 1250) //< vSrcNew (fixed) - 750 mV - vSrcValid
#define VBUS_PD_PPS_DISC(mv)	(((mv * 95) / 100) - 850) //< vPpsNew - 750 mV - vPpsValid

/**
 * Define type-c timer values
 */
#define ktAMETimeout    (900U)
#define ktPDDebounce    (15U)
#define ktCCDebounce    (120U)
#define ktDRPTry        (125U)
#define ktDRPTryWait    (600U)
#define ktErrorRecovery (300U)
#define ktTryCCDebounce (15U)
#define ktTryTimeout    (800U)

// clang-format off
#define CC_RP_CURRENT(x)	(((x) == CC_RP_DEF) ? 500U :                                       \
				 ((x) == CC_RP_1p5A) ? 1500U :                                     \
				 ((x) == CC_RP_3p0A) ? 3000U : 0U)
#define CC_RD_CURRENT(x)	((x) == CC_STAT_RPUSB ? 500U :                                     \
				 (x) == CC_STAT_RP1p5 ? 1500U :                                    \
				 (x) ==  CC_STAT_RP3p0 ? 3000U : 0U)
//clang-format on

/**
 * CC pin definition.
 */
typedef enum cc_t {
	CC_NONE = 0, /**< Undetermined */
	CC1 = 1,
	CC2 = 2,
} CC_T;

typedef enum cc_term_t {
	CC_TERM_RA = 0,
	CC_TERM_RP,
	CC_TERM_RD,
	CC_TERM_OPEN,
} CC_TERM_T;

typedef enum cc_stat_t {
	CC_STAT_SRCOPEN = 0,
	CC_STAT_RA,
	CC_STAT_RD,
	CC_STAT_RESERVED,
	CC_STAT_RPUSB,
	CC_STAT_RP1p5,
	CC_STAT_RP3p0,
	CC_STAT_SNKOPEN,
	CC_STAT_UNDEFINED,
} CC_STAT_T;

typedef enum cc_rp_t {
	CC_RP_OFF = 0,
	CC_RP_DEF = 1,
	CC_RP_1p5A = 2,
	CC_RP_3p0A = 3,
	CC_RP_NOCHANGE,
} CC_RP_T;

typedef enum drp_mode_t {
	DRP_MODE_DRP = 0,
	DRP_MODE_SINKACC,
	DRP_MODE_SINKVPD,
	DRP_MODE_SINKVPDACC,
	DRP_MODE_DISABLED,
	DRP_MODE_NOCHANGE,
} DRP_MODE_T;

typedef enum drp_stat_t {
	DRP_INACTIVE = 0,
	DRP_ACTIVE,
	DRP_SOURCE,
	DRP_SINK,
} DRP_STAT_T;

typedef enum vbus_disch_t {
	VBUS_DISCH_OFF,
	VBUS_DISCH_BLEED,
	VBUS_DISCH_DEFAULT,
	VBUS_DISCH_FAST,
} VBUS_DISCH_T;

typedef enum source_or_sink {
	Source = 0,
	Sink,
} SourceOrSink;

typedef enum tc_port_t {
	TypeC_Source = 0,
	TypeC_Sink,
	TypeC_DRP,
	TypeC_UNDEFINED,
} TC_PORT_T;
#define CREATE_TYPEC_DRP_STATES(ITEM) \
	ITEM(TrySource)                                                                            \
	ITEM(TryWaitSink)                                                                          \
	ITEM(TrySink)                                                                              \
	ITEM(TryWaitSource)                                                                        \

#define CREATE_TYPEC_SRC_STATES(ITEM) \
		ITEM(AttachWaitSource)                                                                     \
		ITEM(AttachedSource)                                                                       \
		ITEM(UnattachedWaitSource)                                                                 \
		ITEM(UnorientedDebugAccessorySource)                                                       \
		ITEM(OrientedDebugAccessorySource)                                                         \

#define CREATE_TYPEC_SNK_STATES(ITEM) \
		ITEM(AttachWaitSink)                                                                     \
		ITEM(AttachedSink)                                                                       \
		ITEM(AttachWaitAccessory)                                                                  \
		ITEM(DebugAccessorySink)                                                                   \
		ITEM(PoweredAccessory)                                                                     \
		ITEM(UnsupportedAccessory) \

#define CREATE_TYPEC_STATE_LIST(ITEM)                                                              \
	ITEM(Disabled)                                                                             \
	ITEM(ErrorRecovery)                                                                        \
	ITEM(Unattached)                                                                           \
	IF(SRC_STATES, CREATE_TYPEC_SRC_STATES(ITEM)) \
	IF(SNK_STATES, CREATE_TYPEC_SNK_STATES(ITEM)) \
	IF(DRP_STATES, CREATE_TYPEC_DRP_STATES(ITEM))\
	ITEM(AudioAccessory)                                                                       \
	ITEM(DelayUnattached)                                                                      \
	ITEM(IllegalCable) \
	ITEM(NUM_TC_STATES)

typedef enum tc_state_t {
#define GEN_ENUM(state) state,
	CREATE_TYPEC_STATE_LIST(GEN_ENUM)
} TC_STATE_T;

typedef enum tc_pins_t {
	TypeCPin_None = 0,
	TypeCPin_GND1,
	TypeCPin_TXp1,
	TypeCPin_TXn1,
	TypeCPin_VBUS1,
	TypeCPin_CC1,
	TypeCPin_Dp1,
	TypeCPin_Dn1,
	TypeCPin_SBU1,
	TypeCPin_VBUS2,
	TypeCPin_RXn2,
	TypeCPin_RXp2,
	TypeCPin_GND2,
	TypeCPin_GND3,
	TypeCPin_TXp2,
	TypeCPin_TXn2,
	TypeCPin_VBUS3,
	TypeCPin_CC2,
	TypeCPin_Dp2,
	TypeCPin_Dn2,
	TypeCPin_SBU2,
	TypeCPin_VBUS4,
	TypeCPin_RXn1,
	TypeCPin_RXp1,
	TypeCPin_GND4,
} TypeCPins_t;

typedef enum frs_mode_t {
	FRS_None = 0,
	FRS_Hub_Sink = 1,
	FRS_Hub_Source = 2,
	FRS_Host_Sink = 3,
} FRSMode;

#endif /* _TYPEC_TYPES_H_ */
