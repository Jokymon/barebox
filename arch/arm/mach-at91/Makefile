obj-y += clock.o gpio.o

lowlevel_init-y = at91sam926x_lowlevel_init.o
lowlevel_init-$(CONFIG_ARCH_AT91RM9200) = at91rm9200_lowlevel_init.o
obj-$(CONFIG_MACH_DO_LOWLEVEL_INIT) += $(lowlevel_init-y)

pbl-$(CONFIG_MACH_DO_LOWLEVEL_INIT) += $(lowlevel_init-y)

obj-$(CONFIG_AT91SAM9_RESET) += at91sam9_reset.o
obj-$(CONFIG_AT91SAM9G45_RESET) += at91sam9g45_reset.o

# CPU-specific support
obj-$(CONFIG_ARCH_AT91RM9200)	+= at91rm9200.o at91rm9200_time.o at91rm9200_devices.o
obj-$(CONFIG_ARCH_AT91SAM9260) += at91sam9260.o at91sam926x_time.o at91sam9260_devices.o sam9_smc.o
obj-$(CONFIG_ARCH_AT91SAM9261) += at91sam9261.o at91sam926x_time.o at91sam9261_devices.o sam9_smc.o
obj-$(CONFIG_ARCH_AT91SAM9263) += at91sam9263.o at91sam926x_time.o at91sam9263_devices.o sam9_smc.o
obj-$(CONFIG_ARCH_AT91SAM9G10) += at91sam9261.o at91sam926x_time.o at91sam9261_devices.o sam9_smc.o
obj-$(CONFIG_ARCH_AT91SAM9G20) += at91sam9260.o at91sam926x_time.o at91sam9260_devices.o sam9_smc.o
obj-$(CONFIG_ARCH_AT91SAM9G45) += at91sam9g45.o at91sam926x_time.o at91sam9g45_devices.o sam9_smc.o
obj-$(CONFIG_ARCH_AT91SAM9X5)	+= at91sam9x5.o at91sam926x_time.o at91sam9x5_devices.o sam9_smc.o
