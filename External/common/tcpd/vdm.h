/* (http://www.onsemi.com/site/pdf/ONSEMI_T&C.pdf)
 *
 * ("ON Semiconductor Standard Terms and Conditions of Sale,
 *   Section 8 Software").
 *
 * DO NOT USE THIS SOFTWARE AND/OR DOCUMENTATION UNLESS YOU HAVE CAREFULLY
 * READ AND YOU AGREE TO THE LIMITED TERMS AND CONDITIONS. BY USING THIS
 * SOFTWARE AND/OR DOCUMENTATION, YOU AGREE TO THE LIMITED TERMS AND CONDITIONS.
 * @endparblock
 ******************************************************************************/
/* **************************************************************************
 *  Declares VDM functionality API.
 * ************************************************************************** */
#ifndef VDM_DRIVER_H_
#define VDM_DRIVER_H_

#include "port.h"
#include "pd_types.h"
#if (CONFIG_ENABLED(CUSTOM_APPLICATION_001))
#include "vif_info_001.h"
#elif (CONFIG_ENABLED(CUSTOM_APPLICATION_002))
#include "vif_info_002.h"
#else
#include "vif_info.h"
#endif

#if (TYPE_C_PORT_COUNT == 2)
#define MODAL_OPERATION_SUPPORTED (PORT_A_MODAL_OPERATION_SUPPORTED_SOP || PORT_B_MODAL_OPERATION_SUPPORTED_SOP)
#define RESPONDS_TO_DISCOV_SOP (PORT_A_RESPONDS_TO_DISCOV_SOP_UFP || PORT_A_RESPONDS_TO_DISCOV_SOP_DFP || PORT_B_RESPONDS_TO_DISCOV_SOP_UFP || PORT_B_RESPONDS_TO_DISCOV_SOP_DFP)
#define ATTEMPTS_DISCOV_SOP (PORT_A_ATTEMPTS_DISCOV_SOP || PORT_B_ATTEMPTS_DISCOV_SOP)
#define ATTEMPTS_DISCVID_SOP_P_FIRST (PORT_A_ATTEMPTS_DISCVID_SOP_P_FIRST || PORT_B_ATTEMPTS_DISCVID_SOP_P_FIRST)
#else
#define MODAL_OPERATION_SUPPORTED (PORT_A_MODAL_OPERATION_SUPPORTED_SOP)
#define RESPONDS_TO_DISCOV_SOP (PORT_A_RESPONDS_TO_DISCOV_SOP_UFP || PORT_A_RESPONDS_TO_DISCOV_SOP_DFP)
#define ATTEMPTS_DISCOV_SOP (PORT_A_ATTEMPTS_DISCOV_SOP)
#define ATTEMPTS_DISCVID_SOP_P_FIRST (PORT_A_ATTEMPTS_DISCVID_SOP_P_FIRST)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    enum svid_vid_t
    {
        SVID_VID_PD = 0xFF00,
        SVID_VID_DP = 0xFF01,
    };

    enum vdm_cmd_t
    {
        VDM_CMD_DISCOVER_IDENTITY = 1,
        VDM_CMD_DISCOVER_SVIDS    = 2,
        VDM_CMD_DISCOVER_MODES    = 3,
        VDM_CMD_ENTER_MODE        = 4,
        VDM_CMD_EXIT_MODE         = 5,
        VDM_CMD_ATTENTION         = 6,
    };

    enum vdm_rsp_t
    {
        VDM_CMD_TYPE_REQ  = 0,
        VDM_CMD_TYPE_ACK  = 1,
        VDM_CMD_TYPE_NAK  = 2,
        VDM_CMD_TYPE_BUSY = 3,
    };

    enum vdm_structured_t
    {
        VDM_UNSTRUCTURED = 0,
        VDM_STRUCTURED   = 1,
    };

    enum vdm_ver_t
    {
        VDM_STRUCTURED_VERSION1p0 = 0,
        VDM_STRUCTURED_VERSION2p0 = 1,
    };

    enum vdm_ver_minor_t
   {
	   VDM_STRUCTURED_VERSION_MINOR2p0 = 0,
	   VDM_STRUCTURED_VERSION_MINOR2p1 = 1,
   };

    enum vdm_ufp_vdo_ver_t
    {
        UFP_VDO_VERSION_1_0 = 0x0,
        UFP_VDO_VERSION_1_3 = 0x3,
    };

    enum vdm_dfp_vdo_ver_t
    {
        DFP_VDO_VERSION_1_0 = 0x0,
        DFP_VDO_VERSION_1_2 = 0x2,
    };
#define VDM_STRUCTURED_VER(pd)                                                                     \
    ((pd > PD_REV2) ? VDM_STRUCTURED_VERSION2p0 : VDM_STRUCTURED_VERSION1p0)
extern uint8_t VDM_MAX_VERSION_MINOR[TYPE_C_PORT_COUNT]; 
#define VDM_STRUCTURED_MAX_VER_MINOR(pd)  ((pd == PD_REV3) ? VDM_STRUCTURED_VERSION_MINOR2p1 : \
                                                   VDM_STRUCTURED_VERSION_MINOR2p0)
    typedef enum vdm_discvid_vdo_index_t
    {
        VDM_DISCVID_INDEX_HEADER = 0,
        VDM_DISCVID_INDEX_ID_HEADER,
        VDM_DISCVID_INDEX_CERT_STAT,
        VDM_DISCVID_INDEX_PRODUCT,
        VDM_DISCVID_INDEX_PRODUCT_TYPE1,
        VDM_DISCVID_INDEX_PRODUCT_TYPE2,
        VDM_DISCVID_INDEX_PRODUCT_TYPE3,
        VDM_DISCVID_MAX_LEN,
    } VDM_DISCVID_VDO_INDEX_T;

    /* Product Type field in ID Header */
    typedef enum vdm_id_ufp_sop_t
    {
        VDM_ID_UFP_SOP_UNDEFINED  = 0x0,
        VDM_ID_UFP_SOP_HUB        = 0x1,
        VDM_ID_UFP_SOP_PERIPHERAL = 0x2,
        VDM_ID_UFP_SOP_PSD        = 0x3,
    } VDM_ID_UFP_SOP_T;

    typedef enum vdm_id_ufp_sop_1_t
    {
        VDM_ID_UFP_SOP_1_UNDEFINED     = 0x0,
        VDM_ID_UFP_SOP_1_PASSIVE_CABLE = 0x3,
        VDM_ID_UFP_SOP_1_ACTIVE_CABLE  = 0x4,
        VDM_ID_UFP_AMA                 = 0x5,
        VDM_ID_UFP_SOP_1_VPD           = 0x6,
    } VDM_ID_UFP_SOP_1_T;

    typedef enum vdm_id_dfp_t
    {
        VDM_ID_DFP_UNDEFINED   = 0x0,
        VDM_ID_DFP_HUB         = 0x1,
        VDM_ID_DFP_HOST        = 0x2,
        VDM_ID_DFP_POWER_BRICK = 0x3,
        VDM_ID_DFP_AMC         = 0x4,
    } VDM_ID_DFP_T;

    typedef enum vdm_cable_max_current_t
    {
        VDM_CABLE_MAX_CURRENT_3A = 1,
        VDM_CABLE_MAX_CURRENT_5A = 2,
    } VDM_CABLE_MAX_CURRENT_T;

    typedef enum vdm_cable_max_voltage_t
    {
        VDM_CABLE_MAX_VOLTAGE_20V = 0,
        VDM_CABLE_MAX_VOLTAGE_30V = 1,
        VDM_CABLE_MAX_VOLTAGE_40V = 2,
        VDM_CABLE_MAX_VOLTAGE_50V = 3,
    } VDM_CABLE_MAX_VOLTAGE_T;

    typedef enum vdm_ufp_vdo_usb_highest_speed_t
	{
		VDM_UFP_VDO_USB_2 = 0,
		VDM_UFP_VDO_USB_3_2_GEN1 = 1,
		VDM_UFP_VDO_USB_3_2_GEN2 = 2,
		VDM_UFP_VDO_USB_4_GEN3 = 3,
		VDM_UFP_VDO_USB_4_GEN4 = 4,
		VDM_UFP_VDO_USB_RESERVED1 = 5,
		VDM_UFP_VDO_USB_RESERVED2 = 6,
		VDM_UFP_VDO_USB_RESERVED3 = 7,
 	} VDM_UFP_VDO_USB_HIGHEST_SPEED_T;
    typedef struct vdm_sm_arg_t
    {
        struct port_tcpd *dev;
        struct port_vif_t       *vif;
        enum sop_t        sop;
        enum pd_rev_t     pdrev;
        bool              tx_msg;
        struct
        {
            uint32_t *vdo;
            unsigned  count;
            unsigned  len;
        } buf;
    } VDM_SM_ARG_T;

    typedef struct svdm_handler_t
    {
        void (*vdm_process)(struct vdm_sm_arg_t *);
        void (*sm)(struct vdm_sm_arg_t *);
    } const SVDM_HANDLER_T;

    typedef struct uvdm_handler_t
    {
        void (*vdm_process)(struct vdm_sm_arg_t *);
        bool (*sm)(struct vdm_sm_arg_t *);
    } const UVDM_HANDLER_T;

    typedef struct vdm_obj_t
    {
        bool     structured;
        uint16_t svid;
        uint8_t sopmask;
        union
        {
            struct svdm_handler_t const *svdmh;
            struct uvdm_handler_t const *uvdmh;
        };
        bool (*active)(struct port_tcpd const *);
        void (*set_active)(struct port_tcpd const *, bool);
        void (*reset)(struct port_tcpd *const, uint16_t);
    } const VDM_OBJ_T;

    typedef struct vdm_item_t
    {
        unsigned long           portmask;
        struct vdm_obj_t const *vdm;
        struct vdm_item_t      *next;
    } VDM_ITEM_T;

    /**
 * @brief register a vdm driver
 */
    void vdm_list_register(struct port_tcpd *const, struct vdm_item_t *);

    /**
 * @brief find VDM driver registered.
 */
    struct vdm_obj_t const *vdm_list_find_svid(struct port_tcpd *const, uint16_t, uint8_t);

    /**
 * Reseet vdm driver
 */
    void vdm_list_reset_all(struct port_tcpd *const, uint16_t, uint8_t, uint16_t);
    /**
 * @brief execute the auto discovery
 */
    bool vdm_list_run_active(struct vdm_sm_arg_t *);

    /**
 * @brief disable vdm driver
 * @param device
 * @param svid
 */
    void vdm_disable_driver(struct port_tcpd const *, uint16_t, uint8_t);
    
    /**
* @brief enable vdm driver
* @param device
* @param svid
*/
    void vdm_enable_driver(struct port_tcpd const *dev, uint16_t svid, uint8_t sop);

    /**
 * @brief Disable all vdm drivers
 * @param device
 */
    void vdm_disable_all(struct port_tcpd const *);
    

    /**
    * @brief returns if driver with SVID exists and is active
    * @param device
    */
    bool vdm_is_active(struct port_tcpd const *dev, uint16_t svid);
    
    /*
     * @brief updates svdm version minor bit to a new minimum. Does nothing in PD2.
     * @param dev 
     * @param svdm_version_minor
     */
    void update_svdm_version_minor(struct port_tcpd const *dev, uint8_t svdm_version_minor);


#ifdef __cplusplus
}
#endif

#endif /*  _VDM_H_ */
