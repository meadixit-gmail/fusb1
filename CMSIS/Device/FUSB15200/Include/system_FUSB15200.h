/*******************************************************************************
 * @file     system_FUSB15200.h
 * @author   USB PD Firmware Team
 *
 * Copyright 2018 ON Semiconductor. All rights reserved.
 *
 * This software and/or documentation is licensed by ON Semiconductor under
 * limited terms and conditions. The terms and conditions pertaining to the
 * software and/or documentation are available at
 * http://www.onsemi.com/site/pdf/ONSEMI_T&C.pdf
 * ("ON Semiconductor Standard Terms and Conditions of Sale,
 *   Section 8 Software").
 *
 * DO NOT USE THIS SOFTWARE AND/OR DOCUMENTATION UNLESS YOU HAVE CAREFULLY
 * READ AND YOU AGREE TO THE LIMITED TERMS AND CONDITIONS. BY USING THIS
 * SOFTWARE AND/OR DOCUMENTATION, YOU AGREE TO THE LIMITED TERMS AND CONDITIONS.
 ******************************************************************************/
#ifndef SYSTEM_ARMCM0plus_H
#define SYSTEM_ARMCM0plus_H

#ifdef __cplusplus
extern "C"
{
#endif

    extern uint32_t SystemCoreClock; /*!< System Clock Frequency (Core Clock) */

    /**
  \brief Setup the microcontroller system.

   Initialize the System and update the SystemCoreClock variable.
 */
    extern void SystemInit(void);
    extern void SystickValue(void);

    /**
  \brief  Update SystemCoreClock variable.

   Updates the SystemCoreClock with current core Clock retrieved from cpu registers.
 */
    extern void SystemCoreClockUpdate(void);

#ifdef __cplusplus
}
#endif

#endif /* SYSTEM_ARMCM0plus_H */
