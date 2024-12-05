/**************************************************************************//**
 * @file     system_ARMCM0plus.c
 * @brief    CMSIS Device System Source File for
 *           ARMCM0plus Device
 * @version  V5.3.1
 * @date     09. July 2018
 ******************************************************************************/
/*
 * Copyright (c) 2009-2018 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "FUSB15xxx.h"

/*----------------------------------------------------------------------------
  Define clocks
 *----------------------------------------------------------------------------*/
#define  XTAL1            (24000000UL)     /* Oscillator frequency */
#define  XTAL2            (120000UL)

#define  SYSTEM_CLOCK1     (XTAL1 / 1U)
#define  SYSTEM_CLOCK2     (XTAL2 / 1U)

/*----------------------------------------------------------------------------
  Externals
 *----------------------------------------------------------------------------*/
#if defined (__VTOR_PRESENT) && (__VTOR_PRESENT == 1U)
  extern uint32_t __Vectors;
#endif

/*----------------------------------------------------------------------------
  System Core Clock Variable
 *----------------------------------------------------------------------------*/
uint32_t SystemCoreClock = SYSTEM_CLOCK1;  /* System Core Clock Frequency */

/*----------------------------------------------------------------------------
  System Core Clock update function
 *----------------------------------------------------------------------------*/
void SystemCoreClockUpdate (void)
{
	SYSTICK_Timer_Registers->SYSCTCK_RLD = SystemCoreClock / 1000 - 1;
}

/*----------------------------------------------------------------------------
  System initialization function
 *----------------------------------------------------------------------------*/
void SystemInit (void)
{
#if defined (__VTOR_PRESENT) && (__VTOR_PRESENT == 1U)
  SCB->VTOR = (uint32_t) &__Vectors;
#endif
#ifdef HAL_SYSTICK_ENABLE
  CMU->PCLKEN |= CMU_PCLKEN_SYSTICK_Enable;
  SYSTICK->RELOAD = SystemCoreClock / 1000 - 1;
  SYSTICK->COUNT = SYSTICK->RELOAD;
  SYSTICK->CONTROL = SYSTICK_CONTROL_EN_Enable |
                     SYSTICK_CONTROL_INTERRUPT_Enable |
                     SYSTICK_CONTROL_CLKSRC_Processor;
#endif
}
