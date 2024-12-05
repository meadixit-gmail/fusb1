/*******************************************************************************
 * @file     pdps_ocp.h
 * @author   USB PD Firmware Team
 *
 * @copyright @parblock
 * Copyright &copy; 2021 ON Semiconductor &reg;. All rights reserved.
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
#ifndef _PDPS_OCP_H_
#define _PDPS_OCP_H_

#include "pd_types.h"
#include "timer.h"

#ifdef __cplusplus
extern "C"
{
#endif
    enum pdps_ocp_state
    {
        PDPS_OCP_DISABLED,
        PDPS_OCP_RECOVERY,
        PDPS_OCP_ATTACH,
        PDPS_OCP_WAIT_DETACH,
        PDPS_OCP_WAIT_SOURCE,
        PDPS_OCP_WAIT_POWER
    };

    struct pdps_ocp
    {
        struct ticktimer_t  pdps_ocp_timer;
        enum pdps_ocp_state ocp_state;
        uint8_t             ocp_count;
    };

#define OCP_COUNT_MAX       10
#define PDPS_OCP_ATTACH_MAX (70) /* ms */

    /**
 * @brief Initialize PDPS OCP flow
 */
    void pdps_ocp_init(void *ocp_dev);

    /**
 * @brief Starts PDPS OCP flow
 */
    void pdps_ocp_reset(void *ocp_dev, void *fusb_tcpd, void *fusb_bc12, void *p_tcpd);

    /**
 * @brief Disables port while maintaining Rp for detach detection
 */
    void pdps_ocp_disable_port(void *ocp_dev, void *ps_dev, void *fusb_bc12, void *p_tcpd);

    /**
 * @brief State machine for PDPS OCP flow
 */
    void pdps_ocp_sm(void *ocp_dev, void *ps_dev, void *fusb_bc12, void * port_dev);

    /**
 * @brief Return PDPS OCP timeout
 */
    uint32_t pdps_ocp_min_timer(void *ocp_dev);

#ifdef __cplusplus
}
#endif
#endif /* _PDPS_OCP_H_ */
