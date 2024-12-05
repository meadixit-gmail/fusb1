#define PORT_A_PORT_LABEL A

#define PORT_A_CONNECTOR_TYPE                        2
#define PORT_A_ID_HEADER_CONNECTOR_TYPE              3
#define PORT_A_MANAGED_GUARANTEED_TYPE               1
#define PORT_A_USB_PD_SUPPORT                        1
#define PORT_A_PD_PORT_TYPE                          4
#define PORT_A_TYPE_C_STATE_MACHINE                  1
#define PORT_A_PORT_BATTERY_POWERED                  1 //0
#define PORT_A_BC_1_2_SUPPORT                        0
#define PORT_A_PD_SPECIFICATION_REVISION             2
#define PORT_A_PD_SPEC_REVISION_MAJOR                            3
#define PORT_A_PD_SPEC_REVISION_MINOR                            1
#define PORT_A_PD_SPEC_VERSION_MAJOR                             1
#define PORT_A_PD_SPEC_VERSION_MINOR                             7
#define PORT_A_SOP_CAPABLE                           1
#define PORT_A_SOP_P_CAPABLE                         1
#define PORT_A_SOP_PP_CAPABLE                        1
#define PORT_A_SOP_P_DEBUG_CAPABLE                   0
#define PORT_A_SOP_PP_DEBUG_CAPABLE                  0
#define PORT_A_MANUFACTURER_INFO_SUPPORTED_PORT      1     //0
#define PORT_A_MANUFACTURER_INFO_VID_PORT            4183  //0
#define PORT_A_MANUFACTURER_INFO_PID_PORT            15200 //0
#define PORT_A_USB_COMMS_CAPABLE                     0
#define PORT_A_DR_SWAP_TO_DFP_SUPPORTED              1
#define PORT_A_DR_SWAP_TO_UFP_SUPPORTED              1
#define PORT_A_UNCONSTRAINED_POWER                   1
#define PORT_A_VCONN_SWAP_TO_ON_SUPPORTED            1
#define PORT_A_VCONN_SWAP_TO_OFF_SUPPORTED           1
#define PORT_A_RESPONDS_TO_DISCOV_SOP_UFP            1
#define PORT_A_RESPONDS_TO_DISCOV_SOP_DFP            1
#define PORT_A_ATTEMPTS_DISCOV_SOP                   1
#define PORT_A_CHUNKING_IMPLEMENTED_SOP              1
#define PORT_A_UNCHUNKED_EXTENDED_MESSAGES_SUPPORTED 0
#define PORT_A_SECURITY_MSGS_SUPPORTED_SOP           0
#define PORT_A_NUM_FIXED_BATTERIES                   1 //0
#define PORT_A_NUM_SWAPPABLE_BATTERY_SLOTS           0
#define PORT_A_TYPE_C_CAN_ACT_AS_HOST                0
#define PORT_A_TYPE_C_CAN_ACT_AS_DEVICE              0
#define PORT_A_TYPE_C_IMPLEMENTS_TRY_SRC             0
#define PORT_A_TYPE_C_IMPLEMENTS_TRY_SNK             0
#define PORT_A_TYPE_C_SUPPORTS_AUDIO_ACCESSORY       1
#define PORT_A_TYPE_C_IS_VCONN_POWERED_ACCESSORY     0
#define PORT_A_TYPE_C_IS_DEBUG_TARGET_SRC            1
#define PORT_A_TYPE_C_IS_DEBUG_TARGET_SNK            1
#define PORT_A_CAPTIVE_CABLE                         0
#define PORT_A_RP_VALUE                              2
#define PORT_A_TYPE_C_PORT_ON_HUB                    0
#define PORT_A_TYPE_C_POWER_SOURCE                   0
#define PORT_A_TYPE_C_SOURCES_VCONN                  1
#define PORT_A_TYPE_C_IS_ALT_MODE_CONTROLLER         0
#if (CONFIG_ENABLED(EPR))
#define PORT_A_PD_POWER_AS_SOURCE 140000
#else
#define PORT_A_PD_POWER_AS_SOURCE 60000
#endif
#define PORT_A_USB_SUSPEND_MAY_BE_CLEARED                        1
#define PORT_A_SENDS_PINGS                                       0
#define PORT_A_FR_SWAP_TYPE_C_CURRENT_CAPABILITY_AS_INITIAL_SINK 3
#define PORT_A_MASTER_PORT                                       0
#define PORT_A_PD_OC_PROTECTION                                  0
#define PORT_A_NUM_SRC_PDOS                                      5
#define PORT_A_EPR_SRC_PDOS                                      1
#define PORT_A_SRC_PDO_SUPPLY_TYPE_1                             0
#define PORT_A_SRC_PDO_PEAK_CURRENT_1                            0
#define PORT_A_SRC_PDO_VOLTAGE_1                                 100
#define PORT_A_SRC_PDO_MAX_CURRENT_1                             300
#define PORT_A_SRC_PDO_SUPPLY_TYPE_2                             0
#define PORT_A_SRC_PDO_PEAK_CURRENT_2                            0
#define PORT_A_SRC_PDO_VOLTAGE_2                                 180
#define PORT_A_SRC_PDO_MAX_CURRENT_2                             300
#define PORT_A_SRC_PDO_SUPPLY_TYPE_3                             0
#define PORT_A_SRC_PDO_PEAK_CURRENT_3                            0
#define PORT_A_SRC_PDO_VOLTAGE_3                                 300
#define PORT_A_SRC_PDO_MAX_CURRENT_3                             300
#define PORT_A_SRC_PDO_SUPPLY_TYPE_4                             0
#define PORT_A_SRC_PDO_PEAK_CURRENT_4                            0
#define PORT_A_SRC_PDO_VOLTAGE_4                                 400
#define PORT_A_SRC_PDO_MAX_CURRENT_4                             300
#define PORT_A_SRC_PDO_SUPPLY_TYPE_5                             3
#define PORT_A_SRC_PDO_MAX_CURRENT_5                             60
#define PORT_A_SRC_PDO_MIN_VOLTAGE_5                             33
#define PORT_A_SRC_PDO_MAX_VOLTAGE_5                             210
#define PORT_A_SRC_PDO_APDO_TYPE_5                               0
#define PORT_A_SRC_PDO_IS_EPR_2                                  0
#define PORT_A_SRC_PDO_IS_EPR_3                                  0
#define PORT_A_SRC_PDO_IS_EPR_4                                  0
#define PORT_A_SRC_PDO_IS_EPR_5                                  0
#define PORT_A_SRC_PDO_IS_EPR_6                                  0
#define PORT_A_SRC_PDO_IS_EPR_7                                  0
#define PORT_A_SRC_PDO_IS_EPR_8                                  0
#if (CONFIG_ENABLED(EPR))
#define PORT_A_PD_POWER_AS_SINK 60000 //140000
#else
#define PORT_A_PD_POWER_AS_SINK 500 //60000
#endif
#define PORT_A_NO_USB_SUSPEND_MAY_BE_SET                     1
#define PORT_A_GIVEBACK_MAY_BE_SET                           0
#define PORT_A_HIGHER_CAPABILITY_SET                         1
#define PORT_A_FR_SWAP_REQD_TYPE_C_CURRENT_AS_INITIAL_SOURCE 0

#define PORT_A_NUM_SNK_PDOS          5
#define PORT_A_EPR_SNK_PDOS          2
#define PORT_A_SNK_PDO_SUPPLY_TYPE_1 0
#define PORT_A_SNK_PDO_VOLTAGE_1     100
#define PORT_A_SNK_PDO_OP_CURRENT_1  10
#define PORT_A_SNK_PDO_SUPPLY_TYPE_2 0
#define PORT_A_SNK_PDO_VOLTAGE_2     100
#define PORT_A_SNK_PDO_OP_CURRENT_2  10
#define PORT_A_SNK_PDO_SUPPLY_TYPE_3 0
#define PORT_A_SNK_PDO_VOLTAGE_3     100
#define PORT_A_SNK_PDO_OP_CURRENT_3  10
#define PORT_A_SNK_PDO_SUPPLY_TYPE_4 0
#define PORT_A_SNK_PDO_VOLTAGE_4     100
#define PORT_A_SNK_PDO_OP_CURRENT_4  10
//pps
#define PORT_A_SNK_PDO_SUPPLY_TYPE_5 0
#define PORT_A_SNK_PDO_VOLTAGE_5     100
#define PORT_A_SNK_PDO_OP_CURRENT_5  10
#define PORT_A_SNK_PDO_APDO_TYPE_5   0
//end pps
#define PORT_A_SNK_PDO_SUPPLY_TYPE_6 0
#define PORT_A_SNK_PDO_VOLTAGE_6     0
#define PORT_A_SNK_PDO_OP_CURRENT_6  0
#define PORT_A_SNK_PDO_SUPPLY_TYPE_7 0
#define PORT_A_SNK_PDO_VOLTAGE_7     0
#define PORT_A_SNK_PDO_OP_CURRENT_7  0
//epr
#define PORT_A_SNK_PDO_SUPPLY_TYPE_8 0
#define PORT_A_SNK_PDO_VOLTAGE_8     100
#define PORT_A_SNK_PDO_OP_CURRENT_8  10

#define PORT_A_SNK_PDO_SUPPLY_TYPE_9                             3   // 3:PPS
#define PORT_A_SNK_PDO_PDP_RATING_9                             140  // 3000 mA
#define PORT_A_SNK_PDO_MIN_VOLTAGE_9                             150 // 100mV units
#define PORT_A_SNK_PDO_MAX_VOLTAGE_9                             280 // 100mV units
#define PORT_A_SNK_PDO_APDO_TYPE_9                               1

#define PORT_A_ACCEPTS_PR_SWAP_AS_SRC            1
#define PORT_A_ACCEPTS_PR_SWAP_AS_SNK            1
#define PORT_A_REQUESTS_PR_SWAP_AS_SRC           0
#define PORT_A_REQUESTS_PR_SWAP_AS_SNK           0
#define PORT_A_FR_SWAP_SUPPORTED_AS_INITIAL_SINK 0
#define PORT_A_XID_SOP                           0
#define PORT_A_DATA_CAPABLE_AS_USB_HOST_SOP      0
#define PORT_A_DATA_CAPABLE_AS_USB_DEVICE_SOP    0
#define PORT_A_PRODUCT_TYPE_UFP_SOP              0
#define PORT_A_PRODUCT_TYPE_DFP_SOP              0 //3
#define PORT_A_DFPVDO_USB2_0_CAPABLE             1
#define PORT_A_DFPVDO_USB3_2_CAPABLE             0
#define PORT_A_DFPVDO_USB4_CAPABLE               0
#define PORT_A_UFPVDO_USB_HIGHEST_SPEED          0
#define PORT_A_MODAL_OPERATION_SUPPORTED_SOP     0
#define PORT_A_USB_VID_SOP                       4183
#define PORT_A_PID_SOP                           15200
#define PORT_A_BCDDEVICE_SOP                     0
#define PORT_A_SVID_FIXED_SOP                    1
#define PORT_A_NUM_SVIDS_MIN_SOP                 1
#define PORT_A_NUM_SVIDS_MAX_SOP                 1
#define PORT_A_SVID_SOP_1                        65281
#define PORT_A_SVID_MODES_FIXED_SOP_1            1
#define PORT_A_SVID_NUM_MODES_MIN_SOP_1          1
#define PORT_A_SVID_NUM_MODES_MAX_SOP_1          1
#define PORT_A_SVID_MODE_ENTER_SOP_1             1
#if (CONFIG_ENABLED(EPR))
#define PORT_A_PRODUCT_TOTAL_SOURCE_POWER_MW 140000
#else
#define PORT_A_PRODUCT_TOTAL_SOURCE_POWER_MW 60000
#endif
#define PORT_A_EPR_MODE_CAPABLE                        1
#define PORT_A_PORT_SOURCE_POWER_TYPE                  0
#define PORT_A_TYPE_C_SUPPORTS_VCONN_POWERED_ACCESSORY 0
#define PORT_A_SVID_SOP1                               0
#define PORT_A_SVID_MODES_FIXED_SOP1                   0
#define PORT_A_SVID_NUM_MODES_MIN_SOP1                 0
#define PORT_A_SVID_NUM_MODES_MAX_SOP1                 0
#define PORT_A_SVID_MODE_ENTER_SOP1                    0
#define PORT_A_ATTEMPTS_DISCVID_SOP_P_FIRST            0
#define PORT_A_DISPLAYPORT_AUTO_MODE_ENTRY             0
#define PORT_A_DISPLAYPORT_ENABLED                     0
#define PORT_A_DISPLAYPORT_PREFERRED_SNK               0
#define PORT_A_DISPLAYPORT_UFP_CAPABLE                 0
#define PORT_A_DISPLAYPORT_DFP_CAPABLE                 0
#define PORT_A_DISPLAYPORT_SIGNALING                   0
#define PORT_A_DISPLAYPORT_RECEPTACLE                  0
#define PORT_A_DISPLAYPORT_USBR2P0SIGNAL_REQ           0
#define PORT_A_DISPLAYPORT_DFP_PIN_MASK                0
#define PORT_A_DISPLAYPORT_UFP_PIN_MASK                0
#define PORT_A_DISPLAYPORT_UFP_PINASSIGN_START         0
#define PORT_A_ATTEMPT_DR_SWAP_TO_DFP_AS_SINK          0
#define PORT_A_ATTEMPT_DR_SWAP_TO_UFP_AS_SRC           0
#define PORT_A_ATTEMPT_VCONN_SWAP_TO_OFF_AS_SRC        0
#define PORT_A_ATTEMPT_VCONN_SWAP_TO_ON_AS_SINK        0
#define PORT_A_SOURCES_VBUS_FOR_POWERED_ACCESSORY      0
#define PORT_A_EPR_SUPPORTED_AS_SRC                    0
#define PORT_A_EPR_SUPPORTED_AS_SNK                    1
#define PORT_A_TOTAL_NUM_SRC_PDOS                      5
#define PORT_A_TOTAL_NUM_SNK_PDOS                      9

#define PORT_B_PORT_LABEL B

#define PORT_B_CONNECTOR_TYPE                        2
#define PORT_B_ID_HEADER_CONNECTOR_TYPE              3
#define PORT_B_MANAGED_GUARANTEED_TYPE               1
#define PORT_B_USB_PD_SUPPORT                        1
#define PORT_B_PD_PORT_TYPE                          4
#define PORT_B_TYPE_C_STATE_MACHINE                  1
#define PORT_B_PORT_BATTERY_POWERED                  0
#define PORT_B_BC_1_2_SUPPORT                        0
#define PORT_B_PD_SPECIFICATION_REVISION             2
#define PORT_B_PD_SPEC_REVISION_MAJOR                            3
#define PORT_B_PD_SPEC_REVISION_MINOR                            1
#define PORT_B_PD_SPEC_VERSION_MAJOR                             1
#define PORT_B_PD_SPEC_VERSION_MINOR                             7
#define PORT_B_SOP_CAPABLE                           1
#define PORT_B_SOP_P_CAPABLE                         1
#define PORT_B_SOP_PP_CAPABLE                        1
#define PORT_B_SOP_P_DEBUG_CAPABLE                   0
#define PORT_B_SOP_PP_DEBUG_CAPABLE                  0
#define PORT_B_MANUFACTURER_INFO_SUPPORTED_PORT      0
#define PORT_B_MANUFACTURER_INFO_VID_PORT            0
#define PORT_B_MANUFACTURER_INFO_PID_PORT            0
#define PORT_B_USB_COMMS_CAPABLE                     0
#define PORT_B_DR_SWAP_TO_DFP_SUPPORTED              1
#define PORT_B_DR_SWAP_TO_UFP_SUPPORTED              1
#define PORT_B_UNCONSTRAINED_POWER                   1
#define PORT_B_VCONN_SWAP_TO_ON_SUPPORTED            1
#define PORT_B_VCONN_SWAP_TO_OFF_SUPPORTED           1
#define PORT_B_RESPONDS_TO_DISCOV_SOP_UFP            1
#define PORT_B_RESPONDS_TO_DISCOV_SOP_DFP            1
#define PORT_B_ATTEMPTS_DISCOV_SOP                   1
#define PORT_B_CHUNKING_IMPLEMENTED_SOP              1
#define PORT_B_UNCHUNKED_EXTENDED_MESSAGES_SUPPORTED 0
#define PORT_B_SECURITY_MSGS_SUPPORTED_SOP           0
#define PORT_B_NUM_FIXED_BATTERIES                   0
#define PORT_B_NUM_SWAPPABLE_BATTERY_SLOTS           0
#define PORT_B_TYPE_C_CAN_ACT_AS_HOST                0
#define PORT_B_TYPE_C_CAN_ACT_AS_DEVICE              0
#define PORT_B_TYPE_C_IMPLEMENTS_TRY_SRC             0
#define PORT_B_TYPE_C_IMPLEMENTS_TRY_SNK             0
#define PORT_B_TYPE_C_SUPPORTS_AUDIO_ACCESSORY       1
#define PORT_B_TYPE_C_IS_VCONN_POWERED_ACCESSORY     0
#define PORT_B_TYPE_C_IS_DEBUG_TARGET_SRC            1
#define PORT_B_TYPE_C_IS_DEBUG_TARGET_SNK            1

#define PORT_B_EPR_SRC_PDOS     1
#define PORT_B_EPR_MODE_CAPABLE 1

#define PORT_B_CAPTIVE_CABLE                 0
#define PORT_B_RP_VALUE                      2
#define PORT_B_TYPE_C_PORT_ON_HUB            0
#define PORT_B_TYPE_C_POWER_SOURCE           0
#define PORT_B_TYPE_C_SOURCES_VCONN          1
#define PORT_B_TYPE_C_IS_ALT_MODE_CONTROLLER 0
#if (CONFIG_ENABLED(EPR))
#define PORT_B_PD_POWER_AS_SOURCE 140000
#else
#define PORT_B_PD_POWER_AS_SOURCE 60000
#endif
#define PORT_B_USB_SUSPEND_MAY_BE_CLEARED                        1
#define PORT_B_SENDS_PINGS                                       0
#define PORT_B_FR_SWAP_TYPE_C_CURRENT_CAPABILITY_AS_INITIAL_SINK 3
#define PORT_B_MASTER_PORT                                       0
#define PORT_B_PD_OC_PROTECTION                                  0
#define PORT_B_NUM_SRC_PDOS                                      5
#define PORT_B_SRC_PDO_SUPPLY_TYPE_1                             0
#define PORT_B_SRC_PDO_PEAK_CURRENT_1                            0
#define PORT_B_SRC_PDO_VOLTAGE_1                                 100
#define PORT_B_SRC_PDO_MAX_CURRENT_1                             300
#define PORT_B_SRC_PDO_SUPPLY_TYPE_2                             0
#define PORT_B_SRC_PDO_PEAK_CURRENT_2                            0
#define PORT_B_SRC_PDO_VOLTAGE_2                                 180
#define PORT_B_SRC_PDO_MAX_CURRENT_2                             300
#define PORT_B_SRC_PDO_SUPPLY_TYPE_3                             0
#define PORT_B_SRC_PDO_PEAK_CURRENT_3                            0
#define PORT_B_SRC_PDO_VOLTAGE_3                                 300
#define PORT_B_SRC_PDO_MAX_CURRENT_3                             300
#define PORT_B_SRC_PDO_SUPPLY_TYPE_4                             0
#define PORT_B_SRC_PDO_PEAK_CURRENT_4                            0
#define PORT_B_SRC_PDO_VOLTAGE_4                                 400
#define PORT_B_SRC_PDO_MAX_CURRENT_4                             300
#define PORT_B_SRC_PDO_SUPPLY_TYPE_5                             3
#define PORT_B_SRC_PDO_MAX_CURRENT_5                             60
#define PORT_B_SRC_PDO_MIN_VOLTAGE_5                             33
#define PORT_B_SRC_PDO_MAX_VOLTAGE_5                             210
#define PORT_B_SRC_PDO_APDO_TYPE_5                               0
#define PORT_B_SRC_PDO_IS_EPR_2                                  0
#define PORT_B_SRC_PDO_IS_EPR_3                                  0
#define PORT_B_SRC_PDO_IS_EPR_4                                  0
#define PORT_B_SRC_PDO_IS_EPR_5                                  0
#define PORT_B_SRC_PDO_IS_EPR_6                                  0
#define PORT_B_SRC_PDO_IS_EPR_7                                  0
#define PORT_B_SRC_PDO_IS_EPR_8                                  0
#if (CONFIG_ENABLED(EPR))
#define PORT_B_PD_POWER_AS_SINK 140000
#else
#define PORT_B_PD_POWER_AS_SINK 60000
#endif
#define PORT_B_NO_USB_SUSPEND_MAY_BE_SET                     1
#define PORT_B_GIVEBACK_MAY_BE_SET                           0
#define PORT_B_HIGHER_CAPABILITY_SET                         1
#define PORT_B_FR_SWAP_REQD_TYPE_C_CURRENT_AS_INITIAL_SOURCE 0

#define PORT_B_NUM_SNK_PDOS          5
#define PORT_B_EPR_SNK_PDOS          2
#define PORT_B_SNK_PDO_SUPPLY_TYPE_1 0
#define PORT_B_SNK_PDO_VOLTAGE_1     100
#define PORT_B_SNK_PDO_OP_CURRENT_1  10
#define PORT_B_SNK_PDO_SUPPLY_TYPE_2 0
#define PORT_B_SNK_PDO_VOLTAGE_2     100
#define PORT_B_SNK_PDO_OP_CURRENT_2  10
#define PORT_B_SNK_PDO_SUPPLY_TYPE_3 0
#define PORT_B_SNK_PDO_VOLTAGE_3     100
#define PORT_B_SNK_PDO_OP_CURRENT_3  10
#define PORT_B_SNK_PDO_SUPPLY_TYPE_4 0
#define PORT_B_SNK_PDO_VOLTAGE_4     100
#define PORT_B_SNK_PDO_OP_CURRENT_4  10
//pps
#define PORT_B_SNK_PDO_SUPPLY_TYPE_5 0
#define PORT_B_SNK_PDO_VOLTAGE_5     100
#define PORT_B_SNK_PDO_OP_CURRENT_5  10
#define PORT_B_SRC_PDO_APDO_TYPE_5   0
//end pps
#define PORT_B_SNK_PDO_SUPPLY_TYPE_6 0
#define PORT_B_SNK_PDO_VOLTAGE_6     0
#define PORT_B_SNK_PDO_OP_CURRENT_6  0
#define PORT_B_SNK_PDO_SUPPLY_TYPE_7 0
#define PORT_B_SNK_PDO_VOLTAGE_7     0
#define PORT_B_SNK_PDO_OP_CURRENT_7  0
//epr
#define PORT_B_SNK_PDO_SUPPLY_TYPE_8 0
#define PORT_B_SNK_PDO_VOLTAGE_8     100
#define PORT_B_SNK_PDO_OP_CURRENT_8  10

#define PORT_B_SNK_PDO_SUPPLY_TYPE_9                             3   // 3:PPS
#define PORT_B_SNK_PDO_PDP_RATING_9                             140  // 3000 mA
#define PORT_B_SNK_PDO_MIN_VOLTAGE_9                             150 // 100mV units
#define PORT_B_SNK_PDO_MAX_VOLTAGE_9                             280 // 100mV units
#define PORT_B_SNK_PDO_APDO_TYPE_9                               1
//end epr
#define PORT_B_ACCEPTS_PR_SWAP_AS_SRC            1
#define PORT_B_ACCEPTS_PR_SWAP_AS_SNK            1
#define PORT_B_REQUESTS_PR_SWAP_AS_SRC           0
#define PORT_B_REQUESTS_PR_SWAP_AS_SNK           0
#define PORT_B_FR_SWAP_SUPPORTED_AS_INITIAL_SINK 0
#define PORT_B_XID_SOP                           0
#define PORT_B_DATA_CAPABLE_AS_USB_HOST_SOP      0
#define PORT_B_DATA_CAPABLE_AS_USB_DEVICE_SOP    0
#define PORT_B_PRODUCT_TYPE_UFP_SOP              0
#define PORT_B_PRODUCT_TYPE_DFP_SOP              3
#define PORT_B_DFPVDO_USB2_0_CAPABLE             1
#define PORT_B_DFPVDO_USB3_2_CAPABLE             0
#define PORT_B_DFPVDO_USB4_CAPABLE               0
#define PORT_B_UFPVDO_USB_HIGHEST_SPEED          0
#define PORT_B_MODAL_OPERATION_SUPPORTED_SOP     0
#define PORT_B_USB_VID_SOP                       4183
#define PORT_B_PID_SOP                           15200
#define PORT_B_BCDDEVICE_SOP                     0
#define PORT_B_SVID_FIXED_SOP                    1
#define PORT_B_NUM_SVIDS_MIN_SOP                 1
#define PORT_B_NUM_SVIDS_MAX_SOP                 1
#define PORT_B_SVID_SOP_1                        65281
#define PORT_B_SVID_MODES_FIXED_SOP_1            1
#define PORT_B_SVID_NUM_MODES_MIN_SOP_1          1
#define PORT_B_SVID_NUM_MODES_MAX_SOP_1          1
#define PORT_B_SVID_MODE_ENTER_SOP_1             1
#if (CONFIG_ENABLED(EPR))
#define PORT_B_PRODUCT_TOTAL_SOURCE_POWER_MW 140000
#else
#define PORT_B_PRODUCT_TOTAL_SOURCE_POWER_MW 60000
#endif
#define PORT_B_PORT_SOURCE_POWER_TYPE                  0
#define PORT_B_TYPE_C_SUPPORTS_VCONN_POWERED_ACCESSORY 0
#define PORT_B_SVID_SOP1                               0
#define PORT_B_SVID_MODES_FIXED_SOP1                   0
#define PORT_B_SVID_NUM_MODES_MIN_SOP1                 0
#define PORT_B_SVID_NUM_MODES_MAX_SOP1                 0
#define PORT_B_ATTEMPTS_DISCVID_SOP_P_FIRST            0
#define PORT_B_DISPLAYPORT_AUTO_MODE_ENTRY             0
#define PORT_B_SVID_MODE_ENTER_SOP1                    0
#define PORT_B_DISPLAYPORT_ENABLED                     0
#define PORT_B_DISPLAYPORT_PREFERRED_SNK               0
#define PORT_B_DISPLAYPORT_UFP_CAPABLE                 0
#define PORT_B_DISPLAYPORT_DFP_CAPABLE                 0
#define PORT_B_DISPLAYPORT_SIGNALING                   0
#define PORT_B_DISPLAYPORT_RECEPTACLE                  0
#define PORT_B_DISPLAYPORT_USBR2P0SIGNAL_REQ           0
#define PORT_B_DISPLAYPORT_DFP_PIN_MASK                0
#define PORT_B_DISPLAYPORT_UFP_PIN_MASK                0
#define PORT_B_DISPLAYPORT_UFP_PINASSIGN_START         0
#define PORT_B_ATTEMPT_DR_SWAP_TO_DFP_AS_SINK          0
#define PORT_B_ATTEMPT_DR_SWAP_TO_UFP_AS_SRC           0
#define PORT_B_ATTEMPT_VCONN_SWAP_TO_OFF_AS_SRC        0
#define PORT_B_ATTEMPT_VCONN_SWAP_TO_ON_AS_SINK        0
#define PORT_B_SOURCES_VBUS_FOR_POWERED_ACCESSORY      0
#define PORT_B_EPR_SUPPORTED_AS_SRC                    0
#define PORT_B_EPR_SUPPORTED_AS_SNK                    1
#define PORT_B_TOTAL_NUM_SRC_PDOS                      5
#define PORT_B_TOTAL_NUM_SNK_PDOS                      9
