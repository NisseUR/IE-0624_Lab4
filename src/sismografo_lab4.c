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
