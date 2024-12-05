/**
 * @file     fusb15xxx_dma.h
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
 * @{
 */
#ifndef __FUSB15XXX_DMA_H__
#define __FUSB15XXX_DMA_H__

#ifdef __cplusplus
extern "C"
{
#endif

/* DMA Memory to Peripheral to memory transfer */
#define HAL_DMA8_XFER_M2M(LEN, SRC, DST)                                                           \
    HAL_DMA_XFER(LEN,                                                                              \
                 HAL_DMACHCTRL_DST_INC_Byte | HAL_DMACHCTRL_DST_SIZE_Byte                          \
                     | HAL_DMACHCTRL_SRC_INC_Byte | HAL_DMACHCTRL_SRC_SIZE_Byte                    \
                     | HAL_DMACHCTRL_RPOW_1 | HAL_DMACHCTRL_CYCCTRL_Basic,                         \
                 SRC, DST, HAL_DMAXFERDIR_M2M)

#define HAL_DMA8_XFER_M2P(LEN, SRC, DST)                                                           \
    HAL_DMA_XFER(LEN,                                                                              \
                 HAL_DMACHCTRL_DST_INC_NoIncrement | HAL_DMACHCTRL_DST_SIZE_Byte                   \
                     | HAL_DMACHCTRL_SRC_INC_Byte | HAL_DMACHCTRL_SRC_SIZE_Byte                    \
                     | HAL_DMACHCTRL_RPOW_1 | HAL_DMACHCTRL_CYCCTRL_Basic,                         \
                 SRC, DST, HAL_DMAXFERDIR_M2P)

#define HAL_DMA8_XFER_P2M(LEN, SRC, DST)                                                           \
    HAL_DMA_XFER(LEN,                                                                              \
                 HAL_DMACHCTRL_DST_INC_Byte | HAL_DMACHCTRL_DST_SIZE_Byte                          \
                     | HAL_DMACHCTRL_SRC_INC_NoIncrement | HAL_DMACHCTRL_SRC_SIZE_Byte             \
                     | HAL_DMACHCTRL_RPOW_1 | HAL_DMACHCTRL_CYCCTRL_Basic,                         \
                 SRC, DST, HAL_DMAXFERDIR_P2M)

#define HAL_DMA16_XFER_M2M(LEN, SRC, DST)                                                          \
    HAL_DMA_XFER(LEN,                                                                              \
                 HAL_DMACHCTRL_DST_INC_HalfWord | HAL_DMACHCTRL_DST_SIZE_HalfWord                  \
                     | HAL_DMACHCTRL_SRC_INC_HalfWord | HAL_DMACHCTRL_SRC_SIZE_HalfWord            \
                     | HAL_DMACHCTRL_RPOW_1 | HAL_DMACHCTRL_CYCCTRL_Basic,                         \
                 SRC, DST, HAL_DMAXFERDIR_M2M)

#define HAL_DMA16_XFER_M2P(LEN, SRC, DST)                                                          \
    HAL_DMA_XFER(LEN,                                                                              \
                 HAL_DMACHCTRL_DST_INC_NoIncrement | HAL_DMACHCTRL_DST_SIZE_HalfWord               \
                     | HAL_DMACHCTRL_SRC_INC_HalfWord | HAL_DMACHCTRL_SRC_SIZE_HalfWord            \
                     | HAL_DMACHCTRL_RPOW_1 | HAL_DMACHCTRL_CYCCTRL_Basic,                         \
                 SRC, DST, HAL_DMAXFERDIR_M2P)

#define HAL_DMA16_XFER_P2M(LEN, SRC, DST)                                                          \
    HAL_DMA_XFER(LEN,                                                                              \
                 HAL_DMACHCTRL_DST_INC_HalfWord | HAL_DMACHCTRL_DST_SIZE_HalfWord                  \
                     | HAL_DMACHCTRL_SRC_INC_NoIncrement | HAL_DMACHCTRL_SRC_SIZE_HalfWord         \
                     | HAL_DMACHCTRL_RPOW_1 | HAL_DMACHCTRL_CYCCTRL_Basic,                         \
                 SRC, DST, HAL_DMAXFERDIR_P2M)

#define HAL_DMA32_XFER_M2M(LEN, SRC, DST)                                                          \
    HAL_DMA_XFER(LEN,                                                                              \
                 HAL_DMACHCTRL_DST_INC_Word | HAL_DMACHCTRL_DST_SIZE_Word                          \
                     | HAL_DMACHCTRL_SRC_INC_Word | HAL_DMACHCTRL_SRC_SIZE_Word                    \
                     | HAL_DMACHCTRL_RPOW_1 | HAL_DMACHCTRL_CYCCTRL_Basic,                         \
                 SRC, DST, HAL_DMAXFERDIR_M2M)

#define HAL_DMA32_XFER_M2P(SIZE, SRC, DST)                                                         \
    HAL_DMA_XFER(LEN,                                                                              \
                 HAL_DMACHCTRL_DST_INC_NoIncrement | HAL_DMACHCTRL_DST_SIZE_Word                   \
                     | HAL_DMACHCTRL_SRC_INC_Word | HAL_DMACHCTRL_SRC_SIZE_Word                    \
                     | HAL_DMACHCTRL_RPOW_1 | HAL_DMACHCTRL_CYCCTRL_Basic,                         \
                 SRC, DST, HAL_DMAXFERDIR_M2P)

#define HAL_CREATE_DMA32_XFER_P2M(LEN, SRC, DST)                                                   \
    HAL_DMA_XFER(LEN,                                                                              \
                 HAL_DMACHCTRL_DST_INC_Word | HAL_DMACHCTRL_DST_SIZE_Word                          \
                     | HAL_DMACHCTRL_SRC_INC_NoIncrement | HAL_DMACHCTRL_SRC_SIZE_Word             \
                     | HAL_DMACHCTRL_RPOW_1 | HAL_DMACHCTRL_CYCCTRL_Basic,                         \
                 SRC, DST, HAL_DMAXFERDIR_P2M)

#define HAL_DMA_XFER(LEN, CTRL, SRC, DST, DIR)                                                     \
    {                                                                                              \
        .len = (uint32_t)LEN, .offset = 0, .control = CTRL, .dir = DIR, .cb = 0, .cbArg = 0,       \
        .pSrc = (uint8_t *)SRC, .pDst = (uint8_t *)DST                                             \
    }

    /* DMA Channel address type */
    typedef enum
    {
        HAL_DMA_PRIMARY_CH0   = 0, ///< Primary CH0
        HAL_DMA_PRIMARY_CH1   = 1, ///< Primary CH1
        HAL_DMA_PRIMARY_CH2   = 2, ///< Primary CH2
        HAL_DMA_PRIMARY_CH3   = 3, ///< Primary CH3
        HAL_DMA_ALTERNATE_CH0 = 4, ///< Alternate CH0
        HAL_DMA_ALTERNATE_CH1 = 5, ///< Alternate CH1
        HAL_DMA_ALTERNATE_CH2 = 6, ///< Alternate CH2
        HAL_DMA_ALTERNATE_CH3 = 7, ///< Alternate CH3
        HAL_DMACHADDR_MAX
    } HAL_DMACHADDR_T;

    /* DMA callback type */
    typedef void (*HAL_DMACB_T)(void *, HAL_ERROR_T);

#define HAL_DMACHCTRL_DST_INC_Mask        (0x3U << 30)
#define HAL_DMACHCTRL_DST_INC_Byte        (0x0U << 30)
#define HAL_DMACHCTRL_DST_INC_HalfWord    (0x1U << 30)
#define HAL_DMACHCTRL_DST_INC_Word        (0x2U << 30)
#define HAL_DMACHCTRL_DST_INC_NoIncrement (0x3U << 30)

#define HAL_DMACHCTRL_DST_SIZE_Mask     (0x3U << 28)
#define HAL_DMACHCTRL_DST_SIZE_Byte     (0x0U << 28)
#define HAL_DMACHCTRL_DST_SIZE_HalfWord (0x1U << 28)
#define HAL_DMACHCTRL_DST_SIZE_Word     (0x2U << 28)

#define HAL_DMACHCTRL_SRC_INC_Mask        (0x3U << 26)
#define HAL_DMACHCTRL_SRC_INC_Byte        (0x0U << 26)
#define HAL_DMACHCTRL_SRC_INC_HalfWord    (0x1U << 26)
#define HAL_DMACHCTRL_SRC_INC_Word        (0x2U << 26)
#define HAL_DMACHCTRL_SRC_INC_NoIncrement (0x3U << 26)

#define HAL_DMACHCTRL_SRC_SIZE_Mask     (0x3U << 24)
#define HAL_DMACHCTRL_SRC_SIZE_Byte     (0x0U << 24)
#define HAL_DMACHCTRL_SRC_SIZE_HalfWord (0x1U << 24)
#define HAL_DMACHCTRL_SRC_SIZE_Word     (0x2U << 24)

#define HAL_DMACHCTRL_DSTHPROT_Mask      (0x7U << 21)
#define HAL_DMACHCTRL_DSTHPROT_Cacheable (0x1U << 23)
#define HAL_DMACHCTRL_DSTHPROT_NoBuff    (0x1U << 22)
#define HAL_DMACHCTRL_DSTHPROT_NonPriv   (0x1U << 21)

#define HAL_DMACHCTRL_SRCHPROT_Mask      (0x7U << 18)
#define HAL_DMACHCTRL_SRCHPROT_Cacheable (0x1U << 20)
#define HAL_DMACHCTRL_SRCHPROT_NoBuff    (0x1U << 19)
#define HAL_DMACHCTRL_SRCHPROT_NonPriv   (0x1U << 18)

#define HAL_DMACHCTRL_RPOW_Mask          (0xfU << 14)
#define HAL_DMACHCTRL_RPOW_1             (0x0U << 14)
#define HAL_DMACHCTRL_RPOW_2             (0x0U << 14)
#define HAL_DMACHCTRL_RPOW_4             (0x0U << 14)
#define HAL_DMACHCTRL_RPOW_8             (0x0U << 14)
#define HAL_DMACHCTRL_RPOW_16            (0x0U << 14)
#define HAL_DMACHCTRL_RPOW_32            (0x0U << 14)
#define HAL_DMACHCTRL_RPOW_64            (0x0U << 14)
#define HAL_DMACHCTRL_RPOW_128           (0x0U << 14)
#define HAL_DMACHCTRL_RPOW_256           (0x0U << 14)
#define HAL_DMACHCTRL_RPOW_512           (0x0U << 14)
#define HAL_DMACHCTRL_RPOW_NoArbitration (0x0U << 14)

#define HAL_DMACHCTRL_NM1_Offset 4
#define HAL_DMACHCTRL_NM1_Mask   (0x3ffU << HAL_DMACHCTRL_NM1_Offset)

#define HAL_DMACHCTRL_NXTUSEBURST_Mask   (0x1U << 3)
#define HAL_DMACHCTRL_NXTUSEBURST_Toggle (0x1U << 3)

#define HAL_DMACHCTRL_CYCCTRL_Mask       (0x7U << 0)
#define HAL_DMACHCTRL_CYCCTRL_Stop       (0x0U << 0)
#define HAL_DMACHCTRL_CYCCTRL_Basic      (0x1U << 0)
#define HAL_DMACHCTRL_CYCCTRL_Auto       (0x2U << 0)
#define HAL_DMACHCTRL_CYCCTRL_PingPong   (0x3U << 0)
#define HAL_DMACHCTRL_CYCCTRL_MemSGPri   (0x4U << 0)
#define HAL_DMACHCTRL_CYCCTRL_MemSGAlt   (0x5U << 0)
#define HAL_DMACHCTRL_CYCCTRL_PeriSGPrim (0x6U << 0)
#define HAL_DMACHCTRL_CYCCTRL_PeriSGAlt  (0x7U << 0)

    /**
 * Transfer types
 */
    typedef enum
    {
        HAL_DMAXFERDIR_M2M, ///< Memory to Memory transfer
        HAL_DMAXFERDIR_M2P, ///< Memory to Peripheral transfer
        HAL_DMAXFERDIR_P2M, ///< Peripheral to Memory transfer
    } HAL_DMAXFERDIR_T;

    /**
 * Transfer Addres increment size
 */
    typedef enum
    {
        HAL_DMA_BYTE        = 0, ///< Address incrmenets by 1 byte alignment
        HAL_DMA_HALFWORD    = 1, ///< Address increments by 2 byte2 alignment
        HAL_DMA_WORD        = 2, ///< Address increments by 4 bytes alignment
        HAL_DMA_NOINCREMENT = 3, ///< Address does not increment
    } HAL_DMAXFERUNIT_T;

    typedef struct
    {
        uint32_t         len;     ///< length of transfer
        uint32_t         offset;  ///< Size of data transferred
        uint32_t         control; ///< Address of control structure
        HAL_DMAXFERDIR_T dir;
        HAL_DMACB_T      cb;    ///< Callback after DMA xfer complete
        void            *cbArg; ///< Argument to the callback
        uint8_t         *pSrc;  ///< Source end pointer
        uint8_t         *pDst;  ///< Destination end pointer
    } HAL_DMAXFER_T;

    /**
 * DMA control block structure
 */
    typedef struct
    {
        uint32_t pSrcEnd;  ///< Address of source end
        uint32_t pDstEnd;  ///< Address of destination end
        uint32_t control;  ///< Control data
        uint32_t reserved; ///< Pointer to channel transfer data
    } HAL_DMACTRL_T;

    /**
 * DMA structure
 */
    typedef struct
    {
        DMA_Type      *dev;
        uint32_t       active;  ///< Active channels
        HAL_DMACTRL_T *pCfg;    ///< Primary config
        HAL_DMAXFER_T *pChXfer; ///< Channel xfer data
    } HAL_DMA_T;

    /**
 * @brief Initialize the DMA configuration with DMA instance, control block.
 * @param DMA instance
 */
    HAL_ERROR_T HAL_DMA_Enable();

    /**
 * @brief Disable DMA
 */
    void HAL_DMA_Disable();

    /**
 * @brief Start DMA transfer
 */
    void HAL_DMA_StartXfer(HAL_DMACHADDR_T chAddr);

    /**
 * @brief Stop DMA transfer
 */
    void HAL_DMA_StopXfer(HAL_DMACHADDR_T chAddr);

    /**
 * @brief Setup transfer before starting
 */
    HAL_ERROR_T HAL_DMA_SetupXfer(HAL_DMACHADDR_T chAddr, const HAL_DMAXFER_T *const xfer);

    /**
 * @brief Remaining count for DMA transfer
 */
    uint32_t HAL_DMA_XferCount(HAL_DMACHADDR_T chAddr);

    /**
 * @brief Completion Interrupt handler
 */
    void HAL_DMA_DONE_IRQHandler();

    /**
 * @brief Error Interrupt handler
 */
    void HAL_DMA_ERROR_IRQHandler();

#ifdef __cplusplus
}
#endif
#endif /** @} __FUSB15XXX_DMA_H__ */
