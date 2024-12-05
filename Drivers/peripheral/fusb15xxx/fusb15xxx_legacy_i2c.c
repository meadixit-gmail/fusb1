/*******************************************************************************
 * @file     fusb15200_hal_i2c.c
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
#include "fusb15xxx.h"

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
} HAL_I2C_BUF_T;

typedef struct HAL_I2C_T_
{
    I2C_T *dev; /* I2C block to use */
#if (DEVICE_TYPE == FUSB15201)
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

static HAL_I2C_T devMem[NUM_I2C];

static HAL_I2C_T *HAL_I2C_Init(I2C_T *const dev)
{
    assert(dev != 0);
    HAL_I2C_T    *driv    = 0;
    HAL_PORTCFG_T portCfg = {.an  = PORT_DIGITAL,
                             .alt = PORT_ALTERNATE,
                             .pu  = PORT_PULLUP_DISABLE,
                             .pd  = PORT_PULLDOWN_DISABLE};
#if (DEVICE_TYPE == FUSB15201)
    if (dev == I2C_Port_1)
    {
        driv         = &devMem[0];
        driv->chAddr = HAL_DMA_PRIMARY_CH0;
        driv->irq    = I2C1_IRQn;
        PORT_DRIVER.Configure(PORT11, &portCfg, HAL_GPIO_SHARED); // sda1
        PORT_DRIVER.Configure(PORT10, &portCfg, HAL_GPIO_SHARED); // scl1
    }
    else if (dev == I2C_Port_2)
    {
        driv         = &devMem[1];
        driv->chAddr = HAL_DMA_PRIMARY_CH1;
        driv->irq    = I2C2_IRQn;
        PORT_DRIVER.Configure(PORT3, &portCfg, HAL_GPIO_SHARED); //sda2
        PORT_DRIVER.Configure(PORT4, &portCfg, HAL_GPIO_SHARED); //scl2
    }
    else if (dev == I2C_Port_3)
    {
        driv         = &devMem[2];
        driv->chAddr = HAL_DMA_PRIMARY_CH2;
        driv->irq    = I2C3_IRQn;
        PORT_DRIVER.Configure(PORT18, &portCfg, HAL_GPIO_SHARED); //sda3
        PORT_DRIVER.Configure(PORT19, &portCfg, HAL_GPIO_SHARED); //scl3
    }
    else if (dev == I2C_Port_4)
    {
        driv         = &devMem[3];
        driv->chAddr = HAL_DMA_PRIMARY_CH3;
        driv->irq    = I2C4_IRQn;
        PORT_DRIVER.Configure(PORT5, &portCfg, HAL_GPIO_SHARED); //sda4
        PORT_DRIVER.Configure(PORT6, &portCfg, HAL_GPIO_SHARED); //scl4
    }
#elif (DEVICE_TYPE == FUSB15201P)
    if (dev == I2C_Port_1)
    {
        driv = &devMem[0];
        //driv->chAddr = HAL_DMA_PRIMARY_CH0;
        driv->irq = I2C1_IRQn;
        PORT_DRIVER.Configure(PORT5, &portCfg, HAL_GPIO_B); // sda1
        PORT_DRIVER.Configure(PORT6, &portCfg, HAL_GPIO_B); // scl1
    }
    else if (dev == I2C_Port_2)
    {
        driv = &devMem[1];
        //driv->chAddr = HAL_DMA_PRIMARY_CH1;
        driv->irq = I2C2_IRQn;
        PORT_DRIVER.Configure(PORT3, &portCfg, HAL_GPIO_A); //sda2
        PORT_DRIVER.Configure(PORT4, &portCfg, HAL_GPIO_A); //scl2
    }
#endif
    if (driv != 0)
    {
        driv->intcb = 0;
        driv->dev   = dev;
        dev->I2C_CTRL |= I2C_Port_I2C_CTRL_reset_Msk;
    }
    return driv;
}

static void HAL_I2C_DeInit(HAL_I2C_T *d)
{
    assert(d != 0);
    d->dev->I2C_CTRL = I2C_Port_I2C_CTRL_disable_i2c_Msk;
    NVIC_DisableIRQ(d->irq);
}

static void HAL_I2C_SlaveEnable(HAL_I2C_T *const d, uint8_t addr, uint32_t prescale)
{
    assert(d != 0);
    d->mode         = I2C_SLAVE;
    uint32_t config = 0;
    config |= ((addr << 1) & I2C_Port_I2C_CFG_slave_address_Msk) | I2C_Port_I2C_CFG_slave_Msk
              | (prescale & I2C_Port_I2C_CFG_slave_prescale_Msk);
    d->dev->I2C_CFG = config;
    d->dev->I2C_CTRL |= I2C_Port_I2C_CTRL_enable_Msk;
    //NVIC_EnableIRQ(d->irq);
}

static void HAL_I2C_MasterEnable(HAL_I2C_T *const d, uint32_t prescale)
{
    assert(d != 0);
    d->mode         = I2C_MASTER;
    uint32_t config = 0;
    config |= prescale & I2C_Port_I2C_CFG_master_prescale_Msk;
    d->dev->I2C_CFG = config;
    d->dev->I2C_CTRL |= I2C_Port_I2C_CTRL_enable_Msk;
    //NVIC_EnableIRQ(d->irq);
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
    d->dev->I2C_CFG &= ~I2C_CONFIG_MASK;
    d->dev->I2C_CFG |= I2C_Port_I2C_CFG_auto_ack_enable_Msk;
    d->dev->I2C_STATUS =
        I2C_Port_I2C_STATUS_stop_detected_clear_Msk | I2C_Port_I2C_STATUS_bus_error_clear_Msk
        | I2C_Port_I2C_STATUS_overrun_clear_Msk | I2C_Port_I2C_STATUS_tx_req_set_Msk;

    d->dev->I2C_ADDR_START = (addr << 1) & I2C_Port_I2C_ADDR_START_address_Msk;

    while (!HAL_I2C_TIMEOUT(tick, timeout))
    {
        status = d->dev->I2C_STATUS;
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
                d->dev->I2C_TX_DATA = pBuf[idx++];
            }
            else
            {
                d->dev->I2C_CTRL |= I2C_Port_I2C_CTRL_last_data_Msk;
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
    uint32_t    status = d->dev->I2C_STATUS;
    uint32_t    tick   = HAL_SYSTICK_Count();
    HAL_ERROR_T err    = HAL_ERROR;

    if (len < 1)
    {
        return err;
    }

    d->dev->I2C_CFG &= ~I2C_CONFIG_MASK;
    d->dev->I2C_CFG |= I2C_Port_I2C_CFG_auto_ack_enable_Msk;
    d->dev->I2C_STATUS = I2C_Port_I2C_STATUS_stop_detected_clear_Msk
                         | I2C_Port_I2C_STATUS_bus_error_clear_Msk
                         | I2C_Port_I2C_STATUS_overrun_clear_Msk;

    d->dev->I2C_ADDR_START = ((addr << 1) & I2C_Port_I2C_ADDR_START_address_Msk)
                             | I2C_Port_I2C_ADDR_START_read_write_Msk;

    while (!HAL_I2C_TIMEOUT(tick, timeout))
    {
        status = d->dev->I2C_STATUS;
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
                    d->dev->I2C_CFG &= ~I2C_Port_I2C_CFG_auto_ack_enable_Msk;
                }
                pBuf[idx++] = (uint8_t)(d->dev->I2C_RX_DATA);
            }
            if (idx == len)
            {
                d->dev->I2C_CTRL |= I2C_Port_I2C_CTRL_last_data_Msk;
                d->dev->I2C_CFG |= I2C_Port_I2C_CFG_auto_ack_enable_Msk;
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
    config = d->dev->I2C_CFG;
    config &= ~I2C_CONFIG_MASK;
    config |= I2C_Port_I2C_CFG_tx_int_enable_Msk | I2C_Port_I2C_CFG_auto_ack_enable_Msk
              | I2C_Port_I2C_CFG_bus_error_int_enable_Msk | I2C_Port_I2C_CFG_overrun_int_enable_Msk
              | I2C_Port_I2C_CFG_stop_int_enable_Msk;
    d->dev->I2C_CFG = config;
    d->dev->I2C_STATUS =
        I2C_Port_I2C_STATUS_stop_detected_clear_Msk | I2C_Port_I2C_STATUS_bus_error_clear_Msk
        | I2C_Port_I2C_STATUS_overrun_clear_Msk | I2C_Port_I2C_STATUS_tx_req_set_Msk;
    /* Copy the transfer pointer and size*/
    d->dmaen          = false;
    d->xfer.buf.ptr   = pBuf;
    d->xfer.buf.size  = len;
    d->xfer.buf.count = 0;
    d->xfer.callback  = cb;
    d->addr           = (addr << 1) & I2C_Port_I2C_ADDR_START_address_Msk;

    /* Write address and start transfer */
    d->dev->I2C_ADDR_START = d->addr;
}

static void HAL_I2C_MasterRead_Irq(HAL_I2C_T *const d, uint8_t addr, uint8_t *pBuf, uint32_t len,
                                   HAL_I2CCB_T cb)
{
    assert(d != 0 && pBuf != 0);
    uint32_t config;
    config = d->dev->I2C_CFG;
    config &= ~I2C_CONFIG_MASK;
    config |= I2C_Port_I2C_CFG_rx_int_enable_Msk | I2C_Port_I2C_CFG_auto_ack_enable_Msk
              | I2C_Port_I2C_CFG_bus_error_int_enable_Msk | I2C_Port_I2C_CFG_overrun_int_enable_Msk
              | I2C_Port_I2C_CFG_stop_int_enable_Msk;
    d->dev->I2C_CFG    = config;
    d->dev->I2C_STATUS = I2C_Port_I2C_STATUS_stop_detected_clear_Msk
                         | I2C_Port_I2C_STATUS_bus_error_clear_Msk
                         | I2C_Port_I2C_STATUS_overrun_clear_Msk;
    /* Copy the transfer pointer and size*/
    d->xfer.buf.ptr   = pBuf;
    d->dmaen          = false;
    d->xfer.buf.size  = len;
    d->xfer.buf.count = 0;
    d->xfer.callback  = cb;
    d->addr           = (addr << 1) & I2C_Port_I2C_ADDR_START_address_Msk;

    /* Write address and start transfer */
    d->dev->I2C_ADDR_START = d->addr | I2C_Port_I2C_ADDR_START_read_write_Msk;
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

/**
 * @brief DMA Master write private callback handler. Additional processing
 * will be done and if the DMA is complete user callback is called.
 */
static void HAL_I2C_DMA_WrCb(void *arg, HAL_ERROR_T error)
{
#if (DEVICE_TYPE == FUSB15201)
    assert(arg != 0);
    HAL_I2C_T *d = (HAL_I2C_T *)arg;
    if (error == HAL_SUCCESS)
    {
        /* All data transferred */
        d->xfer.buf.count = d->xfer.buf.size;
        /* Enable TX interrupt for last data */
        d->dev->I2C_CFG |= I2C_Port_I2C_CFG_tx_int_enable_Msk;
    }
    else
    {
        d->dev->I2C_CTRL |= I2C_Port_I2C_CTRL_stop_Msk;
        d->xfer.buf.count = d->xfer.buf.size - HAL_DMA_XferCount(d->chAddr);
    }
    HAL_DMA_StopXfer(d->chAddr);
    d->dmaen = false;
#endif
}

static void HAL_I2C_DMA_MasterWrite(HAL_I2C_T *const d, uint8_t addr, uint8_t *pBuf, uint32_t len,
                                    HAL_I2CCB_T cb)
{
#if (DEVICE_TYPE == FUSB15201)
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

    config = d->dev->I2C_CFG;
    config &= ~I2C_CONFIG_MASK;
    config |= I2C_Port_I2C_CFG_tx_dma_enable_Msk | I2C_Port_I2C_CFG_bus_error_int_enable_Msk
              | I2C_Port_I2C_CFG_overrun_int_enable_Msk | I2C_Port_I2C_CFG_auto_ack_enable_Msk
              | I2C_Port_I2C_CFG_stop_int_enable_Msk;
    d->dev->I2C_CFG = config;

    HAL_DMAXFER_T xfer = HAL_DMA8_XFER_M2P(len, pBuf, (uint8_t *)(&(d->dev->I2C_TX_DATA)));

    xfer.cbArg = d;
    xfer.cb    = HAL_I2C_DMA_WrCb;

    HAL_DMA_SetupXfer(d->chAddr, &xfer);
    /* Write the i2c address. This will cause dma request from peripheral */
    d->dev->I2C_ADDR_START = d->addr;
    HAL_DMA_StartXfer(d->chAddr);
#endif
}

/**
 * @brief DMA Master read private callback handler. Additional processing
 * will be done and if the DMA is complete user callback is called.
 */
static void HAL_I2C_DMA_MRdCb(void *arg, HAL_ERROR_T error)
{
#if (DEVICE_TYPE == FUSB15201)
    assert(arg != 0);
    HAL_I2C_T *d = (HAL_I2C_T *)arg;
    if (error == HAL_SUCCESS)
    {
        /* Issue stop bit to end transfer */
        d->dev->I2C_CTRL |= I2C_Port_I2C_CTRL_last_data_Msk;
        d->xfer.buf.count = d->xfer.buf.size;
    }
    else
    {
        d->dev->I2C_CTRL |= I2C_Port_I2C_CTRL_stop_Msk;
        d->xfer.buf.count = d->xfer.buf.size - HAL_DMA_XferCount(d->chAddr);
    }
    HAL_DMA_StopXfer(d->chAddr);
#endif
}

static void HAL_I2C_DMA_MasterRead(HAL_I2C_T *const d, uint8_t addr, uint8_t *pBuf, uint32_t len,
                                   HAL_I2CCB_T cb)
{
#if (DEVICE_TYPE == FUSB15201)
    assert(d != 0);
    assert(pBuf != 0);
    assert(len > 0);

    d->dmaen          = true;
    d->xfer.callback  = cb;
    d->xfer.buf.ptr   = pBuf;
    d->xfer.buf.size  = len;
    d->xfer.buf.count = 0;
    d->addr           = (addr << 1) & I2C_Port_I2C_ADDR_START_address_Msk;

    d->dev->I2C_CFG &= ~I2C_CONFIG_MASK;
    d->dev->I2C_CFG |= I2C_Port_I2C_CFG_rx_dma_enable_Msk | I2C_Port_I2C_CFG_auto_ack_enable_Msk
                       | I2C_Port_I2C_CFG_stop_int_enable_Msk
                       | I2C_Port_I2C_CFG_bus_error_int_enable_Msk
                       | I2C_Port_I2C_CFG_overrun_int_enable_Msk;

    /* Start I2C write transfer for memAddr */
    HAL_DMAXFER_T xfer = HAL_DMA8_XFER_P2M(len, (uint8_t *)(&(d->dev->I2C_RX_DATA)), pBuf);
    xfer.cbArg         = d;
    xfer.cb            = HAL_I2C_DMA_MRdCb;

    HAL_DMA_SetupXfer(d->chAddr, &xfer);
    d->dev->I2C_ADDR_START = d->addr | I2C_Port_I2C_ADDR_START_read_write_Msk;
    HAL_DMA_StartXfer(d->chAddr);
    if (d->xfer.buf.size == 1)
    {
        d->dev->I2C_CTRL |= I2C_Port_I2C_CTRL_last_data_Msk;
    }
#endif
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
    d->dev->I2C_CFG |= I2C_CONFIG_TXINT_Mask;
    if (error == HAL_SUCCESS) {
        d->xfer.buf.count = d->xfer.buf.size;
    }
    d->dmaen = false;
}
#endif

static void HAL_I2C_DMA_SlaveTx(HAL_I2C_T *const d, uint8_t *pBuf, uint32_t size, HAL_SLAVE_CB_T cb)
{
#if (DEVICE_TYPE == FUSB15201)
    assert(d != 0);

    uint32_t config;

    d->dmaen          = true;
    d->xfer.buf.ptr   = pBuf;
    d->xfer.buf.size  = size;
    d->xfer.buf.count = 0;
    d->xfer.slavecb   = cb;

    config = d->dev->I2C_CFG;
    config &= ~I2C_CONFIG_MASK;
    config |= I2C_Port_I2C_CFG_tx_dma_enable_Msk | I2C_Port_I2C_CFG_auto_ack_enable_Msk
              | I2C_Port_I2C_CFG_bus_error_int_enable_Msk | I2C_Port_I2C_CFG_overrun_int_enable_Msk
              | I2C_Port_I2C_CFG_stop_int_enable_Msk;
    d->dev->I2C_CFG = config;

    HAL_DMAXFER_T xfer = HAL_DMA8_XFER_M2P(size, pBuf, (uint8_t *)(&(d->dev->I2C_TX_DATA)));
    xfer.cbArg         = d;
    xfer.cb            = 0;

    d->dmaen   = true;
    d->slaverx = false;
    HAL_DMA_SetupXfer(d->chAddr, &xfer);
    HAL_DMA_StartXfer(d->chAddr);
#endif
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
    d->dev->I2C_CTRL = I2C_Port_I2C_CTRL_last_data_Msk;
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
#if (DEVICE_TYPE == FUSB15201)
    assert(d != 0);

    uint32_t config;
    /* Setup buffer for transfer */
    d->xfer.buf.ptr   = pBuf;
    d->xfer.buf.size  = size;
    d->xfer.buf.count = 0;
    d->xfer.slavecb   = cb;

    config = d->dev->I2C_CFG;
    config &= ~I2C_CONFIG_MASK;
    config |= I2C_Port_I2C_CFG_rx_dma_enable_Msk | I2C_Port_I2C_CFG_auto_ack_enable_Msk
              | I2C_Port_I2C_CFG_bus_error_int_enable_Msk | I2C_Port_I2C_CFG_overrun_int_enable_Msk
              | I2C_Port_I2C_CFG_stop_int_enable_Msk;
    d->dev->I2C_CFG = config;

    HAL_DMAXFER_T xfer = HAL_DMA8_XFER_P2M(size, (uint8_t *)(&(d->dev->I2C_RX_DATA)), pBuf);
    xfer.cbArg         = d;
    xfer.cb            = 0;

    d->dmaen   = true;
    d->slaverx = true;
    HAL_DMA_SetupXfer(d->chAddr, &xfer);
    HAL_DMA_StartXfer(d->chAddr);
#endif
}

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
    d->dmaen          = false;

    config = d->dev->I2C_CFG;
    config &= ~(I2C_CONFIG_MASK);
    config |= I2C_Port_I2C_CFG_tx_int_enable_Msk | I2C_Port_I2C_CFG_rx_int_enable_Msk
              | I2C_Port_I2C_CFG_auto_ack_enable_Msk | I2C_Port_I2C_CFG_stop_int_enable_Msk
              | I2C_Port_I2C_CFG_bus_error_int_enable_Msk
              | I2C_Port_I2C_CFG_repeated_start_int_enable_Msk;

    d->dev->I2C_CFG    = config;
    d->dev->I2C_STATUS = I2C_STATUS_CLEAR;
}

static void HAL_I2C_IRQ_SlaveSetOffset(HAL_I2C_T *const d, uint32_t offset)
{
    assert(d != 0 && offset < d->xfer.buf.size);
    d->xfer.buf.count = offset;
    /* Prepare data for TX */
    if (d->xfer.buf.count < d->xfer.buf.size)
    {
        d->dev->I2C_CTRL    = I2C_Port_I2C_CTRL_ack_Msk;
        d->dev->I2C_TX_DATA = d->xfer.buf.ptr[d->xfer.buf.count];
        d->dev->I2C_CFG |= I2C_Port_I2C_CFG_auto_ack_enable_Msk;
    }
}

static void HAL_I2C_SlaveBufferCountReset(HAL_I2C_T *const d)
{
    assert(d != 0);
    d->xfer.buf.count = 0;
}

static void HAL_I2C_Master_IRQHandler(HAL_I2C_T *const d)
{
    assert(d != 0);

    uint32_t    clr       = 0;
    uint32_t    status    = d->dev->I2C_STATUS;
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
                    d->dev->I2C_CTRL = I2C_Port_I2C_CTRL_last_data_Msk;
                }
                if (d->xfer.buf.count < d->xfer.buf.size)
                {
                    d->xfer.buf.ptr[d->xfer.buf.count++] = d->dev->I2C_RX_DATA;
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
                    d->dev->I2C_TX_DATA = d->xfer.buf.ptr[d->xfer.buf.count++];
                }
                else
                {
                    d->dev->I2C_CTRL = I2C_Port_I2C_CTRL_last_data_Msk;
                }
            }
        }
    }

    if (endOfXfer)
    {
        if (d->dmaen == true)
        {
            /* Read out any data */
            status   = d->dev->I2C_RX_DATA;
            d->dmaen = false;
        }
        if (d->xfer.callback)
        {
            d->xfer.callback(error, d->xfer.buf.count);
        }
        d->atomic = false;
    }

    d->dev->I2C_STATUS = clr;
}

static void HAL_I2C_Slave_IrqHandler(HAL_I2C_T *const d)
{
    assert(d != 0);
    /* TODO: Additional IRQ status handling as needed */
    uint32_t    status  = d->dev->I2C_STATUS;
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
        d->dev->I2C_CFG &= ~I2C_Port_I2C_CFG_auto_ack_enable_Msk;
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
        if (d->dmaen == true)
        {
#if (DEVICE_TYPE == FUSB15201)
            /* Disable DMA if end condition encountered before DMA completed */
            d->xfer.buf.count = d->xfer.buf.size - HAL_DMA_XferCount(d->chAddr);
            HAL_DMA_StopXfer(d->chAddr);
#endif
        }
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
                d->xfer.buf.ptr[d->xfer.buf.count++] = (uint8_t)d->dev->I2C_RX_DATA;
                if (d->xfer.buf.count == d->xfer.buf.size)
                {
                    /* Disable auto ack until ready to receive/transmit more data */
                    d->dev->I2C_CFG &= ~I2C_Port_I2C_CFG_auto_ack_enable_Msk;
                    d->dev->I2C_CTRL |= I2C_Port_I2C_CTRL_last_data_Msk;
                }
            }
        }
        else if (status & I2C_Port_I2C_STATUS_tx_req_Msk)
        {
            d->slaverx = false;
            /* Data already in TXDATA */
            d->xfer.buf.count++;
            if (d->xfer.buf.count < d->xfer.buf.size)
            {
                d->dev->I2C_TX_DATA = d->xfer.buf.ptr[d->xfer.buf.count];
            }
            else
            {
                d->dev->I2C_CTRL |= I2C_Port_I2C_CTRL_last_data_Msk;
            }
        }
    }

    d->dev->I2C_STATUS = clr;
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

    if ((d->dev->I2C_INT_CFG & I2C_INT_ENABLE) && d->dev->I2C_INT_STS)
    {
        if (d->intcb)
        {
            d->intcb();
        }
        d->dev->I2C_INT_STS = 0x1;
    }
}

static void HAL_I2C_INT_Enable(HAL_I2C_T *const d, HAL_I2CINT_T cfg)
{
    assert(d != 0);
    HAL_PORTCFG_T portCfg = {.an  = PORT_DIGITAL,
                             .alt = PORT_ALTERNATE,
                             .pu  = PORT_PULLUP_DISABLE,
                             .pd  = PORT_PULLDOWN_DISABLE};
#if (DEVICE_TYPE == FUSB15201)
    if (d->dev == I2C_Port_1)
    {
        PORT_DRIVER.Configure(PORT9, &portCfg, HAL_GPIO_SHARED);
        d->dev->I2C_INT_CFG = cfg;
    }
    else if (d->dev == I2C_Port_2)
    {
        PORT_DRIVER.Configure(PORT2, &portCfg, HAL_GPIO_SHARED);
        d->dev->I2C_INT_CFG = cfg;
    }
    else if (d->dev == I2C_Port_3)
    {
        PORT_DRIVER.Configure(PORT17, &portCfg, HAL_GPIO_SHARED);
        d->dev->I2C_INT_CFG = cfg;
    }
    else if (d->dev == I2C_Port_4)
    {
        PORT_DRIVER.Configure(PORT7, &portCfg, HAL_GPIO_SHARED);
        d->dev->I2C_INT_CFG = cfg;
    }
#elif (DEVICE_TYPE == FUSB15201P)
    if (d->dev == I2C_Port_1)
    {
        PORT_DRIVER.Configure(PORT4, &portCfg, HAL_GPIO_B);
        d->dev->I2C_INT_CFG = cfg;
    }
    else if (d->dev == I2C_Port_2)
    {
        PORT_DRIVER.Configure(PORT2, &portCfg, HAL_GPIO_A);
        d->dev->I2C_INT_CFG = cfg;
    }
#endif
}

static void HAL_I2C_INT_Disable(HAL_I2C_T *const d)
{
    assert(d != 0);
    d->dev->I2C_INT_CFG &= ~(I2C_INT_ENABLE);
}

static void HAL_I2C_INT_Set(HAL_I2C_T *const d, uint32_t val)
{
    assert(d != 0);
    d->dev->I2C_INT_OUT = val & 0x1;
}

static void HAL_I2C_INT_Clear(HAL_I2C_T *const d)
{
    assert(d != 0);
    d->dev->I2C_INT_STS = 0x1;
}

uint8_t HAL_I2C_INT_Status(HAL_I2C_T *const d)
{
    assert(d != 0);
    return d->dev->I2C_INT_STS & 0x1;
}

static void HAL_I2C_INT_SetCallback(HAL_I2C_T *const d, HAL_INTCB_T cb)
{
    assert(d != 0 && cb != 0);
    d->intcb = cb;
}

HAL_I2C_DRIVER_T I2C_DRIVER = {
    .Initialze = HAL_I2C_Init,
    .Disable   = HAL_I2C_DeInit,
    .master    = {.Configure          = HAL_I2C_MasterEnable,
                  .Write              = HAL_I2C_MasterWrite,
                  .Read               = HAL_I2C_MasterRead,
                  .WriteWithInterrupt = HAL_I2C_MasterWrite_Irq,
                  .ReadWithInterrupt  = HAL_I2C_MasterRead_Irq,
                  .ReadRegister       = HAL_I2C_MasterAtomicRead_Irq,
                  .WriteWithDMA       = HAL_I2C_DMA_MasterWrite,
                  .ReadWithDMA        = HAL_I2C_DMA_MasterRead},
    .slave =
        {
            .Configure        = HAL_I2C_SlaveEnable,
            .Start            = HAL_I2C_IRQ_SlaveStart,
            .SetOffset        = HAL_I2C_IRQ_SlaveSetOffset,
            .SetupDMARxBuffer = HAL_I2C_DMA_SlaveWaitRx,
            .SetupDMATxBuffer = HAL_I2C_DMA_SlaveTx,
            .ResetBuffer      = HAL_I2C_SlaveBufferCountReset,
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
