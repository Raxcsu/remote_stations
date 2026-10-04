import sqlite3

from db import DISPOSITIVOS, init_schema, registrar_dispositivos


def tabla_existe(conn, nombre):
    return conn.execute(
        "SELECT 1 FROM sqlite_master WHERE type='table' AND name=?", (nombre,)
    ).fetchone() is not None


def test_init_schema_crea_tablas_e_indice():
    conn = sqlite3.connect(':memory:')
    init_schema(conn)
    assert tabla_existe(conn, 'dispositivos')
    assert tabla_existe(conn, 'lecturas')
    indice = conn.execute(
        "SELECT 1 FROM sqlite_master WHERE type='index' AND name='idx_lecturas_dev_ts'"
    ).fetchone()
    assert indice is not None


def test_init_schema_migra_lecturas_sin_dispositivo_id():
    conn = sqlite3.connect(':memory:')
    conn.execute('CREATE TABLE lecturas (id INTEGER PRIMARY KEY, timestamp TEXT, temperatura REAL)')
    conn.execute("INSERT INTO lecturas (timestamp, temperatura) VALUES ('2026-01-01 00:00:00', 20.0)")
    init_schema(conn)
    cols = [r[1] for r in conn.execute('PRAGMA table_info(lecturas)').fetchall()]
    assert 'dispositivo_id' in cols
    assert conn.execute('SELECT COUNT(*) FROM lecturas').fetchone()[0] == 0


def test_init_schema_es_idempotente_y_conserva_datos():
    conn = sqlite3.connect(':memory:')
    init_schema(conn)
    dev = registrar_dispositivos(conn)['IMARPE-Callao']
    conn.execute(
        "INSERT INTO lecturas (dispositivo_id, timestamp, temperatura) VALUES (?, '2026-01-01 00:00:00', 20.0)",
        (dev,)
    )
    init_schema(conn)
    assert conn.execute('SELECT COUNT(*) FROM lecturas').fetchone()[0] == 1


def test_registrar_dispositivos_inserta_una_sola_vez():
    conn = sqlite3.connect(':memory:')
    init_schema(conn)
    primera = registrar_dispositivos(conn)
    segunda = registrar_dispositivos(conn)
    assert primera == segunda
    assert len(primera) == len(DISPOSITIVOS)
    assert conn.execute('SELECT COUNT(*) FROM dispositivos').fetchone()[0] == len(DISPOSITIVOS)
