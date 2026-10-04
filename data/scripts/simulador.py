import math
import os
import random
import sys
import time
from datetime import datetime

sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..'))

from db import get_db, init_schema, registrar_dispositivos


def insertar(conn, dispositivo_id, timestamp, temperatura):
    conn.execute(
        'INSERT INTO lecturas (dispositivo_id, timestamp, temperatura) VALUES (?, ?, ?)',
        (dispositivo_id, timestamp, temperatura)
    )
    conn.commit()


def generar(conn_factory=get_db, iteraciones=None, intervalo=5):
    conn = conn_factory()
    init_schema(conn)
    ids = registrar_dispositivos(conn)

    estados = {
        nombre: {
            'base': random.uniform(18.5, 23.5),
            'fase': random.uniform(0, 2 * math.pi),
            'ruido': 0.0,
        }
        for nombre in ids
    }

    print(f"Simulador iniciado — {len(ids)} nodos.")

    tick = 0
    while iteraciones is None or tick < iteraciones:
        timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        for nombre, dev_id in ids.items():
            e = estados[nombre]
            e['ruido'] = e['ruido'] * 0.8 + random.gauss(0, 0.05)
            temp = e['base'] + 1.5 * math.sin(e['fase'] + tick * 0.05) + e['ruido']
            temp = round(temp, 2)
            insertar(conn, dev_id, timestamp, temp)
            print(f"{timestamp}  {nombre:<20} {temp:.2f}°C")

        tick += 1
        print()
        time.sleep(intervalo)


if __name__ == '__main__':
    print("Ctrl+C para detener.\n")
    try:
        generar()
    except KeyboardInterrupt:
        print("\nSimulador detenido.")
