/*******************************************************************************
 * @file     vif_macros.h
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
#ifndef VIF_MACROS_H
#define VIF_MACROS_H

/**
 * Macros to help define port PDOs
 */
#define YES     1
#define NO      0
#define SWAP(x) ((x) ? 0 : 1)

#define PORT_SUPPLY_FIRST_FIXED(PREFIX, IDX)                                                       \
    {                                                                                              \
        .FPDOSupply = {                                                                            \
            .MaxCurrent        = PREFIX##SRC_PDO_MAX_CURRENT_##IDX,                                \
            .Voltage           = PREFIX##SRC_PDO_VOLTAGE_##IDX,                                    \
            .PeakCurrent       = PREFIX##SRC_PDO_PEAK_CURRENT_##IDX,                               \
            .DataRoleSwap      = PREFIX##DR_SWAP_TO_UFP_SUPPORTED,                                 \
            .USBCommCapable    = PREFIX##USB_COMMS_CAPABLE,                                        \
            .ExternallyPowered = PREFIX##UNCONSTRAINED_POWER,                                      \
            .USBSuspendSupport = SWAP(PREFIX##USB_SUSPEND_MAY_BE_CLEARED),                         \
            .DualRolePower     = PREFIX##ACCEPTS_PR_SWAP_AS_SRC,                                   \
            .SupplyType        = PREFIX##SRC_PDO_SUPPLY_TYPE_##IDX,                                \
        }                                                                                          \
    }

#define PORT_SUPPLY_TYPE_0_0(PREFIX, IDX)                                                            \
    {                                                                                              \
        .FPDOSupply = {                                                                            \
            .MaxCurrent  = PREFIX##SRC_PDO_MAX_CURRENT_##IDX,                                      \
            .Voltage     = PREFIX##SRC_PDO_VOLTAGE_##IDX,                                          \
            .PeakCurrent = PREFIX##SRC_PDO_PEAK_CURRENT_##IDX,                                     \
            .SupplyType  = PREFIX##SRC_PDO_SUPPLY_TYPE_##IDX,                                      \
        }                                                                                          \
    }

#define PORT_SUPPLY_TYPE_0_1(PREFIX, IDX)                                                            \
    {                                                                                              \
        .FPDOSupply = {                                                                            \
            .MaxCurrent  = PREFIX##SRC_PDO_MAX_CURRENT_##IDX,                                      \
            .Voltage     = PREFIX##SRC_PDO_VOLTAGE_##IDX,                                          \
            .PeakCurrent = PREFIX##SRC_PDO_PEAK_CURRENT_##IDX,                                     \
            .SupplyType  = PREFIX##SRC_PDO_SUPPLY_TYPE_##IDX,                                      \
        }                                                                                          \
    }
#define PORT_SUPPLY_TYPE_1_0(PREFIX, IDX)                                                            \
    {                                                                                              \
        .BPDO = {                                                                                  \
            .MaxPower   = PREFIX##SRC_PDO_MAX_POWER_##IDX,                                         \
            .MinVoltage = PREFIX##SRC_PDO_MIN_VOLTAGE_##IDX,                                       \
            .MaxVoltage = PREFIX##SRC_PDO_MAX_VOLTAGE_##IDX,                                       \
            .SupplyType = PREFIX##SRC_PDO_SUPPLY_TYPE_##IDX,                                       \
        }                                                                                          \
    }
#define PORT_SUPPLY_TYPE_1_1(PREFIX, IDX)                                                            \
    {                                                                                              \
        .BPDO = {                                                                                  \
            .MaxPower   = PREFIX##SRC_PDO_MAX_POWER_##IDX,                                         \
            .MinVoltage = PREFIX##SRC_PDO_MIN_VOLTAGE_##IDX,                                       \
            .MaxVoltage = PREFIX##SRC_PDO_MAX_VOLTAGE_##IDX,                                       \
            .SupplyType = PREFIX##SRC_PDO_SUPPLY_TYPE_##IDX,                                       \
        }                                                                                          \
    }

#define PORT_SUPPLY_TYPE_2_0(PREFIX, IDX)                                                            \
    {                                                                                              \
        .VPDO = {                                                                                  \
            .MaxCurrent = PREFIX##SRC_PDO_MAX_CURRENT_##IDX,                                       \
            .MinVoltage = PREFIX##SRC_PDO_MIN_VOLTAGE_##IDX,                                       \
            .MaxVoltage = PREFIX##SRC_PDO_MAX_VOLTAGE_##IDX,                                       \
            .SupplyType = PREFIX##SRC_PDO_SUPPLY_TYPE_##IDX,                                       \
        }                                                                                          \
    }

#define PORT_SUPPLY_TYPE_2_1(PREFIX, IDX)                                                            \
    {                                                                                              \
        .VPDO = {                                                                                  \
            .MaxCurrent = PREFIX##SRC_PDO_MAX_CURRENT_##IDX,                                       \
            .MinVoltage = PREFIX##SRC_PDO_MIN_VOLTAGE_##IDX,                                       \
            .MaxVoltage = PREFIX##SRC_PDO_MAX_VOLTAGE_##IDX,                                       \
            .SupplyType = PREFIX##SRC_PDO_SUPPLY_TYPE_##IDX,                                       \
        }                                                                                          \
    }
#define PORT_SUPPLY_TYPE_3_0(PREFIX, IDX)                                                            \
    {                                                                                              \
        .PPSAPDO =                                                                                 \
            {                                                                                      \
                .MaxCurrent = PREFIX##SRC_PDO_MAX_CURRENT_##IDX,                                   \
                .MinVoltage = PREFIX##SRC_PDO_MIN_VOLTAGE_##IDX,                                   \
                .MaxVoltage = PREFIX##SRC_PDO_MAX_VOLTAGE_##IDX,                                   \
                .SupplyType = PREFIX##SRC_PDO_SUPPLY_TYPE_##IDX,                                   \
                .APDOType   = PREFIX##SRC_PDO_APDO_TYPE_##IDX,                                     \
            }                                                                                      \
}

#define PORT_SUPPLY_TYPE_3_1(PREFIX, IDX)                                                          \
    {                                                                                              \
        .EPRAVSAPDOSupply = {                                                                         \
            .PDP        = PREFIX##SRC_PDO_PDP_RATING_##IDX,                                        \
            .MinVoltage = PREFIX##SRC_PDO_MIN_VOLTAGE_##IDX,                                       \
            .MaxVoltage = PREFIX##SRC_PDO_MAX_VOLTAGE_##IDX,                                       \
            .SupplyType = PREFIX##SRC_PDO_SUPPLY_TYPE_##IDX,                                       \
            .APDOType   = PREFIX##SRC_PDO_APDO_TYPE_##IDX,                                         \
        }																							\
}

#define PORT_SUPPLY_TYPE_3_2(PREFIX, IDX)                                                          \
    {                                                                                              \
        .SPRAVSAPDOSupply = {                                                                         \
            .maxCurrent20 = PREFIX##SRC_PDO_MAX_CURRENT_20V_##IDX,                                  \
            .maxCurrent15 = PREFIX##SRC_PDO_MAX_CURRENT_15V_##IDX,                                  \
			.PeakCurrent = PREFIX##SRC_PDO_PEAK_CURRENT_##IDX                                      \
            .SupplyType = 3                                                                        \
            .APDOType   = 2                                                                        \
        }																							\
}

#define PORT_SUPPLY_TYPE(PREFIX, IDX, TYPE, APDOTYPE)  PORT_SUPPLY_TYPE_##TYPE##_##APDOTYPE(PREFIX, IDX)
#define CREATE_SUPPLY_PDO(PREFIX, IDX, TYPE, APDOTYPE) PORT_SUPPLY_TYPE(PREFIX, IDX, TYPE, APDOTYPE)
#define CREATE_SUPPLY_PDO_FIRST(PREFIX, IDX) PORT_SUPPLY_FIRST_FIXED(PREFIX, IDX)

#define CREATE_SINK_PDO_FIRST(PREFIX, IDX)                                                         \
    {                                                                                              \
        .FPDOSink = {                                                                              \
            .OperationalCurrent = PREFIX##SNK_PDO_OP_CURRENT_##IDX,                                \
            .Voltage            = PREFIX##SNK_PDO_VOLTAGE_##IDX,                                   \
            .DataRoleSwap       = PREFIX##DR_SWAP_TO_DFP_SUPPORTED,                                \
            .USBCommCapable     = PREFIX##USB_COMMS_CAPABLE,                                       \
            .ExternallyPowered  = PREFIX##UNCONSTRAINED_POWER,                                     \
            .HigherCapability   = PREFIX##HIGHER_CAPABILITY_SET,                                   \
            .DualRolePower      = PREFIX##ACCEPTS_PR_SWAP_AS_SNK,                                  \
            .SupplyType         = PREFIX##SNK_PDO_SUPPLY_TYPE_##IDX,                               \
        }                                                                                          \
    }

#define PORT_SINK_TYPE_0_0(PREFIX, IDX)                                                              \
    {                                                                                              \
        .FPDOSink = {                                                                              \
            .OperationalCurrent = PREFIX##SNK_PDO_OP_CURRENT_##IDX,                                \
            .Voltage            = PREFIX##SNK_PDO_VOLTAGE_##IDX,                                   \
            .SupplyType         = PREFIX##SNK_PDO_SUPPLY_TYPE_##IDX,                               \
        }                                                                                          \
    }
#define PORT_SINK_TYPE_0_1(PREFIX, IDX)                                                              \
    {                                                                                              \
        .FPDOSink = {                                                                              \
            .OperationalCurrent = PREFIX##SNK_PDO_OP_CURRENT_##IDX,                                \
            .Voltage            = PREFIX##SNK_PDO_VOLTAGE_##IDX,                                   \
            .SupplyType         = PREFIX##SNK_PDO_SUPPLY_TYPE_##IDX,                               \
        }                                                                                          \
    }

#define PORT_SINK_TYPE_1_0(PREFIX, IDX)                                                              \
    {                                                                                              \
        .BPDO = {                                                                                  \
            .MaxPower   = PREFIX##SNK_PDO_OP_POWER_##IDX,                                          \
            .MinVoltage = PREFIX##SNK_PDO_MIN_VOLTAGE_##IDX,                                       \
            .MaxVoltage = PREFIX##SNK_PDO_MAX_VOLTAGE_##IDX,                                       \
            .SupplyType = PREFIX##SNK_PDO_SUPPLY_TYPE_##IDX,                                       \
        }                                                                                          \
    }
#define PORT_SINK_TYPE_1_1(PREFIX, IDX)                                                              \
    {                                                                                              \
        .BPDO = {                                                                                  \
            .MaxPower   = PREFIX##SNK_PDO_OP_POWER_##IDX,                                          \
            .MinVoltage = PREFIX##SNK_PDO_MIN_VOLTAGE_##IDX,                                       \
            .MaxVoltage = PREFIX##SNK_PDO_MAX_VOLTAGE_##IDX,                                       \
            .SupplyType = PREFIX##SNK_PDO_SUPPLY_TYPE_##IDX,                                       \
        }                                                                                          \
    }

#define PORT_SINK_TYPE_2_0(PREFIX, IDX)                                                              \
    {                                                                                              \
        .VPDO = {                                                                                  \
            .MaxCurrent = PREFIX##SNK_PDO_OP_CURRENT_##IDX,                                        \
            .MinVoltage = PREFIX##SNK_PDO_MIN_VOLTAGE_##IDX,                                       \
            .MaxVoltage = PREFIX##SNK_PDO_MAX_VOLTAGE_##IDX,                                       \
            .SupplyType = PREFIX##SNK_PDO_SUPPLY_TYPE_##IDX,                                       \
        }                                                                                          \
    }
#define PORT_SINK_TYPE_2_1(PREFIX, IDX)                                                              \
    {                                                                                              \
        .VPDO = {                                                                                  \
            .MaxCurrent = PREFIX##SNK_PDO_OP_CURRENT_##IDX,                                        \
            .MinVoltage = PREFIX##SNK_PDO_MIN_VOLTAGE_##IDX,                                       \
            .MaxVoltage = PREFIX##SNK_PDO_MAX_VOLTAGE_##IDX,                                       \
            .SupplyType = PREFIX##SNK_PDO_SUPPLY_TYPE_##IDX,                                       \
        }                                                                                          \
    }

#define PORT_SINK_TYPE_3_0(PREFIX, IDX)                                                              \
    {                                                                                              \
        .PPSAPDO = {                                                                               \
            .MaxCurrent = PREFIX##SNK_PDO_OP_CURRENT_##IDX,                                        \
            .MinVoltage = PREFIX##SNK_PDO_MIN_VOLTAGE_##IDX,                                       \
            .MaxVoltage = PREFIX##SNK_PDO_MAX_VOLTAGE_##IDX,                                       \
            .SupplyType = PREFIX##SNK_PDO_SUPPLY_TYPE_##IDX,                                       \
            .APDOType   = PREFIX##SNK_PDO_APDO_TYPE_##IDX,                                         \
        }                                                                                          \
    }
#define PORT_SINK_TYPE_3_1(PREFIX, IDX)                                                              \
    {                                                                                              \
        .AVSAPDOSink = {                                                                               \
            .PDP		= PREFIX##SNK_PDO_PDP_RATING_##IDX,                                         \
            .MinVoltage = PREFIX##SNK_PDO_MIN_VOLTAGE_##IDX,                                       \
            .MaxVoltage = PREFIX##SNK_PDO_MAX_VOLTAGE_##IDX,                                       \
            .SupplyType = PREFIX##SNK_PDO_SUPPLY_TYPE_##IDX,                                       \
            .APDOType   = PREFIX##SNK_PDO_APDO_TYPE_##IDX,                                         \
        }                                                                                          \
    }
//no such thing as SPRAVSAPDOSink - just don't define


#define PORT_SINK_TYPE(PREFIX, IDX, TYPE, APDOTYPE)  PORT_SINK_TYPE_##TYPE##_##APDOTYPE(PREFIX, IDX)
#define CREATE_SINK_PDO(PREFIX, IDX, TYPE, APDOTYPE) PORT_SINK_TYPE(PREFIX, IDX, TYPE, APDOTYPE)


#endif /* VIF_MACROS_H */
