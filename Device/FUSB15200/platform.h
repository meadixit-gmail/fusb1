/*******************************************************************************
 * @file     platform.h
 * @author   USB PD Firmware Team
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
 ******************************************************************************/
#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef CONFIG_BC1P2_CDP
#define CONFIG_BC1P2_CDP 0
#endif

#ifndef CONFIG_BC1P2_CSM
#define CONFIG_BC1P2_CSM 0
#endif

#ifndef CONFIG_BC1P2_DCP
#define CONFIG_BC1P2_DCP 0
#endif

#ifndef CONFIG_BC1P2_DCP_ADDIV
#define CONFIG_BC1P2_DCP_ADDIV 0
#endif

#ifndef CONFIG_LEGACY_CHARGING
#define CONFIG_LEGACY_CHARGING 0
#endif

#ifndef CONFIG_DRP
#define CONFIG_DRP 0
#endif

#ifndef CONFIG_SRC
#define CONFIG_SRC 0
#endif

#ifndef CONFIG_SNK
#define CONFIG_SNK 0
#endif

#ifndef CONFIG_EPR
#define CONFIG_EPR 0
#endif

#ifndef CONFIG_EPR_TEST
#define CONFIG_EPR_TEST 0
#endif

/* Temporarily disabling while compliance does not support message*/
#ifndef CONFIG_AUTO_GET_SINK_CAP_EXT
#define CONFIG_AUTO_GET_SINK_CAP_EXT 0
#endif

#ifndef CONFIG_EXTMSG
#define CONFIG_EXTMSG 0
#endif

#ifndef CONFIG_VDM
#define CONFIG_VDM 0
#endif

/**
 * @brief Option to include code for VDM minimal Driver
 */
#ifndef CONFIG_VDM
#define CONFIG_VDM_MINIMAL 0
#endif

#ifndef CONFIG_MINIMAL
#define CONFIG_MINIMAL 0
#endif

#ifndef CONFIG_USB4
#define CONFIG_USB4 0
#endif

#ifndef CONFIG_SLEEP
#define CONFIG_SLEEP 0
#endif

#ifndef CONFIG_LOG
#define CONFIG_LOG 0
#endif

#ifndef CONFIG_FW_UPDATE
#define CONFIG_FW_UPDATE 0
#endif

#ifndef CONFIG_POWER_SHARING
#define CONFIG_POWER_SHARING 0
#endif

#ifndef CONFIG_DCDC
#define CONFIG_DCDC 0
#endif

#ifndef CONFIG_CUSTOM_APPLICATION
#define CONFIG_CUSTOM_APPLICATION 0
#endif

#define CONFIG_ENABLED(x) CONFIG_##x

/*FUSB15XXX generation PD device*/
#if (defined(FUSB15101) || defined(FUSB15200) || defined(FUSB15201) || defined(FUSB15201P))
#define FUSB15XXX_DEVICE 1
#undef FUSB18XXX_DEVICE
/*FUSB18XXX generation PD device*/
#elif (defined(FUSB18101) || defined(FUSB18200) || defined(FUSB18201) || defined(FUSB18201P))
#define FUSB18XXX_DEVICE 1
#undef FUSB15XXX_DEVICE
#endif

#ifdef FUSB15XXX_DEVICE
#include "FUSB15xxx.h"
#define FUSB_DEVICE_TYPE 0x0F
#endif

#ifdef FUSB18XXX_DEVICE
#include "FUSB18xxx.h"
#define FUSB_DEVICE_TYPE 0x12
#endif

#ifdef __cplusplus
}
#endif
#endif /* __PLATFORM_H__ */
