#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>

static void clock_setup(void)
{
	/* Enable GPIOD clock for LED & USARTs. */
	rcc_periph_clock_enable(RCC_GPIOD);
	rcc_periph_clock_enable(RCC_GPIOA);

	/* Enable clocks for USART2. */
	rcc_periph_clock_enable(RCC_USART2);
}

static void usart_setup(void)
{
	/* Setup USART2 parameters. */
	usart_set_baudrate(USART2, 115200);
	usart_set_databits(USART2, 8);
	usart_set_stopbits(USART2, USART_STOPBITS_1);
	usart_set_mode(USART2, USART_MODE_TX);
	usart_set_parity(USART2, USART_PARITY_NONE);
	usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);

	/* Finally enable the USART. */
	usart_enable(USART2);
}

static void gpio_setup(void)
{
	/* Setup GPIO pin GPIO12 on GPIO port D for LED. */
	gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO12);

	/* Setup GPIO pins for USART2 transmit. */
	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO2);

	/* Setup USART2 TX pin as alternate function. */
	gpio_set_af(GPIOA, GPIO_AF7, GPIO2);
}

int _write(int file, char *ptr, int len)
{
	int i;

	if (file == STDOUT_FILENO || file == STDERR_FILENO) {
		for (i = 0; i < len; i++) {
			if (ptr[i] == '\n') {
				usart_send_blocking(USART2, '\r');
			}
			usart_send_blocking(USART2, ptr[i]);
		}
		return i;
	}
	errno = EIO;
	return -1;
}

int main(void) {
	clock_setup();
	gpio_setup();
	usart_setup();

    printf("hello world!\n");
    int i = 0;

    while (1) {
        printf("%d\n", i++);
        for (int j = 0; j < 3000000; j++) {	/* Wait a bit. */
            __asm__("NOP");
        }
    }

	return 0;
}
