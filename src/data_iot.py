#!/usr/bin/env python3

import serial
import paho.mqtt.client as mqtt
import json
import time

# Conexión MQTT
broker_address = "iot.eie.ucr.ac.cr"
mqtt_port = 1883
#mqtt_port = 8080
mqtt_topic = "v1/devices/me/telemetry"
#mqtt_username = "Lab4_C07893_B63561"
mqtt_password = "yq7cgc21emlixma2q1p0"

# Se configura el puerto serie
serial_port = "/dev/ttyACM0"  
baudrate = 115200  # Velocidad de baudios

# Crear instancia del cliente MQTT
mqtt_client = mqtt.Client("STM32_Serial_MQTT")

# Definir los manejadores de eventos MQTT
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT broker")
    else:
        print(f"Failed to connect to MQTT broker with result code {rc}")

def on_disconnect(client, userdata, rc):
    print(f"Disconnected from MQTT broker with result code {rc}")

# Funciones de callback
mqtt_client.on_connect = on_connect

mqtt_client.on_disconnect = on_disconnect

# Nombre de usuario y contraseña para MQTT
mqtt_client.username_pw_set(mqtt_password, "")

# Conectar al broker MQTT
mqtt_client.connect(broker_address, mqtt_port, 60)
mqtt_client.subscribe(mqtt_topic)
mqtt_client.loop_start()  # Iniciar el loop en un hilo separado

# Iniciar la comunicación serial
comunicacion_serial = serial.Serial(serial_port, baudrate, timeout=1)

while True:
    try:
        if comunicacion_serial.inWaiting():

            eje_x = comunicacion_serial.readline(16).decode('utf-8')
            eje_y = comunicacion_serial.readline(16).decode('utf-8')
            eje_z = comunicacion_serial.readline(16).decode('utf-8')
            voltaje = comunicacion_serial.readline(16).decode('utf-8')
 
            print('X:')
            #eje_x = comunicacion_serial.readline(16).decode('utf-8')
            print(eje_x)
            print('Y:')
            #eje_y = comunicacion_serial.readline(16).decode('utf-8')
            print(eje_y)
            print('Z:')
            #eje_z = comunicacion_serial.readline(16).decode('utf-8')
            print(eje_z)
            print('Bateria_voltaje:')
            #voltaje = comunicacion_serial.readline(16).decode('utf-8')
            print(voltaje)

            try:
                # Construir payload MQTT
                payload = json.dumps({"Eje X": int(eje_x),
                                      "Eje Y": int(eje_y),
                                      "Eje Z": int(eje_z),
                                      "Nivel Bateria": int(voltaje)})
                mqtt_client.publish(mqtt_topic, payload)
            except ValueError:
                print("Error al procesar los datos recibidos.")

    except KeyboardInterrupt:
        print("\nScript interrumpido por el usuario.")
        break

# Limpiar antes de salir
comunicacion_serial.close()
mqtt_client.loop_stop()
mqtt_client.disconnect()