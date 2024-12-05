/*******************************************************************************
 * @file     observer.h
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
#ifndef MODULES_OBSERVER_H_
#define MODULES_OBSERVER_H_

#include "port_tcpd.h"
#include "dc_dc_board.h"
#include "batt_ntc_monitor.h"
#include "power_sharing.h"
#include "vdm.h"
#ifdef __cplusplus
extern "C"
{
#endif

#ifndef CONFIG_MAX_EVENTS
#define CONFIG_MAX_EVENTS EVENT_MAX_COUNT
#endif

#define PERCENT(x, y) (((x) * (y)) / 100)
#define CL_PERCENT    96
#define CV_PERCENT    97
#define UV_PERCENT    90
#define PPS_MIN_V     5000

#if CONFIG_ENABLED(VDM) && !CONFIG_ENABLED(VDM_MINIMAL)
#define VDM_EVENTS 1
#else
#define VDM_EVENTS 0
#endif

#if (MODAL_OPERATION_SUPPORTED)
#define VDM_MODAL_EVENTS 1
#else
#define VDM_MODAL_EVENTS 0
#endif

#if CONFIG_ENABLED(BATTERY_EVENTS) || CONFIG_ENABLED(POWER_SHARING)
#define BATTERY_EVENTS 1
#else
#define BATTERY_EVENTS 0
#endif


#if CONFIG_ENABLED(USB4)
#define USB4_EVENTS 1
#else
#define USB4_EVENTS 0
#endif

#if CONFIG_ENABLED(EXTENDED_EVENT)
#define EXTENDED_EVENTS 1
#else
#define EXTENDED_EVENTS 0
#endif

#if CONFIG_ENABLED(POWER_SHARING) || CONFIG_ENABLED(LEGACY_CHARGING)
#define PS_LC_EVENTS 1
#else
#define PS_LC_EVENTS 0
#endif

#if CONFIG_ENABLED(SNK)
#define SNK_EVENTS 1
#else
#define SNK_EVENTS 0
#endif
/**
 * List of Observer Events. Registered callbacks are called for these
 * events: EVENT_*
 */
#define CREATE_EXTENDED_EVENTS(EVENT) \
	EVENT(PD_CBL_ID_RECEIVED)                                                                      \
	EVENT(ENTER_USB_RESPONSE)                                                                      \
	EVENT(PD_STATUS)                                                                               \
	EVENT(PD_GET_EXT_SNK_CAP)                                                                      \
	EVENT(PPS_STATUS_RECIEVED)                                                                     \
	EVENT(PD_GET_EXT_SRC_CAP)                                                                      \
    EVENT(VCONN_REQ)                                                                               \
    EVENT(UNSUPPORTED_ACCESSORY)                                                                   \
    EVENT(DEBUG_ACCESSORY)                                                                         \
    EVENT(AUDIO_ACCESSORY)                                                                         \
    EVENT(ILLEGAL_CBL)                                                                             \
	EVENT(PD_GET_MAN_INFO)

#define CREATE_USB4_EVENTS(EVENT) \
	EVENT(DATA_RESET_EXIT) \
	EVENT(ENTER_USB_RECEIVED) \
	EVENT(ENTER_USB_REQUEST) \
	EVENT(DATA_RESET_ENTER)

#define CREATE_EXT_BATTERY_EVENTS(EVENT) \
	EVENT(PD_GET_BAT_CAP)                                                                          \
	EVENT(PD_GET_BAT_STAT)

#define CREATE_BATTERY_EVENTS(EVENT) \
	EVENT(PD_BAT_CAP_RECEIVED)                                                                     \
	EVENT(PD_BAT_STAT_RECEIVED)                                                                    \
	IF(EXTENDED_EVENTS, CREATE_EXT_BATTERY_EVENTS(EVENT))

#define CREATE_VDM_MODAL_EVENTS(EVENT)\
	EVENT(MODE_ENTER_SUCCESS)                                                                      \
	EVENT(MODE_EXIT_SUCCESS)                                                                       \
	EVENT(MODE_VDM_ATTENTION)

#define CREATE_VDM_EVENTS(EVENT) \
	IF (VDM_MODAL_EVENTS, CREATE_VDM_MODAL_EVENTS(EVENT)) \
	EVENT(IDENTITY_RECEIVED)                                                                       \

#define CREATE_PS_LC_EVENTS(EVENT)\
	EVENT(TC_ATTACHED)                                                                             \
	EVENT(TC_DETACHED)                                                                             \
	EVENT(PD_DEVICE)

#define CREATE_SINK_EVENTS(EVENT) \
	EVENT(PD_SNK_EVAL_SRC_CAP)                                                                     \
	EVENT(VBUS_SINK)

#define CREATE_EVENT_LIST(EVENT)                                                                   \
	IF(USB4_EVENTS, CREATE_USB4_EVENTS(EVENT)) \
	IF(EXTENDED_EVENTS, CREATE_EXTENDED_EVENTS(EVENT)) \
    IF(PS_LC_EVENTS, CREATE_PS_LC_EVENTS(EVENT)) \
	IF(SNK_EVENTS, CREATE_SINK_EVENTS(EVENT)) \
	EVENT(TX_REQ) \
	EVENT(VBUS_REQ)                                                                                \
    EVENT(PD_GET_SRC_CAP)                                                                          \
    EVENT(PD_GET_SNK_CAP)                                                                          \
    EVENT(PD_SNK_CAP_RECEIVED)                                                                     \
    EVENT(EXT_SNK_CAP_RECEIVED)                                                                    \
	IF(BATTERY_EVENTS, CREATE_BATTERY_EVENTS(EVENT))                                                      \
    EVENT(PD_SRC_EVAL_SNK_REQ)                                                                     \
    EVENT(PPS_STATUS_REQUEST)                                                                      \
    EVENT(PPS_MONITOR)                                                                             \
    EVENT(PPS_ALARM)                                                                               \
    IF(VDM_EVENTS, CREATE_VDM_EVENTS(EVENT)) \
    EVENT(HARD_RESET)                                                                              \
    EVENT(ERROR_RECOVERY)                                                                          \
    EVENT(BIST_SHARED_TEST_MODE)                                                                   \
    EVENT(PD_NEW_CONTRACT)                                                                         \
    EVENT(PD_GET_FW_ID)                                                                            \
    EVENT(PD_FW_INITIATE)                                                                          \
    EVENT(PD_INITIATE_RESP_SENT)                                                                   \
    EVENT(EPR_MODE_ENTRY)                                                                          \
    EVENT(PD_GIVE_SOURCE_INFO)                                                                     \
    EVENT(PD_GIVE_REVISION)                                                                        \
    EVENT(PPS_CL)\
    EVENT(PD_GET_SRC_EPR_CAP)                                                                      \
    EVENT(PD_GET_SNK_EPR_CAP)                                                                      \
    EVENT(PD_SNK_EPR_KEEP_ALIVE_EXPIRY)                                                            \
    EVENT(PD_POPULATE_SRC_CAPS) \
    EVENT(PD_GET_ALERT_REQ)                                                                        \
    EVENT(PD_ALERT_RECEIVED)

    /**
 * @brief enum for Event ID
 */
    typedef enum event_t
    {
#define EVENT_ENUM(EVT) EVENT_##EVT,
        /* Create enum of events */
        CREATE_EVENT_LIST(EVENT_ENUM) EVENT_MAX_COUNT
    } EVENT_T;

    /**
 * Callback return types.
 */
    typedef enum event_ret_t
    {
        EVENT_STATUS_SUCCESS,
        EVENT_STATUS_FAILED,
        EVENT_STATUS_DEFAULT,
    } EVENT_RET_T;

    /**
 * Callback argument for Type-C valid attach event.
 */
    typedef struct event_tc_attached_t
    {
        struct
        {
            bool      src;
            enum cc_t cc;
        } arg;
    } EVENT_TC_ATTACHED_T;

    /**
 * Callback argument for Policy Engine Hard Reset event.
 */
    typedef struct event_error_recovery_t
    {
        struct
        {
            enum event_ret_t success;
        } ret;
    } EVENT_ERROR_RECOVERY_T;

    /**
 * Callback argument for request to turn on or off VBUS. Value of 0 mv and 0 ma
 * should turn off vbus.
 */
    typedef struct event_vbus_req_t
    {
        struct
        {
            unsigned   mv;   /**< Voltage output in milli volts */
            unsigned   ma;   /**< Current limit on milliamps */
            enum pdo_t type; /**< PDO type */
        } arg;
        struct
        {
            enum event_ret_t success;
        } ret;
    } EVENT_VBUS_REQ_T;

    /**
 * Callback argument for request to turn sink VBUS. Value of 0 mv and 0 ma
 * should turn off vbus sink.
 */
    typedef struct event_vbus_sink_t
    {
        struct
        {
            unsigned   mv;   /**< Voltage output in milli volts */
            unsigned   ma;   /**< Current limit on milliamps */
            enum pdo_t type; /**< PDO type */
        } arg;
        struct
        {
            enum event_ret_t success;
        } ret;
    } EVENT_VBUS_SINK_T;

    /**
 * Callback argument for request to turn on or off VCONN.
 */
    typedef struct event_vconn_req_t
    {
        struct
        {
            bool      enable;    /**< Enable vconn source */
            enum cc_t vconn_pin; /**< Vconn pin #CC_T*/
        } arg;
        struct
        {
            enum event_ret_t success;
        } ret;
    } EVENT_VCONN_REQ_T;

    /**
 * Callback argument to update source cap. This is sent to port partner.
 */
    typedef struct event_pd_get_src_cap_t
    {
        struct
        {
            enum pd_rev_t    pd_rev;  /**< Active PD contract revision */
            struct pd_msg_t *src_cap; /**< PD buffer of 7 PDOs length*/
            unsigned         max_ma;  /**< Maximum current(ma) cable supports */
        } arg;
        struct
        {
            int              num; /**< Number of PDOs written in the buffer */
            enum event_ret_t success;
        } ret;
    } EVENT_PD_GET_SRC_CAP_T;

    /**
 * Callback argument to update sink cap. This is sent to port partner.
 */
    typedef struct event_pd_get_snk_cap_t
    {
        struct
        {
            enum pd_rev_t    pd_rev;  /**< Active PD contract revision */
            struct pd_msg_t *snk_cap; /**< PD buffer of 7 PDOs length*/
        } arg;
        struct
        {
            int              num; /**< Number of PDOs written in the buffer */
            enum event_ret_t success;
        } ret;
    } EVENT_PD_GET_SNK_CAP_T;

    /**
 * Callback argument to sink when source cap is obtained from partner.
 */
    typedef struct event_pd_snk_eval_src_cap_t
    {
        struct
        {
            int              num; /**< Number of valid pdos in the buffer */
            struct pd_msg_t *pdo; /**< Buffer with PDOs from partner source */
        } arg;
        struct
        {
            int              index; /**< Index of PDO selected by the sink */
            enum pdo_t       type;  /**< Type of PDO selected */
            struct pd_msg_t  req;   /**< Request data object */
            enum event_ret_t success;
        } ret;
    } EVENT_PD_SNK_EVAL_SRC_CAP_T;

    /**
     * Callback argument to sink when source cap is obtained from partner.
     */
    typedef struct
    {
        struct
		{
            PD_MSG_T *caps;       /**< Source cap PDOs that was advertised */
            uint32_t  caps_count; /**< Valid PDOs in source cap list */
        } arg;
        struct
		{
            EVENT_RET_T     success;
        } ret;
    } EVENT_PD_POP_SRC_CAPS_T;


    /* App Ctx */
    struct app_ctx
    {
    	struct dc_dc_data vbus_ctrl[2];
    	POWER_RESERVE_T dpm_power;
    	struct batt_ntc_monitor *monitor;
    	volatile bool *event;
    };
    /**
 * Callback argument to sink when source cap is obtained from partner.
 */
    typedef struct event_pd_src_eval_snk_req_t
    {
        struct
        {
            struct pd_msg_t *req;        /**< Request object obtained from partner sink */
            struct pd_msg_t *caps;       /**< Source cap PDOs that was advertised */
            int              caps_count; /**< Valid PDOs in source cap list */
            enum pd_rev_t    pd_rev;     /**< Current PD contract revision */
        } arg;
        struct
        {
            enum event_ret_t success;
        } ret;
    } EVENT_PD_SRC_EVAL_SNK_REQ_T;

    /**
 * Callback when sink cap is received.
 */
    typedef struct
    {
        struct
        {
            PD_REV_T  pd_rev;  /**< Active PD contract revision */
            PD_MSG_T *snk_cap; /**< PD buffer of 7 PDOs length*/
            uint32_t  num;     /**< Number of PDOs in the buffer */
            bool      success;
        } arg;
    } EVENT_PD_SNK_CAP_RECEIVED_T;

    /**
 * Callback argument when extended sink cap is received.
 */
    typedef struct event_pd_ext_snk_cap_t
    {
        struct
        {
            struct ext_sink_cap_t *buf;     /**< Extended message buffer */
            bool                   success; /**< Capabilities received successfully */
        } arg;
        struct
        {
            enum event_ret_t success;
        } ret;
    } EVENT_PD_EXT_SNK_CAP_T;

    /**
 * Callback argument when request for battery status is received.
 */
    typedef struct event_pd_get_bat_stat_t
    {
        struct
        {
            uint8_t ref;     /**< Battery reference */
            bool    success; /**< Capabilities received successfully */
        } arg;
        struct
        {
            struct pd_msg_t *bat_stat; /**< Battery status value */
            enum event_ret_t success;
        } ret;
    } EVENT_PD_GET_BAT_STAT_T;

    /**
 * Callback argument when request for batter capabilities is received.
 */
    typedef struct event_pd_get_bat_cap_t
    {
        struct
        {
            uint8_t               ref;     /**< Reference for battery */
            struct ext_bat_cap_t *cap;     /**< Buffer to write the battery capabilities. */
            bool                  success; /**< Capabilities received successfully */
        } arg;
        struct
        {
            enum event_ret_t success;
        } ret;
    } EVENT_PD_GET_BAT_CAP_T;

    typedef struct event_pd_cbl_id_received_t
    {
        struct
        {
            struct pd_msg_t *cable_cap;
        } arg;
        struct
        {
            enum event_ret_t success;
        } ret;
    } EVENT_PD_CBL_ID_RECEIVED_T;

        /**
 * @brief      Callback argument for response received after sending EnterUSB
 */
    typedef struct event_enter_usb_response_t
    {
        struct
        {
            bool       response; /**< Response received after sending */
            enum sop_t sop;      /**< SOP */
        } arg;
        struct
        {
            enum event_ret_t success; /**< DPM callback return */
        } ret;
    } EVENT_ENTER_USB_RESPONSE_T;

        /**
 * @brief      Used to notify when an extended status is sent or received
 */
    typedef struct event_pd_status_t
    {
        struct
        {
            struct ext_status_t *stat; /**< Extended Status Message */
            enum sop_t           sop;  /**< SOP */
        } arg;
        struct
        {
            enum event_ret_t success; /**< DPM callback return */
        } ret;
    } EVENT_PD_STATUS_T;

        /**
 * Callback argument when extended source cap is received.
 */
    typedef struct event_pd_get_ext_src_cap_t
    {
        struct
        {
            uint8_t *buf; /**< Extended message buffer */
        } arg;
        struct
        {
            enum event_ret_t success;
        } ret;
    } EVENT_PD_GET_EXT_SRC_CAP_T;

        /**
 * Callback argument when Manufacture Info is requested.
 */
    typedef struct event_pd_get_man_info_t
    {
        struct
        {
            uint8_t *buf; /**< Extended message buffer */
        } arg;
        struct
        {
            int              len; /**< Number of valid data objects  */
            enum event_ret_t success;
        } ret;
    } EVENT_PD_GET_MAN_INFO_T;

    typedef struct event_pps_status_request_t
    {
        struct
        {
            struct ext_pps_stat_t *ppssdb;
        } arg;
        struct
        {
            enum event_ret_t success;
        } ret;
    } EVENT_PPS_STATUS_REQUEST_T;

    typedef struct event_epr_mode_entry_t
    {
        struct
        {
            struct epr_mode_entry_t eprinfo;
        } arg;
        struct
        {
            enum event_ret_t success;
        } ret;
    } EVENT_EPR_MODE_ENTRY_T;

    /**
 * Callback argument for BIST Shared Test Mode Event
 */
    typedef struct
    {
        struct
        {
            bool enter;
        } arg;
    } BIST_SHARED_TEST_MODE_T;

    typedef struct event_pd_data_reset_t
    {
        struct
        {
            bool is_dfp;
        } arg;
        struct
        {
            enum event_ret_t success;
        } ret;
    } EVENT_PD_DATA_RESET_T;

    /**
 * @brief      Callback argument for when EnterUSB message is being sent
 */
    typedef struct event_enter_usb_request_t
    {
        struct
        {
            struct pd_msg_t *enter_usb_msg; /**< Request object */
            enum sop_t       sop;           /**< SOP */
        } arg;
        struct
        {
            enum event_ret_t success; /**< DPM callback return */
        } ret;
    } EVENT_ENTER_USB_REQUEST_T;

    /**
 * @brief      Callback argument for when EnterUSB message is received
 */
    typedef struct event_enter_usb_received_t
    {
        struct
        {
            struct pd_msg_t *enter_usb_msg; /**< Received object */
            enum sop_t       sop;           /**< SOP */
        } arg;
        struct
        {
            enum event_ret_t success; /**< DPM callback return */
        } ret;
    } EVENT_ENTER_USB_RECEIVED_T;

    /**
 * @brief      Used to notify when need to get pd source info
 */
    typedef struct event_pd_give_source_info_t
    {
        struct
        {
            struct pd_msg_t *info; /**< Extended Status Message */
        } arg;
        struct
        {
            enum event_ret_t success; /**< DPM callback return */
        } ret;
    } EVENT_PD_GIVE_SOURCE_INFO_T;

    /**
 * @brief      Used to notify when need to get pd revision
 */
    typedef struct event_pd_give_revision_t
    {
        struct
        {
            struct pd_msg_t *revision; /**< Extended Status Message */
        } arg;
        struct
        {
            enum event_ret_t success; /**< DPM callback return */
        } ret;
    } EVENT_PD_GIVE_REVISION_T;

    typedef struct event_pd_alert_t
    {
        struct
        {
            struct pd_msg_t *alert_msg;
        } arg;
        struct
        {
            enum event_ret_t success;
        } ret;
    } EVENT_PD_ALERT_T;

    //edited Aadish
    typedef struct event_pd_epr_keep_alive_expiry
    {
        struct
        {
            EXT_CONTROL_T msg;
        } arg;
        struct
        {
            enum event_ret_t success;
        } ret;
    } EVENT_PD_EPR_KEEP_ALIVE_EXPIRY;

    /**
 * Callback argument when request for fw id is received.
 */
    typedef struct
    {
        struct
        {
            uint8_t                   ver;
            uint8_t                   command;
            GET_FW_ID_RESP_PAYLOAD_T *fw_id_resp;
        } arg;
        struct
        {
            EVENT_RET_T success;
        } ret;
    } EVENT_PD_GET_FW_ID_T;

    /**
 * Callback argument when fw update initiate is received.
 */
    typedef struct
    {
        struct
        {
            PDFU_INITIATE_REQ_PAYLOAD_T  *fw_init_req;
            PDFU_INITIATE_RESP_PAYLOAD_T *fw_init_resp;
        } arg;
        struct
        {
            EVENT_RET_T success;
        } ret;
    } EVENT_PD_FW_INITIATE_T;

    typedef struct event_pps_alarm_t
    {
        struct
        {
            uint32_t mv_low;
            uint32_t mv_high;
        } arg;
        struct
        {
            enum event_ret_t success;
        } ret;
    } EVENT_PPS_ALARM_T;

    typedef struct event_pps_monitor_enable_t
    {
        struct
        {
            bool pps_active;
        } arg;
        struct
        {
            enum event_ret_t success;
        } ret;
    } EVENT_PPS_MONITOR_ENABLE_T;

    /**
 * @brief Callback for event
 */
    typedef void (*event_callback_fn)(struct port_tcpd *, void *);

    /**
 * @brief register an observer.
 * @param[in] event to subscribe
 * @param[in] handler to be called
 */
    bool event_subscribe(long, event_callback_fn);

    /**
 * @brief removes the observer. Observer stops getting notified
 * @param[in] handler to remove
 */
    void event_unsubscribe(long);

    /**
 * @brief notifies all observer that are listening to the event.
 * @param[in] event that occured
 * @param[in] tcpd_device
 * @param[in] data per context
 */
    void event_notify(long, struct port_tcpd *, void *);

#ifdef __cplusplus
}
#endif

#endif /* MODULES_OBSERVER_H_ */
