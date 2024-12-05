/**
 * @file     fusb15xxx_i2c.h
 * @author   USB PD Firmware Team
 * @brief    I2C HAL Driver Interface
 * @addtogroup HAL_I2C_Driver
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
 *
 * @addtogroup HAL
 * @{
 */

#ifndef __FUSB15XXX_I2C_H__
#define __FUSB15XXX_I2C_H__

#ifdef __cplusplus
extern "C"
{
#endif
#if (!CONFIG_LEGACY_I2C)
    struct _HAL_I2C;
    typedef struct _HAL_I2C HAL_I2C_T;
    /** Callback after i2c transfer */
    typedef void (*HAL_I2CCB_T)(HAL_ERROR_T, uint32_t);
    typedef void (*HAL_SLAVE_CB_T)(HAL_ERROR_T, bool, uint32_t);
    typedef void (*HAL_INTCB_T)(int);
    /**
 * I2C Driver Mode
 */
    typedef enum
    {
        HAL_I2C_MASTER, /**< HAL_I2C_MASTER */
        HAL_I2C_SLAVE,  /**< HAL_I2C_SLAVE  */
    } HAL_I2CMODE_T;

    /**
 * I2C interface
 */
    typedef enum i2c_interface
    {
        I2C_iface_1 = 1,
        I2C_iface_2 = 2,
        I2C_iface_3 = 3,
        I2C_iface_4 = 4,
    } HAL_I2C_ID_T;

    typedef enum
    {
        I2C_INT_DIR_OUT     = 0 << 0,
        I2C_INT_DIR_IN      = 1 << 0,
        I2C_INT_DISABLE     = 0 << 1,
        I2C_INT_ENABLE      = 1 << 1,
        I2C_INT_FALLING_LOW = 0 << 2,
        I2C_INT_RISING_HIGH = 1 << 2,
        I2C_INT_LEVEL       = 0 << 3,
        I2C_INT_EDGE        = 1 << 3,
    } HAL_I2CINT_T;

    typedef struct
    {
        HAL_I2C_T *(*Initialize)(HAL_I2C_ID_T id);
        void (*Disable)(HAL_I2C_T *);
        struct
        {
            void (*Configure)(HAL_I2C_T *const, uint32_t prescale);
            HAL_ERROR_T (*Read)
            (HAL_I2C_T *const, uint8_t addr, uint8_t *pBuf, uint32_t len, uint32_t timeout);
            void (*ReadRegister)(HAL_I2C_T *const, uint8_t addr, uint8_t reg, uint8_t *pBuf,
                                 uint32_t len, HAL_I2CCB_T);
            void (*ReadWithInterrupt)(HAL_I2C_T *const, uint8_t addr, uint8_t *pBuf, uint32_t len,
                                      HAL_I2CCB_T);
            HAL_ERROR_T (*Write)
            (HAL_I2C_T *const, uint8_t addr, uint8_t *pBuf, uint32_t len, uint32_t timeout);
            void (*WriteWithInterrupt)(HAL_I2C_T *const, uint8_t addr, uint8_t *pBuf, uint32_t len,
                                       HAL_I2CCB_T);
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201) || (DEVICE_TYPE == FUSB15201P)
            void (*Disable)(HAL_I2C_T *const);
#endif
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
            void (*WriteWithDMA)(HAL_I2C_T *const, uint8_t addr, uint8_t *pBuf, uint32_t len,
                                 HAL_I2CCB_T);
            void (*ReadWithDMA)(HAL_I2C_T *const, uint8_t addr, uint8_t *pBuf, uint32_t len,
                                HAL_I2CCB_T);
            void (*ReadWithDMAPatched)(HAL_I2C_T *const, uint8_t addr, uint8_t *pBuf, uint32_t len,
                                       HAL_I2CCB_T);
#endif
        } master;
        struct
        {
            void (*Configure)(HAL_I2C_T *const, uint8_t addr, uint32_t prescale);
            void (*Start)(HAL_I2C_T *const, uint8_t *pBuf, uint32_t size, HAL_SLAVE_CB_T);
            void (*SetOffset)(HAL_I2C_T *const, uint32_t offset);
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201) || (DEVICE_TYPE == FUSB15201P)
            void (*Disable)(HAL_I2C_T *const);
            void (*ResetBuffer)(HAL_I2C_T *const);
#endif
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
            void (*SetupDMARxBuffer)(HAL_I2C_T *const, uint8_t *pBuf, uint32_t size,
                                     HAL_SLAVE_CB_T);
            void (*SetupDMATxBuffer)(HAL_I2C_T *const, uint8_t *pBuf, uint32_t size,
                                     HAL_SLAVE_CB_T);
#endif
        } slave;
        struct
        {
            void (*Enable)(HAL_I2C_T *const, HAL_I2CINT_T);
            void (*Disable)(HAL_I2C_T *const);
            void (*Set)(HAL_I2C_T *const, uint32_t val);
            void (*Clear)(HAL_I2C_T *const);
            uint8_t (*Status)(HAL_I2C_T *const);
            void (*SetCallback)(HAL_I2C_T *const, HAL_INTCB_T);
        } irq;
    } const HAL_I2C_DRIVER_T;
extern HAL_I2C_DRIVER_T I2C_DRIVER;

    void HAL_I2C_IRQHandler(HAL_I2C_T *const idx);
#else
#include "FUSB15xxx.h"

typedef struct HAL_I2C_T_ HAL_I2C_T;

/** Callback after i2c transfer */
typedef void (*HAL_I2CCB_T)(HAL_ERROR_T, uint32_t);
typedef void (*HAL_SLAVE_CB_T)(HAL_ERROR_T, bool, uint32_t);
typedef void (*HAL_INTCB_T)(void);

/**
 * I2C Driver Mode
 */
typedef enum
{
    HAL_I2C_MASTER, /**< HAL_I2C_MASTER */
    HAL_I2C_SLAVE,  /**< HAL_I2C_SLAVE  */
} HAL_I2CMODE_T;

typedef enum
{
    I2C_INT_DIR_OUT     = 0 << 0,
    I2C_INT_DIR_IN      = 1 << 0,
    I2C_INT_DISABLE     = 0 << 1,
    I2C_INT_ENABLE      = 1 << 1,
    I2C_INT_FALLING_LOW = 0 << 2,
    I2C_INT_RISING_HIGH = 1 << 2,
    I2C_INT_LEVEL       = 0 << 3,
    I2C_INT_EDGE        = 1 << 3,
} HAL_I2CINT_T;

typedef struct
{
    HAL_I2C_T *(*Initialze)(I2C_T *const);
    void (*Disable)(HAL_I2C_T *);
    struct
    {
        void (*Configure)(HAL_I2C_T *const, uint32_t prescale);
        HAL_ERROR_T (*Read)
        (HAL_I2C_T *const, uint8_t addr, uint8_t *pBuf, uint32_t len, uint32_t timeout);
        void (*ReadRegister)(HAL_I2C_T *const, uint8_t addr, uint8_t reg, uint8_t *pBuf,
                             uint32_t len, HAL_I2CCB_T);
        void (*ReadWithInterrupt)(HAL_I2C_T *const, uint8_t addr, uint8_t *pBuf, uint32_t len,
                                  HAL_I2CCB_T);
        void (*ReadWithDMA)(HAL_I2C_T *const, uint8_t addr, uint8_t *pBuf, uint32_t len,
                            HAL_I2CCB_T);
        HAL_ERROR_T (*Write)
        (HAL_I2C_T *const, uint8_t addr, uint8_t *pBuf, uint32_t len, uint32_t timeout);
        void (*WriteWithInterrupt)(HAL_I2C_T *const, uint8_t addr, uint8_t *pBuf, uint32_t len,
                                   HAL_I2CCB_T);
        void (*WriteWithDMA)(HAL_I2C_T *const, uint8_t addr, uint8_t *pBuf, uint32_t len,
                             HAL_I2CCB_T);
    } master;
    struct
    {
        void (*Configure)(HAL_I2C_T *const, uint8_t addr, uint32_t prescale);
        void (*Start)(HAL_I2C_T *const, uint8_t *pBuf, uint32_t size, HAL_SLAVE_CB_T);
        void (*SetOffset)(HAL_I2C_T *const, uint32_t offset);
        void (*SetupDMARxBuffer)(HAL_I2C_T *const, uint8_t *pBuf, uint32_t size, HAL_SLAVE_CB_T);
        void (*SetupDMATxBuffer)(HAL_I2C_T *const, uint8_t *pBuf, uint32_t size, HAL_SLAVE_CB_T);
        void (*ResetBuffer)(HAL_I2C_T *const);
    } slave;
    struct
    {
        void (*Enable)(HAL_I2C_T *const, HAL_I2CINT_T);
        void (*Disable)(HAL_I2C_T *const);
        void (*Set)(HAL_I2C_T *const, uint32_t val);
        void (*Clear)(HAL_I2C_T *const);
        uint8_t (*Status)(HAL_I2C_T *const);
        void (*SetCallback)(HAL_I2C_T *const, HAL_INTCB_T);
    } irq;
} const HAL_I2C_DRIVER_T;

extern HAL_I2C_DRIVER_T I2C_DRIVER;

void HAL_I2C_IRQHandler(HAL_I2C_T *const idx);
#endif
#ifdef __cplusplus
}
#endif
#endif /** @} __FUSB15XXX_I2C_H__ */
