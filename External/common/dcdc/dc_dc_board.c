/*******************************************************************************
 * @file     dc_dc_board.c
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

#include "pd_types.h"
#include "port.h"
#include "dev_tcpd.h"
#include "dc_dc_board.h"
#include "timer.h"
#include "dpm_util.h"
#include "port_dcdc.h"
HAL_I2C_T *i2c_master[I2C_PORT_COUNT];
#if (DEVICE_TYPE == FUSB15200 || DEVICE_TYPE == FUSB15201 || DEVICE_TYPE == FUSB15201P)
volatile bool        i2c_port_interrupt;
struct dc_dc_data   *dcdc_data;
struct dc_dc_master *dcdc_master;
#endif
#if DEVICE_TYPE == FUSB15200
#define I2C_15200 ((I2C_Port_Type *)0x40C00000)
#define I2C_ERR_MASK (I2C_Port_I2C_STATUS_bus_error_Msk | I2C_Port_I2C_STATUS_overrun_Msk)
#endif
#if CONFIG_ENABLED(DCDC)

static uint8_t write_complete;

static void i2c_master_write_cb(HAL_ERROR_T err, uint32_t n)
{
    write_complete = 1;
}

static void i2c_master_read_cb(HAL_ERROR_T err, uint32_t n)
{
    write_complete = 1;
}

#if (DEVICE_TYPE == FUSB15200 || DEVICE_TYPE == FUSB15201 || DEVICE_TYPE == FUSB15201P)
static void i2c_port_int_cb(int i2c_id)
{
    i2c_port_interrupt = true;

    struct dc_dc_data *data = dcdc_data;
    while (data)
    {
        if (data->i2c_m == i2c_id)
        {
            fusbdev_i2c_irq_disable(data->dcdc_obj->dev->dcdc_device, i2c_id - 1);
            fusbdev_i2c_irq_clear(data->dcdc_obj->dev->dcdc_device, i2c_id - 1);
            break;
        }
        data++;
    }
}

uint32_t dc_dc_init(struct dc_dc_data *obj)
{
    uint32_t error = 0;
    uint8_t timeout = 0;
#if (DEVICE_TYPE == FUSB15200)
    uint8_t  buf[6] = {0};

    HAL_Delay(50);

    /* Set POT */
    buf[0]         = 0x00;
    buf[1]         = POT_D_BASE;
    write_complete = 0;
    fusbdev_i2c_write_with_interrupt(obj->dcdc_device, obj->pot_address, buf, 2,
                                     i2c_master_write_cb, obj->dcdc_master->i2cm_id - 1);
    while (write_complete == 0 && timeout < 100)
    {
    	if ((I2C_15200->I2C_STATUS & I2C_ERR_MASK))
    	{
    		timeout++;
    	}
    }

    /* Clear faults on POR */
    buf[0]         = 0x10;
    write_complete = 0;
    timeout = 0;
    fusbdev_i2c_write_with_interrupt(obj->dcdc_device, obj->slave_address, buf, 6,
                                     i2c_master_read_cb, obj->dcdc_master->i2cm_id - 1);

    while (write_complete == 0 && timeout < 100)
    {
    	if ((I2C_15200->I2C_STATUS & I2C_ERR_MASK))
    	{
    		timeout++;
    	}
    }
#else

    struct dc_dc_obj *dcdc_obj = obj->dcdc_obj;
    obj->dcdc_device           = dcdc_obj->dev->dcdc_device;

    HAL_GPIOCFG_T dis_cfg = {.iodir = HAL_GPIO_INPUT, .ppod = HAL_GPIO_PUSH_PULL};
    fusbdev_disable_interrupt_gpio_pin(obj->enable_gpio, obj->port);
    fusbdev_configure_gpio_pin(obj->enable_gpio, &dis_cfg, obj->port);
    fusbdev_set_gpio_pin(obj->enable_gpio, HAL_GPIO_HIGH, obj->port);

    i2c_port_interrupt = false;
    timer_disable(&obj->dc_dc_timers[DC_DC_DISABLE_TIMER]);

    /* Port interrupts */
    fusbdev_i2c_irq_set_callback(obj->dcdc_device, i2c_port_int_cb, obj->dcdc_master->i2cm_id - 1);
    fusbdev_i2c_irq_enable(obj->dcdc_device,
                           I2C_INT_DIR_IN | I2C_INT_ENABLE | I2C_INT_FALLING_LOW | I2C_INT_LEVEL,
                           obj->dcdc_master->i2cm_id - 1);
#endif
    return error;
}


uint32_t dc_dc_set(struct dc_dc_data *obj, uint32_t mv, uint32_t ma)
{
    uint8_t buf[2] = {0};
    uint32_t error = 0;
    uint8_t timeout = 0;

    /* Writing to a disabled board will have no effect */
    if (!dc_dc_enabled(obj))
    {
        return error;
    }
    if (mv < 600)
    {
        mv = 600;
    }
#if CONFIG_ENABLED(EPR)
    /* For EPR, update current limit */
    /* Set V */
    buf[0]         = 0x5;
    buf[1]         = 0x3;

    write_complete = 0;
    fusbdev_i2c_write_with_interrupt(obj->dcdc_device, obj->slave_address, buf, 2,
                                     i2c_master_write_cb, obj->dcdc_master->i2cm_id - 1);
    while (write_complete == 0 && timeout < 100)
    {
#if DEVICE_TYPE == FUSB15200
    	if ((I2C_15200->I2C_STATUS & I2C_ERR_MASK))
    	{
    		timeout++;
    	}
#endif
    }
#endif

    /* Set V */
    buf[0]         = 0x1;
#if (CONFIG_ENABLED(EPR))
    buf[1]         = mv / 120;
#else
    buf[1]         = mv / 100;
#endif
    buf[2]         = ma;
    write_complete = 0;
    timeout = 0;

    fusbdev_i2c_write_with_interrupt(obj->dcdc_device, obj->slave_address, buf, 2,
                                     i2c_master_write_cb, obj->dcdc_master->i2cm_id - 1);

    while (write_complete == 0 && timeout < 100)
    {
#if DEVICE_TYPE == FUSB15200
    	if ((I2C_15200->I2C_STATUS & I2C_ERR_MASK))
    	{
    		timeout++;
    	}
#endif
    }
    return error;
}
void dc_dc_delayed_disable(struct dc_dc_data *obj)
{
    timer_start(&(obj->dc_dc_timers[DC_DC_DISABLE_TIMER]), T_NCV_DISABLE);
    timer_disable(&obj->dc_dc_timers[DC_DC_ENABLE_TIMER]);
}

void dc_dc_delayed_vbus_write(struct dc_dc_data *obj, uint32_t mv, uint32_t ma)
{
    obj->vbus    = mv;
    obj->current = ma;
    timer_start(&obj->dc_dc_timers[DC_DC_ENABLE_TIMER], T_NCV_ENABLE);
    timer_disable(&obj->dc_dc_timers[DC_DC_DISABLE_TIMER]);
}

void dc_dc_enable(struct dc_dc_data *obj, bool enable)
{
    HAL_GPIOCFG_T en_cfg  = {.iodir = HAL_GPIO_OUTPUT, .ppod = HAL_GPIO_PUSH_PULL};
    HAL_GPIOCFG_T dis_cfg = {.iodir = HAL_GPIO_INPUT, .ppod = HAL_GPIO_PUSH_PULL};
    if (enable)
    {
        timer_disable(&obj->dc_dc_timers[DC_DC_DISABLE_TIMER]);
        fusbdev_set_gpio_pin(obj->enable_gpio, HAL_GPIO_HIGH, obj->port);
        fusbdev_configure_gpio_pin(obj->enable_gpio, &en_cfg, obj->port);
    }
    else
    {
        fusbdev_disable_interrupt_gpio_pin(obj->enable_gpio, obj->port);
        fusbdev_configure_gpio_pin(obj->enable_gpio, &dis_cfg, obj->port);
        fusbdev_set_gpio_pin(obj->enable_gpio, HAL_GPIO_LOW, obj->port);
    }
}

bool dc_dc_enabled(struct dc_dc_data *obj)
{
#if (DEVICE_TYPE == FUSB15201 || DEVICE_TYPE == FUSB15201P)
	bool gpio_enabled =((fusbdev_state_gpio_pin(obj->enable_gpio, HAL_GPIO_OUTPUT, obj->port) == HAL_GPIO_HIGH));
	//if port A connected, port should always be connected
	bool port_enabled = false;
	if (obj->enable_gpio != HAL_GPIO2)
	{
		port_enabled = true;
	}
	else
	{//port B means if SWD enabled, port is disconnected.
		port_enabled = !fusbdev_port_is_swd_enabled();
	}
	return (gpio_enabled && port_enabled);
#else
    return ((fusbdev_state_gpio_pin(obj->enable_gpio, HAL_GPIO_OUTPUT, obj->port) == HAL_GPIO_HIGH) ?
                true :
                false);
#endif
}

uint32_t dc_dc_read_status(struct dc_dc_data *obj)
{
    uint8_t buf[10] = {0};

    buf[0]         = 0x10;
    write_complete = 0;
    fusbdev_i2c_read_register(obj->dcdc_device, obj->slave_address, 0x10, buf, 6,
                              i2c_master_read_cb, obj->i2c_m - 1);
    while (write_complete == 0)
    {
    }
    return 0;
}

uint32_t dc_dc_processing(struct dc_dc_data *obj)
{
    if (i2c_port_interrupt == true)
    {
        dc_dc_read_status(obj);
        fusbdev_i2c_irq_enable(
            obj->dcdc_device, I2C_INT_DIR_IN | I2C_INT_ENABLE | I2C_INT_FALLING_LOW | I2C_INT_LEVEL,
            obj->dcdc_master->i2cm_id - 1);
    }
    return 0;
}

/* Function to run dc_dc interrupts twice as close in time as possible */
void port_processing_dc_dc(struct dc_dc_obj *dcdc_obj)
{
    unsigned i = 0;

    if (i2c_port_interrupt)
    {
        dc_dc_processing(dcdc_obj->dcdc_data[0]);
        dc_dc_processing(dcdc_obj->dcdc_data[1]);

        i2c_port_interrupt = false;
    }
    for (i = 0; i < TYPE_C_PORT_COUNT; i++)
    {
    	struct dpm_t * dpm = dcdc_obj->dev->dpm_priv;
    	struct port * port = dpm->port_tcpd[i]->port_tcpd_priv;
        /* Delayed DC-DC Disable */
        if (!timer_disabled(&dcdc_obj->dcdc_data[i]->dc_dc_timers[DC_DC_DISABLE_TIMER])
            && timer_expired(&dcdc_obj->dcdc_data[i]->dc_dc_timers[DC_DC_DISABLE_TIMER]))
        {
            if (dpm_vbus_valid(dcdc_obj->dev->dpm_priv, i, 0, VBUS_VSAFE0_THRESHOLD(VBUS_VSAFE0)) || !port->policy_is_source)
            {
                timer_disable(&dcdc_obj->dcdc_data[i]->dc_dc_timers[DC_DC_DISABLE_TIMER]);
                dc_dc_enable(dcdc_obj->dcdc_data[i], false);
                if (i == HAL_USBPD_CH_B)
                {
                    enable_swd(dcdc_obj->dev->dpm_priv, true);
                }
            }
            else
            {
                //resend vbus 600mv message
                dc_dc_set(dcdc_obj->dcdc_data[i], 0, 0);
                dc_dc_delayed_disable(dcdc_obj->dcdc_data[i]);
            }
        }
        /* Delayed DC-DC VBUS Write */
        if (!timer_disabled(&dcdc_obj->dcdc_data[i]->dc_dc_timers[DC_DC_ENABLE_TIMER])
            && timer_expired(&dcdc_obj->dcdc_data[i]->dc_dc_timers[DC_DC_ENABLE_TIMER]))
        {
            timer_disable(&dcdc_obj->dcdc_data[i]->dc_dc_timers[DC_DC_ENABLE_TIMER]);
            if (i == HAL_USBPD_CH_B)
            {
                enable_swd(dcdc_obj->dev->dpm_priv, false);
            }
            dc_dc_set(dcdc_obj->dcdc_data[i], dcdc_obj->dcdc_data[i]->vbus,
                      dcdc_obj->dcdc_data[i]->current);
        }
    }
}

uint32_t dc_dc_min_timer(struct dc_dc_data *obj)

{
    unsigned long       min = ~0;
    struct ticktimer_t *t;
    unsigned long       r = 0;
    unsigned long       i = 0;

    for (i = 0; i < DC_DC_TIMER_COUNT; i++)
    {
        t = &obj->dc_dc_timers[i];
        if (!timer_disabled(t))
        {
            r = timer_remaining(t);
            if (r < min)
            {
                min = r;
            }
        }
    }
    return min;
}

#endif

void enable_i2c_clock(int i2c_id)
{
    switch (i2c_id)
    {
        case I2C_iface_1:
            HAL_CMU_I2C1_CLOCK_ENABLE();
            break;
        case I2C_iface_2:
            HAL_CMU_I2C2_CLOCK_ENABLE();
            break;
#if (DEVICE_TYPE == FUSB15200) || (DEVICE_TYPE == FUSB15201)
        case I2C_iface_3:
            HAL_CMU_I2C3_CLOCK_ENABLE();
            break;
        case I2C_iface_4:
            HAL_CMU_I2C4_CLOCK_ENABLE();
            break;
#endif
        default:
            break;
    }
}

/*this function initialize the DCDC obj with I2C interface is functioning as master*/
void port_dcdc_master_init(struct dc_dc_obj *dcdc_obj, struct dc_dc_master *DCDC_master,
                           struct dc_dc_data *DCDC_data)
{
    struct dc_dc_data   *dcdc_data_temp   = 0;
    struct dc_dc_master *dcdc_master_temp = DCDC_master;
    int                  dcdc_master_size = sizeof(struct dc_dc_master) * I2C_PORT_COUNT;
    int                  dcdc_data_size   = 0;
    int                  slave            = 0;

#if (DEVICE_TYPE == FUSB15200 || DEVICE_TYPE == FUSB15201 || DEVICE_TYPE == FUSB15201P)
    /*save dcdc master and data pointers*/
    dcdc_data   = DCDC_data;
    dcdc_master = DCDC_master;
#endif

    while (dcdc_master_temp && (dcdc_master_size > 0))
    {
        if (dcdc_master_temp->i2cm_id != 0)
        {
            enable_i2c_clock(dcdc_master_temp->i2cm_id);
            dcdc_obj->dev->dcdc_device->i2c[dcdc_master_temp->i2cm_id - 1] =
            fusbdev_i2c_enable(dcdc_obj->dev->dcdc_device, dcdc_master_temp->i2cm_id);
            fusbdev_i2c_master_enable(dcdc_obj->dev->dcdc_device, dcdc_master_temp->prescalar,
                                      dcdc_master_temp->i2cm_id - 1);
            dcdc_obj->dcdc_master[dcdc_master_temp->i2cm_id - 1] = dcdc_master_temp;
            i2c_master[dcdc_master_temp->i2cm_id - 1] =
                dcdc_obj->dev->dcdc_device->i2c[dcdc_master_temp->i2cm_id - 1];

            slave          = 0;
            dcdc_data_temp = DCDC_data;
            dcdc_data_size = sizeof(struct dc_dc_data) * TYPE_C_PORT_COUNT;
            while (dcdc_data_temp && (dcdc_data_size > 0))
            {
                if ((dcdc_master_temp->i2cm_id == dcdc_data_temp->i2c_m)
                    && (!dcdc_data_temp->dcdc_master))
                {
                	dcdc_data_temp->dcdc_device = dcdc_obj->dev->dcdc_device;
                    dcdc_obj->dcdc_data[slave]  = dcdc_data_temp;
                    dcdc_data_temp->dcdc_master = dcdc_master_temp;
                    dcdc_data_temp->dcdc_obj    = dcdc_obj;
                    dc_dc_init(dcdc_data_temp);
                }
                slave++;
                dcdc_data_temp++;
                dcdc_data_size -= sizeof(struct dc_dc_data);
            }
        }
        dcdc_master_temp++;
        dcdc_master_size -= sizeof(struct dc_dc_master);
    }
}

/*this function initialize the DCDC obj with I2C interface is functioning as slave*/
void port_dcdc_slave_init(struct dc_dc_obj *dcdc_obj)
{
    //i2C slave implementation
}

#endif //CONFIG_ENABLED(DCDC)
