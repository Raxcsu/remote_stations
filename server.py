import math
import os
import sqlite3
from datetime import datetime, timedelta
from flask import Flask, jsonify, request, send_from_directory

app = Flask(__name__)

DB_PATH = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'data', 'temperatura.db')
DASHBOARD_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'dashboard')


def get_db():
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    return conn


def init_db():
    conn = get_db()
    # Migrate: drop old lecturas if it lacks dispositivo_id
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
    conn.close()


# ── Helpers ──────────────────────────────────────────────────────────────────

def linreg(values):
    """Returns slope of a linear regression over values indexed 0..n-1."""
    n = len(values)
    if n < 2:
        return 0.0
    x_mean = (n - 1) / 2
    y_mean = sum(values) / n
    num = sum((i - x_mean) * (v - y_mean) for i, v in enumerate(values))
    den = sum((i - x_mean) ** 2 for i in range(n))
    return num / den if den else 0.0


def compute_stats(conn, dispositivo_id):
    now      = datetime.now()
    ts_10m   = (now - timedelta(minutes=10)).strftime("%Y-%m-%d %H:%M:%S")
    ts_30m   = (now - timedelta(minutes=30)).strftime("%Y-%m-%d %H:%M:%S")
    ts_1h    = (now - timedelta(hours=1)).strftime("%Y-%m-%d %H:%M:%S")
    ts_24h   = (now - timedelta(hours=24)).strftime("%Y-%m-%d %H:%M:%S")

    latest = conn.execute(
        'SELECT temperatura, timestamp FROM lecturas WHERE dispositivo_id=? ORDER BY id DESC LIMIT 1',
        (dispositivo_id,)
    ).fetchone()

    if not latest:
        return {'status': 'offline', 'temperatura': None}

    temp     = latest['temperatura']
    last_ts  = latest['timestamp']

    # Estado del nodo
    if last_ts >= ts_10m:
        status = 'activo'
    elif last_ts >= ts_30m:
        status = 'alerta'
    else:
        status = 'offline'

    def delta(ts_ref):
        r = conn.execute(
            'SELECT temperatura FROM lecturas WHERE dispositivo_id=? AND timestamp <= ? ORDER BY timestamp DESC LIMIT 1',
            (dispositivo_id, ts_ref)
        ).fetchone()
        if not r:
            return None, None
        d = round(temp - r['temperatura'], 2)
        p = round(d / r['temperatura'] * 100, 1) if r['temperatura'] else None
        return d, p

    d1h,  p1h  = delta(ts_1h)
    d24h, p24h = delta(ts_24h)

    # Tendencia y predicción: regresión sobre últimas 20 lecturas
    last20 = [r['temperatura'] for r in conn.execute(
        'SELECT temperatura FROM lecturas WHERE dispositivo_id=? ORDER BY id DESC LIMIT 20',
        (dispositivo_id,)
    ).fetchall()]
    last20.reverse()

    slope = linreg(last20)
    # Extrapolamos 12 pasos (~1h en producción con lecturas cada 5 min)
    pred_1h = round(temp + slope * 12, 2)

    if slope > 0.02:
        tendencia = 'subiendo'
    elif slope < -0.02:
        tendencia = 'bajando'
    else:
        tendencia = 'estable'

    # Anomalía: desviación > 2σ respecto a las últimas 24h
    vals_24h = [r['temperatura'] for r in conn.execute(
        'SELECT temperatura FROM lecturas WHERE dispositivo_id=? AND timestamp >= ?',
        (dispositivo_id, ts_24h)
    ).fetchall()]

    anomalia = 'normal'
    if len(vals_24h) >= 10:
        mean = sum(vals_24h) / len(vals_24h)
        std  = math.sqrt(sum((x - mean) ** 2 for x in vals_24h) / len(vals_24h))
        if std > 0 and abs(temp - mean) > 2 * std:
            anomalia = 'anomalía'

    return {
        'temperatura':   round(temp, 2),
        'timestamp':     last_ts,
        'status':        status,
        'delta_1h':      d1h,
        'delta_1h_pct':  p1h,
        'delta_24h':     d24h,
        'delta_24h_pct': p24h,
        'tendencia':     tendencia,
        'prediccion_1h': pred_1h,
        'anomalia':      anomalia,
    }


# ── Rutas ─────────────────────────────────────────────────────────────────────

@app.route('/')
def index():
    return send_from_directory(DASHBOARD_DIR, 'index.html')


@app.route('/<path:path>')
def static_files(path):
    return send_from_directory(DASHBOARD_DIR, path)


@app.route('/api/dispositivos')
def dispositivos():
    conn = get_db()
    devs = conn.execute('SELECT id, nombre, lat, lng FROM dispositivos').fetchall()
    result = []
    for d in devs:
        stats = compute_stats(conn, d['id'])
        result.append({'id': d['id'], 'nombre': d['nombre'], 'lat': d['lat'], 'lng': d['lng'], **stats})
    conn.close()
    return jsonify(result)


@app.route('/api/lecturas')
def lecturas():
    dispositivo_id = request.args.get('dispositivo_id', type=int)
    horas          = request.args.get('horas', 24, type=int)
    conn = get_db()

    if dispositivo_id:
        cutoff = (datetime.now() - timedelta(hours=horas)).strftime("%Y-%m-%d %H:%M:%S")
        rows = conn.execute(
            'SELECT id, timestamp, temperatura FROM lecturas WHERE dispositivo_id=? AND timestamp >= ? ORDER BY id ASC',
            (dispositivo_id, cutoff)
        ).fetchall()
    else:
        rows = []

    conn.close()
    return jsonify([dict(r) for r in rows])


if __name__ == '__main__':
    init_db()
    print("Servidor en http://localhost:5000")
    app.run(debug=True, port=5000)
