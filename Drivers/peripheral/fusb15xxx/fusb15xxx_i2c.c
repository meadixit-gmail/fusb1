/*******************************************************************************
 * @file     fusb15xxx_i2c.c
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
 ******************************************************************************/
#include "FUSB15xxx.h"

#if HAL_USE_I2C

#define NUM_I2C I2C_PORT_COUNT

#define I2C_INT_MASK                                                                               \
    (I2C_Port_I2C_CFG_repeated_start_int_enable_Msk | I2C_Port_I2C_CFG_tx_int_enable_Msk           \
     | I2C_Port_I2C_CFG_rx_int_enable_Msk | I2C_Port_I2C_CFG_bus_error_int_enable_Msk              \
     | I2C_Port_I2C_CFG_overrun_int_enable_Msk | I2C_Port_I2C_CFG_stop_int_enable_Msk)
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB152011)
#define I2C_DMA_MASK (I2C_Port_I2C_CFG_tx_dma_enable_Msk | I2C_Port_I2C_CFG_rx_dma_enable_Msk)

#define I2C_CONFIG_MASK                                                                            \
    (I2C_INT_MASK | I2C_DMA_MASK | I2C_Port_I2C_CFG_repeated_start_int_enable_Msk)
#else
#define I2C_CONFIG_MASK (I2C_INT_MASK | I2C_Port_I2C_CFG_repeated_start_int_enable_Msk)
#endif
#define I2C_STATUS_CLEAR                                                                           \
    (I2C_Port_I2C_STATUS_stop_detected_clear_Msk | I2C_Port_I2C_STATUS_bus_error_clear_Msk         \
     | I2C_Port_I2C_STATUS_overrun_clear_Msk)

typedef enum
{
    I2C_MASTER,
    I2C_SLAVE,
} I2C_MODE_T;

/**
 * Private read/write data and buffer information
 */
typedef struct
{
    union
    {
        HAL_I2CCB_T    callback; /* Callback */
        HAL_SLAVE_CB_T slavecb;
    };
    struct
    {
        uint8_t *ptr;   /* Pointer to buffer */
        uint32_t size;  /* Max size of buffer */
        uint32_t count; /* Length of valid data in buffer */
    } buf;
#if (!CONFIG_LEGACY_I2C)
    bool useDMAFwPatch; /* Use the firmware patch for I2C DMA read */
#endif
} HAL_I2C_BUF_T;

typedef struct _HAL_I2C
{
    I2C_T       *device; /* I2C block to use */
    HAL_I2C_ID_T id;
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
    HAL_DMACHADDR_T chAddr; /* Channel to use */
#endif
    I2C_MODE_T    mode; /* Master/slave mode */
    HAL_I2C_BUF_T xfer;
    IRQn_Type     irq;
    bool          dmaen; /* DMA Enabled */
    uint8_t       addr;
    bool          atomic; /* Atomic read operation */
    bool          slaverx;
    HAL_INTCB_T   intcb;
} HAL_I2C_T;

static HAL_I2C_T i2cdrivMemPool[NUM_I2C];
#if (!CONFIG_LEGACY_I2C)
static HAL_I2C_T *HAL_I2C_Init(enum i2c_interface i2c_id)
{
    HAL_I2C_T *driv = 0;
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201) || (DEVICE_TYPE == FUSB15201P)
    HAL_PORTCFG_T portCfg = {.an  = PORT_DIGITAL,
                             .alt = PORT_ALTERNATE,
                             .pu  = PORT_PULLUP_DISABLE,
                             .pd  = PORT_PULLDOWN_DISABLE};
#else
    HAL_PORTCFG_T portCfg = {.an  = PORT_DIGITAL,
                             .alt = PORT_ALTERNATE,
                             .pu  = PORT_PULLUP_ENABLE,
                             .pd  = PORT_PULLDOWN_DISABLE};
#endif

#if (DEVICE_TYPE == FUSB15101)
    driv      = &i2cdrivMemPool[0];
    driv->irq = I2C1_IRQn;
    PORT_DRIVER.Configure(PORT1, &portCfg, HAL_GPIO_SHARED); // sda1
    PORT_DRIVER.Configure(PORT2, &portCfg, HAL_GPIO_SHARED); // scl1

    if (driv != 0)
    {
        driv->intcb  = 0;
        driv->device = I2C_Port_1;
        driv->device->I2C_CTRL |= I2C_Port_I2C_CTRL_reset_Msk;
    }
#elif (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201) || (DEVICE_TYPE == FUSB15201P)

    if (i2c_id == I2C_iface_1)
    {
        driv         = &i2cdrivMemPool[0];
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
        driv->chAddr = HAL_DMA_PRIMARY_CH0;
#endif
        driv->irq    = I2C1_IRQn;
        driv->id     = I2C_iface_1;
        driv->device = I2C_Port_1;
        NVIC_SetPriority(I2C1_IRQn, 1);
        NVIC_EnableIRQ(I2C1_IRQn);
#if (DEVICE_TYPE == FUSB15201P)
        PORT_DRIVER.Configure(PORT5, &portCfg, HAL_GPIO_B); // sda1
        PORT_DRIVER.Configure(PORT6, &portCfg, HAL_GPIO_B); // scl1
#else
        PORT_DRIVER.Configure(PORT11, &portCfg, HAL_GPIO_SHARED); // sda1
        PORT_DRIVER.Configure(PORT10, &portCfg, HAL_GPIO_SHARED); // scl1
#endif
    }
    else if (i2c_id == I2C_iface_2)
    {
        driv         = &i2cdrivMemPool[1];
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
        driv->chAddr = HAL_DMA_PRIMARY_CH1;
#endif
        driv->irq    = I2C2_IRQn;
        driv->id     = I2C_iface_2;
        driv->device = I2C_Port_2;
        NVIC_SetPriority(I2C2_IRQn, 1);
        NVIC_EnableIRQ(I2C2_IRQn);
#if (DEVICE_TYPE == FUSB15201P)
        PORT_DRIVER.Configure(PORT3, &portCfg, HAL_GPIO_A); //sda2
        PORT_DRIVER.Configure(PORT4, &portCfg, HAL_GPIO_A); //scl2
#else

        PORT_DRIVER.Configure(PORT3, &portCfg, HAL_GPIO_SHARED); //sda2
        PORT_DRIVER.Configure(PORT4, &portCfg, HAL_GPIO_SHARED); //scl2
#endif
    }

#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
    else if (i2c_id == I2C_iface_3)
    {
        driv         = &i2cdrivMemPool[2];
        driv->chAddr = HAL_DMA_PRIMARY_CH2;
        driv->irq    = I2C3_IRQn;
        driv->id     = I2C_iface_3;
        driv->device = I2C_Port_3;
        NVIC_SetPriority(I2C3_IRQn, 1);
        NVIC_EnableIRQ(I2C3_IRQn);
        PORT_DRIVER.Configure(PORT18, &portCfg, HAL_GPIO_SHARED); //sda3
        PORT_DRIVER.Configure(PORT19, &portCfg, HAL_GPIO_SHARED); //scl3
    }
    else if (i2c_id == I2C_iface_4)
    {
        driv         = &i2cdrivMemPool[3];
        driv->chAddr = HAL_DMA_PRIMARY_CH3;
        driv->irq    = I2C4_IRQn;
        driv->id     = I2C_iface_4;
        driv->device = I2C_Port_4;
        NVIC_SetPriority(I2C4_IRQn, 1);
        NVIC_EnableIRQ(I2C4_IRQn);
        PORT_DRIVER.Configure(PORT5, &portCfg, HAL_GPIO_SHARED); //sda4
        PORT_DRIVER.Configure(PORT6, &portCfg, HAL_GPIO_SHARED); //scl4
    }
#endif
    if (driv != 0)
    {
        driv->intcb = 0;
        driv->device->I2C_CTRL |= I2C_Port_I2C_CTRL_reset_Msk;
    }

#endif
    return driv;
}
#else
static HAL_I2C_T *HAL_I2C_Init(void *i2c_p)
{
    I2C_Port_Type *i2c     = (I2C_Port_Type *)i2c_p;
    HAL_I2C_T     *driv    = 0;
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201) || (DEVICE_TYPE == FUSB15201P)
    HAL_PORTCFG_T  portCfg = {.an  = PORT_DIGITAL,
                              .alt = PORT_ALTERNATE,
                              .pu  = PORT_PULLUP_DISABLE,
                              .pd  = PORT_PULLDOWN_DISABLE};
#else
    HAL_PORTCFG_T portCfg = {.an  = PORT_DIGITAL,
                             .alt = PORT_ALTERNATE,
                             .pu  = PORT_PULLUP_ENABLE,
                             .pd  = PORT_PULLDOWN_DISABLE};
#endif

#if (DEVICE_TYPE == FUSB15101)
    driv                   = &i2cdrivMemPool[0];
    driv->irq              = I2C1_IRQn;
    PORT_DRIVER.Configure(PORT1, &portCfg, HAL_GPIO_SHARED); // sda1
    PORT_DRIVER.Configure(PORT2, &portCfg, HAL_GPIO_SHARED); // scl1

    if (driv != 0)
    {
        driv->intcb  = 0;
        driv->device = I2C_Port_1;
        driv->device->I2C_CTRL |= I2C_Port_I2C_CTRL_reset_Msk;
    }
#elif (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201) || (DEVICE_TYPE == FUSB15201P)

    if (i2c == I2C_Port_1)
    {
        driv         = &i2cdrivMemPool[0];
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
        driv->chAddr = HAL_DMA_PRIMARY_CH0;
#endif
        driv->irq    = I2C1_IRQn;
        driv->id     = I2C_iface_1;
        driv->device = I2C_Port_1;
        NVIC_SetPriority(I2C1_IRQn, 1);
        NVIC_EnableIRQ(I2C1_IRQn);
#if (DEVICE_TYPE == FUSB15201P)
        PORT_DRIVER.Configure(PORT5, &portCfg, HAL_GPIO_B); // sda1
        PORT_DRIVER.Configure(PORT6, &portCfg, HAL_GPIO_B); // scl1
#else
        PORT_DRIVER.Configure(PORT11, &portCfg, HAL_GPIO_SHARED); // sda1
        PORT_DRIVER.Configure(PORT10, &portCfg, HAL_GPIO_SHARED); // scl1
#endif
    }
    else if (i2c == I2C_Port_2)
    {
        driv         = &i2cdrivMemPool[1];
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
        driv->chAddr = HAL_DMA_PRIMARY_CH1;
#endif
        driv->irq    = I2C2_IRQn;
        driv->id     = I2C_iface_2;
        driv->device = I2C_Port_2;
        NVIC_SetPriority(I2C2_IRQn, 1);
        NVIC_EnableIRQ(I2C2_IRQn);
#if (DEVICE_TYPE == FUSB15201P)
        PORT_DRIVER.Configure(PORT3, &portCfg, HAL_GPIO_A); //sda2
        PORT_DRIVER.Configure(PORT4, &portCfg, HAL_GPIO_A); //scl2
#else

        PORT_DRIVER.Configure(PORT3, &portCfg, HAL_GPIO_SHARED); //sda2
        PORT_DRIVER.Configure(PORT4, &portCfg, HAL_GPIO_SHARED); //scl2
#endif
    }

#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
    else if (i2c == I2C_Port_3)
    {
        driv         = &i2cdrivMemPool[2];
        driv->chAddr = HAL_DMA_PRIMARY_CH2;
        driv->irq    = I2C3_IRQn;
        driv->id     = I2C_iface_3;
        driv->device = I2C_Port_3;
        NVIC_SetPriority(I2C3_IRQn, 1);
        NVIC_EnableIRQ(I2C3_IRQn);
        PORT_DRIVER.Configure(PORT18, &portCfg, HAL_GPIO_SHARED); //sda3
        PORT_DRIVER.Configure(PORT19, &portCfg, HAL_GPIO_SHARED); //scl3
    }
    else if (i2c == I2C_Port_4)
    {
        driv         = &i2cdrivMemPool[3];
        driv->chAddr = HAL_DMA_PRIMARY_CH3;
        driv->irq    = I2C4_IRQn;
        driv->id     = I2C_iface_4;
        driv->device = I2C_Port_4;
        NVIC_SetPriority(I2C4_IRQn, 1);
        NVIC_EnableIRQ(I2C4_IRQn);
        PORT_DRIVER.Configure(PORT5, &portCfg, HAL_GPIO_SHARED); //sda4
        PORT_DRIVER.Configure(PORT6, &portCfg, HAL_GPIO_SHARED); //scl4
    }
    if (driv != 0)
    {
        driv->intcb = 0;
        driv->device->I2C_CTRL |= I2C_Port_I2C_CTRL_reset_Msk;
    }
#endif
#endif
    return driv;
}
#endif
static void HAL_I2C_DeInit(HAL_I2C_T *d)
{
    assert(d != 0);
    d->device->I2C_CTRL = I2C_Port_I2C_CTRL_disable_i2c_Msk;
    NVIC_DisableIRQ(d->irq);
}

static void HAL_I2C_SlaveEnable(HAL_I2C_T *const d, uint8_t addr, uint32_t prescale)
{
    assert(d != 0);
    d->mode         = I2C_SLAVE;
    uint32_t config = 0;
    config |=
        ((addr << I2C_Port_I2C_CFG_slave_address_Pos) & I2C_Port_I2C_CFG_slave_address_Msk)
        | I2C_Port_I2C_CFG_slave_Msk
        | ((prescale << I2C_Port_I2C_CFG_slave_prescale_Pos) & I2C_Port_I2C_CFG_slave_prescale_Msk);
    d->device->I2C_CFG = config;
    d->device->I2C_CTRL |= I2C_Port_I2C_CTRL_enable_Msk;
}

static void HAL_I2C_MasterEnable(HAL_I2C_T *d, uint32_t prescale)
{
    assert(d != 0);
    d->mode         = I2C_MASTER;
    uint32_t config = 0;
    config |=
        (prescale << I2C_Port_I2C_CFG_master_prescale_Pos) & I2C_Port_I2C_CFG_master_prescale_Msk;
    d->device->I2C_CFG = config;
    d->device->I2C_CTRL |= I2C_Port_I2C_CTRL_enable_Msk;
}

static void HAL_I2C_SlaveDisable(HAL_I2C_T *const d)
{
    assert(d != 0);
    d->device->I2C_CTRL = I2C_Port_I2C_CTRL_disable_i2c_Msk;
    d->device->I2C_CTRL = I2C_Port_I2C_CTRL_reset_Msk;
}

static void HAL_I2C_MasterDisable(HAL_I2C_T *const d)
{
    assert(d != 0);
    assert(d != 0);
    d->device->I2C_CTRL = I2C_Port_I2C_CTRL_disable_i2c_Msk;
    d->device->I2C_CTRL = I2C_Port_I2C_CTRL_reset_Msk;
}

static inline bool HAL_I2C_TIMEOUT(uint32_t start, uint32_t timeout)
{
    return HAL_SYSTICK_Count() - start < timeout ? false : true;
}

static HAL_ERROR_T HAL_I2C_MasterWrite(HAL_I2C_T *const d, uint8_t addr, uint8_t *pBuf,
                                       uint32_t len, uint32_t timeout)
{
    assert(d != 0 && pBuf != 0);
    uint32_t idx = 0;
    uint32_t status;
    uint32_t tick = HAL_SYSTICK_Count();

    HAL_ERROR_T err = HAL_ERROR;

    if (len < 1)
    {
        return err;
    }
    d->device->I2C_CFG &= ~I2C_CONFIG_MASK;
    d->device->I2C_CFG |= I2C_Port_I2C_CFG_auto_ack_enable_Msk;
    d->device->I2C_STATUS = I2C_STATUS_CLEAR;

    d->device->I2C_ADDR_START = (addr << 1) & I2C_Port_I2C_ADDR_START_address_Msk;

    while ((!HAL_I2C_TIMEOUT(tick, timeout)) && (err != HAL_SUCCESS))
    {
        status = d->device->I2C_STATUS;
        if ((status & I2C_Port_I2C_STATUS_bus_error_Msk)
            || (status & I2C_Port_I2C_STATUS_overrun_Msk)
            || (status & I2C_Port_I2C_STATUS_stop_detected_Msk))
        {
            break;
        }
        else if (status & I2C_Port_I2C_STATUS_tx_req_Msk)
        {
            if (idx < len)
            {
                d->device->I2C_TX_DATA = pBuf[idx++];
            }
            else
            {
                d->device->I2C_CTRL |= I2C_Port_I2C_CTRL_last_data_Msk;
                err = HAL_SUCCESS;
            }
        }
    }
    return err;
}

static HAL_ERROR_T HAL_I2C_MasterRead(HAL_I2C_T *const d, uint8_t addr, uint8_t *pBuf, uint32_t len,
                                      uint32_t timeout)
{
    assert(d != 0 && pBuf != 0);
    uint32_t    idx    = 0;
    uint32_t    status = d->device->I2C_STATUS;
    uint32_t    tick   = HAL_SYSTICK_Count();
    HAL_ERROR_T err    = HAL_ERROR;

    if (len < 1)
    {
        return err;
    }

    d->device->I2C_CFG &= ~I2C_CONFIG_MASK;
    d->device->I2C_CFG |= I2C_Port_I2C_CFG_auto_ack_enable_Msk;
    d->device->I2C_STATUS = I2C_STATUS_CLEAR;

    d->device->I2C_ADDR_START = ((addr << 1) & I2C_Port_I2C_ADDR_START_address_Msk)
                                | I2C_Port_I2C_ADDR_START_read_write_Msk;

    while (!HAL_I2C_TIMEOUT(tick, timeout))
    {
        status = d->device->I2C_STATUS;
        if ((status & I2C_Port_I2C_STATUS_bus_error_Msk)
            || (status & I2C_Port_I2C_STATUS_stop_detected_Msk))
        {
            break;
        }
        else if (status & I2C_Port_I2C_STATUS_rx_req_Msk)
        {
            if (idx < len)
            {
                if (idx + 1 == len)
                {
                    /* prevent peripheral from moving to next data */
                    d->device->I2C_CFG &= ~I2C_Port_I2C_CFG_auto_ack_enable_Msk;
                }
                pBuf[idx++] = (uint8_t)(d->device->I2C_RX_DATA);
            }
            if (idx == len)
            {
                d->device->I2C_CTRL = I2C_Port_I2C_CTRL_last_data_Msk;
                d->device->I2C_CFG |= I2C_Port_I2C_CFG_auto_ack_enable_Msk;
                err = HAL_SUCCESS;
            }
        }
    }
    return err;
}

static void HAL_I2C_MasterWrite_Irq(HAL_I2C_T *const d, uint8_t addr, uint8_t *pBuf, uint32_t len,
                                    HAL_I2CCB_T cb)
{
    assert(d != 0 && pBuf != 0);
    uint32_t config;

    /* Enable interrupts */
    config = d->device->I2C_CFG;
    config &= ~I2C_CONFIG_MASK;
    config |= I2C_Port_I2C_CFG_tx_int_enable_Msk | I2C_Port_I2C_CFG_auto_ack_enable_Msk
              | I2C_Port_I2C_CFG_bus_error_int_enable_Msk | I2C_Port_I2C_CFG_overrun_int_enable_Msk
              | I2C_Port_I2C_CFG_stop_int_enable_Msk;
    d->device->I2C_CFG    = config;
    d->device->I2C_STATUS = I2C_STATUS_CLEAR | I2C_Port_I2C_STATUS_tx_req_set_Msk;
    /* Copy the transfer pointer and size*/
    d->xfer.buf.ptr       = pBuf;
    d->xfer.buf.size      = len;
    d->xfer.buf.count     = 0;
#if (!CONFIG_LEGACY_I2C)
    d->xfer.useDMAFwPatch = false;
#endif
    d->xfer.callback = cb;
    d->addr          = (addr << 1) & I2C_Port_I2C_ADDR_START_address_Msk;

    /* Write address and start transfer */
    d->device->I2C_ADDR_START = d->addr;
}

static void HAL_I2C_MasterRead_Irq(HAL_I2C_T *const d, uint8_t addr, uint8_t *pBuf, uint32_t len,
                                   HAL_I2CCB_T cb)
{
    assert(d != 0 && pBuf != 0);
    uint32_t config;
    config = d->device->I2C_CFG;
    config &= ~I2C_CONFIG_MASK;
    config |= I2C_Port_I2C_CFG_rx_int_enable_Msk | I2C_Port_I2C_CFG_auto_ack_enable_Msk
              | I2C_Port_I2C_CFG_bus_error_int_enable_Msk | I2C_Port_I2C_CFG_overrun_int_enable_Msk
              | I2C_Port_I2C_CFG_stop_int_enable_Msk;
    d->device->I2C_CFG    = config;
    d->device->I2C_STATUS = I2C_STATUS_CLEAR;
    /* Copy the transfer pointer and size*/
    d->xfer.buf.ptr       = pBuf;
    d->xfer.buf.size      = len;
    d->xfer.buf.count     = 0;
#if (!CONFIG_LEGACY_I2C)
    d->xfer.useDMAFwPatch = false;
#endif
    d->xfer.callback = cb;
    d->addr          = (addr << 1) & I2C_Port_I2C_ADDR_START_address_Msk;

    /* Write address and start transfer */
    d->device->I2C_ADDR_START = d->addr | I2C_Port_I2C_ADDR_START_read_write_Msk;
}

static void HAL_I2C_MasterAtomicRead_Irq(HAL_I2C_T *const d, uint8_t addr, uint8_t reg,
                                         uint8_t *pBuf, uint32_t len, HAL_I2CCB_T cb)
{
    assert(d != 0);
    d->atomic = true;
    if (len > 0)
    {
        pBuf[0] = reg;
    }
    HAL_I2C_MasterWrite_Irq(d, addr, pBuf, len, cb);
}

#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
/**
 * @brief DMA Master write private callback handler. Additional processing
 * will be done and if the DMA is complete user callback is called.
 */
static void HAL_I2C_DMA_WrCb(void *arg, HAL_ERROR_T error)
{
    assert(arg != 0);
    HAL_I2C_T *d = (HAL_I2C_T *)arg;
    if (error == HAL_SUCCESS)
    {
        /* All data transferred */
        d->xfer.buf.count = d->xfer.buf.size;
        /* Enable TX interrupt for last data */
        d->device->I2C_CFG |= I2C_Port_I2C_CFG_tx_int_enable_Msk;
    }
    else
    {
        d->device->I2C_CTRL |= I2C_Port_I2C_CTRL_stop_Msk;
        d->xfer.buf.count = d->xfer.buf.size - HAL_DMA_XferCount(d->chAddr);
    }
    HAL_DMA_StopXfer(d->chAddr);
    d->dmaen = false;
}

static void HAL_I2C_DMA_MasterWrite(HAL_I2C_T *const d, uint8_t addr, uint8_t *pBuf, uint32_t len,
                                    HAL_I2CCB_T cb)
{
    assert(d != 0);
    assert(pBuf != 0);
    assert(len > 0);

    uint32_t config;

    d->dmaen = true;
    /* Copy the transmit data from higher level */
    d->xfer.callback  = cb;
    d->xfer.buf.ptr   = pBuf;
    d->xfer.buf.size  = len;
    d->xfer.buf.count = 0;
    d->addr           = (addr << 1) & I2C_Port_I2C_ADDR_START_address_Msk;

    config = d->device->I2C_CFG;
    config &= ~I2C_CONFIG_MASK;
    config |= I2C_Port_I2C_CFG_tx_dma_enable_Msk | I2C_Port_I2C_CFG_bus_error_int_enable_Msk
              | I2C_Port_I2C_CFG_overrun_int_enable_Msk | I2C_Port_I2C_CFG_stop_int_enable_Msk
              | I2C_Port_I2C_CFG_auto_ack_enable_Msk;
    d->device->I2C_CFG = config;

    HAL_DMAXFER_T xfer = HAL_DMA8_XFER_M2P(len, pBuf, (uint8_t *)(&(d->device->I2C_TX_DATA)));

    xfer.cbArg = d;
    xfer.cb    = HAL_I2C_DMA_WrCb;

    HAL_DMA_SetupXfer(d->chAddr, &xfer);
    /* Write the i2c address. This will cause dma request from peripheral */
    d->device->I2C_ADDR_START = d->addr;
    HAL_DMA_StartXfer(d->chAddr);
}

/**
 * @brief DMA Master read private callback handler. Additional processing
 * will be done and if the DMA is complete user callback is called.
 */
static void HAL_I2C_DMA_MRdCb(void *arg, HAL_ERROR_T error)
{
    assert(arg != 0);
    HAL_I2C_T *d = (HAL_I2C_T *)arg;
    if (error == HAL_SUCCESS)
    {
        /* Issue stop bit to end transfer */
        d->device->I2C_CTRL |= I2C_Port_I2C_CTRL_last_data_Msk;
        d->xfer.buf.count = d->xfer.buf.size;
    }
    else
    {
        d->device->I2C_CTRL |= I2C_Port_I2C_CTRL_stop_Msk;
        d->xfer.buf.count = d->xfer.buf.size - HAL_DMA_XferCount(d->chAddr);
    }
    HAL_DMA_StopXfer(d->chAddr);
}

static void HAL_I2C_DMA_MasterRead(HAL_I2C_T *const d, uint8_t addr, uint8_t *pBuf, uint32_t len,
                                   HAL_I2CCB_T cb)
{
    assert(d != 0);
    assert(pBuf != 0);
    assert(len > 0);

    d->dmaen          = true;
    d->xfer.callback  = cb;
    d->xfer.buf.ptr   = pBuf;
    d->xfer.buf.size  = len;
    d->xfer.buf.count = 0;
    d->addr           = (addr << 1) & I2C_Port_I2C_ADDR_START_address_Msk;

    d->device->I2C_CFG &= ~I2C_CONFIG_MASK;
    d->device->I2C_CFG |= I2C_Port_I2C_CFG_rx_dma_enable_Msk | I2C_Port_I2C_CFG_auto_ack_enable_Msk
                          | I2C_Port_I2C_CFG_stop_int_enable_Msk
                          | I2C_Port_I2C_CFG_bus_error_int_enable_Msk
                          | I2C_Port_I2C_CFG_overrun_int_enable_Msk;

    /* Start I2C write transfer for memAddr */
    HAL_DMAXFER_T xfer = HAL_DMA8_XFER_P2M(len, (uint8_t *)(&(d->device->I2C_RX_DATA)), pBuf);
    xfer.cbArg         = d;
    xfer.cb            = HAL_I2C_DMA_MRdCb;

    HAL_DMA_SetupXfer(d->chAddr, &xfer);
    d->device->I2C_ADDR_START = d->addr | I2C_Port_I2C_ADDR_START_read_write_Msk;
    HAL_DMA_StartXfer(d->chAddr);
    if (d->xfer.buf.size == 1)
    {
        d->device->I2C_CTRL |= I2C_Port_I2C_CTRL_last_data_Msk;
    }
}

static void HAL_I2C_DMA_MasterRead_Patched(HAL_I2C_T *const d, uint8_t addr, uint8_t *pBuf,
                                           uint32_t len, HAL_I2CCB_T cb)
{
#if (!CONFIG_LEGACY_I2C)
    d->xfer.useDMAFwPatch = true;
#endif
    HAL_I2C_DMA_MasterRead(d, addr, pBuf, len - 1, cb);
}

static void HAL_I2C_DMA_MasterRead_Unpatched(HAL_I2C_T *const d, uint8_t addr, uint8_t *pBuf,
                                             uint32_t len, HAL_I2CCB_T cb)
{
#if (!CONFIG_LEGACY_I2C)
    d->xfer.useDMAFwPatch = false;
#endif
    HAL_I2C_DMA_MasterRead(d, addr, pBuf, len, cb);
}

#if 0
/**
 * @brief DMA slave read private callback handler. Additional processing
 * will be done and if the DMA is complete user callback is called.
 */
static void HAL_I2C_DMA_SlaveTxCb(void* arg, HAL_ERROR_T error)
{
    HAL_I2C_T* d = (HAL_I2C_T*)arg;
    HAL_DMA_StopXfer(d->chAddr);
    /* Transmission completed */
    d->dev->I2C_CFG |= I2C_Port_I2C_CFG_tx_int_enable_Msk;
    if (error == HAL_SUCCESS) {
        d->xfer.buf.count = d->xfer.buf.size;
    }
    d->dmaen = false;
}
#endif

static void HAL_I2C_DMA_SlaveTx(HAL_I2C_T *const d, uint8_t *pBuf, uint32_t size, HAL_SLAVE_CB_T cb)
{
    assert(d != 0);

    uint32_t config;

    d->dmaen          = true;
    d->xfer.buf.ptr   = pBuf;
    d->xfer.buf.size  = size;
    d->xfer.buf.count = 0;
    d->xfer.slavecb   = cb;

    config = d->device->I2C_CFG;
    config &= ~I2C_CONFIG_MASK;
    config |= I2C_Port_I2C_CFG_tx_dma_enable_Msk | I2C_Port_I2C_CFG_auto_ack_enable_Msk
              | I2C_Port_I2C_CFG_bus_error_int_enable_Msk | I2C_Port_I2C_CFG_overrun_int_enable_Msk
              | I2C_Port_I2C_CFG_stop_int_enable_Msk;
    d->device->I2C_CFG = config;

    HAL_DMAXFER_T xfer = HAL_DMA8_XFER_M2P(size, pBuf, (uint8_t *)(&(d->device->I2C_TX_DATA)));
    xfer.cbArg         = d;
    xfer.cb            = 0;

    d->dmaen   = true;
    d->slaverx = false;
    HAL_DMA_SetupXfer(d->chAddr, &xfer);
    HAL_DMA_StartXfer(d->chAddr);
}

#if 0
/**
 * @brief DMA slave write private callback handler. Additional processing
 * will be done and if the DMA is complete user callback is called.
 */
static void HAL_I2C_DMA_SlaveRxCb(void* arg, HAL_ERROR_T error)
{
    HAL_I2C_T* d = (HAL_I2C_T*)arg;

    HAL_DMA_StopXfer(d->chAddr);
    d->dev->I2C_CTRL = I2C_CONTROL_LASTDATA;
    /* Gets here only if buffer is full */
    if (error == HAL_SUCCESS) {
        d->xfer.buf.count = d->xfer.buf.size;
    }
    d->dmaen = false;
}
#endif

static void HAL_I2C_DMA_SlaveWaitRx(HAL_I2C_T *const d, uint8_t *pBuf, uint32_t size,
                                    HAL_SLAVE_CB_T cb)
{
    assert(d != 0);

    uint32_t config;
    /* Setup buffer for transfer */
    d->xfer.buf.ptr   = pBuf;
    d->xfer.buf.size  = size;
    d->xfer.buf.count = 0;
    d->xfer.slavecb   = cb;

    config = d->device->I2C_CFG;
    config &= ~I2C_CONFIG_MASK;
    config |= I2C_Port_I2C_CFG_rx_dma_enable_Msk | I2C_Port_I2C_CFG_auto_ack_enable_Msk
              | I2C_Port_I2C_CFG_bus_error_int_enable_Msk | I2C_Port_I2C_CFG_overrun_int_enable_Msk
              | I2C_Port_I2C_CFG_stop_int_enable_Msk;
    d->device->I2C_CFG = config;

    HAL_DMAXFER_T xfer = HAL_DMA8_XFER_P2M(size, (uint8_t *)(&(d->device->I2C_RX_DATA)), pBuf);
    xfer.cbArg         = d;
    xfer.cb            = 0;

    d->dmaen   = true;
    d->slaverx = true;
    HAL_DMA_SetupXfer(d->chAddr, &xfer);
    HAL_DMA_StartXfer(d->chAddr);
}
#endif
static void HAL_I2C_IRQ_SlaveStart(HAL_I2C_T *const d, uint8_t *pBuf, uint32_t size,
                                   HAL_SLAVE_CB_T cb)
{
    assert(d != 0 && pBuf != 0);
    uint32_t config;

    /* Setup buffer for transfer */
    d->xfer.buf.ptr   = pBuf;
    d->xfer.buf.size  = size;
    d->xfer.buf.count = 0;
    d->xfer.slavecb   = cb;

    config = d->device->I2C_CFG;
    config &= ~(I2C_CONFIG_MASK);
    config |= I2C_Port_I2C_CFG_tx_int_enable_Msk | I2C_Port_I2C_CFG_rx_int_enable_Msk
              | I2C_Port_I2C_CFG_auto_ack_enable_Msk | I2C_Port_I2C_CFG_stop_int_enable_Msk
              | I2C_Port_I2C_CFG_bus_error_int_enable_Msk
              | I2C_Port_I2C_CFG_repeated_start_int_enable_Msk;

    d->device->I2C_CFG    = config;
    d->device->I2C_STATUS = I2C_STATUS_CLEAR;
}

static void HAL_I2C_IRQ_SlaveSetOffset(HAL_I2C_T *const d, uint32_t offset)
{
    assert(d != 0 && offset < d->xfer.buf.size);
    d->xfer.buf.count = offset;
    /* Prepare data for TX */
    if (d->xfer.buf.count < d->xfer.buf.size)
    {
        /* Ack the unread data or slave address after repeated start. Slave clock stretches
         * master until data is ready from interrupt */
        d->device->I2C_CTRL    = I2C_Port_I2C_CTRL_ack_Msk;
        d->device->I2C_TX_DATA = d->xfer.buf.ptr[d->xfer.buf.count];
        d->device->I2C_CFG |= I2C_Port_I2C_CFG_auto_ack_enable_Msk;
    }
}
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201) || (DEVICE_TYPE == FUSB15201P)
static void HAL_I2C_SlaveBufferCountReset(HAL_I2C_T *const d)
{
    assert(d != 0);
    d->xfer.buf.count = 0;
}
#endif

static void HAL_I2C_Master_IRQHandler(HAL_I2C_T *const d)
{
    assert(d != 0);

    uint32_t    clr       = 0;
    uint32_t    status    = d->device->I2C_STATUS;
    HAL_ERROR_T error     = HAL_SUCCESS;
    bool        endOfXfer = false;

    if (status & I2C_Port_I2C_STATUS_bus_error_Msk)
    {
        clr |= I2C_Port_I2C_STATUS_bus_error_clear_Msk;
        error     = HAL_ERROR;
        endOfXfer = true;
    }

    if (status & I2C_Port_I2C_STATUS_overrun_Msk)
    {
        clr |= I2C_Port_I2C_STATUS_overrun_clear_Msk;
        error     = HAL_ERROR;
        endOfXfer = true;
    }

    if (status & I2C_Port_I2C_STATUS_stop_detected_Msk)
    {
        clr |= I2C_Port_I2C_STATUS_stop_detected_clear_Msk;
        endOfXfer = true;
    }

    if (status & I2C_Port_I2C_STATUS_repeated_start_detected_Msk)
    {
        clr |= I2C_Port_I2C_STATUS_repeated_start_detected_clear_Msk;
    }

    if (status & I2C_Port_I2C_STATUS_busy_Msk)
    {
        if (status & I2C_Port_I2C_STATUS_read_write_Msk)
        {
            if (status & I2C_Port_I2C_STATUS_rx_req_Msk)
            {
                if (d->xfer.buf.count + 1 == d->xfer.buf.size)
                {
                    d->device->I2C_CTRL = I2C_Port_I2C_CTRL_last_data_Msk;
                }
                if (d->xfer.buf.count < d->xfer.buf.size)
                {
                    d->xfer.buf.ptr[d->xfer.buf.count++] = d->device->I2C_RX_DATA;
                }
            }
        }
        else
        {
            if ((status & I2C_Port_I2C_STATUS_ack_Msk) == 0)
            {
                if (d->atomic && (d->xfer.buf.count == 1))
                {
                    HAL_I2C_MasterRead_Irq(d, d->addr >> 1, d->xfer.buf.ptr, d->xfer.buf.size,
                                           d->xfer.callback);
                }
                else if (d->xfer.buf.count < d->xfer.buf.size)
                {
                    d->device->I2C_TX_DATA = d->xfer.buf.ptr[d->xfer.buf.count++];
                }
                else
                {
                    d->device->I2C_CTRL = I2C_Port_I2C_CTRL_last_data_Msk;
                }
            }
        }
    }

    if (endOfXfer)
    {
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
        if (d->dmaen == true)
        {
            /* Read out any data */
            status = d->device->I2C_RX_DATA;

            /* Compensation for the Master I2C DMA Read bug */
#if (!CONFIG_LEGACY_I2C)
            if ((d->device->I2C_CFG & I2C_Port_I2C_CFG_rx_dma_enable_Msk) != 0
                && d->xfer.useDMAFwPatch)
            {
                d->xfer.buf.size++;
                d->xfer.buf.ptr[d->xfer.buf.count++] = d->device->I2C_RX_DATA;
            }
#endif
            d->dmaen = false;
        }
#endif
        if (d->xfer.callback)
        {
            d->xfer.callback(error, d->xfer.buf.count);
        }
        d->atomic = false;
    }

    d->device->I2C_STATUS = clr;
}

static void HAL_I2C_Slave_IrqHandler(HAL_I2C_T *const d)
{
    assert(d != 0);

    uint32_t    status  = d->device->I2C_STATUS;
    uint32_t    clr     = 0;
    uint32_t    endXfer = false;
    HAL_ERROR_T err     = HAL_SUCCESS;

    if (status & (I2C_Port_I2C_STATUS_overrun_Msk | I2C_Port_I2C_STATUS_bus_error_Msk))
    {
        clr |= I2C_Port_I2C_STATUS_overrun_clear_Msk | I2C_Port_I2C_STATUS_bus_error_clear_Msk;
        err     = HAL_ERROR;
        endXfer = true;
    }

    if (status & I2C_Port_I2C_STATUS_repeated_start_detected_Msk)
    {
        /* Repeated start wait for data ready to re-enable auto-ack */
        d->device->I2C_CFG &= ~I2C_Port_I2C_CFG_auto_ack_enable_Msk;
        clr |= I2C_Port_I2C_STATUS_repeated_start_detected_clear_Msk;
        err     = HAL_SUCCESS;
        endXfer = true;
    }

    if (status & I2C_Port_I2C_STATUS_stop_detected_Msk)
    {
        clr |= I2C_Port_I2C_STATUS_stop_detected_clear_Msk;
        err     = HAL_SUCCESS;
        endXfer = true;
    }

    if (endXfer)
    {
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
        if (d->dmaen == true)
        {
            /* Disable DMA if end condition encountered before DMA completed */
            d->xfer.buf.count = d->xfer.buf.size - HAL_DMA_XferCount(d->chAddr);
            HAL_DMA_StopXfer(d->chAddr);
        }
#endif
        if (d->xfer.slavecb)
        {
            d->xfer.slavecb(err, d->slaverx, d->xfer.buf.count);
        }
    }

    if (status & I2C_Port_I2C_STATUS_busy_Msk)
    {
        if (status & I2C_Port_I2C_STATUS_rx_req_Msk)
        {
            d->slaverx = true;
            if (d->xfer.buf.count < d->xfer.buf.size)
            {
                d->xfer.buf.ptr[d->xfer.buf.count++] = (uint8_t)d->device->I2C_RX_DATA;
                if (d->xfer.buf.count == d->xfer.buf.size)
                {
                    /* Disable auto ack until ready to receive/transmit more data */
                    d->device->I2C_CFG &= ~I2C_Port_I2C_CFG_auto_ack_enable_Msk;
                    d->device->I2C_CTRL = I2C_Port_I2C_CTRL_last_data_Msk;
                }
            }
        }
        else if (status & I2C_Port_I2C_STATUS_tx_req_Msk)
        {
            d->slaverx = false;
            /* Data already in I2C1_TX_DATA */
            d->xfer.buf.count++;
            if (d->xfer.buf.count < d->xfer.buf.size)
            {
                d->device->I2C_TX_DATA = d->xfer.buf.ptr[d->xfer.buf.count];
            }
            else
            {
                d->device->I2C_CTRL = I2C_Port_I2C_CTRL_last_data_Msk;
            }
        }
    }

    d->device->I2C_STATUS = clr;
}

void HAL_I2C_IRQHandler(HAL_I2C_T *const d)
{
    assert(d != 0);

    if (d->mode == I2C_MASTER)
    {
        HAL_I2C_Master_IRQHandler(d);
    }
    else
    {
        HAL_I2C_Slave_IrqHandler(d);
    }

    if ((d->device->I2C_INT_CFG & I2C_INT_ENABLE) && d->device->I2C_INT_STS)
    {
        if (d->intcb)
        {
            d->intcb(d->id);
        }
        d->device->I2C_INT_STS = I2C_Port_I2C_INT_STS_int_sts_Msk;
    }
}

static void HAL_I2C_INT_Enable(HAL_I2C_T *const d, HAL_I2CINT_T cfg)
{
    assert(d != 0);
    HAL_PORTCFG_T portCfg = {.an  = PORT_DIGITAL,
                             .alt = PORT_ALTERNATE,
                             .pu  = PORT_PULLUP_DISABLE,
                             .pd  = PORT_PULLDOWN_DISABLE};
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
    if (d->device == I2C_Port_1)
    {
        PORT_DRIVER.Configure(PORT9, &portCfg, HAL_GPIO_SHARED);
        d->device->I2C_INT_CFG = cfg;
    }
    else if (d->device == I2C_Port_2)
    {
        PORT_DRIVER.Configure(PORT2, &portCfg, HAL_GPIO_SHARED);
        d->device->I2C_INT_CFG = cfg;
    }
    else if (d->device == I2C_Port_3)
    {
        PORT_DRIVER.Configure(PORT17, &portCfg, HAL_GPIO_SHARED);
        d->device->I2C_INT_CFG = cfg;
    }
    else if (d->device == I2C_Port_4)
    {
        PORT_DRIVER.Configure(PORT7, &portCfg, HAL_GPIO_SHARED);
        d->device->I2C_INT_CFG = cfg;
    }
#elif (DEVICE_TYPE == FUSB15201P)
    if (d->device == I2C_Port_1)
    {
        PORT_DRIVER.Configure(PORT4, &portCfg, HAL_GPIO_B);
        d->device->I2C_INT_CFG = cfg;
    }
    else
    {
        PORT_DRIVER.Configure(PORT2, &portCfg, HAL_GPIO_A);
        d->device->I2C_INT_CFG = cfg;
    }
#else
    PORT_DRIVER.Configure(PORT0, &portCfg, HAL_GPIO_SHARED);
    d->device->I2C_INT_CFG = cfg;
#endif
}

static void HAL_I2C_INT_Disable(HAL_I2C_T *const d)
{
    assert(d != 0);
    d->device->I2C_INT_CFG &= ~(I2C_INT_ENABLE);
}

static void HAL_I2C_INT_Set(HAL_I2C_T *const d, uint32_t val)
{
    assert(d != 0);
    d->device->I2C_INT_OUT = val & 0x1;
}

static void HAL_I2C_INT_Clear(HAL_I2C_T *const d)
{
    assert(d != 0);
    d->device->I2C_INT_STS = 0x1;
}

uint8_t HAL_I2C_INT_Status(HAL_I2C_T *const d)
{
    assert(d != 0);
    return d->device->I2C_INT_STS & 0x1;
}

static void HAL_I2C_INT_SetCallback(HAL_I2C_T *const d, HAL_INTCB_T cb)
{
    assert(d != 0 && cb != 0);
    d->intcb = cb;
}

HAL_I2C_DRIVER_T I2C_DRIVER = {
    .Initialize = HAL_I2C_Init,
    .Disable    = HAL_I2C_DeInit,
    .master     = {.Configure          = HAL_I2C_MasterEnable,
                   .Write              = HAL_I2C_MasterWrite,
                   .Read               = HAL_I2C_MasterRead,
                   .WriteWithInterrupt = HAL_I2C_MasterWrite_Irq,
                   .ReadWithInterrupt  = HAL_I2C_MasterRead_Irq,
                   .ReadRegister       = HAL_I2C_MasterAtomicRead_Irq,
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201) || (DEVICE_TYPE == FUSB15201P)
               .Disable = HAL_I2C_MasterDisable,
#endif
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
               .WriteWithDMA       = HAL_I2C_DMA_MasterWrite,
               .ReadWithDMA        = HAL_I2C_DMA_MasterRead_Unpatched,
               .ReadWithDMAPatched = HAL_I2C_DMA_MasterRead_Patched
#endif
    },
    .slave = {.Configure = HAL_I2C_SlaveEnable,
              .Start     = HAL_I2C_IRQ_SlaveStart,
              .SetOffset = HAL_I2C_IRQ_SlaveSetOffset,
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201) || (DEVICE_TYPE == FUSB15201P)
              .Disable     = HAL_I2C_SlaveDisable,
              .ResetBuffer = HAL_I2C_SlaveBufferCountReset,
#endif
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
              .SetupDMARxBuffer = HAL_I2C_DMA_SlaveWaitRx,
              .SetupDMATxBuffer = HAL_I2C_DMA_SlaveTx
#endif
    },
    .irq =
        {
            .Enable      = HAL_I2C_INT_Enable,
            .Disable     = HAL_I2C_INT_Disable,
            .Set         = HAL_I2C_INT_Set,
            .Clear       = HAL_I2C_INT_Clear,
            .Status      = HAL_I2C_INT_Status,
            .SetCallback = HAL_I2C_INT_SetCallback,
        },
};

#endif /* HAL_USE_I2C */
