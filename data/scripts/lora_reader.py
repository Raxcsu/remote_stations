import os
import sys
from datetime import datetime

import serial

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))

from db import get_db, init_schema

PORT               = "COM5"
BAUDRATE           = 9600
DISPOSITIVO_NOMBRE = "IMARPE-Callao"   # debe coincidir exactamente con la tabla dispositivos


def get_dispositivo_id(conn, nombre):
    row = conn.execute('SELECT id FROM dispositivos WHERE nombre = ?', (nombre,)).fetchone()
    if not row:
        raise RuntimeError(
            f"Dispositivo '{nombre}' no encontrado en la DB.\n"
            f"Corre el simulador al menos una vez para registrar los nodos,\n"
            f"o verifica que DISPOSITIVO_NOMBRE coincida exactamente."
        )
    return row['id']


def insertar(conn, dispositivo_id, timestamp, temperatura):
    conn.execute(
        'INSERT INTO lecturas (dispositivo_id, timestamp, temperatura) VALUES (?, ?, ?)',
        (dispositivo_id, timestamp, temperatura)
    )
    conn.commit()


conn = get_db()
init_schema(conn)
dispositivo_id = get_dispositivo_id(conn, DISPOSITIVO_NOMBRE)

ser = serial.Serial(PORT, BAUDRATE, timeout=1)
print(f"Escuchando en {PORT} → '{DISPOSITIVO_NOMBRE}' (id={dispositivo_id})")

try:
    while True:
        line = ser.readline().decode("utf-8").strip()
        if line.startswith("TempC:") and line.endswith("C"):
            try:
                temp = float(line[6:-1])
                timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                insertar(conn, dispositivo_id, timestamp, temp)
                print(f"{timestamp}  {DISPOSITIVO_NOMBRE:<20} {temp:.2f}°C")
            except ValueError:
                print(f"Formato inválido: {line}")
except KeyboardInterrupt:
    print("\nLectura finalizada.")
finally:
    ser.close()
    conn.close()
