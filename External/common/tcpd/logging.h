/*******************************************************************************
 * @file   loggin.h
 * @author USB Firmware Team

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
 *
 *******************************************************************************/
#ifndef CORE_LOGGING_H_
#define CORE_LOGGING_H_
#ifdef __cplusplus
extern "C"
{
#endif

#if CONFIG_LOG
    /**
 * @brief define dbg_printf for platform
 */
    extern int dbg_printf(const char *, ...);

#define dbg_msg(fmt, ...)   dbg_printf("I:%s:%d:" fmt "\r\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define dbg_warn(fmt, ...)  dbg_printf("W:%s:%d:" fmt "\r\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define dbg_error(fmt, ...) dbg_printf("E:%s:%d:" fmt "\r\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)

#else
#define dbg_msg(msg, ...)
#define dbg_warn(msg, ...)
#define dbg_error(msg, ...)
#endif /* CONFIG_LOG */

#ifdef __cplusplus
}
#endif

#endif /* CORE_LOGGING_H_ */
