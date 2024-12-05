/**
 * @file     fusb15xxx_tcpd.h
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
#ifndef __FUSB15XXX_TCPD_H__
#define __FUSB15XXX_TCPD_H__

#ifdef __cplusplus
extern "C"
{
#endif

#if (DEVICE_TYPE == FUSB15101)
    struct _HAL_VBUS;
#endif
    /**
 * Select PD Revision to set
 */
    typedef enum
    {
        HAL_PD_REV1        = 0,          ///< PD Revision 1.0
        HAL_PD_REV2        = 1,          ///< PD Revision 2.0
        HAL_PD_REV3        = 2,          ///< PD Revision 3.0
        HAL_PD_REV_DEFAULT = HAL_PD_REV2 ///< Default PD Revision
    } HAL_PD_REV_T;

    /**
 * Select the Peripheral to initialize
 */
    typedef enum
    {
        HAL_USBPD_CH_A = 0, ///< USBPD Peripheral A
        HAL_USBPD_CH_B = 1, ///< USBPD Peripheral B
        HAL_USBPD_CH_COUNT,
    } HAL_USBPD_CHID_T;

    /**
 * DRP status of the CC pin
 */
    typedef enum
    {
        HAL_DRP_INACTIVE = 0, ///< DRP is not active
        HAL_DRP_ACTIVE,       ///< DRP is active and looking for connection
        HAL_DRP_SOURCE,       ///< DRP Settled as source
        HAL_DRP_SINK,         ///< DRP Settled as sink
    } HAL_DRP_STAT_T;

    /**
 * Mode to use when DRP toggling
 */
    typedef enum
    {
        HAL_DRP_MODE_DRP        = 0, ///< DRP Mode
        HAL_DRP_MODE_SINKACC    = 1, ///< DRP with Sink Accessory
        HAL_DRP_MODE_SINKVPD    = 2, ///< DRP with Sink VPD
        HAL_DRP_MODE_SINKVPDACC = 3, ///< DRP with Sink Accessory and VPD
        HAL_DRP_MODE_DISABLED   = 4, ///< Disable DRP
        HAL_DRP_MODE_NOCHANGE   = 5, ///< Do not change DRP mode
    } HAL_DRP_MODE_T;

    /**
 * Represents the device configured termination
 */
    typedef enum
    {
        HAL_CC_TERM_RA   = 0, ///< Termination Ra
        HAL_CC_TERM_RP   = 1, ///< Termination Rp
        HAL_CC_TERM_RD   = 2, ///< Termination Rd
        HAL_CC_TERM_OPEN = 3, ///< Termination None
    } HAL_CC_TERM_T;

    /**
 * Pullup value
 */
    typedef enum
    {
        HAL_CC_RP_OFF  = 0,
        HAL_CC_RP_DEF  = 1,
        HAL_CC_RP_1p5A = 2,
        HAL_CC_RP_3p0A = 3,
        HAL_CC_RP_NOCHANGE,
    } HAL_CC_RP_T;

    /**
 * Represents the measured state of the attached port partner
 */
    typedef enum
    {
        HAL_CC_STAT_SRCOPEN  = 0,
        HAL_CC_STAT_RA       = 1,
        HAL_CC_STAT_RD       = 2,
        HAL_CC_STAT_RESERVED = 3,
        HAL_CC_STAT_RPUSB    = 4,
        HAL_CC_STAT_RP1p5    = 5,
        HAL_CC_STAT_RP3p0    = 6,
        HAL_CC_STAT_SNKOPEN  = 7,
        HAL_CC_STAT_UNDEFINED,
    } HAL_CC_STAT_T;

    /**
 * CC pin
 */
    typedef enum
    {
        HAL_CC1 = 0x1,
        HAL_CC2 = 0x2,
    } HAL_CC_T;

    /**
 * Discharge Resistor value for VBUS
 */
    typedef enum
    {
        HAL_VBUS_DISCH_NONE = 0,
        HAL_VBUS_DISCH0     = 1,
        HAL_VBUS_DISCH1     = 2,
        HAL_VBUS_DISCH2     = 3,
        HAL_VBUS_DISCH      = HAL_VBUS_DISCH2, ///< Default discharge
        HAL_VBUS_DISCH3     = 4,
        HAL_VBUS_DISCH4     = 5,
        HAL_VBUS_DISCH5     = 6,
        HAL_VBUS_BLEED      = HAL_VBUS_DISCH5, ///< Bleed Discharge
    } HAL_VBUS_DISCH_T;

    /**
 * Configure VBUS Source/Sink switch active
 * low or active High
 */
    typedef enum
    {
        HAL_VBUSIO_SOURCE = 0x1 << 0, ///< VBUS Source/Sink is Active High
        HAL_VBUSIO_SINK   = 0x1 << 1, ///< VBUS Sink is Active High
    } HAL_VBUSIO_T;

    /**
 * Discharge Resistor value for VCONN
 */

#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
    typedef enum
    {
        HAL_VCONN_DISCH_NONE    = 0,
        HAL_VCONN_DISCH_DEFAULT = 1, ///< Default discharge
    } HAL_VCONN_DISCH_T;

#elif (DEVICE_TYPE == FUSB15201P) || (DEVICE_TYPE == FUSB15101)
typedef enum
{
    HAL_VCONN_DISCH_NONE = 0,
    HAL_VCONN_DISCH      = 1, ///< Default discharge
    HAL_VCONN_DISCH1     = 2, ///< Stronger discharge
} HAL_VCONN_DISCH_T;
#endif
    /**
 * SOP type
 */
    typedef enum
    {
        HAL_SOP_DISABLE_ALL     = 0 << 0, ///< Disable all
        HAL_SOP_TYPE_SOP        = 1 << 0, ///< SOP
        HAL_SOP_TYPE_SOP1       = 1 << 1, ///< SOP'
        HAL_SOP_TYPE_SOP2       = 1 << 2, ///< SOP''
        HAL_SOP_TYPE_SOP1_DEBUG = 1 << 3, ///< SOP' Debug
        HAL_SOP_TYPE_SOP2_DEBUG = 1 << 4, ///< SOP'' Debug
        HAL_SOP_TYPE_ERROR      = 1 << 5, ///< Unknown SOP
    } HAL_SOP_T;

    /**
 * Status of Transmitter
 */
    typedef enum
    {
        HAL_TX_SUCCESS = 0, ///< Message transmitted successfully
        HAL_TX_BUSY,        ///< Message is being transmitted
        HAL_TX_RETRY,       ///< Retry is active
        HAL_TX_DISCARD,     ///< Message is discarded
        HAL_TX_FAILED,      ///< Message transmit failed
    } HAL_TX_STAT_T;

    typedef struct
    {
        uint16_t cc1       :4; ///< CC1 termination
        uint16_t cc2       :4; ///< CC2 termination
        uint16_t drp       :2; ///< DRP is active
        uint16_t vconn     :1; ///< Vconn is enabled
        uint16_t vconn_ocp :1; ///< Vconn OCP triggered
        uint16_t cc_ovp    :1; ///< CC OVP triggered
        uint16_t frswap    :1; ///< Fast Role swap detected
    } HAL_CCSTAT_T;

    /**
 * Status of VBUS
 */
    typedef struct
    {
        uint8_t valid  :1; ///< Vbus is valid
        uint8_t vsafe0 :1; ///< Vbus is vsafe0V
        uint8_t ocp    :1; ///< ocp trigerred
        uint8_t ovp    :1; ///< ovp triggered
        uint8_t alarml :1; ///< Alarm low is set
        uint8_t alarmh :1; ///< Alarm high is set
        uint8_t uvp    :1;
    } HAL_VBUSSTAT_T;

    /**
 * Status of port
 */
    typedef struct
    {
        uint8_t otp :1; ///< OTP Condition
    } HAL_PORTSTAT_T;
    /**
 * PD Status shared fields.
 */
    typedef struct
    {
        uint8_t rxmsg  :1; ///< new message receive
        uint8_t txstat :4; ///< Tx failed
        uint8_t hrdrst :1; ///< Rx hard reset
        uint8_t cblrst :1; ///< Rx cable rst
        uint8_t ccbusy :1; ///< PD Tx/Rx cc is is busy or not
    } HAL_PDSTAT_T;

    /**
 * VCONN OCP Values
 */
    typedef enum
    {
        HAL_VCONN_OCP_10mA  = 0,
        HAL_VCONN_OCP_20mA  = 1,
        HAL_VCONN_OCP_30mA  = 2,
        HAL_VCONN_OCP_40mA  = 3,
        HAL_VCONN_OCP_50mA  = 4,
        HAL_VCONN_OCP_60mA  = 5,
        HAL_VCONN_OCP_70mA  = 6,
        HAL_VCONN_OCP_80mA  = 7,
        HAL_VCONN_OCP_100mA = 8,
        HAL_VCONN_OCP_200mA = 9,
        HAL_VCONN_OCP_300mA = 10,
        HAL_VCONN_OCP_400mA = 11,
        HAL_VCONN_OCP_500mA = 12,
        HAL_VCONN_OCP_600mA = 13,
        HAL_VCONN_OCP_700mA = 14,
        HAL_VCONN_OCP_800mA = 15,
        HAL_VCONN_OCP_OFF   = ~0,
    } HAL_VCONN_OCP_T;

    /**
 * Set BIST state
 */
    typedef enum
    {
        HAL_BIST_TX = 0,  ///< Transmit BIST signal
        HAL_BIST_RX,      ///< Receive BIST messages
        HAL_BIST_DISABLE, ///< Disable BIST
    } HAL_BIST_T;

    /**
 *
 */
    typedef enum
    {
        HAL_FRSWAP_DISABLED       = 0,
        HAL_FRSWAP_SOURCE_TO_SINK = 1,
        HAL_FRSWAP_SINK_TO_SOURCE = 2,
        HAL_FRSWAP_HOLD_STATE     = 3,
    } HAL_FRSWAP_T;

    /* Definition is private */
    struct _HAL_USBPD;
    typedef struct _HAL_USBPD HAL_USBPD_T;

    typedef struct
    {
        HAL_USBPD_T *(*Initialize)(HAL_USBPD_CHID_T, bool src_snk_enable, bool reinit);
        void (*DeIntialize)(HAL_USBPD_T *);
        void (*EventsEnable)(HAL_USBPD_T *, bool);
        HAL_PORTSTAT_T *(*Status)(HAL_USBPD_T *);
        void (*Enable)(HAL_USBPD_T *);
        /* CC related functions */
        struct
        {
            void (*Termination)(HAL_USBPD_T *, int, HAL_CC_TERM_T);
            void (*SetCC)(HAL_USBPD_T *, int, HAL_CC_TERM_T, HAL_CC_RP_T, HAL_DRP_MODE_T);
            void (*Pullup)(HAL_USBPD_T *, int, HAL_CC_RP_T);
            void (*Drp)(HAL_USBPD_T *, HAL_DRP_MODE_T);
            void (*Orientation)(HAL_USBPD_T *, HAL_CC_T);
            void (*SetVconnOcp)(HAL_USBPD_T *, HAL_VCONN_OCP_T);
            void (*VconnEnable)(HAL_USBPD_T *, bool, HAL_VCONN_OCP_T ocp);
            void (*VconnDischarge)(HAL_USBPD_T *, HAL_VCONN_DISCH_T);
            HAL_CCSTAT_T *(*Status)(HAL_USBPD_T *);
            void (*EventsEnable)(HAL_USBPD_T *, bool);
            void (*FastRoleSwapConfigure)(HAL_USBPD_T *, HAL_FRSWAP_T, HAL_VBUSIO_T);
            void (*FastRoleSwap)(HAL_USBPD_T *);
        } cc;
        /* PD Related functions */
        struct
        {
            void (*Enable)(HAL_USBPD_T *);
            void (*Disable)(HAL_USBPD_T *);
            void (*SetDfp)(HAL_USBPD_T *, bool);
            void (*SetPdRevision)(HAL_USBPD_T *, HAL_PD_REV_T);
            void (*SetGoodCRCRevision)(HAL_USBPD_T *, HAL_PD_REV_T);
            void (*Source)(HAL_USBPD_T *, bool);
            void (*SopEnable)(HAL_USBPD_T *, HAL_SOP_T);
            void (*BistEnable)(HAL_USBPD_T *, HAL_BIST_T);
            void (*TransmitHardReset)(HAL_USBPD_T *, HAL_SOP_T);
            HAL_SOP_T (*Receive)(HAL_USBPD_T *, uint8_t *, uint32_t);
            HAL_ERROR_T (*Transmit)(HAL_USBPD_T *, uint8_t const *, uint32_t, HAL_SOP_T, uint32_t);
            HAL_PDSTAT_T *(*Status)(HAL_USBPD_T *);
            void (*EventsEnable)(HAL_USBPD_T *, bool);
        } pd;
        /* VBUS related functions*/
        struct
        {
            void (*Initialize)(HAL_USBPD_T *);
            uint32_t (*Value)(HAL_USBPD_T *);
#if DEVICE_TYPE == FUSB15101
            uint32_t (*PPSValue)(HAL_USBPD_T *);
#endif
            void (*Set)(HAL_USBPD_T *, HAL_VBUSIO_T, uint32_t);
            void (*SetResolution)(HAL_USBPD_T *, uint32_t);
            void (*Clear)(HAL_USBPD_T *, HAL_VBUSIO_T, uint32_t);
            void (*Alarm)(HAL_USBPD_T *, uint32_t, uint32_t);
            void (*Discharge)(HAL_USBPD_T *, HAL_VBUS_DISCH_T);
            HAL_VBUSSTAT_T *(*Status)(HAL_USBPD_T *);
            void (*EventsEnable)(HAL_USBPD_T *, bool);
            struct _HAL_VBUS *(*GetVbus)(HAL_USBPD_T *);
            void (*Enable)(HAL_USBPD_T *d, HAL_VBUSIO_T io);
        } vbus;
    } HAL_TCPD_DRIVER_T;

    extern HAL_TCPD_DRIVER_T TCPORT_DRIVER;

    void HAL_PD_IRQHandler(HAL_USBPD_CHID_T id);

#ifdef __cplusplus
}
#endif
#endif /**@} __FUSB15XXX_TCPD_H__ */
