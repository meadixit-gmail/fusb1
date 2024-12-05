/*******************************************************************************
 * @file     dev_tcpd.h
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
 * @
 ******************************************************************************/
#ifndef __DEV_TCPD_H__
#define __DEV_TCPD_H__

#include "port_tcpd.h"
#include "port.h"
#include "FUSB15xxx.h"
#include "fusbdev_hal.h"

#ifdef __cplusplus
extern "C"
{
#endif

    enum sink_tx_state_t dev_tcpd_port_sinktx_state(struct port *);

    /**
 *
 */
    bool dev_tcpd_port_set_sinktx(struct port *, enum sink_tx_state_t);

    /**
 * @brief Enable vconn
 */
    bool dev_tcpd_port_set_vconn(struct port *, bool);

    void dev_tcpd_port_pd_set_sop_ver(struct port *port, enum pd_rev_t rev);

    void dev_tcpd_port_pd_set_cable_ver(struct port *port, enum pd_rev_t rev);

    void dev_tcpd_port_pd_set_ver(struct port *port, enum sop_t sop, enum pd_rev_t rev);

    bool dev_tcpd_port_vbus_src(struct port *port, unsigned mv, unsigned ma, enum pdo_t type);

    bool dev_tcpd_port_vbus_snk(struct port *port, unsigned mv, unsigned ma, enum pdo_t type);

    void dev_tcpd_port_vbus_init(struct port *port, bool en);

    /**
 * @brief check the current PD version active
 */
    static inline enum pd_rev_t port_pd_ver(struct port *port, enum sop_t sop)
    {
        if (sop < SOP_SIZE)
        {
            return port->pd_rev[sop];
        }
        else
        {
            return port->pd_rev[SOP_SOP0];
        }
    }

    /**
 * @brief
 */
    static inline bool port_vbus_in_range(struct port *port, unsigned low, unsigned high)
    {
        unsigned voltage = fusbdev_tcpd_vbus_value(port->tcpd_device);
        return (voltage <= high && voltage >= low) ? true : false;
    }

    /**
 * @brief check if vbus is greater than
 */
    static inline bool port_vbus_above(struct port *port, unsigned mv)
    {
        /* Returns true when voltage is greather than (mv + %5)*/
#if(CONFIG_ENABLED(EPR_TEST))
        if (mv >21000)
    	{
        	mv = 21000;
    	}
#endif
        return port_vbus_in_range(port, VBUS_LOW_THRESHOLD(mv), ~0);
    }

    /**
 * @brief check if vbus is greater than
 */
    static inline bool port_vbus_below(struct port *port, unsigned mv)
    {
        /* Returns true when voltage is less than (mv - 5%) */
        return port_vbus_in_range(port, 0, VBUS_LOW_THRESHOLD(mv));
    }

    /**
 * @brief
 */
    static inline bool port_vbus_valid(struct port *port, unsigned mv)
    {
#if (CONFIG_ENABLED(EPR_TEST))
        if (mv > 21000)
        {
            mv = 21000;
        }
#endif
        /* VBUS within 5% */
        return port_vbus_in_range(port, VBUS_LOW_THRESHOLD(mv), VBUS_HIGH_THRESHOLD(mv));
    }

    /**
 * @brief check if vbus is vsafe0v
 */
    static inline bool port_vbus_vsafe0v(struct port *port)
    {
        /* Returns true when voltage is less than (0.8 - 10%) V */
        return port_vbus_in_range(port, 0, VBUS_VSAFE0_THRESHOLD(VBUS_VSAFE0));
    }

    /**
 * @brief check if vbus is within vsafe5v
 */
    static inline bool port_vbus_sink_min(struct port *port)
    {
        /* Returns true when voltage is within 4.3V - 5.5V */
        return port_vbus_in_range(port, VBUS_SINK_LOW_THRESHOLD, VBUS_10PCT_HIGH_THRESHOLD(5000));
    }

    /**
 * @brief check if vbus is within vsafe5v
 */
    static inline bool port_vbus_vsafe5v(struct port *port)
    {
        /* Returns true when voltage is within 4.75V - 5.5V */
        return port_vbus_in_range(port, VBUS_LOW_THRESHOLD(VBUS_5V),
                                  VBUS_10PCT_HIGH_THRESHOLD(VBUS_5V));
    }

    /**
 * @brief
 * @return max cable current in ma
 */
    unsigned port_max_cable_current(struct port *);

    /**
 * @brief
 * @return maximum cable voltage in mv
 */
    unsigned port_max_cable_voltage(struct port *);

#ifdef __cplusplus
}
#endif

#endif /* __DEV_TCPD_H__ */
