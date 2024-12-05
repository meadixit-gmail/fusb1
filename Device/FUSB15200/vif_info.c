/*******************************************************************************
 * @file     vendor_info.c
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

#include "vif_types.h"
#if (CONFIG_CUSTOM_APPLICATION_002)
#include "vif_info_002.h"
#elif (CONFIG_CUSTOM_APPLICATION_001)
#include "vif_info_001.h"
#else
#include "vif_info.h"
#endif
PORT_VIF_T port1_vif = {
    /* Defines that are used by PDOs */
    .Connector_Type                          = PORT_A_CONNECTOR_TYPE,
    .Port_Managed_Guaranteed_Type            = PORT_A_MANAGED_GUARANTEED_TYPE,
	.ID_Header_Connector_Type                = PORT_A_ID_HEADER_CONNECTOR_TYPE,
    .USB_PD_Support                          = PORT_A_USB_PD_SUPPORT,
    .PD_Port_Type                            = PORT_A_PD_PORT_TYPE,
    .Type_C_State_Machine                    = PORT_A_TYPE_C_STATE_MACHINE, // DRP
    .Port_Battery_Powered                    = PORT_A_PORT_BATTERY_POWERED,
    .BC_1_2_Support                          = PORT_A_BC_1_2_SUPPORT,
    .PD_Specification_Revision               = PORT_A_PD_SPECIFICATION_REVISION,
    .PD_Spec_Revision_Major                  = PORT_A_PD_SPEC_REVISION_MAJOR,
    .PD_Spec_Revision_Minor                  = PORT_A_PD_SPEC_REVISION_MINOR,
    .PD_Spec_Version_Major                   = PORT_A_PD_SPEC_VERSION_MAJOR,
    .PD_Spec_Version_Minor                   = PORT_A_PD_SPEC_VERSION_MINOR,
    .SOP_Capable                             = PORT_A_SOP_CAPABLE,
    .SOP_P_Capable                           = PORT_A_SOP_P_CAPABLE,
    .SOP_PP_Capable                          = PORT_A_SOP_PP_CAPABLE,
    .SOP_P_Debug_Capable                     = PORT_A_SOP_P_DEBUG_CAPABLE,
    .SOP_PP_Debug_Capable                    = PORT_A_SOP_PP_DEBUG_CAPABLE,
    .Manufacturer_Info_Supported_Port        = PORT_A_MANUFACTURER_INFO_SUPPORTED_PORT,
    .Manufacturer_Info_VID_Port              = PORT_A_MANUFACTURER_INFO_VID_PORT,
    .Manufacturer_Info_PID_Port              = PORT_A_MANUFACTURER_INFO_PID_PORT,
    .Num_Fixed_Batteries                     = PORT_A_NUM_FIXED_BATTERIES,
    .Num_Swappable_Battery_Slots             = PORT_A_NUM_SWAPPABLE_BATTERY_SLOTS,
    .USB_Comms_Capable                       = PORT_A_USB_COMMS_CAPABLE,
	.Is_USB4_Peripheral                      = PORT_X_USB4_IS_PERIPHERAL,
    .DR_Swap_To_DFP_Supported                = PORT_A_DR_SWAP_TO_DFP_SUPPORTED,
    .DR_Swap_To_UFP_Supported                = PORT_A_DR_SWAP_TO_UFP_SUPPORTED,
    .Unconstrained_Power                     = PORT_A_UNCONSTRAINED_POWER,
    .Attempts_Discov_SOP                     = PORT_A_ATTEMPTS_DISCOV_SOP,
    .Chunking_Implemented_SOP                = PORT_A_CHUNKING_IMPLEMENTED_SOP,
    .Unchunked_Extended_Messages_Supported   = PORT_A_UNCHUNKED_EXTENDED_MESSAGES_SUPPORTED,
    .Security_Msgs_Supported_SOP             = PORT_A_SECURITY_MSGS_SUPPORTED_SOP,
    .Type_C_Implements_Try_SRC               = PORT_A_TYPE_C_IMPLEMENTS_TRY_SRC,
    .Type_C_Implements_Try_SNK               = PORT_A_TYPE_C_IMPLEMENTS_TRY_SNK,
    .Rp_Value                                = PORT_A_RP_VALUE,
    .Type_C_Supports_VCONN_Powered_Accessory = PORT_A_TYPE_C_SUPPORTS_VCONN_POWERED_ACCESSORY,
    .Type_C_Is_VCONN_Powered_Accessory       = PORT_A_TYPE_C_IS_VCONN_POWERED_ACCESSORY,
    .Type_C_Is_Debug_Target_SRC              = PORT_A_TYPE_C_IS_DEBUG_TARGET_SRC,
    .Type_C_Is_Debug_Target_SNK              = PORT_A_TYPE_C_IS_DEBUG_TARGET_SNK,
    .Type_C_Can_Act_As_Host                  = PORT_A_TYPE_C_CAN_ACT_AS_HOST,
    .Type_C_Can_Act_As_Device                = PORT_A_TYPE_C_CAN_ACT_AS_DEVICE,
    .Type_C_Power_Source                     = PORT_A_TYPE_C_POWER_SOURCE,

    .Type_C_Supports_Audio_Accessory = PORT_A_TYPE_C_SUPPORTS_AUDIO_ACCESSORY,

    .Type_C_Sources_VCONN          = PORT_A_TYPE_C_SOURCES_VCONN,
    .Product_Total_Source_Power_mW = PORT_A_PRODUCT_TOTAL_SOURCE_POWER_MW,
    .PD_Power_as_Source            = PORT_A_PD_POWER_AS_SOURCE,
    .USB_Suspend_May_Be_Cleared    = PORT_A_USB_SUSPEND_MAY_BE_CLEARED,
    .Sends_Pings                   = PORT_A_SENDS_PINGS,
    .FR_Swap_Reqd_Type_C_Current_As_Initial_Source =
        PORT_A_FR_SWAP_REQD_TYPE_C_CURRENT_AS_INITIAL_SOURCE,
    .FR_Swap_Type_C_Current_Capability_As_Initial_Sink =
        PORT_B_FR_SWAP_TYPE_C_CURRENT_CAPABILITY_AS_INITIAL_SINK,

    .PD_Power_as_Sink          = PORT_A_PD_POWER_AS_SINK,
    .PD_Min_Power_as_Sink      = PORT_A_PD_MIN_SINK_PDP,
    .No_USB_Suspend_May_Be_Set = PORT_A_NO_USB_SUSPEND_MAY_BE_SET,
    .GiveBack_May_Be_Set       = PORT_A_GIVEBACK_MAY_BE_SET,
    .Higher_Capability_Set     = PORT_A_HIGHER_CAPABILITY_SET,
    .Accepts_PR_Swap_As_Src    = PORT_A_ACCEPTS_PR_SWAP_AS_SRC,
    .Accepts_PR_Swap_As_Snk    = PORT_A_ACCEPTS_PR_SWAP_AS_SNK,
    .Requests_PR_Swap_As_Src   = PORT_A_REQUESTS_PR_SWAP_AS_SRC,
    .Requests_PR_Swap_As_Snk   = PORT_A_REQUESTS_PR_SWAP_AS_SNK,
    .EPR_Supported_As_Src      = PORT_A_EPR_SUPPORTED_AS_SRC,
    .EPR_Supported_As_Snk      = PORT_A_EPR_SUPPORTED_AS_SNK,

    .XID_SOP                        = PORT_A_XID_SOP,
    .Data_Capable_as_USB_Host_SOP   = PORT_A_DATA_CAPABLE_AS_USB_HOST_SOP,
    .Data_Capable_as_USB_Device_SOP = PORT_A_DATA_CAPABLE_AS_USB_DEVICE_SOP,
    .Product_Type_UFP_SOP           = PORT_A_PRODUCT_TYPE_UFP_SOP,
    .Product_Type_DFP_SOP           = PORT_A_PRODUCT_TYPE_DFP_SOP,
    .DFPVDO_USB2_0_Capable          = PORT_A_DFPVDO_USB2_0_CAPABLE,
    .DFPVDO_USB3_2_Capable          = PORT_A_DFPVDO_USB3_2_CAPABLE,
    .DFPVDO_USB4_Capable            = PORT_A_DFPVDO_USB4_CAPABLE,
	.UFPVDO_USB_Highest_Speed       = PORT_A_UFPVDO_USB_HIGHEST_SPEED,
    .Modal_Operation_Supported_SOP  = PORT_A_MODAL_OPERATION_SUPPORTED_SOP,
    .USB_VID_SOP                    = PORT_A_USB_VID_SOP,
    .PID_SOP                        = PORT_A_PID_SOP,
    .bcdDevice_SOP                  = PORT_A_BCDDEVICE_SOP,
    .SVID_fixed_SOP                 = PORT_A_SVID_FIXED_SOP,
    .Num_SVIDs_min_SOP              = PORT_A_NUM_SVIDS_MIN_SOP,
    .Num_SVIDs_max_SOP              = PORT_A_NUM_SVIDS_MAX_SOP,
    .SVID_SOP1                      = PORT_A_SVID_SOP1,
    .SVID_modes_fixed_SOP1          = PORT_A_SVID_MODES_FIXED_SOP1,
    .SVID_num_modes_min_SOP1        = PORT_A_SVID_NUM_MODES_MIN_SOP1,
    .SVID_num_modes_max_SOP1        = PORT_A_SVID_NUM_MODES_MAX_SOP1,
    .SVID_mode_enter_SOP1           = PORT_A_SVID_MODE_ENTER_SOP1,

    .DR_Swap_To_DFP_Supported    = PORT_A_DR_SWAP_TO_DFP_SUPPORTED,
    .DR_Swap_To_UFP_Supported    = PORT_A_DR_SWAP_TO_UFP_SUPPORTED,
    .VCONN_Swap_To_On_Supported  = PORT_A_VCONN_SWAP_TO_ON_SUPPORTED,
    .VCONN_Swap_To_Off_Supported = PORT_A_VCONN_SWAP_TO_OFF_SUPPORTED,

    .Responds_To_Discov_SOP_UFP = PORT_A_RESPONDS_TO_DISCOV_SOP_UFP,
    .Responds_To_Discov_SOP_DFP = PORT_A_RESPONDS_TO_DISCOV_SOP_DFP,

    /* These are custom flags not from vif so changes these as needed */
    .Attempts_DiscvId_SOP_P_First     = PORT_A_ATTEMPTS_DISCOV_SOP,
    .Attempt_Vconn_Swap_to_Off_As_Src = PORT_A_ATTEMPT_VCONN_SWAP_TO_OFF_AS_SRC,
    .Attempt_Vconn_Swap_to_On_As_Sink = PORT_A_ATTEMPT_VCONN_SWAP_TO_ON_AS_SINK,
    .Attempt_DR_Swap_to_Ufp_As_Src    = PORT_A_ATTEMPT_DR_SWAP_TO_UFP_AS_SRC,
    .Attempt_DR_Swap_to_Dfp_As_Sink   = PORT_A_ATTEMPT_DR_SWAP_TO_DFP_AS_SINK,
    .DisplayPort_Auto_Mode_Entry      = YES,
    .DisplayPort_Enabled              = YES,
    .DisplayPort_Preferred_Snk        = NO,
    .DisplayPort_UFP_Capable          = YES,
    .DisplayPort_DFP_Capable          = YES,
    .DisplayPort_Signaling            = YES,
    .DisplayPort_Receptacle           = YES,
    .DisplayPort_USBr2p0Signal_Req    = NO,
    .DisplayPort_DFP_Pin_Mask         = 0x4 | 0x8 | 0x10,
    .DisplayPort_UFP_Pin_Mask         = 0x4 | 0x8 | 0x10,
    .DisplayPort_UFP_PinAssign_Start  = 'C',

    .Num_Src_PDOs = PORT_A_NUM_SRC_PDOS,
    .Num_Snk_PDOs = PORT_A_NUM_SNK_PDOS,
#if CONFIG_ENABLED(EPR)
    .Num_Src_EPR_PDOs = PORT_A_EPR_SRC_PDOS,
    .EPR_Supported_As_Src      = PORT_A_EPR_SUPPORTED_AS_SRC,
	.EPR_Supported_As_Snk      = PORT_A_EPR_SUPPORTED_AS_SNK,
#endif
    /* macro expects index starting at 1 and type */
    .src_caps =
        {
#if PORT_A_TOTAL_NUM_SRC_PDOS > 0
            CREATE_SUPPLY_PDO_FIRST(PORT_A_, 1),
#endif
#if PORT_A_TOTAL_NUM_SRC_PDOS > 1
            CREATE_SUPPLY_PDO(PORT_A_, 2, PORT_A_SRC_PDO_SUPPLY_TYPE_2, 0),
#endif
#if PORT_A_TOTAL_NUM_SRC_PDOS > 2
            CREATE_SUPPLY_PDO(PORT_A_, 3, PORT_A_SRC_PDO_SUPPLY_TYPE_3, 0),
#endif
#if PORT_A_TOTAL_NUM_SRC_PDOS > 3
            CREATE_SUPPLY_PDO(PORT_A_, 4, PORT_A_SRC_PDO_SUPPLY_TYPE_4, 0),
#endif
#if PORT_A_TOTAL_NUM_SRC_PDOS > 4
            CREATE_SUPPLY_PDO(PORT_A_, 5, PORT_A_SRC_PDO_SUPPLY_TYPE_5, 0),
#endif
#if PORT_A_TOTAL_NUM_SRC_PDOS > 5
            CREATE_SUPPLY_PDO(PORT_A_, 6, PORT_A_SRC_PDO_SUPPLY_TYPE_6, 0),
#endif
#if PORT_A_TOTAL_NUM_SRC_PDOS > 6
            CREATE_SUPPLY_PDO(PORT_A_, 7, PORT_A_SRC_PDO_SUPPLY_TYPE_7, 0),
#endif
#if PORT_A_TOTAL_NUM_SRC_PDOS > 7
            CREATE_SUPPLY_PDO(PORT_A_, 8, PORT_A_SRC_PDO_SUPPLY_TYPE_8, 0),
#endif
#if PORT_A_TOTAL_NUM_SRC_PDOS > 8
            CREATE_SUPPLY_PDO(PORT_A_, 9, PORT_A_SRC_PDO_SUPPLY_TYPE_9, 0),
#endif
#if PORT_A_TOTAL_NUM_SRC_PDOS > 9
            CREATE_SUPPLY_PDO(PORT_A_, 10, PORT_A_SRC_PDO_SUPPLY_TYPE_10, 0),
#endif
#if PORT_A_TOTAL_NUM_SRC_PDOS > 10
            CREATE_SUPPLY_PDO(PORT_A_, 11, PORT_A_SRC_PDO_SUPPLY_TYPE_11, 0),
#endif
        },

    /* macro expects index start at 1 and type */
    .snk_caps =
        {
#if PORT_A_NUM_SNK_PDOS == 0
            0,
#endif
#if PORT_A_TOTAL_NUM_SNK_PDOS > 0
            CREATE_SINK_PDO_FIRST(PORT_A_, 1),
#endif
#if PORT_A_TOTAL_NUM_SNK_PDOS > 1
            CREATE_SINK_PDO(PORT_A_, 2, PORT_A_SNK_PDO_SUPPLY_TYPE_2, 0),
#endif
#if PORT_A_TOTAL_NUM_SNK_PDOS > 2
            CREATE_SINK_PDO(PORT_A_, 3, PORT_A_SNK_PDO_SUPPLY_TYPE_3, 0),
#endif
#if PORT_A_TOTAL_NUM_SNK_PDOS > 3
            CREATE_SINK_PDO(PORT_A_, 4, PORT_A_SNK_PDO_SUPPLY_TYPE_4, 0),
#endif
#if PORT_A_TOTAL_NUM_SNK_PDOS > 4
            CREATE_SINK_PDO(PORT_A_, 5, PORT_A_SNK_PDO_SUPPLY_TYPE_5, 0),
#endif
#if PORT_A_TOTAL_NUM_SNK_PDOS > 5
            CREATE_SINK_PDO(PORT_A_, 6, PORT_A_SNK_PDO_SUPPLY_TYPE_6, 0),
#endif
#if PORT_A_TOTAL_NUM_SNK_PDOS > 6
            CREATE_SINK_PDO(PORT_A_, 7, PORT_A_SNK_PDO_SUPPLY_TYPE_7, 0),
#endif

#if PORT_A_TOTAL_NUM_SNK_PDOS > 7
            CREATE_SINK_PDO(PORT_A_, 8, PORT_A_SNK_PDO_SUPPLY_TYPE_8, 1),
#endif
#if PORT_A_TOTAL_NUM_SNK_PDOS > 8
            CREATE_SINK_PDO(PORT_A_, 9, PORT_A_SNK_PDO_SUPPLY_TYPE_9, 1),
#endif
#if PORT_A_TOTAL_NUM_SNK_PDOS > 9
            CREATE_SINK_PDO(PORT_A_, 10, PORT_A_SNK_PDO_SUPPLY_TYPE_10),
#endif
#if PORT_A_TOTAL_NUM_SNK_PDOS > 10
            CREATE_SINK_PDO(PORT_A_, 11, PORT_A_SNK_PDO_SUPPLY_TYPE_11),
#endif
        },
};

PORT_VIF_T port2_vif = {
    /* Defines that are used by PDOs */
    .Connector_Type                        = PORT_B_CONNECTOR_TYPE,
    .Port_Managed_Guaranteed_Type          = PORT_B_MANAGED_GUARANTEED_TYPE,
	.ID_Header_Connector_Type              = PORT_B_ID_HEADER_CONNECTOR_TYPE,
    .USB_PD_Support                        = PORT_B_USB_PD_SUPPORT,
    .PD_Port_Type                          = PORT_B_PD_PORT_TYPE,
    .Type_C_State_Machine                  = PORT_B_TYPE_C_STATE_MACHINE, // DRP
    .Port_Battery_Powered                  = PORT_B_PORT_BATTERY_POWERED,
    .BC_1_2_Support                        = PORT_B_BC_1_2_SUPPORT,
    .PD_Specification_Revision             = PORT_B_PD_SPECIFICATION_REVISION,
    .PD_Spec_Revision_Major                = PORT_B_PD_SPEC_REVISION_MAJOR,
    .PD_Spec_Revision_Minor                = PORT_B_PD_SPEC_REVISION_MINOR,
    .PD_Spec_Version_Major                 = PORT_B_PD_SPEC_VERSION_MAJOR,
    .PD_Spec_Version_Minor                 = PORT_B_PD_SPEC_VERSION_MINOR,
    .SOP_Capable                           = PORT_B_SOP_CAPABLE,
    .SOP_P_Capable                         = PORT_B_SOP_P_CAPABLE,
    .SOP_PP_Capable                        = PORT_B_SOP_PP_CAPABLE,
    .SOP_P_Debug_Capable                   = PORT_B_SOP_P_DEBUG_CAPABLE,
    .SOP_PP_Debug_Capable                  = PORT_B_SOP_PP_DEBUG_CAPABLE,
    .Manufacturer_Info_Supported_Port      = PORT_B_MANUFACTURER_INFO_SUPPORTED_PORT,
    .Manufacturer_Info_VID_Port            = PORT_B_MANUFACTURER_INFO_VID_PORT,
    .Manufacturer_Info_PID_Port            = PORT_B_MANUFACTURER_INFO_PID_PORT,
    .Num_Fixed_Batteries                   = PORT_B_NUM_FIXED_BATTERIES,
    .Num_Swappable_Battery_Slots           = PORT_B_NUM_SWAPPABLE_BATTERY_SLOTS,
    .USB_Comms_Capable                     = PORT_B_USB_COMMS_CAPABLE,
	.Is_USB4_Peripheral                    = PORT_X_USB4_IS_PERIPHERAL,
    .DR_Swap_To_DFP_Supported              = PORT_B_DR_SWAP_TO_DFP_SUPPORTED,
    .DR_Swap_To_UFP_Supported              = PORT_B_DR_SWAP_TO_UFP_SUPPORTED,
    .Unconstrained_Power                   = PORT_B_UNCONSTRAINED_POWER,
    .Attempts_Discov_SOP                   = PORT_B_ATTEMPTS_DISCOV_SOP,
    .Chunking_Implemented_SOP              = PORT_B_CHUNKING_IMPLEMENTED_SOP,
    .Unchunked_Extended_Messages_Supported = PORT_B_UNCHUNKED_EXTENDED_MESSAGES_SUPPORTED,
    .Security_Msgs_Supported_SOP           = PORT_B_SECURITY_MSGS_SUPPORTED_SOP,
    .Type_C_Implements_Try_SRC             = PORT_B_TYPE_C_IMPLEMENTS_TRY_SRC,
    .Type_C_Implements_Try_SNK             = PORT_B_TYPE_C_IMPLEMENTS_TRY_SNK,
    .Rp_Value                              = PORT_B_RP_VALUE,
    /* Consumer Only */
    .Type_C_Supports_VCONN_Powered_Accessory = PORT_B_TYPE_C_SUPPORTS_VCONN_POWERED_ACCESSORY,
    .Type_C_Is_VCONN_Powered_Accessory       = PORT_B_TYPE_C_IS_VCONN_POWERED_ACCESSORY,
    .Type_C_Is_Debug_Target_SRC              = PORT_B_TYPE_C_IS_DEBUG_TARGET_SRC,
    .Type_C_Is_Debug_Target_SNK              = PORT_B_TYPE_C_IS_DEBUG_TARGET_SNK,
    .Type_C_Can_Act_As_Host                  = PORT_B_TYPE_C_CAN_ACT_AS_HOST,
    .Type_C_Can_Act_As_Device                = PORT_B_TYPE_C_CAN_ACT_AS_DEVICE,
    .Type_C_Power_Source                     = PORT_B_TYPE_C_POWER_SOURCE,

    .Type_C_Supports_Audio_Accessory = PORT_B_TYPE_C_SUPPORTS_AUDIO_ACCESSORY,

    .Type_C_Sources_VCONN          = PORT_B_TYPE_C_SOURCES_VCONN,
    .Product_Total_Source_Power_mW = PORT_B_PRODUCT_TOTAL_SOURCE_POWER_MW,
    .PD_Power_as_Source            = PORT_B_PD_POWER_AS_SOURCE,
    .USB_Suspend_May_Be_Cleared    = PORT_B_USB_SUSPEND_MAY_BE_CLEARED,
    .Sends_Pings                   = PORT_B_SENDS_PINGS,
    .FR_Swap_Reqd_Type_C_Current_As_Initial_Source =
        PORT_B_FR_SWAP_REQD_TYPE_C_CURRENT_AS_INITIAL_SOURCE,
    .FR_Swap_Type_C_Current_Capability_As_Initial_Sink =
        PORT_B_FR_SWAP_TYPE_C_CURRENT_CAPABILITY_AS_INITIAL_SINK,

    .PD_Power_as_Sink          = PORT_B_PD_POWER_AS_SINK,
    .PD_Min_Power_as_Sink      = PORT_B_PD_MIN_SINK_PDP,
    .No_USB_Suspend_May_Be_Set = PORT_B_NO_USB_SUSPEND_MAY_BE_SET,
    .GiveBack_May_Be_Set       = PORT_B_GIVEBACK_MAY_BE_SET,
    .Higher_Capability_Set     = PORT_B_HIGHER_CAPABILITY_SET,
    .Accepts_PR_Swap_As_Src    = PORT_B_ACCEPTS_PR_SWAP_AS_SRC,
    .Accepts_PR_Swap_As_Snk    = PORT_B_ACCEPTS_PR_SWAP_AS_SNK,
    .Requests_PR_Swap_As_Src   = PORT_B_REQUESTS_PR_SWAP_AS_SRC,
    .Requests_PR_Swap_As_Snk   = PORT_B_REQUESTS_PR_SWAP_AS_SNK,
    .EPR_Supported_As_Src      = PORT_B_EPR_SUPPORTED_AS_SRC,
    .EPR_Supported_As_Snk      = PORT_B_EPR_SUPPORTED_AS_SNK,

    .XID_SOP                        = PORT_B_XID_SOP,
    .Data_Capable_as_USB_Host_SOP   = PORT_B_DATA_CAPABLE_AS_USB_HOST_SOP,
    .Data_Capable_as_USB_Device_SOP = PORT_B_DATA_CAPABLE_AS_USB_DEVICE_SOP,
    .Product_Type_UFP_SOP           = PORT_B_PRODUCT_TYPE_UFP_SOP,
    .Product_Type_DFP_SOP           = PORT_B_PRODUCT_TYPE_DFP_SOP,
    .DFPVDO_USB2_0_Capable          = PORT_B_DFPVDO_USB2_0_CAPABLE,
    .DFPVDO_USB3_2_Capable          = PORT_B_DFPVDO_USB3_2_CAPABLE,
    .DFPVDO_USB4_Capable            = PORT_B_DFPVDO_USB4_CAPABLE,
	.UFPVDO_USB_Highest_Speed       = PORT_B_UFPVDO_USB_HIGHEST_SPEED,
    .Modal_Operation_Supported_SOP  = PORT_B_MODAL_OPERATION_SUPPORTED_SOP,
    .USB_VID_SOP                    = PORT_B_USB_VID_SOP,
    .PID_SOP                        = PORT_B_PID_SOP,
    .bcdDevice_SOP                  = PORT_B_BCDDEVICE_SOP,
    .SVID_fixed_SOP                 = PORT_B_SVID_FIXED_SOP,
    .Num_SVIDs_min_SOP              = PORT_B_NUM_SVIDS_MIN_SOP,
    .Num_SVIDs_max_SOP              = PORT_B_NUM_SVIDS_MAX_SOP,
    .SVID_SOP1                      = PORT_B_SVID_SOP1,
    .SVID_modes_fixed_SOP1          = PORT_B_SVID_MODES_FIXED_SOP1,
    .SVID_num_modes_min_SOP1        = PORT_B_SVID_NUM_MODES_MIN_SOP1,
    .SVID_num_modes_max_SOP1        = PORT_B_SVID_NUM_MODES_MAX_SOP1,
    .SVID_mode_enter_SOP1           = PORT_B_SVID_MODE_ENTER_SOP1,

    .DR_Swap_To_DFP_Supported    = PORT_B_DR_SWAP_TO_DFP_SUPPORTED,
    .DR_Swap_To_UFP_Supported    = PORT_B_DR_SWAP_TO_UFP_SUPPORTED,
    .VCONN_Swap_To_On_Supported  = PORT_B_VCONN_SWAP_TO_ON_SUPPORTED,
    .VCONN_Swap_To_Off_Supported = PORT_B_VCONN_SWAP_TO_OFF_SUPPORTED,

    .Responds_To_Discov_SOP_UFP = PORT_B_RESPONDS_TO_DISCOV_SOP_UFP,
    .Responds_To_Discov_SOP_DFP = PORT_B_RESPONDS_TO_DISCOV_SOP_DFP,

    /* These are custom flags not from vif so changes these as needed */
    .Attempts_DiscvId_SOP_P_First     = PORT_B_ATTEMPTS_DISCOV_SOP,
    .Attempt_Vconn_Swap_to_Off_As_Src = PORT_B_ATTEMPT_VCONN_SWAP_TO_OFF_AS_SRC,
    .Attempt_Vconn_Swap_to_On_As_Sink = PORT_B_ATTEMPT_VCONN_SWAP_TO_ON_AS_SINK,
    .Attempt_DR_Swap_to_Ufp_As_Src    = PORT_B_ATTEMPT_DR_SWAP_TO_UFP_AS_SRC,
    .Attempt_DR_Swap_to_Dfp_As_Sink   = PORT_B_ATTEMPT_DR_SWAP_TO_DFP_AS_SINK,
    .DisplayPort_Auto_Mode_Entry      = YES,
    .DisplayPort_Enabled              = YES,
    .DisplayPort_Preferred_Snk        = NO,
    .DisplayPort_UFP_Capable          = YES,
    .DisplayPort_DFP_Capable          = YES,
    .DisplayPort_Signaling            = YES,
    .DisplayPort_Receptacle           = YES,
    .DisplayPort_USBr2p0Signal_Req    = NO,
    .DisplayPort_DFP_Pin_Mask         = 0x4 | 0x8 | 0x10,
    .DisplayPort_UFP_Pin_Mask         = 0x4 | 0x8 | 0x10,
    .DisplayPort_UFP_PinAssign_Start  = 'C',

    .Num_Src_PDOs = PORT_B_NUM_SRC_PDOS,
    .Num_Snk_PDOs = PORT_B_NUM_SNK_PDOS,
#if CONFIG_ENABLED(EPR)
    .Num_Src_EPR_PDOs = PORT_B_EPR_SRC_PDOS,
    .EPR_Supported_As_Src      = PORT_A_EPR_SUPPORTED_AS_SRC,
	.EPR_Supported_As_Snk      = PORT_A_EPR_SUPPORTED_AS_SNK,
#endif

    /* macro expects index starting at 1 and type */
    .src_caps =
        {
#if PORT_B_TOTAL_NUM_SRC_PDOS > 0
            CREATE_SUPPLY_PDO_FIRST(PORT_B_, 1),
#endif
#if PORT_B_TOTAL_NUM_SRC_PDOS > 1
            CREATE_SUPPLY_PDO(PORT_B_, 2, PORT_B_SRC_PDO_SUPPLY_TYPE_2,0),
#endif
#if PORT_B_TOTAL_NUM_SRC_PDOS > 2
            CREATE_SUPPLY_PDO(PORT_B_, 3, PORT_B_SRC_PDO_SUPPLY_TYPE_3,0),
#endif
#if PORT_B_TOTAL_NUM_SRC_PDOS > 3
            CREATE_SUPPLY_PDO(PORT_B_, 4, PORT_B_SRC_PDO_SUPPLY_TYPE_4,0),
#endif
#if PORT_B_TOTAL_NUM_SRC_PDOS > 4
            CREATE_SUPPLY_PDO(PORT_B_, 5, PORT_B_SRC_PDO_SUPPLY_TYPE_5, 0),
#endif
#if PORT_B_TOTAL_NUM_SRC_PDOS > 5
            CREATE_SUPPLY_PDO(PORT_B_, 6, PORT_B_SRC_PDO_SUPPLY_TYPE_6,0),
#endif
#if PORT_B_TOTAL_NUM_SRC_PDOS > 6
            CREATE_SUPPLY_PDO(PORT_B_, 7, PORT_B_SRC_PDO_SUPPLY_TYPE_7,0),
#endif
#if PORT_B_TOTAL_NUM_SRC_PDOS > 7
            CREATE_SUPPLY_PDO(PORT_B_, 8, PORT_B_SRC_PDO_SUPPLY_TYPE_8,0),
#endif
#if PORT_B_TOTAL_NUM_SRC_PDOS > 8
            CREATE_SUPPLY_PDO(PORT_B_, 9, PORT_B_SRC_PDO_SUPPLY_TYPE_9, 0),
#endif
#if PORT_B_TOTAL_NUM_SRC_PDOS > 9
            CREATE_SUPPLY_PDO(PORT_B_, 10, PORT_B_SRC_PDO_SUPPLY_TYPE_10),
#endif
#if PORT_B_TOTAL_NUM_SRC_PDOS > 10
            CREATE_SUPPLY_PDO(PORT_B_, 11, PORT_B_SRC_PDO_SUPPLY_TYPE_11),
#endif
        },

    /* macro expects index start at 1 and type */
    .snk_caps =
        {
#if PORT_B_NUM_SNK_PDOS == 0
            0,
#endif
#if PORT_B_TOTAL_NUM_SNK_PDOS > 0
            CREATE_SINK_PDO_FIRST(PORT_B_, 1),
#endif
#if PORT_B_TOTAL_NUM_SNK_PDOS > 1
            CREATE_SINK_PDO(PORT_B_, 2, PORT_B_SNK_PDO_SUPPLY_TYPE_2, 0),
#endif
#if PORT_B_TOTAL_NUM_SNK_PDOS > 2
            CREATE_SINK_PDO(PORT_B_, 3, PORT_B_SNK_PDO_SUPPLY_TYPE_3, 0),
#endif
#if PORT_B_TOTAL_NUM_SNK_PDOS > 3
            CREATE_SINK_PDO(PORT_B_, 4, PORT_B_SNK_PDO_SUPPLY_TYPE_4, 0),
#endif
#if PORT_B_TOTAL_NUM_SNK_PDOS > 4
            CREATE_SINK_PDO(PORT_B_, 5, PORT_B_SNK_PDO_SUPPLY_TYPE_5, 0),
#endif
#if PORT_B_TOTAL_NUM_SNK_PDOS > 5
            CREATE_SINK_PDO(PORT_B_, 6, PORT_B_SNK_PDO_SUPPLY_TYPE_6, 0),
#endif
#if PORT_B_TOTAL_NUM_SNK_PDOS > 6
            CREATE_SINK_PDO(PORT_B_, 7, PORT_B_SNK_PDO_SUPPLY_TYPE_7, 0),
#endif

#if PORT_B_TOTAL_NUM_SNK_PDOS > 7
            CREATE_SINK_PDO(PORT_B_, 8, PORT_B_SNK_PDO_SUPPLY_TYPE_8, 1),
#endif
#if PORT_B_TOTAL_NUM_SNK_PDOS > 8
            CREATE_SINK_PDO(PORT_B_, 9, PORT_B_SNK_PDO_SUPPLY_TYPE_9, 1),
#endif
#if PORT_B_TOTAL_NUM_SNK_PDOS > 9
            CREATE_SINK_PDO(PORT_B_, 10, PORT_B_SNK_PDO_SUPPLY_TYPE_10),
#endif
#if PORT_B_TOTAL_NUM_SNK_PDOS > 10
            CREATE_SINK_PDO(PORT_B_, 11, PORT_B_SNK_PDO_SUPPLY_TYPE_11),
#endif
        },
};
