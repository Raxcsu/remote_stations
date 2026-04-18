import os
import sqlite3
import serial
from datetime import datetime

PORT               = "COM5"
BAUDRATE           = 9600
DISPOSITIVO_NOMBRE = "IMARPE-Callao"   # debe coincidir exactamente con la tabla dispositivos
DB_PATH            = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'temperatura.db')


def get_db():
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    return conn


def init_db():
    conn = get_db()
    conn.execute('''
        CREATE TABLE IF NOT EXISTS dispositivos (
            id     INTEGER PRIMARY KEY AUTOINCREMENT,
            nombre TEXT    NOT NULL,
            lat    REAL    NOT NULL,
            lng    REAL    NOT NULL
        )
    ''')
    conn.execute('''
        CREATE TABLE IF NOT EXISTS lecturas (
            id             INTEGER PRIMARY KEY AUTOINCREMENT,
            dispositivo_id INTEGER NOT NULL REFERENCES dispositivos(id),
            timestamp      TEXT    NOT NULL,
            temperatura    REAL    NOT NULL
        )
    ''')
    conn.commit()
    conn.close()


def get_dispositivo_id(nombre):
    conn = get_db()
    row = conn.execute('SELECT id FROM dispositivos WHERE nombre = ?', (nombre,)).fetchone()
    conn.close()
    if not row:
        raise RuntimeError(
            f"Dispositivo '{nombre}' no encontrado en la DB.\n"
            f"Corre el simulador al menos una vez para registrar los nodos,\n"
            f"o verifica que DISPOSITIVO_NOMBRE coincida exactamente."
        )
    return row['id']


def insertar(dispositivo_id, timestamp, temperatura):
    conn = get_db()
    conn.execute(
        'INSERT INTO lecturas (dispositivo_id, timestamp, temperatura) VALUES (?, ?, ?)',
        (dispositivo_id, timestamp, temperatura)
    )
    conn.commit()
    conn.close()


init_db()
dispositivo_id = get_dispositivo_id(DISPOSITIVO_NOMBRE)

ser = serial.Serial(PORT, BAUDRATE, timeout=1)
print(f"Escuchando en {PORT} → '{DISPOSITIVO_NOMBRE}' (id={dispositivo_id})")

try:
    while True:
        line = ser.readline().decode("utf-8").strip()
        if line.startswith("TempC:") and line.endswith("C"):
            try:
                temp = float(line[6:-1])
                timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
                insertar(dispositivo_id, timestamp, temp)
                print(f"{timestamp}  {DISPOSITIVO_NOMBRE:<20} {temp:.2f}°C")
            except ValueError:
                print(f"Formato inválido: {line}")
except KeyboardInterrupt:
    print("\nLectura finalizada.")
finally:
    ser.close()
