/*
 * Laboratorio #4 STM32: GPIO, ADC, comunicaciones, Iot
 *
 * Estudiantes: Denisse Ugalde Rivera y Alonso José Jiménez Anchía 
 * 
 */
// Bibliotecas

// Bibliotecas estandares

#include <stdint.h> // Define tipos de datos de ancho fijo.
#include <math.h> // Para realizar cálculos matemáticos complejos.

// Bibliotecas obtenidas de la carpeta example
#include "clock.h"
#include "console.h"
#include "sdram.h"
#include "lcd-spi.h"
#include "gfx.h"

// bibliotecas de libopencm3
// #include <libopencm3/stm32/rcc.h>
// #include <libopencm3/stm32/usart.h>
// #include <libopencm3/stm32/spi.h>
// #include <libopencm3/stm32/gpio.h>

// Registros para la configuración específica del giroscopio, obtenidas de .../libopencm3-examples/examples/stm32/f3/stm32f3-discovery/spi/spi.c
#define GYR_RNW			(1 << 7) /* Write when zero */
#define GYR_MNS			(1 << 6) /* Multiple reads when 1 */
#define GYR_WHO_AM_I		0x0F /* Registro Who Am I, para identificar el dispositivo.*/
#define GYR_OUT_TEMP		0x26 /* contiene la lectura de temperatura del giroscopio*/
#define GYR_STATUS_REG		0x27 /* para verificar si hay datos nuevos disponibles */

#define GYR_CTRL_REG1		0x20 /* para configurar el dispositivo (encender el giroscopio y habilitar los ejes) */
#define GYR_CTRL_REG1_PD	(1 << 3) /* activa o desactiva el modo de encendido (Power Down) */

// habilitan los ejes X, Y y Z del giroscopio para la lectura.
#define GYR_CTRL_REG1_XEN	(1 << 1) 
#define GYR_CTRL_REG1_YEN	(1 << 0) 
#define GYR_CTRL_REG1_ZEN	(1 << 2) 

#define GYR_CTRL_REG1_BW_SHIFT	4 /* Define cuántos bits se deben desplazar para establecer el ancho de banda en el registro de control 1*/
#define GYR_CTRL_REG4		0x23 /* para configuraciones adicionales como la escala de sensibilidad */
#define GYR_CTRL_REG4_FS_SHIFT	4 /* cuántos bits se deben desplazar para establecer la escala de sensibilidad completa */

// direcciones de los registros que contienen las partes baja y alta de las lecturas de los ejes X, Y y Z. 
#define GYR_OUT_X_L		0x28 /* */
#define GYR_OUT_X_H		0x29 /* */
#define GYR_OUT_Y_L		0x2A /* */
#define GYR_OUT_Y_H		0x2B /* */
#define GYR_OUT_Z_L		0x2C /* */
#define GYR_OUT_Z_H		0x2D /* */


/* Basada en la logica de .../libopencm3-examples/examples/stm32/f3/stm32f3-discovery/spi/spi.c
 * y en .../libopencm3-examples/examples/stm32/f4/stm32f429i-discovery/spi/spi-mems.c 
 */


/* Funcion utilizada para la configuracion de SPI5 y GPIOs */
static void spi_setup(void)
{

	/* Setup GPIOC1 pin for spi mode l3gd20 select. */
	gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO1);
	/* Start with spi communication disabled */
	gpio_set(GPIOC, GPIO1);

	/* Setup GPIO pins for AF5 for SPI1 signals. */
	gpio_mode_setup(GPIOF, GPIO_MODE_AF, GPIO_PUPD_PULLDOWN, GPIO7 | GPIO8 | GPIO9);
	gpio_set_af(GPIOF, GPIO_AF5, GPIO7 | GPIO8 | GPIO9);

	//spi initialization;
	spi_set_master_mode(SPI5);
	spi_set_baudrate_prescaler(SPI5, SPI_CR1_BR_FPCLK_DIV_64);
	spi_set_clock_polarity_0(SPI5);
	spi_set_clock_phase_0(SPI5);
	spi_set_full_duplex_mode(SPI5);
	spi_set_unidirectional_mode(SPI5); /* bidirectional but in 3-wire */
	spi_set_data_size(SPI5, SPI_CR2_DS_8BIT);
	spi_enable_software_slave_management(SPI5);
	spi_send_msb_first(SPI5);
	spi_set_nss_high(SPI5);
	
	spi_fifo_reception_threshold_8bit(SPI5);
	SPI_I2SCFGR(SPI5) &= ~SPI_I2SCFGR_I2SMOD;
	spi_enable(SPI5);
}

/* Funcion utilizada para la configuracion del USART1 y GPIOs */
static void usart_setup(void)
{

	/* Setup GPIO pin GPIO_USART1_TX/GPIO9 on GPIO port A for transmit. */
	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO9);
	gpio_set_af(GPIOA, GPIO_AF7, GPIO9);

	/* Setup UART parameters. */
	usart_set_baudrate(USART1, 115200);
	usart_set_databits(USART1, 8);
	usart_set_stopbits(USART1, USART_STOPBITS_1);
	usart_set_mode(USART1, USART_MODE_TX);
	usart_set_parity(USART1, USART_PARITY_NONE);
	usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);

	/* Finally enable the USART. */
	usart_enable(USART1);
}

/* Configuracion de GPIOs */
static void gpio_setup(void)
{
    gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO0);
    gpio_mode_setup(GPIOG, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO13);
    gpio_mode_setup(GPIOG, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO14);
}

static void clock_setup(void)
{
rcc_clock_setup_pll (&rcc_hse_8mhz_3v3 [RCC_CLOCK_3V3_84MHZ]);
rcc_periph_clock_enable(RCC_SPI5);

/* Habilitacion de los relojes de GPIOs*/
rcc_periph_clock_enable (RCC_GPIOA);
rcc_periph_clock_enable (RCC_GPIOB);
rcc_periph_clock_enable (RCC_GPIOC);
rcc_periph_clock_enable (RCC_GPIOD);
rcc_periph_clock_enable (RCC_GPIOE);
rcc_periph_clock_enable (RCC_GPIOF);
rcc_periph_clock_enable(RCC_USART1);
rcc_periph_clock_enable(RCC_ADC1);
}

int main(void) {

/* se inicializan las funciones */

spi_setup();
usart_setup();
gpio_setup();
clock_setup();
sdram_init(); /* obtenido de sdram.c */
lcd_spi_init(); /* obtenido de lcd-spi.c */

}

/* falta funcion para configurar pantalla (mostrar texto) 
*y otra funcion para leer el nivel de bateria  
*/ 