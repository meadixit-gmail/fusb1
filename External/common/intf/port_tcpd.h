/*******************************************************************************
 * @file     port_tcpd.h
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
#ifndef __PORT_TCPD_H__
#define __PORT_TCPD_H__

#include "timer.h"
#include "pd_types.h"
#include "vdm.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define MAX_CABLE_LOOP 20
    /**
 * Defines Timer IDs for various timers
 */
    typedef enum tcpd_timer_id_t
    {
        /* Type-C timers */
        TC_TIMER = 0,
        CC_DBNC_TIMER,
        CC_PDDBNC_TIMER,
        VC_DBNC_TIMER,
        VC_PDDBNC_TIMER,
#if CONFIG_ENABLED(DRP)
        TC_TRY_TIMEOUT_TIMER, //DRP
#endif
        /* Policy Timers */
        POLICY_TIMER,
        SINKTX_TIMER,
        PD_RSP_TIMER,
        PPS_TIMER,
        PROT_TIMER,
        SWAP_SOURCE_TIMER,
        VDM_TIMER,
#if CONFIG_ENABLED(USB4)
        VCONN_POWER_TIMER,     // USB4
        VCONN_DISCHARGE_TIMER, // USB4
        DATA_RESET_TIMER,      // USB4
		ENTER_USB_TIMER,
		ENTER_USB_WAIT_TIMER,
#endif
        RP_CHANGE_TIMER,
        DISCOVER_IDENTITY_TIMER,
        EPR_KEEPALIVE_TIMER,
        SINK_EPR_ENTER_TIMER,
		EXTENDED_TIMER,
		UVP_DBNC_TIMER,
        NUM_TIMERS,
    } TIMER_ID_T;

#if (CONFIG_ENABLED(EPR))
#define NUM_SRC_CAP 11
#else
#define NUM_SRC_CAP 7
#endif

#if (CONFIG_ENABLED(SNK))
#define NUM_SNK_CAP 7
#else
#define NUM_SNK_CAP 0
#endif

#if (CONFIG_ENABLED(EXTMSG))
#define EXT_BUFFER (MAX_EXT_MSG_LEN + sizeof(PD_HEADER_T))
#else
#define EXT_BUFFER 0
#endif

#ifndef ABS
#define ABS(N) ((N<0)?(-N):(N))
#endif

typedef struct msgtx_t {
	union
	{
		uint32_t value;
		struct
		{
			bool gotomin:1;
			bool get_src_caps:1;
			bool get_snk_caps:1;
			bool drswap:1;
			bool prswap:1;
			bool vconnswap:1;
			bool request:1;
			bool snk_caps:1;
			bool src_caps:1;
			bool bist:1;
			bool vdm:1;
			bool datareset:1;
			bool getsrccapext:1;
			bool getstatus:1;
			bool frswap:1;
			bool getppsstatus:1;
			bool getcountrycodes:1;
			bool getsinkcapext:1;
			bool getsourceinfo:1;
			bool getrevision:1;
			bool alert:1;
			bool getcountryinfo:1;
			bool enterusb:1;
			bool eprrequest:1;
			bool eprmode:1;
			bool getbatterycap:1;
			bool getbatterystatus:1;
			bool getmaninfo:1;
			bool securityrequest:1;
			bool fwupdaterequest:1;
			bool extendedcontrol:1;
			bool vdmext:1;
		} booltxmsg;
		struct
		{
			uint32_t rev2txmsg:11;
			uint32_t rev3txmsg:21;
		}bytetxmsg;
	};
} MSGTX_T;

typedef enum {
	VDMExtended = 0,
	Rev3Index = VDMExtended,
	ExtendedControl,
	FWUpdateRequest,
	SecurityRequest,
	GetManufacturerInfo,
	GetBatteryStatus,
	GetBatteryCapabilities,
	EPRMode,
	EPRRequest,
	EnterUSB,
	GetCountryInfo,
	Alert,
	GetRevision,
    GetSourceInfo,
	GetSinkCapExt,
	GetCountryCodes,
	GetPPSStatus,
	FRSwap,
	GetStatus,
	GetSrcCapExt,
	DataReset,
	VDM,
	Rev2Index = VDM,
	BIST,
	SrcCaps,
	SinkCaps,
	Request,
	VconnSwap,
	PRSwap,
	DRSwap,
	GetSinkCaps,
	GetSourceCaps,
	GoToMin,
	NumTxMsgs,
} TXMSG_LS_T;

    /*
 * The Port struct contains all port-related data and state information,
 * timer references, register map, etc.
 */
    struct port
    {
        struct port_vif_t       *vif;
        struct port_tcpd        *dev;
        struct fusb_tcpd_device *tcpd_device;
        struct ticktimer_t timers[NUM_TIMERS];
        enum tc_port_t        type;
        enum pd_rev_t         pd_rev[SOP_SIZE];
        enum tc_state_t       tc_state;
        enum policy_state_t   policy_state;
        enum policy_state_t   policy_state_prev;
        enum protocol_state_t prl_state;
        enum policy_state_t   vdm_success_ps;
        enum prot_tx_stat_t   prl_tx_status;
        enum cbl_rst_state_t  cbl_rst_state;
        enum pdo_t 			  PDO_Type;
        uint8_t               hard_reset_counter;
        int                   tc_substate;
        int                   policy_substate;

        struct pd_msg_t prl_tx_data[7]; //Protocol Tx Buffer
        struct pd_msg_t prl_rx_data[7]; //Protocol Rx Buffer
        
        struct pd_msg_t caps_source[NUM_SRC_CAP];    //Used to tx Source Cap (can likely make local)
        struct pd_msg_t caps_sink[NUM_SRC_CAP];      //Used to tx Sink Cap
        struct pd_msg_t caps_received[NUM_SRC_CAP];  //Used by Sink to store Source Cap
        struct pd_msg_t policy_pd_data[NUM_SRC_CAP]; //Stores rx PD message - could maybe be local
#if (CONFIG_ENABLED(EPR))
        struct pd_msg_t epr_mode_message;
#endif

        struct pd_msg_t usb_pd_contract;
#if (CONFIG_ENABLED(EPR))
        struct pd_msg_t sink_request[2];
#else
        struct pd_msg_t sink_request[2];
#endif
#if CONFIG_ENABLED(SNK)
        struct pd_msg_t stored_apdo;
#endif
        struct pd_msg_t partner_caps;
        struct pd_msg_t previous_vdm_tx_header;
#if CONFIG_ENABLED(USB4)
        struct pd_msg_t partner_product_type_ufp;
        uint8_t enter_usb_sent_sopx;
        uint8_t usb_supported_sopx;
        uint8_t enter_usb_rejected;
        uint8_t enter_usb_waited_sopx;
        bool enter_usb_timeout;
        uint8_t sop_p_traffic_tx_rx;
#if MODAL_OPERATION_SUPPORTED
        bool svid_enabled;
#endif
#endif
#if CONFIG_ENABLED(USB4) || MODAL_OPERATION_SUPPORTED
        uint8_t disc_id_sent_sopx;
#endif
#if MODAL_OPERATION_SUPPORTED || CONFIG_ENABLED(USB4)
        uint8_t disc_svid_sent_sopx;
        uint8_t disc_svid_received_sopx;
        uint8_t disc_modes_sent_sopx;
        uint8_t disc_modes_received_sopx;
#endif
        unsigned vdm_msg_length; //Looks like this can easily be removed by improving VDM flow
        unsigned sink_selected_voltage;
        unsigned sink_selected_current;
        struct pd_header_t policy_pd_header;
        struct pd_header_t caps_header_sink; //Used for Sink Caps - can be made local
        struct pd_header_t caps_header_source;
        struct pd_header_t caps_header_received;
        struct pd_header_t prl_rx_header;
        struct pd_header_t prl_tx_header;

        uint16_t prl_ext_num_bytes;
        uint16_t discv_id_counter;

        uint8_t caps_counter;
        uint32_t sink_contract_power;
        uint8_t prl_ext_buf[EXT_BUFFER];
        uint8_t prl_ext_chunk_num;
        uint8_t prl_ext_req_chunk;
        uint8_t prl_ext_state_active;
        uint8_t prl_ext_req_cmd;
        uint8_t prl_tx_msgid[SOP_SIZE];
        uint8_t prl_rx_msgid[SOP_SIZE];
        uint8_t bat_stat_ref; //Stores battery reference for Alert handling
        MSGTX_T msgtx;
        struct pd_msg_t optionalmsginfo;
        struct
        {
            enum sop_t     prl_msg_rx_sop;
            enum sop_t     prl_msg_tx_sop;
            enum sop_t     vdm_msg_tx_sop;
            enum cc_stat_t cc_term_raw;
            enum cc_stat_t cc_term_cc_debounce;
            enum cc_stat_t cc_term_pd_debounce;

            enum cc_stat_t      vc_term_raw;
            enum cc_stat_t      vc_term_cc_debounce;
            enum cc_stat_t      vc_term_pd_debounce;
            enum cc_rp_t        src_current;
            enum source_or_sink source_or_sink;

            enum cc_t cc_pin;
#if CONFIG_ENABLED(FAULT_PROTECTION)
            bool fault_active;
            bool uvp_active;
#endif
            bool      alert_response;
            bool      src_preferred;
            bool      snk_preferred;
            bool      current_limited;
            bool      acc_support;
            bool      tc_enabled;
            bool      is_vconn_source;
            bool      is_hard_reset;
            bool      pd_tx_flag;

            bool      is_pr_swap;
            bool      is_vconn_swap;
            bool      pd_active;
            bool      pd_enabled;
            bool      protocol_use_sinktx;
            bool      prl_msg_rx;
            bool      prl_msg_rx_pending;
            uint8_t cable_vdo_cable_type_ufp;
            uint8_t cable_vdo_cable_current;
            uint8_t cable_vdo_cable_voltage;
            uint16_t vdm_tx_svid;
            bool    partner_caps_available;
            bool    source_is_apdo;
            bool    vdm_cbl_present;
            bool    vdm_check_cbl;
            bool    higher_cable_cap;
            bool    policy_has_contract;
            bool    renegotiate;
            bool    needs_goto_min;
            bool    policy_is_source;
            bool    policy_is_dfp;
            bool    is_contract_valid;
            bool    policy_ams_start;
            bool    policy_ams_active;
            bool    req_dr_swap_to_dfp_as_sink;
            bool    req_dr_swap_To_ufp_as_src;
            bool    req_vconn_swap_to_on_as_sink;
            bool    req_vconn_swap_to_off_as_src;
            bool    req_pr_swap_as_src;
            bool    req_pr_swap_as_snk;
            bool    pps_mode_change;
            bool    vdm_auto_state;
            bool    prl_ext_send_chunk;
            bool    event;
            bool    req_status;
            bool    mode_entered;
            bool    policy_sending_message;
#if (CONFIG_ENABLED(USB4))
            bool usb_mode;
            uint8_t cable_vdo_cable_highest_usb_speed;
            bool cable_vdo_cable_highest_usb2_mode;
            bool cable_vdo_cable_highest_usb3_2_mode;
        	bool cable_vdo_cable_highest_usb4_mode;
        	struct pd_msg_t tbt3vdo;
#endif
#if (CONFIG_ENABLED(EPR))
            bool captive_cable;
            bool epr_mode;
            bool epr_capable_rdo;
            bool epr_capable_pdo;
            bool epr_capable_cable;
            bool epr_pdo;
            bool negotiate_epr_contract :1;
            uint8_t epr_src_cap_len;
            uint8_t epr_snk_cap_len;
            bool epr_try_exit;
#if CONFIG_ENABLED(SNK)
            bool epr_try_entry;
#endif
#endif
        };
    };

    struct port_tcpd *dpm_port_init(void *dpm_info, int port_id, void *vif);
    /**
 * @brief called by dpm to initalize a port
 * @param[in] a device
 */
    void port_tcpd_init(void *);

    /**
 * @brief called by dpm to enable a port
 * @param[in] a device
 */
    void port_tcpd_pd_enable(struct port_tcpd *);

    /**
 * @brief called by dpm to disable a port
 * @param[in] a device
 */
    void port_tcpd_pd_disable(struct port_tcpd *);

    /**
 * @brief called by dpm to set Type-C state to Attached.SRC
 * @param[in] a device
 */
    void port_tcpd_set_attached_source(struct port_tcpd *dev);

    void port_tcpd_set_unattached_wait_source(struct port_tcpd *dev);
    /**
 * @brief returns the type-c state
 * @param[in] a type-c device
 * @return type-c state
 */
    enum tc_state_t port_tc_state(struct port_tcpd *);

    /**
 * @brief called by dpm execute TCPD statemachine
 * @param[in] a device
 */
    void port_sm(struct port_tcpd *);

    /**
 * @brief Executes statemachine until it can yield. This will not return
 * statemachine until it has nothing to do or waiting on a timer. If you need to schedule
 * some other task while the statemachine is active use port_yeild().
 * @param tcpd device
 */
    void port_sm_exec(struct port_tcpd *);



    void port_send_hard_reset(struct port_tcpd *);

    /**
 * @brief called by dpm to set error recovery
 * @param[in] a device
 */
    void port_set_error_recovery(struct port_tcpd *);

    /**
 * @brief called by dpm to set disabled
 * @param[in] a device
 */
    void port_set_disabled(struct port_tcpd *);

    /**
 * @brief called by dpm to set unattached
 * @param[in] a device
 */
    void port_set_unattached(struct port_tcpd *);

    enum cc_t port_get_cc_orientation(struct port_tcpd *dev);

    /**
 * Check if the port is yielding on a timer. If yield is true then
 * the port can be put to sleep.
 * @param tcpd device
 */
    bool port_yield(struct port_tcpd *);


    /**
 * Check if the port has type-c enabled.
 * @param tcpd device
 */
    bool port_tc_enabled(struct port_tcpd *dev);


    /**
 * Check if the port is sending a message.
 * @param tcpd device
 */
    bool port_sending_message(struct port_tcpd *dev);

    /**
 * @brief Prevent type-c statemachine from running/disable. Type-c statemachine
 * will resume when enabled.
 * @param a typec device
 */
    void port_disable(struct port_tcpd *);

    /**
 * @brief Resume type-c statemachine/enable.
 * @param a typec device
 */
    void port_enable(struct port_tcpd *);

    /**
 * @brief called by dpm to find the earliest deadline
 * @param[in] a device
 */
    unsigned long port_tcpd_next_schedule(struct port_tcpd *);

    /**
 * @brief called by dpm to run cable reset
 * @param[in] a device
 */
    void port_cable_reset(struct port_tcpd *);
    /**
 * @brief Returns if policy is in a PD contract
 * @param[in] a device
 */
    bool port_policy_has_contract(struct port_tcpd *);

    /**
 ** @brief called by dpm to get sink selected voltage in pps contract
 */
    uint32_t port_sink_selected_voltage(struct port_tcpd *);

    /**
 * @brief Set PD contract to valid/invalid
 * @param[in] a device
 */
    void port_set_contract_valid(struct port_tcpd *, bool);

    /**
 * @brief Transmit source capabilities on port
 * @param[in] a device
 */
    void port_transmit_source_caps(struct port_tcpd *);

    /**
 * @brief Transmit Get Battery Capabilities
 * @param[in] a device
 */
    void port_transmit_get_battery_capability(struct port_tcpd *dev);

    /**
 * @brief Transmit Get Battery Status
 * @param[in] a device
 */
    void port_transmit_get_battery_status(struct port_tcpd *dev);

    /**
 * @brief Set port Rp current advertisement
 * @param[in] a device
 * @return
 */
    void port_source_set_rp_current(struct port_tcpd *dev, enum cc_rp_t current);

#ifdef __cplusplus
}
#endif

#endif /* __PORT_TCPD_H__ */
