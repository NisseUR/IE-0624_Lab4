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
#include <stdio.h> // Para funcion sprintf()
#include <stdbool.h> // librería estándar booleana

// Bibliotecas obtenidas de la carpeta example
#include "clock.h"
#include "console.h"
#include "sdram.h"
#include "lcd-spi.h"
#include "gfx.h"

// bibliotecas de libopencm3
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/spi.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/adc.h>

// Registros para la configuración específica del giroscopio, obtenidas de .../libopencm3-examples/examples/stm32/f3/stm32f3-discovery/spi/spi.c
#define GYR_RNW			(1 << 7) /* Write when zero */
#define GYR_MNS			(1 << 6) /* Multiple reads when 1 */
#define GYR_WHO_AM_I		0x0F /* Registro Who Am I, para identificar el dispositivo.*/
#define GYR_OUT_TEMP		0x26 /* contiene la lectura de temperatura del giroscopio*/
#define GYR_STATUS_REG		0x27 /* para verificar si hay datos nuevos disponibles */

#define GYR_CTRL_REG1		0x20 /* Para configurar el dispositivo (encender el giroscopio y habilitar los ejes) */
#define GYR_CTRL_REG1_PD	(1 << 3) /* Power Down = HIGH, en REGISTRO1 */

#define GYR_CTRL_REG2		0x20
#define GYR_CTRL_REG2_HPM1	(0 << 5)
#define GYR_CTRL_REG2_HPM2	(0 << 4)
#define GYR_CTRL_REG2_HPCF0	(1 << 0)
#define GYR_CTRL_REG2_HPCF1	(1 << 1)
#define GYR_CTRL_REG2_HPCF2	(1 << 2)
#define GYR_CTRL_REG2_HPCF3	(1 << 3)

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
#define LED_DISCO_RED_PORT GPIOG
#define LED_DISCO_RED_PIN GPIO14
#define LED_DISCO_GREEN_PORT GPIOG
#define LED_DISCO_GREEN_PIN GPIO13

// Resistencias divisoras de voltage
const float RESISTOR1 = 10000.0;
const float RESISTOR2 = 10000.0;

/* Declaracion de funciones */
static void spi_setup(void);
char console_getc_GR(int wait);


/* Basada en la logica de .../libopencm3-examples/examples/stm32/f3/stm32f3-discovery/spi/spi.c
 * y en .../libopencm3-examples/examples/stm32/f4/stm32f429i-discovery/spi/spi-mems.c 
 */


/* Funcion utilizada para la configuracion del giroscopio */
static void spi_setup(void)
{

	/* Setup GPIOC1 pin for spi mode l3gd20 select. */
	gpio_mode_setup(GPIOC, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO1); // Pin del CS
	/* Start with spi communication disabled */
	gpio_set(GPIOC, GPIO1);

	/* Setup GPIO pins for AF5 for SPI1 signals. */
	gpio_mode_setup(GPIOF, GPIO_MODE_AF, GPIO_PUPD_PULLDOWN, GPIO7 | GPIO8 | GPIO9); // Pin del SCK - MOSI
	gpio_set_af(GPIOF, GPIO_AF5, GPIO7 | GPIO8 | GPIO9);

	//spi initialization for gyroscope;
	spi_set_master_mode(SPI5);
	spi_set_baudrate_prescaler(SPI5, SPI_CR1_BR_FPCLK_DIV_64);
	spi_set_clock_polarity_0(SPI5);
	spi_set_clock_phase_0(SPI5);
	spi_set_full_duplex_mode(SPI5);
	spi_set_unidirectional_mode(SPI5); /* bidirectional but in 3-wire */
	spi_enable_software_slave_management(SPI5);
	spi_send_msb_first(SPI5);
	spi_set_nss_high(SPI5);
	SPI_I2SCFGR(SPI5) &= ~SPI_I2SCFGR_I2SMOD;
	spi_enable(SPI5);
}

/* Funcion utilizada para la configuracion del USART1 y GPIOs */
static void usart_setup(void)
{

	/* Setup GPIO pin GPIO_USART1_TX/GPIO9 on GPIO port A for transmit. */
	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO9);
	gpio_set_af(GPIOA, GPIO_AF7, GPIO9); // Pin USART1 TX 

	/* Setup UART parameters. */
	usart_set_baudrate(USART1, 115200);
	usart_set_databits(USART1, 8);
	usart_set_stopbits(USART1, USART_STOPBITS_1);
	usart_set_mode(USART1, USART_MODE_TX_RX);
	usart_set_parity(USART1, USART_PARITY_NONE);
	usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);

	/* Finally enable the USART. */
	usart_enable(USART1);
}

/* Configuracion de GPIOs */
static void gpio_setup(void)
{
    gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO0);

    /* Set GPIO13 (in GPIO port G) to 'output push-pull'. */
    gpio_mode_setup(GPIOG, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO13);

    gpio_mode_setup(GPIOG, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO14);
}

static void clock_setup_G(void)
{
rcc_clock_setup_pll (&rcc_hse_8mhz_3v3 [RCC_CLOCK_3V3_84MHZ]);
rcc_periph_clock_enable(RCC_SPI5);

/* Habilitacion de los relojes de GPIOs*/
rcc_periph_clock_enable (RCC_GPIOA); // USART1
rcc_periph_clock_enable (RCC_GPIOB); 
rcc_periph_clock_enable (RCC_GPIOC);
rcc_periph_clock_enable (RCC_GPIOD);
rcc_periph_clock_enable (RCC_GPIOE);
rcc_periph_clock_enable (RCC_GPIOF);
rcc_periph_clock_enable(RCC_USART1);
rcc_periph_clock_enable(RCC_ADC1);
}

// funcion para el boton 
static void button_setup(void){

    // Configuracion del boton, obtenida de ..libopencm3-examples/examples/stm32/f4/stm32f429i-discovery/button/button.c line 47
	
    /* Enable GPIOA clock. */

    /* Set GPIO0 (in GPIO port A) to 'input open-drain'. */
	gpio_mode_setup(GPIOA, GPIO_MODE_INPUT, GPIO_PUPD_NONE, GPIO0);


}



/* Lógica de la función obtenida de:
 .../libopencm3-examples/examples/stm32/f4/stm32f429i-discovery/adc-dac-printf/adc-dac-printf.c */
static void adc_setup(void)
{
    /* Configuración para leer valores analógicos de dos pines específicos usando 
    el ADC (Conversor Analógico a Digital)*/

	gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO3);
	adc_power_off(ADC1);
	adc_disable_scan_mode(ADC1);
	adc_set_sample_time_on_all_channels(ADC1, ADC_SMPR_SMP_3CYC);
    //gpio_set(GPIOA, GPIO3);

	adc_power_on(ADC1);

}

// Funcion para leer voltaje bateria
static uint16_t read_adc_naiive(uint8_t channel)
{
	uint8_t channel_array[16];
	channel_array[0] = channel;
	adc_set_regular_sequence(ADC1, 1, channel_array);
	adc_start_conversion_regular(ADC1);
	while (!adc_eoc(ADC1));
	uint16_t reg16 = adc_read_regular(ADC1);
	return reg16;
}

/* Funciones para imprimir caracteres en la consola, lógica obtenida de usart_console.c en el directorio f4 de examples*/

/*
 * char = console_getc_GR(int wait)
 *
 * Check the console for a character. If the wait flag is
 * non-zero. Continue checking until a character is received
 * otherwise return 0 if called and no character was available.
 */
char console_getc_GR(int wait)
{
	uint32_t	reg;
	do {
		reg = USART_SR(CONSOLE_UART);
	} while ((wait != 0) && ((reg & USART_SR_RXNE) == 0));
	return (reg & USART_SR_RXNE) ? USART_DR(CONSOLE_UART) : '\000';
}

static void my_usart_print_int(uint32_t usart, int32_t value) // obtenida de spi.c
{
	int8_t i;
	int8_t nr_digits = 0;
	char buffer[25];

	if (value < 0) {
		usart_send_blocking(usart, '-');
		value = value * -1;
	}

	if (value == 0) {
		usart_send_blocking(usart, '0');
	}

	while (value > 0) {
		buffer[nr_digits++] = "0123456789"[value % 10];
		value /= 10;
	}

	for (i = nr_digits-1; i >= 0; i--) {
		usart_send_blocking(usart, buffer[i]);
	}

	//usart_send_blocking(usart, '\r');
	//usart_send_blocking(usart, '\n');
}


int main(void) {

    clock_setup();
    clock_setup_G();
	console_setup(115200);
    adc_setup();
    spi_setup(); 
    usart_setup();
    gpio_setup();
    button_setup(); /* obtenido de button.c del directorio f4 en examples*/

    /* se inicializan las funciones */

    // lecturas ejes X Y Z del giroscopio
    int16_t eje_x, eje_y, eje_z;
    char frase[50];

    int usart_enable = 0; //para la habilitacion USART

    sdram_init(); /* obtenido de sdram.c */
    lcd_spi_init(); /* obtenido de lcd-spi.c */

    /* Inicializacion de la pantalla */
    gfx_init(lcd_draw_pixel, 240, 320);
	gfx_fillScreen(LCD_WHITE);
	gfx_setTextSize(2);
	gfx_setCursor(15, 25);
	gfx_puts("Sismografo");
    // Eje X
    gfx_setTextColor(LCD_BLACK, LCD_WHITE);
	gfx_setCursor(15, 80);
	gfx_puts("X: ");
    // Eje Y
    gfx_setTextColor(LCD_BLACK, LCD_WHITE);
	gfx_setCursor(15, 120);
	gfx_puts("Y: ");
    // Eje Z
    gfx_setTextColor(LCD_BLACK, LCD_WHITE);
    gfx_setCursor(15, 160);
	gfx_puts("Z: ");

	lcd_show_frame();

    //Configuración inicial del giroscopio, basado en spic.c ubicado en el directorio f3: 
    gpio_clear(GPIOC, GPIO1);
    spi_send(SPI5, GYR_CTRL_REG1);
    spi_read(SPI5);

    // Se habilitan los ejes
    spi_send(SPI5, GYR_CTRL_REG1_PD | GYR_CTRL_REG1_XEN | GYR_CTRL_REG1_YEN | GYR_CTRL_REG1_ZEN | (3 << GYR_CTRL_REG1_BW_SHIFT));
    spi_read(SPI5);
    gpio_set(GPIOC, GPIO1);

    gpio_clear(GPIOC, GPIO1);
   
    spi_send(SPI5, GYR_CTRL_REG2);
    spi_read(SPI5);
    spi_send(SPI5, ~GYR_CTRL_REG2_HPM1 | GYR_CTRL_REG2_HPM2| GYR_CTRL_REG2_HPCF0 | GYR_CTRL_REG2_HPCF1| GYR_CTRL_REG2_HPCF2 | GYR_CTRL_REG2_HPCF3);
    spi_read(SPI5);
    gpio_set(GPIOC, GPIO1);
    gpio_clear(GPIOC, GPIO1);
    spi_send(SPI5, GYR_CTRL_REG4);
    spi_read(SPI5);
    spi_send(SPI5, (1 << GYR_CTRL_REG4_FS_SHIFT));
    spi_read(SPI5);

    
    /* red led for ticking */
	gpio_mode_setup(LED_DISCO_RED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
	LED_DISCO_RED_PIN);
    /* green led for ticking */
	gpio_mode_setup(LED_DISCO_GREEN_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE,
	LED_DISCO_GREEN_PIN);

    while (1) {

        gfx_fillScreen(LCD_WHITE);
        gfx_setCursor(15, 25);
	    gfx_puts("Sismografo");

        //Giroscopio eje X 
        gpio_clear(GPIOC, GPIO1);
        spi_send(SPI5, GYR_OUT_X_L | GYR_RNW);
        spi_read(SPI5);
        spi_send(SPI5, 0);
        eje_x =spi_read(SPI5);
        gpio_set(GPIOC, GPIO1);

        gpio_clear(GPIOC, GPIO1); //lectura de datos empieza 
        spi_send(SPI5, GYR_OUT_X_H | GYR_RNW);
        spi_read(SPI5);
        spi_send(SPI5, 0);
        eje_x|= spi_read(SPI5) << 8;
        
        // Mostrar eje x en la pantalla 
        gfx_setTextColor(LCD_BLACK, LCD_WHITE);
        sprintf(frase, "X: %d", eje_x);
        gfx_setCursor(15, 80);
        gfx_puts(frase);

        gpio_set(GPIOC, GPIO1);//lectura de datos termina 

        //Giroscopio eje Y 
        gpio_clear(GPIOC, GPIO1);
        spi_send(SPI5, GYR_OUT_Y_L | GYR_RNW);
        spi_read(SPI5);
        spi_send(SPI5, 0);
        eje_y=spi_read(SPI5);
        gpio_set(GPIOC, GPIO1);

        gpio_clear(GPIOC, GPIO1);
        spi_send(SPI5, GYR_OUT_Y_H | GYR_RNW);
        spi_read(SPI5);
        spi_send(SPI5, 0);
        eje_y|= spi_read(SPI5) << 8;
        
        // Mostrar eje y en la pantalla 
        sprintf(frase, "Y: %d", eje_y);
        gfx_setCursor(15, 120);
        gfx_puts(frase);

        gpio_set(GPIOC, GPIO1);

        //Giroscopio eje Z 
        gpio_clear(GPIOC, GPIO1);
        spi_send(SPI5, GYR_OUT_Z_L | GYR_RNW);
        spi_read(SPI5);
        spi_send(SPI5, 0);
        eje_z=spi_read(SPI5);
        gpio_set(GPIOC, GPIO1);

        gpio_clear(GPIOC, GPIO1);
        spi_send(SPI5, GYR_OUT_Z_H | GYR_RNW);
        spi_read(SPI5);
        spi_send(SPI5, 0);
        eje_z|= spi_read(SPI5) << 8;
        
        // Mostrar eje z en la pantalla
        sprintf(frase, "Z: %d", eje_z);
        gfx_setCursor(15, 160);
        gfx_puts(frase); 

        // Chequear bateria
        uint16_t input_adc0 = read_adc_naiive(3); // Se lee voltaje

        uint16_t voltaje_real  = (input_adc0 * 9.0f) / 4095.0f; 

        //voltaje_real = input_adc0*(RESISTOR1 + RESISTOR2)/RESISTOR1;

        if(voltaje_real <= 7){
            // LED on/off bateria baja
		    gpio_toggle(LED_DISCO_RED_PORT, LED_DISCO_RED_PIN);
            // Mostrar nivel bateria en pantalla
            sprintf(frase, "Bateria baja: %d V", voltaje_real);
            gfx_setCursor(10, 210);
            gfx_puts(frase);
        }else if(voltaje_real > 7){
            gpio_clear(LED_DISCO_RED_PORT, LED_DISCO_RED_PIN);
            // Mostrar nivel bateria en pantalla
            sprintf(frase, "Carga bateria: %d V", voltaje_real);
            gfx_setCursor(10, 210);
            gfx_puts(frase); 
        }

        // Boton para habilitar la comunicacion USART
        // Verificación del estado del botón y actualización de la variable de habilitación

        bool botonPresionado = gpio_get(GPIOA, GPIO0); 
        if (botonPresionado) {
            usart_enable = !usart_enable; 
        }

        if(usart_enable){
            gfx_setCursor(15, 280);
            gfx_puts("Encendido"); // Indica en la pantalla que la comunicación está habilitada
            gpio_toggle(GPIOG, GPIO13); // Cambia el estado del LED de comunicación

            // Envío de los valores de los ejes al USART
            my_usart_print_int(USART1, eje_x);
            console_puts("\n");
            my_usart_print_int(USART1, eje_y);
            console_puts("\n");
            my_usart_print_int(USART1, eje_z);
            console_puts("\n");
            // Envío valor/nivel de la bateria al USART
            my_usart_print_int(USART1, voltaje_real);
            console_puts("\n");
        }else{
            gfx_setCursor(15, 280);
            gfx_puts("Apagado"); // Indica en la pantalla que la comunicación está deshabilitada
            gpio_clear(GPIOG, GPIO13); // Apaga el LED de comunicación
        }
        
        lcd_show_frame();

        // se agrega delay, obtenido de usart.c de examples f4
        int i;
        for (i = 0; i < 3000000; i++) {	/* Wait a bit. */
			__asm__("NOP");
		}
    }

    return 0; 
}