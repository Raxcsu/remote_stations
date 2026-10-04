import math
from datetime import datetime, timedelta

FORMATO_TS = "%Y-%m-%d %H:%M:%S"


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
    ts_10m   = (now - timedelta(minutes=10)).strftime(FORMATO_TS)
    ts_30m   = (now - timedelta(minutes=30)).strftime(FORMATO_TS)
    ts_1h    = (now - timedelta(hours=1)).strftime(FORMATO_TS)
    ts_24h   = (now - timedelta(hours=24)).strftime(FORMATO_TS)

    latest = conn.execute(
        'SELECT temperatura, timestamp FROM lecturas WHERE dispositivo_id=? ORDER BY id DESC LIMIT 1',
        (dispositivo_id,)
    ).fetchone()

    if not latest:
        return {'status': 'offline', 'temperatura': None}

    temp     = latest['temperatura']
    last_ts  = latest['timestamp']

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

    last20 = [r['temperatura'] for r in conn.execute(
        'SELECT temperatura FROM lecturas WHERE dispositivo_id=? ORDER BY id DESC LIMIT 20',
        (dispositivo_id,)
    ).fetchall()]
    last20.reverse()

    slope = linreg(last20)

    if slope > 0.02:
        tendencia = 'subiendo'
    elif slope < -0.02:
        tendencia = 'bajando'
    else:
        tendencia = 'estable'

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
        'anomalia':      anomalia,
    }
