/*******************************************************************************
 * @file     dc_dc_board.h
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
#ifndef __DC_DC_BOARD_H__
#define __DC_DC_BOARD_H__

#include "timer.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define VBUS_ADDR_PORTA        0x74
#define VBUS_ADDR_PORTB        0x75
#define POT_ADDR_PORTA         0x2C
#define POT_ADDR_PORTB         0x2D
#define POT_D_BASE             50
#define POT_X                  90000
#define POT_Y                  2000
#define POT_20MV(mv100, rem20) (((POT_X * (rem20)) / (((mv100) + POT_Y) * 20)) + POT_D_BASE)
#define T_NCV_DISABLE          (50)
#define T_NCV_ENABLE           (5)

    enum
    {
        DC_DC_ENABLE_TIMER,
        DC_DC_DISABLE_TIMER,
        DC_DC_TIMER_COUNT,
    };

    typedef struct dc_dc_master
    {
        const int i2cm_id;
        uint32_t  prescalar;
    } DCDC_MASTER_T;

    //TODO: Rename?
    typedef struct dc_dc_data
    {
    	struct fusb_dcdc_device *dcdc_device;
        const int            i2c_m;
        struct port_dcdc *dev;
        const uint8_t              slave_address;
        const uint8_t              pot_address;
        struct dc_dc_master *dcdc_master;
        struct dc_dc_obj    *dcdc_obj;
        HAL_I2C_T            *i2c[I2C_PORT_COUNT];
#if (DEVICE_TYPE == FUSB15200 || DEVICE_TYPE == FUSB15201 || DEVICE_TYPE == FUSB15201P)
        uint32_t           vbus;
        uint32_t           current;
        struct ticktimer_t dc_dc_timers[DC_DC_TIMER_COUNT];
        const HAL_GPIOx_T        enable_gpio;
        const HAL_GPIO_PORT_T    port;
#endif
    } DCDC_DATA_T;

    /* POT */
    /* W0: 	1 << 6: Midscale Reset
 * 		1 << 5: Shutdown (high-z at A)
 * W1:	POT D Setting
 */
    uint32_t      dc_dc_init(struct dc_dc_data *obj);
    uint32_t      dc_dc_set(struct dc_dc_data *obj, uint32_t mv, uint32_t ma);
    bool          dc_dc_detected(struct dc_dc_data *obj);
    void          dc_dc_delayed_disable(struct dc_dc_data *obj);
    void          dc_dc_enable(struct dc_dc_data *obj, bool enable);
    bool          dc_dc_enabled(struct dc_dc_data *obj);
    unsigned long dc_dc_min_timer(struct dc_dc_data *obj);
    void          dc_dc_delayed_vbus_write(struct dc_dc_data *obj, uint32_t mv, uint32_t ma);
    uint32_t      dc_dc_processing(struct dc_dc_data *obj);
    uint32_t      dc_dc_read_status(struct dc_dc_data *obj);
    void port_dcdc_master_init(struct dc_dc_obj *dcdc_obj, struct dc_dc_master *DCDC_Master,
                               struct dc_dc_data *DCDC_Data);
    void port_dcdc_slave_init(struct dc_dc_obj *dcdc_obj);
    void port_processing_dc_dc(struct dc_dc_obj *dcdc_obj);

#ifdef __cplusplus
}
#endif
#endif /* __DC_DC_BOARD_H__ */
