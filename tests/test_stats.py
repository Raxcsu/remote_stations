import sqlite3
from datetime import datetime, timedelta

import pytest

from db import init_schema, registrar_dispositivos
from stats import FORMATO_TS, compute_stats, linreg


@pytest.fixture
def conn():
    c = sqlite3.connect(':memory:')
    c.row_factory = sqlite3.Row
    init_schema(c)
    yield c
    c.close()


@pytest.fixture
def dev(conn):
    return registrar_dispositivos(conn)['IMARPE-Callao']


def insertar_lecturas(conn, dev, lecturas):
    for segundos_atras, temp in lecturas:
        ts = (datetime.now() - timedelta(seconds=segundos_atras)).strftime(FORMATO_TS)
        conn.execute(
            'INSERT INTO lecturas (dispositivo_id, timestamp, temperatura) VALUES (?, ?, ?)',
            (dev, ts, temp)
        )
    conn.commit()


def test_sin_lecturas_esta_offline(conn, dev):
    stats = compute_stats(conn, dev)
    assert stats == {'status': 'offline', 'temperatura': None}


@pytest.mark.parametrize('antiguedad_s, esperado', [
    (60, 'activo'),
    (15 * 60, 'alerta'),
    (45 * 60, 'offline'),
])
def test_status_segun_antiguedad_de_la_ultima_lectura(conn, dev, antiguedad_s, esperado):
    insertar_lecturas(conn, dev, [(antiguedad_s, 20.0)])
    assert compute_stats(conn, dev)['status'] == esperado


def test_delta_1h_none_sin_referencia_anterior(conn, dev):
    insertar_lecturas(conn, dev, [(60, 20.0), (30, 20.5)])
    stats = compute_stats(conn, dev)
    assert stats['delta_1h'] is None
    assert stats['delta_24h'] is None


def test_delta_1h_con_referencia_de_hace_dos_horas(conn, dev):
    insertar_lecturas(conn, dev, [(2 * 3600, 19.0), (60, 20.0)])
    stats = compute_stats(conn, dev)
    assert stats['delta_1h'] == 1.0
    assert stats['delta_1h_pct'] == round(1.0 / 19.0 * 100, 1)


def test_tendencia_subiendo_con_lecturas_crecientes(conn, dev):
    insertar_lecturas(conn, dev, [(5 * (20 - i), 20.0 + 0.1 * i) for i in range(20)])
    assert compute_stats(conn, dev)['tendencia'] == 'subiendo'


def test_tendencia_estable_con_lecturas_constantes(conn, dev):
    insertar_lecturas(conn, dev, [(5 * (20 - i), 20.0) for i in range(20)])
    assert compute_stats(conn, dev)['tendencia'] == 'estable'


def test_no_expone_prediccion_1h(conn, dev):
    insertar_lecturas(conn, dev, [(5 * (20 - i), 20.0 + 0.001 * i) for i in range(20)])
    assert 'prediccion_1h' not in compute_stats(conn, dev)


def test_anomalia_normal_con_menos_de_10_muestras(conn, dev):
    insertar_lecturas(conn, dev, [(5 * i, 20.0) for i in range(5)] + [(0, 99.0)])
    assert compute_stats(conn, dev)['anomalia'] == 'normal'


def test_anomalia_detecta_desviacion_mayor_a_2_sigma(conn, dev):
    base = [(5 * (10 + i), 20.0 + (0.1 if i % 2 else -0.1)) for i in range(15)]
    insertar_lecturas(conn, dev, base + [(0, 25.0)])
    assert compute_stats(conn, dev)['anomalia'] == 'anomalía'


def test_anomalia_normal_si_desviacion_cero(conn, dev):
    insertar_lecturas(conn, dev, [(5 * i, 20.0) for i in range(15)])
    assert compute_stats(conn, dev)['anomalia'] == 'normal'


def test_linreg_pendiente_de_recta():
    assert linreg([1.0, 2.0, 3.0, 4.0]) == pytest.approx(1.0)


def test_linreg_menos_de_dos_valores_es_cero():
    assert linreg([5.0]) == 0.0


