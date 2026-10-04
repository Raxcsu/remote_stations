import os
import sqlite3

DB_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'data', 'temperatura.db')

DISPOSITIVOS = [
    {'nombre': 'IMARPE-Callao', 'lat': -12.050, 'lng': -77.160},
    {'nombre': 'Nodo-Huacho',   'lat': -11.110, 'lng': -77.620},
    {'nombre': 'Nodo-Chimbote', 'lat':  -9.070, 'lng': -78.600},
    {'nombre': 'Nodo-Paracas',  'lat': -13.830, 'lng': -76.250},
    {'nombre': 'Nodo-Pisco',    'lat': -13.710, 'lng': -76.210},
]


def get_db(path=DB_PATH):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    conn = sqlite3.connect(path)
    conn.row_factory = sqlite3.Row
    return conn


def init_schema(conn):
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


def registrar_dispositivos(conn, dispositivos=DISPOSITIVOS):
    ids = {}
    for d in dispositivos:
        row = conn.execute('SELECT id FROM dispositivos WHERE nombre = ?', (d['nombre'],)).fetchone()
        if row:
            ids[d['nombre']] = row[0]
            print(f"  = {d['nombre']} (ya registrado)")
        else:
            cur = conn.execute(
                'INSERT INTO dispositivos (nombre, lat, lng) VALUES (?, ?, ?)',
                (d['nombre'], d['lat'], d['lng'])
            )
            ids[d['nombre']] = cur.lastrowid
            print(f"  + {d['nombre']}")
    conn.commit()
    return ids
