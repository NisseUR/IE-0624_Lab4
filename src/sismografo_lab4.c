/*
 * Laboratorio #4 STM32: GPIO, ADC, comunicaciones, Iot
 *
 * Estudiantes: Denisse Ugalde Rivera y Alonso José Jiménez Anchía 
 * 
 */

// Bibliotecas

// Bibliotecas estandares
#include <stdio.h>  // Para operaciones de entrada y salida básicas.
#include <stdint.h> // Define tipos de datos de ancho fijo.
#include <stdlib.h> // Para operaciones generales como conversión de números a cadenas.
#include <string.h> // Para manipular arrays de caracteres y strings.
#include <math.h>   // Para realizar cálculos matemáticos complejos.


// Bibliotecas obtenidas de la carpeta example
#include "adc.h"
#include "clock.h"
#include "console.h"
#include "dac.h"
#include "gfx.h"
#include "gpio.h"
#include "lcd-spi.h"
#include "rcc.h"
#include "sdram.h"
#include "spi.h"
#include "usart.h"


// Librerias para el MCU STM32F4

/* DECLARACION FUNCIONES */
void inicializar_sistema();


/* FUNCIONES */
          
void inicializar_sistema(){
    /* Funcion para inicializar el sistema 
    basado en ejemplo lcd-serial.c*/
    clock_setup();
	console_setup(115200);
	sdram_init();
	lcd_spi_init();
    gfx_init(lcd_draw_pixel, 240, 320);
    gfx_fillScreen(LCD_GREY);
}


int main(void){

    inicializar_sistema();

    while(1){
        gfx_puts("HOLA MUNDO!");
        gfx_fillScreen(LCD_RED);
        gfx_fillScreen(LCD_WHITE);
        gfx_fillScreen(LCD_BLUE);
    }
}