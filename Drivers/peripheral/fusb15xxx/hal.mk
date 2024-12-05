HAL_PATH = $(PROJDIR)/Drivers/$(DEVICE)
INCDIR += -I$(HAL_PATH)
CSRC += $(HAL_PATH)/FUSB152xx_hal.c\
	$(HAL_PATH)/fusb15200_hal_adc.c\
	$(HAL_PATH)/fusb15200_hal_dma.c\
	$(HAL_PATH)/fusb15200_hal_usbchg.c\
	$(HAL_PATH)/fusb15200_hal_gpio.c\
	$(HAL_PATH)/fusb15200_hal_hpd.c\
	$(HAL_PATH)/fusb15200_hal_i2c.c\
	$(HAL_PATH)/fusb15200_hal_port.c\
	$(HAL_PATH)/fusb15200_hal_timer.c\
	$(HAL_PATH)/fusb15200_hal_tcpd.c\
	$(HAL_PATH)/fusb15200_hal_ntc.c\
	$(HAL_PATH)/fusb15200_hal_wdt.c
