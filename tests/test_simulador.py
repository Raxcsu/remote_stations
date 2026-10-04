import sqlite3
import sys
import os

sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', 'data', 'scripts'))

from db import DISPOSITIVOS
from simulador import generar


def test_generar_escribe_una_lectura_por_nodo_por_iteracion():
    conn = sqlite3.connect(':memory:')
    generar(conn_factory=lambda: conn, iteraciones=2, intervalo=0)
    total = conn.execute('SELECT COUNT(*) FROM lecturas').fetchone()[0]
    assert total == 2 * len(DISPOSITIVOS)


def test_generar_registra_todos_los_nodos():
    conn = sqlite3.connect(':memory:')
    generar(conn_factory=lambda: conn, iteraciones=1, intervalo=0)
    nombres = {r[0] for r in conn.execute('SELECT nombre FROM dispositivos').fetchall()}
    assert nombres == {d['nombre'] for d in DISPOSITIVOS}
