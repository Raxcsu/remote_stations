import math
import os
import random
import sqlite3
import time
from datetime import datetime

DB_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'temperatura.db')

DISPOSITIVOS = [
    {'nombre': 'IMARPE-Callao', 'lat': -12.050, 'lng': -77.160},
    {'nombre': 'Nodo-Huacho',   'lat': -11.110, 'lng': -77.620},
    {'nombre': 'Nodo-Chimbote', 'lat':  -9.070, 'lng': -78.600},
    {'nombre': 'Nodo-Paracas',  'lat': -13.830, 'lng': -76.250},
    {'nombre': 'Nodo-Pisco',    'lat': -13.710, 'lng': -76.210},
]


def init_db():
    conn = sqlite3.connect(DB_PATH)
    cols = [r[1] for r in conn.execute("PRAGMA table_info(lecturas)").fetchall()]
    if cols and 'dispositivo_id' not in cols:
        conn.execute('DROP TABLE lecturas')

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
    conn.execute('CREATE INDEX IF NOT EXISTS idx_lecturas_dev_ts ON lecturas(dispositivo_id, timestamp)')
    conn.commit()

    # Insertar dispositivos si no existen
    ids = {}
    for d in DISPOSITIVOS:
        row = conn.execute('SELECT id FROM dispositivos WHERE nombre=?', (d['nombre'],)).fetchone()
        if row:
            ids[d['nombre']] = row[0]
        else:
            cur = conn.execute(
                'INSERT INTO dispositivos (nombre, lat, lng) VALUES (?, ?, ?)',
                (d['nombre'], d['lat'], d['lng'])
            )
            ids[d['nombre']] = cur.lastrowid
            print(f"  Dispositivo registrado: {d['nombre']}")
    conn.commit()
    conn.close()
    return ids


def insertar(dispositivo_id, timestamp, temperatura):
    conn = sqlite3.connect(DB_PATH)
    conn.execute(
        'INSERT INTO lecturas (dispositivo_id, timestamp, temperatura) VALUES (?, ?, ?)',
        (dispositivo_id, timestamp, temperatura)
    )
    conn.commit()
    conn.close()


ids = init_db()

# Estado interno de cada nodo: temperatura base + fase para variación senoidal
estados = {
    nombre: {
        'base': random.uniform(18.5, 23.5),
        'fase': random.uniform(0, 2 * math.pi),
        'ruido': 0.0,
    }
    for nombre in ids
}

print(f"Simulador iniciado — {len(ids)} nodos. Ctrl+C para detener.\n")

tick = 0
try:
    while True:
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        for nombre, dev_id in ids.items():
            e = estados[nombre]
            # Variación lenta senoidal + ruido
            e['ruido'] = e['ruido'] * 0.8 + random.gauss(0, 0.05)
            temp = e['base'] + 1.5 * math.sin(e['fase'] + tick * 0.05) + e['ruido']
            temp = round(temp, 2)
            insertar(dev_id, timestamp, temp)
            print(f"{timestamp}  {nombre:<20} {temp:.2f}°C")

        tick += 1
        print()
        time.sleep(5)
except KeyboardInterrupt:
    print("\nSimulador detenido.")
