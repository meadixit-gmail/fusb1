/**
 * @file     fusb15xx_usbchg.h
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
 * @addtogroup HAL
 * @{
 */
#ifndef __FUSB15XXX_USBCHG_H__
#define __FUSB15XXX_USBCHG_H__

#ifdef __cplusplus
extern "C"
{
#endif

    /**
* Select the Peripheral to initialize
*/
    typedef enum
    {
        HAL_USBCHG_A = 0, ///< USBPD Peripheral A
        HAL_USBCHG_B = 1, ///< USBPD Peripheral B
        HAL_USBCHG_COUNT,
    } HAL_USBCHG_CHID_T;

    /* Definition is private */
    struct _HAL_USBCHG;
    typedef struct _HAL_USBCHG HAL_USBCHG_T;

    //new format for the usbchg driver
    typedef struct
    {
        HAL_USBCHG_T *(*Enable)(HAL_USBCHG_CHID_T);
        void (*Disable)(HAL_USBCHG_T *);
        void (*Switch)(HAL_USBCHG_T *, bool);
        void (*HostSwitchEnable)(HAL_USBCHG_T *, bool);
        void (*SetIdpSource)(HAL_USBCHG_T *, bool);
        void (*SetVdpSource)(HAL_USBCHG_T *, bool);
        void (*SetVdmSource)(HAL_USBCHG_T *, bool);
        void (*SetIdpSink)(HAL_USBCHG_T *, bool);
        void (*SetIdmSink)(HAL_USBCHG_T *, bool);
        void (*SetRdpDown)(HAL_USBCHG_T *, bool);
        void (*SetRdmDown)(HAL_USBCHG_T *, bool);
        void (*SetRdiv)(HAL_USBCHG_T *, bool);
        void (*SetRdcp)(HAL_USBCHG_T *, bool);
        void (*SetRdpLeakage)(HAL_USBCHG_T *, bool);
        void (*SetRdmLeakage)(HAL_USBCHG_T *, bool);
        void (*SetMoistureDetection)(HAL_USBCHG_T *, bool);
        void (*SetMoisturePullup)(HAL_USBCHG_T                                             *d,
                                  Legacy_DP_DM_Protocol_Port_USB_PROT_CTRL_EN_RPMOS_HI_Enum rp);
        void (*OVPEnable)(HAL_USBCHG_T *d, bool en);
        void (*OVPInterruptEnable)(HAL_USBCHG_T *d, bool en);
        bool (*OVPInterruptStatus)(HAL_USBCHG_T *d);
        void (*OVPInterruptClear)(HAL_USBCHG_T *d);
        void (*OVPDebounceInterruptEnable)(HAL_USBCHG_T *d, bool en);
        bool (*OVPDebounceInterruptStatus)(HAL_USBCHG_T *d);
        void (*OVPDebounceInterruptClear)(HAL_USBCHG_T *d);
        bool (*SwitchStatus)(HAL_USBCHG_T *d);
#if (DEVICE_TYPE == FUSB15101)
        void (*DPDetachEnable)(HAL_USBCHG_T *,
                               Legacy_DP_DM_Protocol_Port_USB_PROT_CTRL_DPDETACH_DBNC_Enum);
        void (*DPDetachInterruptEnable)(HAL_USBCHG_T *, bool);
        bool (*DPDetachInterruptStatus)(HAL_USBCHG_T *);
        void (*DPDetachInterruptClear)(HAL_USBCHG_T *);
        bool (*DPDetachStatus)(HAL_USBCHG_T *);
        void (*DPDetachDisable)(HAL_USBCHG_T *);
#endif
    } const HAL_USBCHG_DRIVER_T;

    extern HAL_USBCHG_DRIVER_T USBCHG_DRIVER;

#ifdef __cplusplus
}
#endif
#endif /**@} __FUSB15XXX_USBCHG_H__ */
