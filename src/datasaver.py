#!/home/yeskot/anaconda3/bin/python

import serial
import csv

import paho.mqtt.client as mqtt
import json

# Configuración para el puerto serie.
# Asegúrate de tener permisos para acceder al puerto.
ser = serial.Serial(
    port='/tmp/ttyS1',
    baudrate=9600,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS,
    timeout=0
)

print(f"Conectado a: {ser.portstr}")

f = open('data.csv', 'w+')

# Read and write lines from the serial port
while True:
    try:
        # Read bytes from the serial port
        byte_data = ser.readline()

        # Decode the bytes to a string using UTF-8
        decoded_data = byte_data.decode('utf-8')

        # Print the decoded string
        print(decoded_data, end='', flush=True)
        
        # Write the entire decoded_data as one row in the CSV file
        f.write(decoded_data)

    except Exception as e:
        print(f"Error: {e}")

# The file will be automatically closed when the script exits the 'with' block
            






