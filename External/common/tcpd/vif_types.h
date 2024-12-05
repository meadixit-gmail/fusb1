/*******************************************************************************
 * @file     vendor_info.h
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
#ifndef HAL_INCLUDE_VENDOR_INFO_H_
#define HAL_INCLUDE_VENDOR_INFO_H_

#include "pd_types.h"
#include "vif_macros.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct port_vif_t
    {
        PD_MSG_T src_caps[11];
        PD_MSG_T snk_caps[11];
        struct
        {
            unsigned Connector_Type;
            unsigned Port_Managed_Guaranteed_Type;
			unsigned ID_Header_Connector_Type;
            unsigned USB_PD_Support;
            unsigned PD_Port_Type;
            unsigned BC_1_2_Support; //8

            unsigned Type_C_State_Machine;
            unsigned Port_Battery_Powered;
            unsigned PD_Specification_Revision;
            unsigned PD_Spec_Revision_Major;
            unsigned PD_Spec_Revision_Minor;
            unsigned PD_Spec_Version_Major;
            unsigned PD_Spec_Version_Minor;
            unsigned SOP_Capable;
            unsigned SOP_P_Capable; //8

            unsigned SOP_PP_Capable;
            unsigned SOP_P_Debug_Capable;
            unsigned SOP_PP_Debug_Capable;
            unsigned Manufacturer_Info_Supported_Port;
            unsigned Is_USB4_Peripheral;
            unsigned USB_Comms_Capable;
            unsigned Unconstrained_Power;
            unsigned Attempts_Discov_SOP;
            unsigned Chunking_Implemented_SOP; //8

            unsigned Unchunked_Extended_Messages_Supported;
            unsigned Security_Msgs_Supported_SOP;
            unsigned Type_C_Implements_Try_SRC;
            unsigned Type_C_Implements_Try_SNK;
            unsigned Rp_Value;
            unsigned Type_C_Supports_VCONN_Powered_Accessory;
            unsigned Type_C_Is_VCONN_Powered_Accessory; //8-32

            unsigned Manufacturer_Info_VID_Port;
            unsigned Manufacturer_Info_PID_Port; //32

            unsigned Num_Fixed_Batteries;
            unsigned Num_Swappable_Battery_Slots; //32

            unsigned Type_C_Is_Debug_Target_SRC;
            unsigned Type_C_Is_Debug_Target_SNK;
            unsigned Type_C_Can_Act_As_Host;
            unsigned Type_C_Can_Act_As_Device;
            unsigned Type_C_Power_Source;
            unsigned Type_C_Supports_Audio_Accessory; //8

            unsigned PD_Power_as_Source; //24 -32

            unsigned Type_C_Sources_VCONN;
            unsigned USB_Suspend_May_Be_Cleared;
            unsigned Sends_Pings;
            unsigned FR_Swap_Type_C_Current_Capability_As_Initial_Sink;
            unsigned FR_Swap_Reqd_Type_C_Current_As_Initial_Source;
            unsigned No_USB_Suspend_May_Be_Set; //8

            unsigned PD_Power_as_Sink; //24-32
            unsigned PD_Min_Power_as_Sink;

            unsigned GiveBack_May_Be_Set;
            unsigned Higher_Capability_Set;
            unsigned Accepts_PR_Swap_As_Src;
            unsigned Accepts_PR_Swap_As_Snk;
            unsigned Requests_PR_Swap_As_Src;
            unsigned Requests_PR_Swap_As_Snk;
            unsigned Data_Capable_as_USB_Host_SOP;
            unsigned Data_Capable_as_USB_Device_SOP; //8

            unsigned Product_Type_UFP_SOP;
            unsigned Product_Type_DFP_SOP;
            unsigned Modal_Operation_Supported_SOP;
            unsigned SVID_fixed_SOP; //8

            unsigned XID_SOP; //16-32

            unsigned USB_VID_SOP;
            unsigned PID_SOP; //32

            unsigned bcdDevice_SOP;
            unsigned SVID_SOP1; //32

            unsigned Num_SVIDs_min_SOP;
            unsigned Num_SVIDs_max_SOP; //8

            unsigned SVID_modes_fixed_SOP1;
            unsigned SVID_num_modes_min_SOP1;
            unsigned SVID_num_modes_max_SOP1;
            unsigned SVID_mode_enter_SOP1; //8

            unsigned DR_Swap_To_DFP_Supported;
            unsigned DR_Swap_To_UFP_Supported;
            unsigned VCONN_Swap_To_On_Supported;
            unsigned VCONN_Swap_To_Off_Supported;
            unsigned Attempts_DiscvId_SOP_P_First;
            unsigned Responds_To_Discov_SOP_UFP;
            unsigned Responds_To_Discov_SOP_DFP;
            unsigned DisplayPort_Auto_Mode_Entry; //8

            unsigned Num_Src_PDOs;
            unsigned Num_Src_EPR_PDOs;
            unsigned Num_Snk_PDOs;
            unsigned Num_Snk_EPR_PDOs;
            unsigned EPR_Mode_Capable;
            unsigned DisplayPort_Enabled;
            unsigned DisplayPort_Preferred_Snk; //8-32

            unsigned DisplayPort_UFP_Capable;
            unsigned DisplayPort_DFP_Capable;
            unsigned DisplayPort_Signaling;
            unsigned DisplayPort_Receptacle;
            unsigned DisplayPort_USBr2p0Signal_Req; //8

            unsigned DisplayPort_DFP_Pin_Mask;
            unsigned DisplayPort_UFP_Pin_Mask;
            unsigned DisplayPort_UFP_PinAssign_Start; //32

            unsigned Attempt_DR_Swap_to_Dfp_As_Sink;
            unsigned Attempt_DR_Swap_to_Ufp_As_Src;
            unsigned Attempt_Vconn_Swap_to_Off_As_Src;
            unsigned Attempt_Vconn_Swap_to_On_As_Sink;
            unsigned Sources_VBus_For_Powered_Accessory;
            unsigned DFPVDO_USB2_0_Capable;
            unsigned DFPVDO_USB3_2_Capable;
            unsigned DFPVDO_USB4_Capable; //8
            unsigned UFPVDO_USB_Highest_Speed;

            unsigned Product_Total_Source_Power_mW;
            unsigned EPR_Supported_As_Src;
            unsigned EPR_Supported_As_Snk;
        };
    } const PORT_VIF_T;

#ifdef __cplusplus
}
#endif
#endif /* HAL_INCLUDE_VENDOR_INFO_H_ */
