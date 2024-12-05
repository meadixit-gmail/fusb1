/**
 * @file     fusb15xxx_hpd.h
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
 */
#ifndef __FUSB15XXX_HPD_H__
#define __FUSB15XXX_HPD_H__

#ifdef __cplusplus
extern "C"
{
#endif

    /**
 * HPD Source or sink for display port
 */
    typedef enum
    {
        HPD_TRANSMITTER, ///< HPD Transmitter
        HPD_RECEIVER,    ///< HPD Receiver
    } HAL_HPDMODE_T;

    /**
 * Level of HPD signal
 */
    typedef enum
    {
        HPD_LOW,  ///< HPD Signal low
        HPD_HIGH, ///< HPD Signal High
    } HAL_HPDLVL_T;

    /**
 * HPD Event
 */
    typedef enum
    {
        HAL_HPD_EVENT_NONE  = 0x0,      ///< No HPD Event
        HAL_HPD_EVENT_LOW   = 0x1 << 0, ///< HPD Signal Transitioned to Low
        HAL_HPD_EVENT_HIGH  = 0x1 << 1, ///< HPD Signal Transitioned to High
        HAL_HPD_EVENT_IRQRX = 0x1 << 2, ///< HPD IRQ received
        HAL_HPD_EVENT_IRQTX = 0x1 << 3, ///< HPD IRQ sent
    } HAL_HPDEVT_T;

    /**
 * Callback event for HPD Signal transition
 */
    typedef void (*HAL_HPDCB_T)(HAL_HPDEVT_T);
    typedef struct _HAL_HPD_T HAL_HPD_T;

    typedef struct
    {
        HAL_HPD_T *(*Enable)(HPD_T *, HAL_HPDMODE_T, HAL_HPDCB_T);
        void (*Disable)(HAL_HPD_T *);
        void (*SetHpd)(HAL_HPD_T *, HAL_HPDLVL_T);
        HAL_HPDLVL_T (*HpdStatus)(HAL_HPD_T *);
        void (*TransmitIrq)(HAL_HPD_T *);
        void (*EventEnable)(HAL_HPD_T *, HAL_HPDEVT_T, bool);
        HAL_HPDEVT_T (*Event)(HAL_HPD_T *);
    } const HAL_HPD_DRIVER_T;

    extern HAL_HPD_DRIVER_T HPD_DRIVER;

    /**
 * IRQ Handler for HPD Event
 */
    void HAL_HPD_IRQHandler(HAL_HPD_T *d);

#ifdef __cplusplus
}
#endif
#endif /** @} __FUSB15XXX_HPD_H__ */
