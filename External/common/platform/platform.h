#ifndef PLATFORM_DEFINITONS_H
#define PLATFORM_DEFINITONS_H

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

#ifndef CONFIG_MINIMAL
#define CONFIG_MINIMAL 0
#endif

#ifndef CONFIG_FW_UPDATE
#define CONFIG_FW_UPDATE 0
#endif

#ifndef CONFIG_USB4
#define CONFIG_USB4 0
#endif

#ifndef CONFIG_EXTMSG
#define CONFIG_EXTMSG 0
#endif

#ifndef CONFIG_VDM
#define CONFIG_VDM 0
#endif

#ifndef CONFIG_SLEEP
#define CONFIG_SLEEP 0
#endif

#ifndef CONFIG_LOG
#define CONFIG_LOG 0
#endif

#ifndef CONFIG_EXTENDED_EVENT
#define CONFIG_EXTENDED_EVENT 0
#endif

#define CONFIG_ENABLED(x) (CONFIG_##x)

#if defined(FUSB15200)
#define TYPE_C_PORT_COUNT 2
#define TCPD_PORT_COUNT   2
#define LEGACY_PORT_COUNT 2
#elif defined(FUSB15101)
#define TYPE_C_PORT_COUNT 1
#define TCPD_PORT_COUNT   1
#define LEGACY_PORT_COUNT 1
#elif defined(ARM_CM0PLUS) ///< Generic CM0+
#define TYPE_C_PORT_COUNT 1
#define TCPD_PORT_COUNT   1
#define LEGACY_PORT_COUNT 1
#elif defined(__unix__)
#define TYPE_C_PORT_COUNT 1
#define TCPD_PORT_COUNT   1
#define LEGACY_PORT_COUNT 1
#elif defined(_WIN32) || defined(WIN32)
#define TYPE_C_PORT_COUNT 1
#define TCPD_PORT_COUNT   1
#define LEGACY_PORT_COUNT 1
#else
#error "Platform not selected"
#endif

#ifdef __cplusplus
#ifndef STATIC_ASSERT
#define STATIC_ASSERT(expr, msg) static_assert(expr, #msg)
#endif
#ifndef assert
#define assert(cond)             assert(cond)
#endif
#else
#ifndef STATIC_ASSERT
#define STATIC_ASSERT(expr, msg)
#endif
#ifndef assert
#define assert(cond)
#endif
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#endif
