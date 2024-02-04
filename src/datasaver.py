#!/usr/bin/env python3
import serial
import paho.mqtt.client as mqtt
import json
import time

# Se configura el puerto serie
serial_port = "/dev/ttyACM0"  
baudrate = 115200  # Velocidad de baudios

# Conexión MQTT
broker_address = ""
mqtt_port = 1883
mqtt_topic = "v1/devices/me/telemetry"
mqtt_username = "STM32_User"
mqtt_password = "your_mqtt_password_here"

# Crear instancia del cliente MQTT
mqtt_client = mqtt.Client("STM32_Serial_MQTT")

# Definir los manejadores de eventos MQTT
def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))

def on_disconnect(client, userdata, rc):
    print("Disconnected from MQTT broker")

# Funciones de callback
mqtt_client.on_connect = on_connect
mqtt_client.on_disconnect = on_disconnect

# Nombre de usuario y contraseña para MQTT
mqtt_client.username_pw_set(mqtt_username, password=mqtt_password)

# Conectar al broker MQTT
mqtt_client.connect(broker_address, mqtt_port, 60)
mqtt_client.loop_start()  # Iniciar el loop en un hilo separado

# Iniciar la comunicación serial
ser = serial.Serial(serial_port, baudrate, timeout=1)

while True:
    try:
        if ser.inWaiting() > 0:
            # Leer línea del puerto serie y decodificar
            line = ser.readline().decode('utf-8').strip()
            # Esperamos una línea con el formato "x,y,z"
            if "," in line:
                x, y, z = line.split(',')
                print(f"X: {x}, Y: {y}, Z: {z}")
                
                try:
                    # Construir payload MQTT
                    payload = json.dumps({"Eje X": int(x), "Eje Y": int(y), "Eje Z": int(z)})
                    mqtt_client.publish(mqtt_topic, payload)
                except ValueError:
                    print("Error al procesar los datos recibidos.")

    except KeyboardInterrupt:
        print("\nScript interrumpido por el usuario.")
        break

# Limpiar antes de salir
ser.close()
mqtt_client.loop_stop()
mqtt_client.disconnect()

