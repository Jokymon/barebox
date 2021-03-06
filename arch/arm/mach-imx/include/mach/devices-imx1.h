#include <mach/devices.h>

static inline struct device_d *imx1_add_uart0(void)
{
	return imx_add_uart((void *)MX1_UART1_BASE_ADDR, 0);
}

static inline struct device_d *imx1_add_uart1(void)
{
	return imx_add_uart((void *)MX1_UART2_BASE_ADDR, 1);
}
