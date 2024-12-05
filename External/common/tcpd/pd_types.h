/*******************************************************************************
 * @file     pd_types.h
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
#ifndef _PDTYPES_H_
#define _PDTYPES_H_

#include "typec_types.h"

// add conditional extension for X macro
#define IF(cond, foo...) IF_IMPL(cond, foo)
#define IF_IMPL(cond, foo...) IF_ ## cond (foo)
#define IF_0(foo...)
#define IF_1(foo...) foo

#define MIN(a, b)         (((a) < (b)) ? (a) : (b))

#if (CONFIG_ENABLED(SRC))
#define SRC_STATES 1
#if CONFIG_ENABLED(EPR)
#define SRC_EPR_STATES 1
#else
#define SRC_EPR_STATES 0
#endif
#else
#define SRC_STATES 0
#define SRC_EPR_STATES 0
#endif

#if (CONFIG_ENABLED(SNK))
#define SNK_STATES 1
#if (CONFIG_ENABLED(EPR))
#define SNK_EPR_STATES 1
#else
#define SNK_EPR_STATES 0
#endif
#else
#define SNK_EPR_STATES 0
#define SNK_STATES 0
#endif

#if (CONFIG_ENABLED(DRP))
#define DRP_STATES 1
#else
#define DRP_STATES 0
#endif

#if (CONFIG_ENABLED(VDM))
#define VDM_STATES 1
#if (CONFIG_ENABLED(VDM_MINIMAL))
#define VDM_EXTRA 0
#else
#define VDM_EXTRA 1
#endif
#else
#define VDM_STATES 0
#define VDM_EXTRA 0
#endif

#if (CONFIG_ENABLED(USB4))
#define USB4_STATES 1
#else
#define USB4_STATES 0
#endif

#ifdef __cplusplus
extern "C"
{
#endif

/* USB PD Extended Message Values */
#define MAX_MSG_LEGACY_LEN     28
#define MAX_EXT_MSG_LEGACY_LEN 26
#define MAX_EXT_MSG_LEN        260

#define STAT_BUSY    0
#define STAT_SUCCESS 1
#define STAT_ERROR   2
#define STAT_ABORT   3

/* USB PD Header Message Definitions */
#define PDSpecRev1p0 0x0
#define PDSpecRev2p0 0x1
#define PDSpecRev3p0 0x2
/* EPR Mode Data failure bits */
#define EPREnterFailUnknown          0
#define EPREnterFailCable            1
#define EPREnterFailVConn            2
#define EPREnterFailNotEPRCapableRDO 3
#define EPREnterFailEPRNotAvailable  4
#define EPREnterFailNotEPRCapablePDO 5
#define CAPTIVE_CABLE                3U

#define PDPortRoleSink    0
#define PDPortRoleSource  1
#define PDDataRoleUFP     0
#define PDDataRoleDFP     1
#define PDCablePlugSource 0
#define PDCablePlugPlug   1
#define PDMsgTypeMask     0x1F

#define USBPDSPECREV PDSpecRev2p0 /* Revision 2.0 */

/* USB PD Control Message Types */
#define CMTHardreset          0x00U
#define CMTGoodCRC            0x01U
#define CMTGotoMin            0x02U
#define CMTAccept             0x03U
#define CMTReject             0x04U
#define CMTPing               0x05U
#define CMTPS_RDY             0x06U
#define CMTGetSourceCap       0x07U
#define CMTGetSinkCap         0x08U
#define CMTDR_Swap            0x09U
#define CMTPR_Swap            0x0AU
#define CMTVCONN_Swap         0x0BU
#define CMTWait               0x0CU
#define CMTSoftReset          0x0DU
#define CMTDataReset          0x0EU
#define CMTDataResetComplete  0x0FU
#define CMTNotSupported       0x10U
#define CMTGetSourceCapExt    0x11U
#define CMTGetStatus          0x12U
#define CMTFR_Swap            0x13U
#define CMTGetPPSStatus       0x14U
#define CMTGetCountryCodes    0x15U
#define CMTGetSinkCapExtended 0x16U
#define CMTGetSourceInfo 0x17U
#define CMTGetRevision 0x18U

/* USB PD Data Message Types */
#define DMTSourceCapabilities 0x01U
#define DMTRequest            0x02U
#define DMTBIST               0x03U
#define DMTSinkCapabilities   0x04U
#define DMTBatteryStatus      0x05U
#define DMTAlert              0x06U
#define DMTGetCountryInfo     0x07U
#define DMTEnterUSB           0x08U
#define DMTVendorDefined      0x0FU

#define DMTSourceInfo     0x0B
#define DMTRevision       0x0C
/* USB PD EPR MODE */
#define DMTEPRMode   0x0A
#define DMTEPRequest 0x09

/* Extended Message Types */
#define EMTSourceCapsExtended  0x01U
#define EMTStatus              0x02U
#define EMTGetBatteryCap       0x03U
#define EMTGetBatteryStatus    0x04U
#define EMTBatteryCapabilities 0x05U
#define EMTGetManufacturerInfo 0x06U
#define EMTManufacturerInfo    0x07U
#define EMTSecurityRequest     0x08U
#define EMTSecurityResponse    0x09U
#define EMTFWUpdateRequest     0x0AU
#define EMTFWUpdateResponse    0x0BU
#define EMTPPSStatus           0x0CU
#define EMTCountryInfo         0x0DU
#define EMTCountryCodes        0x0EU
#define EMTSinkCapExtended     0x0FU
/* New EPR codes */
#define EMTExtendedControl       0x10U
#define EMTEPRSourceCapabilities 0x11
#define EMTEPRSinkCapabilities   0x12
/* Extended Control Message Types */
#define EPR_Get_Source_Cap 1
#define EPR_Get_Sink_Cap   2
#define EPR_KeepAlive      3
#define EPR_KeepAlive_Ack  4
/* BIST Data Objects */
#define BDO_BIST_Receiver_Mode          0x0U
#define BDO_BIST_Transmit_Mode          0x1U
#define BDO_Returned_BIST_Counters      0x2U
#define BDO_BIST_Carrier_Mode_0         0x3U
#define BDO_BIST_Carrier_Mode_1         0x4U
#define BDO_BIST_Carrier_Mode_2         0x5U
#define BDO_BIST_Carrier_Mode_3         0x6U
#define BDO_BIST_Eye_Pattern            0x7U
#define BDO_BIST_Test_Data              0x8U
#define BDO_BIST_Shared_Test_Mode_Enter 0x9U
#define BDO_BIST_Shared_Test_Mode_Exit  0xAU

/* Timers need to have tolerance of +/-5% so using spec/0.95  */
#define ktDeviceToggle  (3U)
#define ktTOG2          (30U)
#define ktIllegalCable  (150U)
#define ktVBusPollShort (10U)

#define ktNoResponse         (5000U)
#define ktSenderResponse     (28U)
#define ktExtendedMessage    (26U)
#define ktTypeCSendSourceCap (150U)
#define ktRejectRecovery     (250U)
#define ktTypeCSinkWaitCap   (350U)
#define ktSrcTransition      (30U)
#define ktHardResetComplete  (5U)
#if (DEVICE_TYPE == FUSB15201P)
#define ktPSHardReset (28U)
#else
#define ktPSHardReset (30U)
#endif
#define ktPSHardResetMax        (35U)
#define ktPSHardResetOverhead   (3U)
#define ktPSTransition          (500U)
#define ktPSSourceOff           (770U)
#define ktPSSourceOn            (410U)
#define ktVCONNSourceOn         (90U)
#define ktVCONNOnWait           (2U)
#define ktBISTContMode          (50U)
#define ktSwapSourceStart       (25U)
#define ktSrcRecover            (860U)
#define ktSrcRecoverMax         (1000U)
#define ktSrcRecoverEPR         (1130U)
#define ktSrcRecoverEPRMax      (1425U)
#define ktGoodCRCDelay          (1U)
#define ktSwitchDelay           (10U)
#define ktSafe0V                (650U)
#define tEnterEPR               (500U)
#define ktSrcTurnOn             (275U)
#define ktSrcStartupVbus        (150U)
#define ktSnkTransDefVbus       (150U) /* 1.5s for VBus */
#define ktSinkTx                (18U)
#define ktChunkReceiverRequest  (15U)
#define ktChunkReceiverResponse (15U)
#define ktChunkSenderRequest    (30U)
#define ktChunkSenderResponse   (30U)
#define ktChunkingNotSupported  (40U)

#define ktPPSTimeout       (12640U)
#define ktPPSRequest       (10000U)

#if (DEVICE_TYPE == FUSB15101)
#define ktPpsSrcTransLarge (230U)
#else
#define ktPpsSrcTransLarge (235U)
#endif

#if (CONFIG_ENABLED(CUSTOM_APPLICATION))
#if (DEVICE_TYPE == FUSB15201P)
#define ktPpsSrcTransSmall (12U)
#else
#define ktPpsSrcTransSmall (18U)
#endif
#else	/* NO PDPS ENABLED */
#if (DEVICE_TYPE == FUSB15101)
#define ktPpsSrcTransSmall (13U)
#else
#define ktPpsSrcTransSmall (20U)
#endif
#endif /* PDPS */

#define ktDiscoverIdentity  (40U)
#define ktVDMSenderResponse (27U)
#define ktVDMWaitModeEntry  (50U)
#define ktVDMWaitModeExit   (50U)

#define ktDataResetFail        (350U)
#define ktDataResetFailUFP     (500U)
#define ktDataReset            (200U)
#define ktVCONNSourceDischarge (200U)
#define ktVCONNZero            (125U)
#define ktVCONNReapplied       (15U)
#define ktVCONNValid           (2U)
#define ktRPValueChange        (13U)
#define ktUVPDebounce          (60U)

/* USB EPR keepAlive */
#define ktEPRSourceKeepAlive (875U)
#define ktEPRSinkKeepAlive   (375U)
#define ktEnterUSB           (600U)
#define ktEnterUSBWait        (200U)
#define ktEnterUSBUFP        (200U)
/* TODO: What is this? */
#define tVbusTransTime (20U)

#define vPpsSmallStep (500U)

#define HARD_RESET_COUNT 2
#define RETRIES_PD20     3
#define RETRIES_PD30     2
#define MAX_CAPS_COUNT   50

/* Maximum number of discover sop'. Actual number of message sent will need
 * to account for retries.
 */
#define MAX_DISC_ID_COUNT 20

/* PD Voltage and Current conversion */
#define PD_100MV_STEP_TO_MV(x) ((x)*100)
#define PD_50MV_STEP_TO_MV(x)  ((x)*50)
#define PD_20MV_STEP_TO_MV(x)  ((x)*20)
#define PD_10MA_STEP_TO_MA(x)  ((x)*10)
#define PD_50MA_STEP_TO_MA(x)  ((x)*50)
#define PD_250MW_STEP_TO_MW(x) ((x)*250)

/** Convert to PD voltage, current and power */
#define PD_MV_STEP_TO_50MV_STEP(x)  ((x) / 50)
#define PD_MA_STEP_TO_10MA_STEP(x)  ((x) / 10)
#define PD_MA_STEP_TO_50MA_STEP(x)  ((x) / 50)
#define PD_MW_STEP_TO_250MW_STEP(x) ((x) / 250)

/* This is same as (mV*50*mA*10/1000)  */
#define PD_0P5MW_STEP_TO_MW(x) ((x) / 2)

#define PD_MV_TO_50MV_STEP(x)  ((x) / 50)
#define PD_MA_TO_10MA_STEP(x)  ((x) / 10)
#define PD_MW_TO_250MW_STEP(x) ((x) / 250)
#if CONFIG_ENABLED(DCDC)
#define DCDC_VBUS_SCALE(mv) ((mv)+200)
#endif
/* PD Voltage values in 50mV resolution */
#define PD_05V PD_MV_TO_50MV_STEP(5000)
#define PD_09V PD_MV_TO_50MV_STEP(9000)
#define PD_12V PD_MV_TO_50MV_STEP(12000)
#define PD_15V PD_MV_TO_50MV_STEP(15000)
#define PD_20V PD_MV_TO_50MV_STEP(20000)
    /**
 * Defines the Status message from Protocol to Policy
 */
    typedef enum port_tx_stat_t
    {
        TX_SUCCESS = 0,
        TX_BUSY,
        TX_RETRY,
        TX_DISCARD,
        TX_FAILED,
    } PORT_TX_STAT_T;

    typedef enum pps_mode_t
    {
        PPS_MODE_CV = 0,
        PPS_MODE_CL,
    } PPS_MODE_T;

    typedef enum pdfu_protocol_ver_t
    {
        PDFU_PROTOCOL_V1 = 1,
    } PDFU_PROTOCOL_VER_T;

    typedef enum pd_rev_t
    {
        PD_REV1        = 0,
        PD_REV2        = 1,
        PD_REV3        = 2,
        PD_REV_DEFAULT = PD_REV2,
    } PD_REV_T;

    typedef enum eusb_cabletype_t
	{
		USB_PASSIVE          = 0,
		USB_ACTIVE_RETIMER   = 1,
		USB_ACTIVE_REDRIVER  = 2,
		USB_OPTICAL_ISOLATED = 3,
	} EUSB_CABLETYPE_T;
        
    typedef enum sop_t
    {
        SOP_SOP0 = 0,
        SOP_SOP1 = 1,
        SOP_SOP2 = 2,
        SOP_SIZE,
        SOP_ERROR = SOP_SIZE
    } SOP_T;

#define SOP_MASK(x) (1U << x)

    typedef enum sop_mask_t
    {
        SOP_MASK_SOP0 = SOP_MASK(SOP_SOP0),
        SOP_MASK_SOP1 = SOP_MASK(SOP_SOP1),
        SOP_MASK_SOP2 = SOP_MASK(SOP_SOP2),
    } SOP_MASK_T;

    typedef enum bist_mode_t
    {
        BIST_MODE_DISABLE = 0,
        BIST_MODE_RX      = 1,
        BIST_MODE_TX,
    } BIST_MODE_T;

    typedef enum alert_type_t
    {
        ALERT_TYPE_BATTERY   = (1U << 1),
        ALERT_TYPE_OCP       = (1U << 2),
        ALERT_TYPE_OTP       = (1U << 3),
        ALERT_TYPE_OPERATION = (1U << 4),
        ALERT_TYPE_SOURCE_IN = (1U << 5),
        ALERT_TYPE_OVP       = (1U << 6),
    } ALERT_TYPE_T;

#define PD_HEADER_LEN  2U
#define EXT_HEADER_LEN 2U

    typedef struct pd_header_t
    {
        union
        {
            uint16_t word;
            uint8_t  byte[PD_HEADER_LEN];
            struct
            {
                uint16_t MessageType    :5; ///< 0-4      : Message Type
                uint16_t PortDataRole   :1; ///< 5        : Port Data Role
                uint16_t SpecRevision   :2; ///< 6-7      : Specification Revision
                uint16_t PortPowerRole  :1; ///< 8        : Port Power Role
                uint16_t MessageID      :3; ///< 9-11     : Message ID
                uint16_t NumDataObjects :3; ///< 12-14    : Number of Data Objects
                uint16_t Extended       :1; ///< 15       : Extended Message Flag
            } sop;
            struct
            {
                uint16_t MessageType    :5; ///< 0-4      : Message Type
                uint16_t Reserved       :1; ///< 5        : Reserved */
                uint16_t SpecRevision   :2; ///< 6-7      : Specification Revision
                uint16_t CablePlug      :1; ///< 8        : Cable Plug
                uint16_t MessageID      :3; ///< 9-11     : Message ID
                uint16_t NumDataObjects :3; ///< 12-14    : Number of Data Objects
                uint16_t Extended       :1; ///< 15       : Extended Message Flag
            } cbl;
            struct
            {
                uint16_t DataSize     :9; ///< 0-8        : Full Message Length
                uint16_t Reserved     :1; ///< 9          : Message ID
                uint16_t RequestChunk :1; ///< 10         : Request Chunk
                uint16_t ChunkNumber  :4; ///< 11-14      : Chunk Number
                uint16_t Chunked      :1; ///< 15         : Chunked Message Flag
            } ext;
        };
    } PD_HEADER_T;
    STATIC_ASSERT(sizeof(PD_HEADER_T) == PD_HEADER_LEN, pd_header_t_must_be_2_bytes);

#define PD_DATA_OBJECT_LEN 4

    typedef struct pd_msg_t
    {
        union
        {
            uint32_t object;
            uint16_t word[2];
            uint8_t  byte[PD_DATA_OBJECT_LEN];
            struct
            {
                uint32_t            :30;
                uint32_t SupplyType :2;
            } PDO;
            struct
            {
                uint32_t MaxCurrent        :10;
                uint32_t Voltage           :10;
                uint32_t PeakCurrent       :2;
                uint32_t                   :1;
                uint32_t EPRCapable        :1;
                uint32_t UnchunkedExtCap   :1;
                uint32_t DataRoleSwap      :1;
                uint32_t USBCommCapable    :1;
                uint32_t ExternallyPowered :1;
                uint32_t USBSuspendSupport :1;
                uint32_t DualRolePower     :1;
                uint32_t SupplyType        :2;
            } FPDOSupply;
            struct
            {
                uint32_t OperationalCurrent :10;
                uint32_t Voltage            :10;
                uint32_t                    :5;
                uint32_t DataRoleSwap       :1;
                uint32_t USBCommCapable     :1;
                uint32_t ExternallyPowered  :1;
                uint32_t HigherCapability   :1;
                uint32_t DualRolePower      :1;
                uint32_t SupplyType         :2;
            } FPDOSink;
            struct
            {
                uint32_t MaxCurrent :10;
                uint32_t MinVoltage :10;
                uint32_t MaxVoltage :10;
                uint32_t SupplyType :2;
            } VPDO;
            struct
            {
                uint32_t MaxPower   :10;
                uint32_t MinVoltage :10;
                uint32_t MaxVoltage :10;
                uint32_t SupplyType :2;
            } BPDO;
            struct
            {
                uint32_t            :28;
                uint32_t APDOType   :2;
                uint32_t SupplyType :2;
            } APDO;
            struct
            {
                uint32_t MaxCurrent   :7;
                uint32_t              :1;
                uint32_t MinVoltage   :8;
                uint32_t              :1;
                uint32_t MaxVoltage   :8;
                uint32_t              :2;
                uint32_t PowerLimited :1;
                uint32_t APDOType     :2;
                uint32_t SupplyType   :2;
            } PPSAPDO;
            struct
            {
                uint32_t PDP         :8; /* PDP in 1W increments */
                uint32_t MinVoltage  :8; /* Min voltage in 100mV increments */
                uint32_t             :1;
                uint32_t MaxVoltage  :9; /* Max voltage in 100mV increments */
                uint32_t PeakCurrent :2;
                uint32_t APDOType    :2;
                uint32_t SupplyType  :2;
            } EPRAVSAPDOSupply;
            struct
            {
                uint32_t PDP        :8; /* PDP in 1W increments */
                uint32_t MinVoltage :8; /* Min voltage in 100mV increments */
                uint32_t            :1;
                uint32_t MaxVoltage :9; /* Max voltage in 100mV increments */
                uint32_t            :2;
                uint32_t APDOType     :2;
                uint32_t SupplyType :2;
            } EPRAVSAPDOSink;
            struct 
			{
            	uint32_t maxCurrent20:10;
            	uint32_t maxCurrent15:10;
            	uint32_t             :6;
            	uint32_t PeakCurrent :2;
            	uint32_t APDOType    :2;
            	uint32_t SupplyType  :2;
			} SPRAVSAPDOSupply;
            struct
            {
                uint32_t            :22;
                uint32_t EPRCapable :1;
                uint32_t            :9;
            } EPRCAPRDO;
            struct
            {
                uint32_t MinMaxCurrent      :10;
                uint32_t OpCurrent          :10;
                uint32_t Reserved0          :2;
                uint32_t EPRCapable         :1;
                uint32_t UnchunkedExtMsg    :1;
                uint32_t NoUSBSuspend       :1;
                uint32_t USBCommCapable     :1;
                uint32_t CapabilityMismatch :1;
                uint32_t GiveBack           :1;
                uint32_t ObjectPosition     :4;
            } FVRDO;
            struct
            {
                uint32_t MinMaxPower        :10;
                uint32_t OpPower            :10;
                uint32_t                    :2;
                uint32_t EPRCapable         :1;
                uint32_t UnchunkedExtMsg    :1;
                uint32_t NoUSBSuspend       :1;
                uint32_t USBCommCapable     :1;
                uint32_t CapabilityMismatch :1;
                uint32_t GiveBack           :1;
                uint32_t ObjectPosition     :4;
            } BRDO;
            struct
            {
                uint32_t OpCurrent          :7;
                uint32_t                    :2;
                uint32_t OpVoltage          :12; /* 20mV increments */
                uint32_t                    :1;
                uint32_t EPRCapable         :1;
                uint32_t UnchunkedExtMsg    :1;
                uint32_t NoUSBSuspend       :1;
                uint32_t USBCommCapable     :1;
                uint32_t CapabilityMismatch :1;
                uint32_t                    :1;
                uint32_t ObjectPosition     :4;
            } PPSRDO;
            struct
            {
                uint32_t OpCurrent          :7;
                uint32_t                    :2;
                uint32_t OpVoltage          :12; /* 25mV increments */
                uint32_t                    :1;
                uint32_t EPRCapable         :1;
                uint32_t UnchunkedExtMsg    :1;
                uint32_t NoUSBSuspend       :1;
                uint32_t USBCommCapable     :1;
                uint32_t CapabilityMismatch :1;
                uint32_t                    :1;
                uint32_t ObjectPosition     :4;
            } AVSRDO;
            struct
            {
                uint32_t                  :8;
                uint32_t InvalidReference :1;
                uint32_t BatteryPresent   :1;
                uint32_t BatteryStatus    :2;
                uint32_t                  :4;
                uint32_t BatteryCapacity  :16;
            } BSDO;
            struct
            {
                uint32_t VendorDefined :15;
                uint32_t VDMType       :1;
                uint32_t VendorID      :16;
            } UVDM;
            union
            {
                struct
                {
                    uint32_t Command     :5;
                    uint32_t Reserved0   :1;
                    uint32_t CommandType :2;
                    uint32_t ObjPos      :3;
                    uint32_t VersionMinor:2;
                    uint32_t VersionMajor:2;
                    uint32_t VDMType     :1;
                    uint32_t SVID        :16;
                } HEADER;
                struct
                {
                    uint32_t usb_vid     :16;
                    uint32_t             :5;
                    uint32_t conn_type   :2;
                    uint32_t type_dfp    :3;
                    uint32_t modal_op    :1;
                    uint32_t type_ufp    :3;
                    uint32_t device_data :1;
                    uint32_t host_data   :1;
                } ID;
                struct
                {
                    uint32_t xid :32;
                } CERTSTAT;
                struct
                {
                    uint32_t bcd_device     :16;
                    uint32_t usb_product_id :16;
                } PROD;
                struct
                {
                    uint32_t usb_highest_speed        :3;
                    uint32_t alt_mode_tbt3            :1;
                    uint32_t alt_mode_reconfig        :1;
                    uint32_t alt_mode_no_reconfig     :1;
                    uint32_t vbus_required            :1;
                    uint32_t vconn_required           :1;
                    uint32_t vconn_power              :3;
                    uint32_t                          :11;
                    uint32_t connector_type           :2;
                    uint32_t device_cap_usb_2         :1;
                    uint32_t device_cap_usb_billboard :1;
                    uint32_t device_cap_usb_3_2       :1;
                    uint32_t device_cap_usb_4         :1;
                    uint32_t                          :1;
                    uint32_t ufp_vdo_version          :3;
                } UFPVDO;
                struct
                {
                    uint32_t port_number      :5;
                    uint32_t                  :17;
                    uint32_t connector_type   :2;
                    uint32_t host_cap_usb_2   :1;
                    uint32_t host_cap_usb_3_2 :1;
                    uint32_t host_cap_usb_4   :1;
                    uint32_t                  :2;
                    uint32_t dfp_vdo_version  :3;
                } DFPVDO;
                struct
                {
                    uint32_t svid_low  :16;
                    uint32_t svid_high :16;
                } SVIDO;
                struct
                {
                    uint32_t speed      :3;
                    uint32_t            :2;
                    uint32_t current    :2;
                    uint32_t            :2;
                    uint32_t voltage    :2;
                    uint32_t term       :2;
                    uint32_t latency    :4;
                    uint32_t EPRCapable :1;
                    uint32_t connector  :2;
                    uint32_t            :1;
                    uint32_t vdover     :3;
                    uint32_t fwver      :4;
                    uint32_t hwver      :4;
                } PCBL;
                struct
                {
                    uint32_t speed      :3;
                    uint32_t sopp       :1;
                    uint32_t vbusthru   :1;
                    uint32_t current    :2;
                    uint32_t sbutype    :1;
                    uint32_t sbu        :1;
                    uint32_t voltage    :2;
                    uint32_t term       :2;
                    uint32_t latency    :4;
                    uint32_t EPRCapable :1;
                    uint32_t connector  :2;
                    uint32_t            :1;
                    uint32_t vdover     :3;
                    uint32_t fwver      :4;
                    uint32_t hwver      :4;
                } ACBL1;
                struct
                {
                    uint32_t usb_gen               :1;
                    uint32_t usb4_asymmetric_mode  :1;
                    uint32_t optically_isolated    :1;
                    uint32_t usb_lanes             :1;
                    uint32_t usb_3_2               :1;
                    uint32_t usb_2                 :1;
                    uint32_t usb_2_hub_hops        :2;
                    uint32_t usb_4                 :1;
                    uint32_t active_element        :1;
                    uint32_t physical_connection   :1;
                    uint32_t u3_u0_transition_mode :1;
                    uint32_t u3_cld_power          :3;
                    uint32_t                       :1;
                    uint32_t shutdown_temp         :8;
                    uint32_t max_operating_temp    :8;
                } ACBL2;
                struct
                {
                    uint32_t charge_thru_sup         :1;
                    uint32_t ground_impedance        :6;
                    uint32_t vbus_impedance          :6;
                    uint32_t                         :1;
                    uint32_t charge_thru_current_sup :1;
                    uint32_t max_vbus_voltage        :2;
                    uint32_t                         :4;
                    uint32_t vdo_version             :3;
                    uint32_t fw_version              :4;
                    uint32_t hw_version              :4;
                } VPDVDO;
                struct
                {
                    uint32_t usb_highest_speed :3;
                    uint32_t vbus_required     :1;
                    uint32_t vconn_required    :1;
                    uint32_t vconn_power       :3;
                    uint32_t                   :13;
                    uint32_t vdo_version       :3;
                    uint32_t fw_version        :4;
                    uint32_t hw_version        :4;
                } AMAVDO;
                struct
				{
					uint32_t AlternateMode     :16;
					uint32_t CableSpeed        :3;
					uint32_t RoundedRounded    :2;
					uint32_t Optical           :1;
					uint32_t Retimer           :1;
					uint32_t Unidirectional    :1;
					uint32_t                   :1;
					uint32_t Active            :1;
					uint32_t                   :6;
				} TBT3MODEVDO;
            } SVDM;
            struct
            {
                uint32_t          :16;
                uint32_t HSwapBat :4;
                uint32_t FixedBat :4;
                uint32_t Type     :8;
            } ALERT;
            struct
            {
                uint32_t              :13;
                uint32_t HostPresent  :1;
                uint32_t TBTSupport   :1;
                uint32_t DPSupport    :1;
                uint32_t PCIeSupport  :1;
                uint32_t CableCurrent :2;
                uint32_t CableType    :2;
                uint32_t CableSpeed   :3;
                uint32_t              :1;
                uint32_t USB3DRD      :1;
                uint32_t USB4DRD      :1;
                uint32_t              :1;
                uint32_t USBMode      :3;
                uint32_t              :1;
            } EUDO;
            /*EPR MODE and Extended Control Data block */
            struct
            {
                uint32_t        :16;
                uint32_t data   :8;
                uint32_t action :8;
            } EPRMDO;
            struct
            {
                uint32_t        :16;
                uint32_t type   :8;
                uint32_t data :8;
            } EXTCTRLDO;
	struct {
                uint32_t                 :16;
                uint32_t VersionMinor    :4;
                uint32_t VersionMajor    :4;
                uint32_t RevisionMinor   :4;
                uint32_t RevisionMajor   :4;
            } RMDO;
            struct {
                uint32_t PortReportedPDP :8;
                uint32_t PortPresentPDP  :8;
                uint32_t PortMaximumPDP  :8;
                uint32_t                 :7;
                uint32_t PortType        :1;
            } SIDO;
        };
    } PD_MSG_T;
    STATIC_ASSERT(sizeof(PD_MSG_T) == PD_DATA_OBJECT_LEN, pd_msg_t_must_be_4_bytes);
#define EXT_SRC_CAP_LEN       25U
#define EXT_STATUS_LEN        7U
#define EXT_GET_BAT_CAP_LEN   1U
#define EXT_GET_BAT_STAT_LEN  1U
#define EXT_BAT_CAP_LEN       9U
#define EXT_GET_MFI_LEN       2U
#define EXT_MFI_LEN           26U
#define EXT_SEC_REQ_LEN       260U
#define EXT_SEC_RESP_LEN      260U
#define EXT_FWU_REQ_LEN       260U
#define EXT_FWU_RESP_LEN      260U
#define EXT_PPS_STAT_LEN      4U
#define EPR_MODE_LEN          6U
#define EXT_COUNTRY_CODES_LEN 26U
#define EXT_COUNTRY_INFO_LEN  26U
#define EXT_SINK_CAP_LEN      24U
#define EXT_CONTROL_LEN       2U

#define FWVER_TYPEC(PHASE, BUILD)  (((PHASE & 0x7) << 5) | (BUILD & 0x1F))

/* HWVER */
#define HWVER_FUSB15101  	0x1
#define HWVER_FUSB15201P 	0x2
#define HWVER_FUSB15201  	0x3
#define HWVER_FUSB15200  	0x4
#define HWVER_UNDEF      	0xFF

/*
   FWVER:
            bits [7-5]: Dev Phase
                        T:0x0 C:0x1 I:0x2
                        A:0x3 B:0x4 RC:0x5 UNDEF:0x7
            bits [4-0]: Build Increment
*/
#define FWVER_PHASE_TEST	0x0
#define FWVER_PHASE_COMPLIANCE	0x1
#define FWVER_PHASE_INTEROP	0x2
#define FWVER_PHASE_ALPHA	0x3
#define FWVER_PHASE_BETA	0x4
#define FWVER_PHASE_RC		0x5
#define FWVER_PHASE_UNDEF	0x7


#if defined (FUSB15101)
#define HWVER             HWVER_FUSB15101
#define FWVER_PHASE       FWVER_PHASE_COMPLIANCE
#define FWVER_BUILD       0xF
#elif defined (FUSB15201P)
#define HWVER             HWVER_FUSB15201P
#define FWVER_PHASE       FWVER_PHASE_UNDEF
#define FWVER_BUILD       0x0
#elif defined (FUSB15201)
#define HWVER             HWVER_FUSB15201
#define FWVER_PHASE       FWVER_PHASE_RC
#define FWVER_BUILD       0x0
#elif defined (FUSB15200)
#define HWVER             HWVER_FUSB15200
#define FWVER_PHASE       FWVER_PHASE_RC
#define FWVER_BUILD       0x1
#else
#define HWVER             HWVER_UNDEF
#define FWVER_PHASE       FWVER_PHASE_UNDEF
#define FWVER_BUILD       0x0
#endif

    typedef struct ext_src_cap_t
    {
        union
        {
            uint8_t byte[EXT_SRC_CAP_LEN];
            struct
            {
                uint16_t VID;       ///< 0 : Vendor ID
                uint16_t PID;       ///< 2 : Product ID
                uint32_t XID;       ///< 4 : USB-IF extended ID
                uint8_t  FWVersion; ///< 8 : Firmware Version
                uint8_t  HWVersion; ///< 9 : Hardware Version
                union
                {
                    uint8_t VoltageRegulation; ///< 10 : Bitfield (See Spec)
                    struct
                    {
                        uint8_t LoadStep :2; ///< Bit[1:0] : 00b 150mA/us, 01b 500mA/us
                        uint8_t IoC      :1; ///< Bit[2] 0b 25%, 1b 90%
                    };
                };
                uint8_t HoldupTime; ///< 11 : Output Hold Time
                union
                {
                    uint8_t Compliance; ///< 12 : Bitfield (See spec)
                    struct
                    {
                        uint8_t LPS :1; ///< LPS compliant
                        uint8_t PS1 :1; /// PS1 Compliant
                        uint8_t PS2 :1; /// PS2 Compliant
                    };
                };
                union
                {
                    uint8_t TouchCurrent; ///< 13 : Bitfield (See spec)
                    struct
                    {
                        uint8_t LowTouchCurrentEPS :1; ///< Low Touch Current EPS when set
                        uint8_t GroundPinSupported :1; ///< Ground pin supported when set
                        uint8_t GroundPinEarth     :1; ///< Ground pin intended for earth when set
                    };
                };
                union
                {
                    uint16_t PeakCurrent1; ///< 14 : Bitfield (See spec)
                    struct
                    {
                        uint16_t PercentOverLoad1 :5; ///< Percent overload in 10% increments
                        uint16_t OverloadPeriod1  :6; ///< Overload period in 20ms
                        uint16_t DutyCycle1       :4; ///< Duty cycle in 5% increments
                        uint16_t VbusDroop1       :1; ///< VBUS Voltage droop
                    };
                };
                union
                {
                    uint16_t PeakCurrent2; ///< 16 : Bitfield (See spec)
                    struct
                    {
                        uint16_t PercentOverLoad2 :5; ///< Percent overload in 10% increments
                        uint16_t OverloadPeriod2  :6; ///< Overload period in 20ms
                        uint16_t DutyCycle2       :4; ///< Duty cycle in 5% increments
                        uint16_t VbusDroop2       :1; ///< VBUS Voltage droop
                    };
                };
                union
                {
                    uint16_t PeakCurrent3; ///< 18 : Bitfield (See spec)
                    struct
                    {
                        uint16_t PercentOverLoad3 :5; ///< Percent overload in 10% increments
                        uint16_t OverloadPeriod3  :6; ///< Overload period in 20ms
                        uint16_t DutyCycle3       :4; ///< Duty cycle in 5% increments
                        uint16_t VbusDroop3       :1; ///< VBUS Voltage droop
                    };
                };
                uint8_t TouchTemp; ///< 20 : Temp Spec
                union
                {
                    uint8_t SourceInputs; ///< 21 : Bitfield (See spec)
                    struct
                    {
                        uint8_t ExternalSupply :1; ///< External Supply Present when set
                        uint8_t
                            ExternalSupplyUnconstrained :1; /// External Supply Unconstrained when set
                        uint8_t InternalBatteryPresent :1; ///< Internal Battery Present when set
                    };
                };
                union
                {
                    uint8_t Battery; ///< 22 : Number of batteries Bitfield (See spec)
                    struct
                    {
                        uint8_t NumFixedBatteries      :4; ///< Lower nibble
                        uint8_t NumHotSwappableBattery :4; ///< Upper nibble
                    };
                };
                uint8_t PDP;    ///< 23 : Source's PDP
                uint8_t EPRPDP; ///< 24 : EPR Source's PDP
            };
        };
    } EXT_SRC_CAP_T;
    STATIC_ASSERT(sizeof(EXT_SRC_CAP_T) == EXT_SRC_CAP_LEN + 3, ext_srccap_t_must_be_24_bytes);

    typedef struct ext_status_t
    {
        union
        {
            uint8_t bytes[EXT_STATUS_LEN];
            struct
            {
                uint8_t InternalTemp; ///< 0 - Not supported otherwise 2-255 Celsius
                union
                {
                    uint8_t PresentInput; ///< 1 : Bitfield (See spec)
                    struct
                    {
                        uint8_t                      :1;
                        uint8_t ExternalPower        :1; ///< 1 External power when set
                        uint8_t ACDC                 :1; ///< 0 - DC, 1 - AC
                        uint8_t InternalBatteryPower :1; ///< Internal Battery Power
                        uint8_t InternalPower        :1; ///< Internal non-Battery Power
                    };
                    struct
                    {
                        uint8_t ThermalShutdown :1; ///< Thermal Shutdown when set
                        uint8_t                 :7;
                    };
                };
                union
                {
                    uint8_t PresentBatteryInput; ///< 2 : Bitfield (See spec)
                    struct
                    {
                        uint8_t FixedBattery        :4; ///< Fixed battery b3...0
                        uint8_t HotSwappableBattery :4; ///< Hot Swap Battery b7...4
                    };
                };
                union
                {
                    uint8_t EventFlags; ///< 3 : Bitfield (See spec)
                    struct
                    {
                        uint8_t      :1;
                        uint8_t OCP  :1; ///< OCP when set
                        uint8_t OTP  :1; ///< OTP when set
                        uint8_t OVP  :1; ///< OVP when set
                        uint8_t CFCV :1; ///< CF when set, CV when cleared
                    };
                };
                union
                {
                    uint8_t TemperatureStatus; ///< 4 : Bitfield (See spec)
                    struct
                    {
                        uint8_t            :1;
                        uint8_t TempStatus :2; ///< Temperature status
                    };
                };
                union
                {
                    uint8_t PowerStatus; ///< 5 : Bitfield (See spec)
                    struct
                    {
                        uint8_t                :1;
                        uint8_t LimitedByCable :1; ///< limited due to cable
                        uint8_t
                            LimitedByInsufficientPower :1; ///< limited due to insufficient power
                        uint8_t LimitedByExternalPower :1; ///< limited due to external power
                        uint8_t LimtedByEvent          :1; ///< limited due to event flags
                        uint8_t LimitedByTemperature   :1; ///< limited due to temperature
                    };
                };
                union
                {
                    uint8_t PowerStateChange; ///< 5 : Bitfield (See spec)
                    struct
                    {
                        uint8_t NewPowerState          :3; ///< new power state
                        uint8_t NewPowerStateIndicator :3; ///< new power state indicator
                        uint8_t                        :2;
                    };
                };
            };
        };
    } EXT_STATUS_T;
    STATIC_ASSERT(sizeof(EXT_STATUS_T) == EXT_STATUS_LEN, ext_status_t_must_be_7_bytes);

    typedef struct ext_get_bat_cap_t
    {
        union
        {
            uint8_t byte[EXT_GET_BAT_CAP_LEN];
            struct
            {
                uint8_t BatteryCapRef; ///< Battery Cap Ref
            };
        };
    } EXT_GET_BAT_CAP_T;
    STATIC_ASSERT(sizeof(EXT_GET_BAT_CAP_T) == EXT_GET_BAT_CAP_LEN,
                  ext_get_bat_cap_t_must_be_2_bytes);

    typedef struct ext_get_bat_stat_t
    {
        union
        {
            uint8_t byte[EXT_GET_BAT_STAT_LEN];
            struct
            {
                uint8_t BatteryStatRef; ///< Battery index 0...3 Fixed, 4...7 Hot swappable
            };
        };
    } EXT_GET_BAT_STAT_T;
    STATIC_ASSERT(sizeof(EXT_GET_BAT_STAT_T) == EXT_GET_BAT_STAT_LEN,
                  ext_get_bat_stat_t_size_must_be_2_bytes);

    typedef struct ext_bat_cap_t
    {
        union
        {
            uint8_t byte[EXT_BAT_CAP_LEN];
            struct
            {
                uint16_t VID;         ///< Vendor ID
                uint16_t PID;         ///< Product ID
                uint16_t DesignCap;   ///< Design Capacity (1/10 WH)
                uint16_t LastFullCap; ///< Full Charge Cap (1/10 WH)
                union
                {
                    uint8_t Type; ///< Battery Type
                    struct
                    {
                        uint8_t InvalidReference :1; ///< Invalid reference when set
                    };
                };
            };
        };
    } EXT_BAT_CAP_T;
    STATIC_ASSERT(sizeof(EXT_BAT_CAP_T) == EXT_BAT_CAP_LEN + 1, ext_bat_cap_t_must_be_10_bytes);

    typedef struct ext_get_mfi_t
    {
        union
        {
            uint8_t byte[EXT_GET_MFI_LEN];
            struct
            {
                uint8_t Target;    ///< 0 - Port, 1 - Battery
                uint8_t Reference; ///< 0...4 Fixed, 4...7 Hot Swappable Battries
            };
        };
    } EXT_GET_MFI_T;
    STATIC_ASSERT(sizeof(EXT_GET_MFI_T) == EXT_GET_MFI_LEN, ext_get_mfi_t_must_be_2_bytes);

    typedef struct ext_mfi_t
    {
        union
        {
            uint8_t byte[EXT_MFI_LEN];
            struct
            {
                uint16_t VID;     ///< Vendor ID
                uint16_t PID;     ///< Product ID
                uint8_t  str[22]; ///< Manufacturer String
            };
        };
    } EXT_MFI_T;
    STATIC_ASSERT(sizeof(EXT_MFI_T) == EXT_MFI_LEN, ext_mfi_t_size_must_be_26_bytes);

    typedef struct ext_sec_req_t
    {
        union
        {
            uint8_t bytes[EXT_SEC_REQ_LEN];
        };
    } EXT_SEC_REQ_T;
    STATIC_ASSERT(sizeof(EXT_SEC_REQ_T) == EXT_SEC_REQ_LEN, ext_sec_req_t_size_must_be_260_bytes);

    typedef struct ext_sec_resp_t
    {
        union
        {
            uint8_t bytes[EXT_SEC_RESP_LEN];
        };
    } EXT_SEC_RESP_T;
    STATIC_ASSERT(sizeof(EXT_SEC_RESP_T) == EXT_SEC_RESP_LEN,
                  ext_sec_resp_t_size_must_be_260_bytes);

    typedef struct ext_fwu_req_t
    {
        union
        {
            uint8_t bytes[EXT_FWU_REQ_LEN];
        };
    } EXT_FWU_REQ_T;
    STATIC_ASSERT(sizeof(EXT_FWU_REQ_T) == EXT_FWU_REQ_LEN, ext_fwu_resq_t_size_must_be_260_bytes);

    typedef struct ext_fwu_resp_t
    {
        union
        {
            uint8_t bytes[EXT_FWU_RESP_LEN];
        };
    } EXT_FWU_RESP_T;
    STATIC_ASSERT(sizeof(EXT_FWU_RESP_T) == EXT_FWU_RESP_LEN,
                  ext_fwu_resp_t_size_must_be_260_bytes);

    typedef struct ext_pps_stat_t
    {
        union
        {
            uint8_t byte[EXT_PPS_STAT_LEN];
            struct
            {
                uint16_t OutputVoltage; ///< 0 Output Voltage (20mV)
                uint8_t  OutputCurrent; ///< 2 Output Current (50mA)
                union
                {
                    uint8_t Flags; ///< 3 : Bitfield (See spec)
                    struct
                    {
                        uint8_t :1;
                        uint8_t
                            PTF :2; ///< b00 Not Supported, b01 Normal, b10 warning, b11 Over temperature
                        uint8_t OMF :1; ///< OMF set when in current limit mode
                    };
                };
            };
        };
    } EXT_PPS_STAT_T;
    STATIC_ASSERT(sizeof(EXT_PPS_STAT_T) == EXT_PPS_STAT_LEN, ext_pps_stat_t_size_must_be_4_bytes);
    typedef struct epr_mode_entry_t
    {
        union
        {
            uint8_t byte[EPR_MODE_LEN];
            struct
            {
                uint16_t maxVoltage;     ///< 0 max Voltage available (100mV)
                uint16_t peakCurrent;    ///< peak Current (50mA)
                uint16_t availablePower; ///< available power for the port (nearest 1/2 W)
            };
        };
    } EPR_MODE_ENTRY_T;
    STATIC_ASSERT(sizeof(EPR_MODE_ENTRY_T) == EPR_MODE_LEN, ext_epr_mode_t_size_must_be_6_bytes);

    typedef struct ext_country_codes_t
    {
        union
        {
            uint8_t bytes[EXT_COUNTRY_CODES_LEN];
            struct
            {
                uint8_t  Length;
                uint8_t  Reserved[1];
                uint16_t Code[12];
            };
        };
    } EXT_COUNTRY_CODES_T;
    STATIC_ASSERT(sizeof(EXT_COUNTRY_CODES_T) == EXT_COUNTRY_CODES_LEN,
                  ext_country_codes_t_size_must_be_26_bytes);

    typedef struct ext_country_info_t
    {
        union
        {
            uint8_t bytes[EXT_COUNTRY_INFO_LEN];
            struct
            {
                uint16_t Code; ///< Country Code alpha-2 characters
                uint8_t  Reserved[2];
                uint8_t  Data[22];
            };
        };
    } EXT_COUNTRY_INFO_T;
    STATIC_ASSERT(sizeof(EXT_COUNTRY_INFO_T) == EXT_COUNTRY_INFO_LEN,
                  ext_country_info_t_size_must_be_26_bytes);

    typedef struct ext_sink_cap_t
    {
        union
        {
            uint8_t bytes[EXT_SINK_CAP_LEN];
            struct
            {
                uint16_t VID;          ///< 0 : Vendor ID
                uint16_t PID;          ///< 2 : Product ID
                uint32_t XID;          ///< 4 : XID from USB-IF
                uint8_t  FWVersion;    ///< 8 : Fimrware Version
                uint8_t  HWVersion;    ///< 9 : Hardware Version
                uint8_t  SKEDBVersion; ///< SKEDB Version 1
                union
                {
                    uint8_t LoadStep; ///< 11 : Bitfield (See spec)
                    struct
                    {
                        uint8_t LoadStepValue :2; ///< 00b 150mA/uS, 01 500mA/uS
                    };
                };
                union
                {
                    uint16_t SinkLoadChar; ///< 12 : Bitfield (See spec)
                    struct
                    {
                        uint16_t PercentOverload :5; ///< 10% increment
                        uint16_t OverloadPeriod  :6; ///< 20ms increment
                        uint16_t DutyCycle       :4; ///< 5% increment
                        uint16_t VbusDroop       :1; ///< Can support VBUS voltage droop
                    };
                };
                union
                {
                    uint8_t Compliance; ///< 14 : Bitfield (See spec)
                    struct
                    {
                        uint8_t RequiresLPS :1; ///< Requires LPS When set
                        uint8_t RequiresPS1 :1; ///< Requires PS1 when set
                        uint8_t RequiresPS2 :1; ///< Requires PS2 when set
                    };
                };
                uint8_t TouchTemp; ///< Temperature confirms to IEC standard
                union
                {
                    uint8_t BatteryInfo; ///< 16 : Bitfield (See spec)
                    struct
                    {
                        uint8_t FixedBatteries        :4; ///< Lower nibble
                        uint8_t HotSwappableBatteries :4; ///< Upper nibble
                    };
                };
                union
                {
                    uint8_t SinkModes; ///< 17 : Bitfield (See spec)
                    struct
                    {
                        uint8_t PPSChargingSupported :1; ///< PPS charging supported
                        uint8_t VBUSPowered          :1; ///< VBUS powered
                        uint8_t MainsPowered         :1; ///< Mains powered
                        uint8_t BatteryPowered       :1; ///< Battery powered
                        uint8_t UnlimitedBattery     :1; ///< Battery essentiallly unlimited
                    };
                };
                union
                {
                    uint8_t SinkMinPDP; ///< 18 : Bitfield (See spec)
                    struct
                    {
                        uint8_t SinkMinimumPDP :7; ///< Minimum PDP required by sink
                    };
                };
                union
                {
                    uint8_t SinkOpPDP; ///< 19 : Bitfield (See spec)
                    struct
                    {
                        uint8_t SinkOperationalPDP :7; ///< PDP Sink requires to operate
                    };
                };
                union
                {
                    uint8_t SinkMaxPDP; ///< 20 : Bitfield (See spec)
                    struct
                    {
                        uint8_t SinkMaximumPDP :7; ///< Maximum PDP sink can consume
                    };
                };
                uint8_t EPRSinkMinPDP; ///< 21 : EPR Sink Min PDP
                uint8_t EPRSinkOpPDP;  ///< 22 : EPR Sink Operating PDP
                uint8_t EPRSinkMaxPDP; ///< 23 : EPR Sink Max PDP
            };
        };
    } EXT_SINK_CAP_T;

    STATIC_ASSERT(sizeof(EXT_SINK_CAP_T) == EXT_SINK_CAP_LEN, ext_sink_cap_t_size_must_be_24_bytes);
    typedef struct ext_control_t
    {
        union
        {
            uint8_t bytes[EXT_CONTROL_LEN];
            struct
            {
                uint8_t type;
                uint8_t data;
            };
        };
    } EXT_CONTROL_T;

    STATIC_ASSERT(sizeof(EXT_CONTROL_T) == EXT_CONTROL_LEN, ext_control_t_size_must_be_2_bytes);

    /* Firmware Update Definitions */
    typedef enum
    {
        RESP_GET_FW_ID = 0x1,
        RESP_PDFU_INITIATE,
        RESP_PDFU_DATA,     //transfer data block (response required)
        RESP_PDFU_DATA_NR,  //transfer data block (response not required)
        RESP_PDFU_VALIDATE, //request image validation
        RESP_PDFU_ABORT,
        RESP_PDFU_DATA_PAUSE,
        RESP_VENDOR_SPECIFIC = 0x7F,
    } FWUpdate_RESP_Message_Type_T;

    typedef enum
    {
        REQ_GET_FW_ID = 0x81,
        REQ_PDFU_INITIATE,
        REQ_PDFU_DATA,     //transfer data block (response required)
        REQ_PDFU_DATA_NR,  //transfer data block (response not required)
        REQ_PDFU_VALIDATE, //request image validation
        REQ_PDFU_ABORT,
        REQ_PDFU_DATA_PAUSE,
        REQ_VENDOR_SPECIFIC = 0xFF,
    } FWUpdate_REQ_Message_Type_T;

    typedef enum
    {
        OK = 0x00,
        errTarget,
        errFile,
        errWrite,
        errERASE,
        errCHECK_ERASED,
        errPROG,
        errVERIFY,
        errADDRESS,
        errNOTDONE,
        errFIRMWARE,
        errPOR = 0x0D,
        errUNKNOWN,
        errUNEXPECTED_HARD_RESET = 0x80,
        errUNEXPECTED_SOFT_RESET,
        errUNEXPECTED_REQUEST,
        errREJECT_PAUSE,
    } FWUpdate_Response_Status_T;

    typedef union
    {
        uint8_t  byte[2];
        uint16_t hword[1];
        struct
        {
            uint16_t ProtocolVersion :8;
            uint16_t MessageType     :8;
        };
    } FW_UPDATE_HEADER_T;

#define PDFU_INITIATE_REQ_SIZE 10
    typedef union
    {
        uint8_t byte[PDFU_INITIATE_REQ_SIZE];
        struct
        {
            uint16_t ProtocolVersion :8;
            uint16_t MessageType     :8;
            uint16_t FWVersion1      :16;
            uint16_t FWVersion2      :16;
            uint16_t FWVersion3      :16;
            uint16_t FWVersion4      :16;
        };
    } PDFU_INITIATE_REQ_PAYLOAD_T;

#define PDFU_DATA_REQ_SIZE 260
    typedef union
    {
        uint8_t byte[PDFU_DATA_REQ_SIZE];
        struct
        {
            uint16_t ProtocolVersion :8;
            uint16_t MessageType     :8;
            uint16_t DataBlockIndex  :16;
            uint8_t  DataBlock[256];
        };
    } PDFU_DATA_REQ_PAYLOAD_T;

#define PDFU_VENDOR_SPECIFIC_REQ_SIZE 260
    typedef union
    {
        uint8_t byte[PDFU_VENDOR_SPECIFIC_REQ_SIZE];
        struct
        {
            uint16_t ProtocolVersion :8;
            uint16_t MessageType     :8;
            uint16_t VID             :16;
            uint8_t  VendorDefined[256];
        };
    } VENDOR_SPECIFIC_REQ_PAYLOAD_T;

#define PDFU_GET_FW_ID_RESP_SIZE 22
    typedef union
    {
        uint8_t byte[PDFU_GET_FW_ID_RESP_SIZE];
        struct
        {
            uint32_t ProtocolVersion         :8;
            uint32_t MessageType             :8;
            uint32_t Status                  :8;
            uint32_t VIDL                    :8;
            uint32_t VIDH                    :8;
            uint32_t PIDL                    :8;
            uint32_t PIDH                    :8;
            uint32_t HWVersionMinor          :4;
            uint32_t HWVersionMajor          :4;
            uint32_t SiVersionReserved       :4;
            uint32_t SiVersionBase           :4;
            uint32_t FWVersion1L             :8;
            uint32_t FWVersion1H             :8;
            uint32_t FWVersion2L             :8;
            uint32_t FWVersion2H             :8;
            uint32_t FWVersion3L             :8;
            uint32_t FWVersion3H             :8;
            uint32_t FWVersion4L             :8;
            uint32_t FWVersion4H             :8;
            uint32_t ImageBank               :8;
            uint32_t Flags1PDUpdate          :1;
            uint32_t Flags1USBUpdate         :1;
            uint32_t Flags1NotUpdatable      :1;
            uint32_t Flags1Silent            :1;
            uint32_t Flags1Reserved          :4;
            uint32_t Flags2FullyFunctional   :1;
            uint32_t Flags2UnplugSafe        :1;
            uint32_t Flags2Reserved          :6;
            uint32_t Flags3HardResetRequired :1;
            uint32_t Flags3USBFunction       :1;
            uint32_t Flags3AltModes          :1;
            uint32_t Flags3PowerLimit        :1;
            uint32_t Flags3MorePower         :1;
            uint32_t Flags3                  :3;
            uint32_t Flags4UnmountStorage    :1;
            uint32_t Flags4Replug            :1;
            uint32_t Flags4SwapCable         :1;
            uint32_t Flags4PowerCycle        :1;
            uint32_t Flags4                  :4;
        };
    } GET_FW_ID_RESP_PAYLOAD_T;

#define PDFU_INITIATE_RESP_SIZE 7
    typedef union
    {
        uint8_t byte[PDFU_INITIATE_RESP_SIZE];
        struct
        {
            uint32_t ProtocolVersion :8;
            uint32_t MessageType     :8;
            uint32_t Status          :8;
            uint32_t WaitTime        :8;
            uint32_t MaxImageSize    :24;
        };
    } PDFU_INITIATE_RESP_PAYLOAD_T;

#define PDFU_DATA_RESP_SIZE 7
    typedef union
    {
        uint8_t byte[PDFU_DATA_RESP_SIZE];
        struct
        {
            uint32_t ProtocolVersion :8;
            uint32_t MessageType     :8;
            uint32_t Status          :8;
            uint32_t WaitTime        :8;
            uint32_t NumDataNR       :8;
            uint32_t DataBlockNum    :16;
        };
    } PDFU_DATA_RESP_PAYLOAD_T;

#define PDFU_VALIDATE_RESP_SIZE 5
    typedef union
    {
        uint8_t byte[PDFU_VALIDATE_RESP_SIZE];
        struct
        {
            uint32_t ProtocolVersion :8;
            uint32_t MessageType     :8;
            uint32_t Status          :8;
            uint32_t WaitTime        :8;
            uint32_t FlagsSuccess    :1;
            uint32_t FlagsReserved   :7;
        };
    } PDFU_VALIDATE_RESP_PAYLOAD_T;

#define PDFU_DATA_PAUSE_RESP_SIZE 3
    typedef union
    {
        uint8_t byte[PDFU_DATA_PAUSE_RESP_SIZE];
        struct
        {
            uint32_t ProtocolVersion :8;
            uint32_t MessageType     :8;
            uint32_t Status          :8;
        };
    } PDFU_DATA_PAUSE_RESP_PAYLOAD_T;

#define PDFU_VENDOR_SPECIFIC_RESP_SIZE 260
    typedef union
    {
        uint8_t byte[PDFU_VENDOR_SPECIFIC_RESP_SIZE];
        struct
        {
            uint16_t ProtocolVersion :8;
            uint16_t MessageType     :8;
            uint16_t Status          :8;
            uint16_t VIDL            :8;
            uint16_t VIDH            :8;
            uint8_t  VendorDefined[255];
        };
    } VENDOR_SPECIFIC_RESP_PAYLOAD_T;
//alert states combined for SRC/SNK

#define CREATE_SRC_EPR_POLICY_STATES(STATE)\
	    STATE(PE_SRC_Evaluate_EPR_Mode_Entry)                                                          \
	    STATE(PE_SRC_EPR_Mode_Entry_ACK)                                                               \
	    STATE(PE_SRC_EPR_Mode_Entry_Failed)                                                            \
	    STATE(PE_SRC_EPR_Mode_Discover_Cable)                                                          \
	    STATE(PE_SRC_EPR_Mode_Evaluate_Cable_EPR)                                                      \
	    STATE(PE_SRC_EPR_Mode_Entry_Succeeded)                                                         \
	    STATE(PE_SRC_EPR_Keep_Alive)                                                                   \
	    STATE(PE_SRC_Send_EPR_Mode_Exit)
#define CREATE_SRC_POLICY_STATES(STATE)\
	STATE(PE_SRC_Startup)\
	STATE(PE_SRC_Ready)                                                                            \
	STATE(PE_SRC_Soft_Reset)                                                                       \
	STATE(PE_SRC_Send_Soft_Reset)                                                                  \
	STATE(PE_SRC_Hard_Reset)                                                                       \
	STATE(PE_SRC_Transition_To_Default)                                                           \
	STATE(PE_SRC_Send_Not_Supported)                                                               \
	STATE(PE_SRC_Not_Supported_Received) \
    STATE(PE_SRC_Discovery)                                                                        \
    STATE(PE_SRC_Send_Capabilities)                                                                \
    STATE(PE_SRC_Give_Source_Cap)                                                                  \
    STATE(PE_SRC_Negotiate_Capability)                                                             \
    STATE(PE_SRC_Transition_Supply)                                                                \
    STATE(PE_SRC_Capability_Response)                                                              \
    STATE(PE_SRC_Get_Sink_Cap)                                                                     \
    STATE(PE_SRC_Wait_New_Capabilities)                                                            \
    STATE(PE_SRC_Ping) \
    STATE(PE_SRC_Get_Sink_Cap_Ext)                                                                 \
    STATE(PE_SRC_Give_Source_Cap_Ext)                                                              \
    STATE(PE_SRC_Give_PPS_Status) \
	STATE(PE_SRC_Give_Source_Info) \
	IF(SRC_EPR_STATES, CREATE_SRC_EPR_POLICY_STATES(STATE))

#define CREATE_SNK_EPR_STATES(STATE) \
	    STATE(PE_SNK_Send_Epr_Mode_Entry)                                                              \
	    STATE(PE_SNK_Epr_Mode_Entry_Wait_For_Response)                                                 \
	    STATE(PE_SNK_EPR_Keep_Alive)                                                                   \
	    STATE(PE_SNK_Send_EPR_Mode_Exit)                                                               \
		STATE(PE_SNK_EPR_Mode_Exit_Received)

//alert states combined for SRC/SNK
#define CREATE_SNK_POLICY_STATES(STATE) \
		STATE(PE_SNK_Startup)\
	    STATE(PE_SNK_Ready)                                                                            \
	    STATE(PE_SNK_Soft_Reset)                                                                       \
	    STATE(PE_SNK_Send_Soft_Reset)                                                                  \
	    STATE(PE_SNK_Hard_Reset)                                                                       \
	    STATE(PE_SNK_Transition_To_Default)                                                               \
	    STATE(PE_SNK_Send_Not_Supported)                                                               \
		STATE(PE_SNK_Not_Supported_Received) \
	    STATE(PE_SNK_Discovery)                                                                        \
	    STATE(PE_SNK_Wait_For_Capabilities)                                                            \
	    STATE(PE_SNK_Evaluate_Capability)                                                              \
	    STATE(PE_SNK_Select_Capability)                                                                \
	    STATE(PE_SNK_Transition_Sink)                                                                  \
	    STATE(PE_SNK_Give_Sink_Cap)                                                                    \
	    STATE(PE_SNK_Get_Source_Cap)                                                                   \
	    STATE(PE_SNK_Give_Sink_Cap_Ext)                                                                \
	    STATE(PE_SNK_Get_Source_Cap_Ext)                                                               \
	    STATE(PE_SNK_Get_PPS_Status)                                                                   \
		STATE(PE_SNK_Get_Source_Info) \
        IF(SNK_EPR_STATES, CREATE_SNK_EPR_STATES(STATE)) \
//FR_Swap States unused
#define CREATE_FRS_POLICY_STATES(STATE) \
	    STATE(PE_FRS_SRC_SNK_CC_Signal)                                                                \
	    STATE(PE_FRS_SRC_SNK_Evaluate_Swap)                                                            \
	    STATE(PE_FRS_SRC_SNK_Accept_Swap)                                                              \
	    STATE(PE_FRS_SRC_SNK_Transition_to_off)                                                        \
	    STATE(PE_FRS_SRC_SNK_Assert_Rd)                                                                \
	    STATE(PE_FRS_SRC_SNK_Wait_Source_on)                                                           \
	    STATE(PE_FRS_SNK_SRC_Start_AMS)                                                                \
	    STATE(PE_FRS_SNK_SRC_Send_Swap)                                                                \
	    STATE(PE_FRS_SNK_SRC_Transition_to_off)                                                        \
	    STATE(PE_FRS_SNK_SRC_Vbus_Applied)                                                             \
	    STATE(PE_FRS_SNK_SRC_Assert_Rp)                                                                \
	    STATE(PE_FRS_SNK_SRC_Source_on)                                                                \
// note for PRSwap, Send and Evaluate states cover all additional states
// so only 4 PE_PRS_XXX states required
#define CREATE_DRP_POLICY_STATES(STATE) \
	    STATE(PE_PRS_SRC_SNK_Evaluate_Swap)                                                            \
	    STATE(PE_PRS_SRC_SNK_Send_Swap)                                                                \
	    STATE(PE_PRS_SNK_SRC_Evaluate_Swap)                                                            \
	    STATE(PE_PRS_SNK_SRC_Send_Swap)                                                                \
	    STATE(PE_DR_SRC_Get_Source_Cap)                                                                \
	    STATE(PE_DR_SRC_Give_Sink_Cap)                                                                 \
	    STATE(PE_DR_SNK_Get_Sink_Cap)                                                                  \
	    STATE(PE_DR_SNK_Give_Source_Cap)                                                               \
	    STATE(PE_DR_SRC_Get_Source_Cap_Ext)                                                            \
	    STATE(PE_DR_SNK_Give_Source_Cap_Ext)                                                           \
	    STATE(PE_DR_SNK_Get_Sink_Cap_Ext) \
	    STATE(PE_DR_SRC_Give_Sink_Cap_Ext)

//VDM Compressed into two states instead of ~30
#define CREATE_VDM_POLICY_STATES(STATE) \
	    STATE(PE_CBL_Query)                                                                            \
	    IF(VDM_EXTRA, STATE(PE_GIVE_VDM))

#define CREATE_USB4_POLICY_STATES(STATE) \
	    STATE(PE_DEU_Send_Enter_USB)                                                                   \
	    STATE(PE_UEU_Enter_USB_Received)                                                               \
	    STATE(PE_Send_Data_Reset)                                                                      \
	    STATE(PE_Evaluate_Data_Reset)                                                                  \
	    STATE(PE_Execute_Data_Reset_DFP)                                                               \
	    STATE(PE_Execute_Data_Reset_UFP)                                                               \
//state SRC/SNK Hard_Reset_Received handled without a state
//not_supported received not handled through state either
#define CREATE_POLICY_STATES(STATE)                                                                                                                                      \
    IF(SRC_STATES, CREATE_SRC_POLICY_STATES(STATE)) \
	IF(SNK_STATES, CREATE_SNK_POLICY_STATES(STATE)) \
	IF(DRP_STATES, CREATE_DRP_POLICY_STATES(STATE)) \
	IF(VDM_STATES, CREATE_VDM_POLICY_STATES(STATE)) \
	IF(USB4_STATES, CREATE_USB4_POLICY_STATES(STATE)) \
    STATE(PE_Get_Status)                                                                           \
    STATE(PE_Give_Status)                                                                          \
    STATE(PE_Chunk_NotSupported)                                                                   \
    STATE(PE_Get_Battery_Cap)                                                                      \
    STATE(PE_Give_Battery_Cap)                                                                     \
    STATE(PE_Get_Battery_Status)                                                                   \
    STATE(PE_Give_Battery_Status)                                                                  \
    STATE(PE_Get_Manufacturer_Info)                                                                \
    STATE(PE_Give_Manufacturer_Info)                                                               \
    STATE(PE_Get_Country_Codes)                                                                    \
    STATE(PE_Give_Country_Codes)                                                                   \
    STATE(PE_Get_Country_Info)                                                                     \
    STATE(PE_Give_Country_Info)                                                                    \
    STATE(PE_Send_Alert)                                                                           \
    STATE(PE_Alert_Received)                                                                       \
    STATE(PE_Send_Security_Request)                                                                \
    STATE(PE_Send_Security_Response)                                                               \
    STATE(PE_Security_Response_Received)                                                           \
    STATE(PE_Send_Firmware_Update_Request)                                                         \
    STATE(PE_Send_Firmware_Update_Response)                                                        \
    STATE(PE_Firmware_Update_Response_Received)                                                    \
    STATE(PE_DRS_DFP_UFP_Send_Swap)                                                                \
    STATE(PE_DRS_DFP_UFP_Evaluate_Swap)                                                            \
    STATE(PE_DRS_UFP_DFP_Send_Swap)                                                                \
    STATE(PE_DRS_UFP_DFP_Evaluate_Swap)                                                            \
    STATE(PE_VCS_Send_Swap)                                                                        \
    STATE(PE_VCS_Evaluate_Swap)                                                                    \
    STATE(PE_CBL_Soft_Reset)                                                                       \
    STATE(PE_CBL_Hard_Reset)                                                                       \
    STATE(PE_DFP_CBL_Send_Soft_Reset)                                                              \
    STATE(PE_UFP_CBL_Send_Soft_Reset)                                                              \
    STATE(PE_DFP_CBL_Send_Cable_Reset)                                                             \
    STATE(PE_BIST_Carrier_Mode)                                                                    \
    STATE(PE_BIST_Test_Data)                                                                       \
    STATE(PE_ErrorRecovery)                                                                        \
	STATE(PE_Get_Revision)                                                                     \
	STATE(PE_Give_Revision)                                                                    \
    STATE(PE_Send_Generic_Cmd)                                                                     \
    STATE(PE_Send_Generic_Data)                                                                    \
    STATE(PE_SRC_Disabled)                                                                         \
    STATE(NUM_PE_STATES)

    typedef enum policy_state_t
    {
#define GEN_ENUM(state) state,
        CREATE_POLICY_STATES(GEN_ENUM)
    } POLICY_STATE_T;

#define CREATE_PROT_STATES(STATE)                                                                  \
    STATE(PRLDisabled)                                                                             \
    STATE(PRLIdle)                                                                                 \
    STATE(PRLResetWait)                                                                            \
    STATE(PRLRxWait)                                                                               \
    STATE(PRLTxSendingMessage)                                                                     \
    STATE(PRLSinkTx)                                                                               \
    STATE(PRL_BIST_Rx_Reset_Counter)                                                               \
    STATE(PRL_BIST_Rx_Test_Frame)                                                                  \
    STATE(PRL_BIST_Rx_Error_Count)                                                                 \
    STATE(PRL_BIST_Rx_Inform_Policy)

    typedef enum protocol_state_t
    {
#define GEN_ENUM(state) state,
        CREATE_PROT_STATES(GEN_ENUM)
    } PROTOCOL_STATE_T;

    typedef enum prot_tx_stat_t
    {
        txIdle = 0,
        txReset,
        txSend,
        txBusy,
        txWait,
        txSuccess,
        txError,
        txCollision,
        txPending,
        txAbort
    } PROT_TXSTAT_T;

    typedef enum pdo_t
    {
        pdoTypeFixed = 0,
        pdoTypeBattery,
        pdoTypeVariable,
        pdoTypeAugmented,
    } PDO_T;

    typedef enum apdo_t
    {
        apdoTypePPS = 0,
		apdoTypeEPRAVS,
        apdoTypeSPRAVS,
        apdoReserved2,
    } APDO_T;

    typedef enum sink_tx_state_t
    {
        SinkTxOK = CC_RP_3p0A,
        SinkTxNG = CC_RP_1p5A,
    } SINK_TX_STATE_T;

    /*
 * @brief State for cable reset
 */
    typedef enum cbl_rst_state_t
    {
        CBL_RST_DISABLED,
        CBL_RST_START,
        CBL_RST_VCONN_SOURCE,
        CBL_RST_DR_DFP,
        CBL_RST_SEND,
    } CBL_RST_STATE_T;

    /* EPR */
    typedef enum
    {
        EprModeEnter     = 0x01,
        EprModeEnterAck  = 0x02,
        EprModeEnterSucc = 0x03,
        EprModeEnterFail = 0x04,
        EprModeExit      = 0x05,
    } EPR_MODE_ACTION_T;
	
	    /* USB4 */
    typedef enum
    {
        USB2 = 0x00,
        USB3 = 0x01,
        USB4 = 0x02,
    } USBMODE_T;

#ifdef __cplusplus
}
#endif

#endif /* _PDTYPES_H_ */
