import os
from datetime import datetime, timedelta
from flask import Flask, jsonify, request, send_from_directory

from db import get_db, init_schema
from stats import FORMATO_TS, compute_stats

app = Flask(__name__)

DASHBOARD_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'dashboard')

_conn = get_db()
init_schema(_conn)
_conn.close()


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
        cutoff = (datetime.now() - timedelta(hours=horas)).strftime(FORMATO_TS)
        rows = conn.execute(
            'SELECT id, timestamp, temperatura FROM lecturas WHERE dispositivo_id=? AND timestamp >= ? ORDER BY id ASC',
            (dispositivo_id, cutoff)
        ).fetchall()
    else:
        rows = []

    conn.close()
    return jsonify([dict(r) for r in rows])


if __name__ == '__main__':
    print("Servidor en http://localhost:5000")
    app.run(debug=os.environ.get('FLASK_DEBUG') == '1', port=5000)
