"""
Inicialización del proyecto: crea la base de datos y registra los nodos.
Ejecutado automáticamente por setup.bat.
"""

import os
import sqlite3

DB_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'data', 'temperatura.db')

DISPOSITIVOS = [
    ('IMARPE-Callao', -12.050, -77.160),
    ('Nodo-Huacho',   -11.110, -77.620),
    ('Nodo-Chimbote',  -9.070, -78.600),
    ('Nodo-Paracas',  -13.830, -76.250),
    ('Nodo-Pisco',    -13.710, -76.210),
]


def init():
    os.makedirs(os.path.dirname(DB_PATH), exist_ok=True)
    conn = sqlite3.connect(DB_PATH)

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
    conn.execute(
        'CREATE INDEX IF NOT EXISTS idx_lecturas_dev_ts ON lecturas(dispositivo_id, timestamp)'
    )

    print("\nRegistrando nodos:")
    for nombre, lat, lng in DISPOSITIVOS:
        exists = conn.execute(
            'SELECT id FROM dispositivos WHERE nombre = ?', (nombre,)
        ).fetchone()
        if not exists:
            conn.execute(
                'INSERT INTO dispositivos (nombre, lat, lng) VALUES (?, ?, ?)',
                (nombre, lat, lng)
            )
            print(f"  + {nombre}")
        else:
            print(f"  ✓ {nombre} (ya registrado)")

    conn.commit()
    conn.close()


if __name__ == '__main__':
    init()
    print("\nBase de datos lista en data/temperatura.db")
