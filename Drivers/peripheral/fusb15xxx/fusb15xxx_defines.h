/*******************************************************************************
 * @file     fusb15xxx_defines.h
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
#ifndef __FUSB15XXX_DEFINES_H__
#define __FUSB15XXX_DEFINES_H__

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        HAL_SUCCESS = 0,
        HAL_BUSY    = 1,
        HAL_ERROR   = -1,
    } HAL_ERROR_T;

#if defined(__GNUC__)
#define __NO_INLINE       __attribute__((noinline))
#define __SECTION_RAMCODE __attribute__((section(".RAMCODE")))
#else
#define __NO_INLINE
#define __SECTION_RAMCODE
#endif

#ifdef HAL_USE_ASSERT
#ifdef static_assert
#define STATIC_ASSERT(COND, MSG) static_assert(COND, #MSG)
#else
#define STATIC_ASSERT(COND, MSG) typedef char static_assertion_##MSG[(COND) ? 1 : -1]
#endif /* static_assert */
#ifndef assert
#ifndef __STATIC_INLINE
#define __STATIC_INLINE static inline
#endif
    __STATIC_INLINE void assert(int cond)
    {
        if (!cond)
        {
            while (1)
            {
            }
        }
    }
#endif /* assert */
#else
#ifndef STATIC_ASSERT
#define STATIC_ASSERT(expr, msg)
#endif
#ifndef assert
#define assert(cond)
#endif
#endif /* HAL_USE_ASSERT */

    /**
 * @return Systick counter value
 */
    unsigned long HAL_SYSTICK_Count();

    /**
 * Blocking delay in milliseconds
 */
    void HAL_Delay(unsigned long);

#ifdef __cplusplus
}
#endif
#endif /* __FUSB15XXX_DEFINES_H__ */
