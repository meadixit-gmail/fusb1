/*******************************************************************************
 * @file     port.h
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

#ifndef __PORT_H__
#define __PORT_H__

#ifdef __cplusplus
extern "C"
{
#endif

    /**
 * Struct to store tcpd device structure
 */
    typedef struct port_tcpd
    {
        int                      port_tcpd_id;
        struct fusb_tcpd_device *tcpd_device;
        void                    *port_tcpd_priv;
        void                    *dpm_info_priv;
        struct port_vif_t       *vif;
    } PORT_TCPD_DEVICE_T;

    /**
 * Struct to store PS device structure
 */
    typedef struct port_ps
    {
        int                    port_ps_id;
        struct fusb_ps_device *ps_device;
        void                  *port_ps_priv;
        void                  *dpm_priv;
        //struct port_vif_t const *vif;
    } PORT_PS_DEVICE_T;

    /**
 * Struct to store DCDC device structure
 */
    typedef struct port_dcdc
    {
        struct fusb_dcdc_device *dcdc_device;
        void                    *port_dcdc_priv;
        void                    *dpm_priv;
        //struct port_vif_t const *vif;
    } PORT_DCDC_DEVICE_T;

    /**
 * Struct to store BC1P2 device structure
 */
    typedef struct port_bc1p2
    {
        int                       port_bc1p2_id;
        struct fusb_bc1p2_device *bc1p2_device;
        void                     *port_bc1p2_priv;
        void                     *dpm_priv;
    } PORT_BC1P2_DEVICE_T;

#ifdef __cplusplus
}
#endif

#endif //__PORT_H__
