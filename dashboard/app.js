// ── State ─────────────────────────────────────────────────────────────────
let map, markers = {};
let selectedId = null;
let horasHistorico = 6;
let histChart = null;
let initialFitDone = false;
let lastDevicesCache = [];

// ── Init ──────────────────────────────────────────────────────────────────
initMap();
fetchDispositivos();
setInterval(fetchDispositivos, 5000);

document.querySelectorAll('.btn-range').forEach(btn => {
    btn.addEventListener('click', () => {
        document.querySelectorAll('.btn-range').forEach(b => b.classList.remove('active'));
        btn.classList.add('active');
        horasHistorico = parseInt(btn.dataset.horas);
        if (selectedId) fetchHistorico(selectedId);
    });
});

// ── Map ───────────────────────────────────────────────────────────────────
function initMap() {
    const peruBounds = L.latLngBounds(L.latLng(-18.5, -82.0), L.latLng(0.5, -68.0));
    map = L.map('map', {
        zoomControl: true,
        maxBounds: peruBounds,
        maxBoundsViscosity: 0.7,
        minZoom: 3,
        maxZoom: 10,
    }).fitBounds(peruBounds);

    L.tileLayer('https://server.arcgisonline.com/ArcGIS/rest/services/Ocean/World_Ocean_Base/MapServer/tile/{z}/{y}/{x}', {
        attribution: '© Esri, GEBCO, NOAA',
        maxZoom: 10,
    }).addTo(map);

    L.tileLayer('https://server.arcgisonline.com/ArcGIS/rest/services/Ocean/World_Ocean_Reference/MapServer/tile/{z}/{y}/{x}', {
        maxZoom: 10,
        pane: 'shadowPane',
    }).addTo(map);
}

function fitToNodes(devices) {
    const active = devices.filter(d => d.lat && d.lng);
    if (active.length === 0) return;
    const bounds = L.latLngBounds(active.map(d => [d.lat, d.lng]));
    map.fitBounds(bounds, { padding: [60, 60], maxZoom: 8 });
}

function nodeColor(status) {
    return status === 'offline' ? '#2a4a63' : '#00e096';
}

function tempLabel(t) {
    return t !== null && t !== undefined ? t.toFixed(1) + '°C' : 'Sin datos';
}

// ── API calls ─────────────────────────────────────────────────────────────
async function fetchDispositivos() {
    try {
        const data = await fetch('/api/dispositivos').then(r => r.json());
        lastDevicesCache = data;
        updateStatus(data);
        updateMap(data);
        updateNodeList(data);
        if (!initialFitDone && data.length > 0) {
            fitToNodes(data);
            initialFitDone = true;
        }
        if (selectedId) {
            const dev = data.find(d => d.id === selectedId);
            if (dev) updateDetail(dev);
            fetchHistorico(selectedId);
        }
    } catch {
        setStatusBar('error');
    }
}

async function fetchHistorico(deviceId) {
    const data = await fetch(`/api/lecturas?dispositivo_id=${deviceId}&horas=${horasHistorico}`).then(r => r.json());
    renderHistorico(data);
}

// ── Status bar ────────────────────────────────────────────────────────────
function updateStatus(devices) {
    const active = devices.filter(d => d.status === 'activo').length;
    document.getElementById('activeCount').textContent =
        `${active} nodo${active !== 1 ? 's' : ''} activo${active !== 1 ? 's' : ''}`;
    setStatusBar(active > 0 ? 'live' : 'waiting');
}

function setStatusBar(state) {
    const dot  = document.getElementById('statusDot');
    const text = document.getElementById('statusText');
    const states = {
        live:       ['dot--live',       'En vivo'],
        waiting:    ['dot--waiting',    'Sin datos'],
        connecting: ['dot--connecting', 'Conectando...'],
        error:      ['dot--error',      'Error de conexión'],
    };
    const [cls, label] = states[state] || states.connecting;
    dot.className = `dot ${cls}`;
    text.textContent = label;
}

// ── Map markers ───────────────────────────────────────────────────────────
function updateMap(devices) {
    devices.forEach(dev => {
        const color = nodeColor(dev.status);
        const opts  = {
            radius: 11,
            color: '#04111f',
            weight: 2,
            fillColor: color,
            fillOpacity: dev.status === 'offline' ? 0.35 : 0.9,
        };

        if (markers[dev.id]) {
            markers[dev.id].setStyle(opts);
            markers[dev.id].setPopupContent(popupHTML(dev));
        } else {
            const m = L.circleMarker([dev.lat, dev.lng], opts);

            // Tooltip izquierdo al hover
            m.bindTooltip(dev.nombre, {
                direction: 'left',
                permanent: false,
                offset: [-14, 0],
            });

            // Popup resumen al click (aparece arriba del nodo)
            m.bindPopup(popupHTML(dev), {
                maxWidth: 220,
                offset: [0, -6],
                className: 'node-map-popup',
            });

            m.on('click', function () {
                m.openPopup();
                selectDevice(dev.id, lastDevicesCache);
            });

            m.addTo(map);
            markers[dev.id] = m;
        }
    });
}

function popupHTML(dev) {
    const ts   = dev.timestamp || '';
    const date = ts.split(' ')[0] || '--';
    const time = ts.split(' ')[1] || '--';
    const statusLabel = dev.status === 'activo' ? '● ACTIVO' : dev.status === 'alerta' ? '● ALERTA' : '○ OFFLINE';
    const statusCls   = dev.status === 'offline' ? 'offline' : dev.status === 'alerta' ? 'alerta' : 'activo';
    return `
        <div class="popup-name">${dev.nombre}</div>
        <div class="popup-temp">${tempLabel(dev.temperatura)}</div>
        <div class="popup-meta">
            <span class="popup-status ${statusCls}">${statusLabel}</span>
        </div>
        <div class="popup-meta popup-ts">
            <span>${date}</span><span>${time}</span>
        </div>
    `;
}

// ── Node list ─────────────────────────────────────────────────────────────
function updateNodeList(devices) {
    const list = document.getElementById('nodeList');
    list.innerHTML = '';
    devices.forEach(dev => {
        const card = document.createElement('div');
        card.className = `node-card ${dev.id === selectedId ? 'active' : ''}`;
        card.innerHTML = `
            <div class="node-indicator ${dev.status}"></div>
            <div class="node-info">
                <div class="node-name">${dev.nombre}</div>
                <div class="node-sub">${dev.tendencia ? trendArrow(dev.tendencia) + ' ' + dev.tendencia : dev.status}</div>
            </div>
            <div class="node-temp ${dev.temperatura === null ? 'offline' : ''}">${tempLabel(dev.temperatura)}</div>
        `;
        card.addEventListener('click', () => selectDevice(dev.id, devices));
        list.appendChild(card);
    });
}

// ── Select device ─────────────────────────────────────────────────────────
function selectDevice(id, devices) {
    selectedId = id;
    const dev = devices.find(d => d.id === id);
    if (!dev) return;

    // Highlight card
    document.querySelectorAll('.node-card').forEach(c => c.classList.remove('active'));
    document.querySelectorAll('.node-card').forEach(c => {
        if (c.querySelector('.node-name').textContent === dev.nombre) c.classList.add('active');
    });

    map.panTo([dev.lat, dev.lng]);
    updateDetail(dev);
    fetchHistorico(id);

    document.getElementById('historyPanel').classList.remove('hidden');
    document.getElementById('historyTitle').textContent = `Histórico — ${dev.nombre}`;
    setTimeout(() => map.invalidateSize(), 50);
}

// ── Detail panel ──────────────────────────────────────────────────────────
function updateDetail(dev) {
    document.getElementById('nodeDetail').classList.remove('hidden');
    document.getElementById('detailName').textContent = dev.nombre;

    const statusEl = document.getElementById('detailStatus');
    statusEl.textContent = dev.status;
    statusEl.className = `detail-status ${dev.status}`;

    document.getElementById('detailTemp').textContent =
        dev.temperatura !== null ? dev.temperatura.toFixed(2) : '--';

    setMetric('m-d1h',      fmtDelta(dev.delta_1h),       deltaClass(dev.delta_1h));
    setMetric('m-d1h-pct',  fmtPct(dev.delta_1h_pct),    deltaClass(dev.delta_1h));
    setMetric('m-d24h',     fmtDelta(dev.delta_24h),      deltaClass(dev.delta_24h));
    setMetric('m-d24h-pct', fmtPct(dev.delta_24h_pct),   deltaClass(dev.delta_24h));
    setMetric('m-trend',    trendArrow(dev.tendencia) + ' ' + (dev.tendencia || '--'), '');
    setMetric('m-pred',     dev.prediccion_1h !== null ? `${dev.prediccion_1h}°C` : '--', '');
    setMetric('m-anomaly',  dev.anomalia || '--', dev.anomalia === 'anomalía' ? 'warn' : '');
    setMetric('m-ts',       dev.timestamp || '--', '');
}

function setMetric(id, text, cls) {
    const el = document.getElementById(id);
    el.textContent = text;
    el.className = `metric-value ${cls}`;
}

// ── History ───────────────────────────────────────────────────────────────

// Detecta cortes de señal e inserta nulls para romper la línea en el gráfico
function insertSignalGaps(data) {
    if (data.length < 3) return data;

    // Calcula el intervalo mediano entre lecturas
    const intervals = [];
    for (let i = 1; i < data.length; i++) {
        intervals.push(new Date(data[i].timestamp) - new Date(data[i - 1].timestamp));
    }
    intervals.sort((a, b) => a - b);
    const median = intervals[Math.floor(intervals.length / 2)];
    const threshold = median * 3; // gap > 3x el intervalo normal = pérdida de señal

    const result = [];
    for (let i = 0; i < data.length; i++) {
        result.push(data[i]);
        if (i < data.length - 1) {
            const gap = new Date(data[i + 1].timestamp) - new Date(data[i].timestamp);
            if (gap > threshold) {
                // Punto nulo en el centro del gap para romper la línea
                const midMs  = (new Date(data[i].timestamp).getTime() + new Date(data[i + 1].timestamp).getTime()) / 2;
                const midTs  = new Date(midMs).toISOString().replace('T', ' ').slice(0, 19);
                result.push({ timestamp: midTs, temperatura: null, isGap: true });
            }
        }
    }
    return result;
}

function renderHistorico(rawData) {
    const data   = insertSignalGaps(rawData);
    const labels = data.map(r => r.timestamp.split(' ')[1]);
    const values = data.map(r => r.temperatura);

    if (!histChart) {
        const ctx = document.getElementById('histChart').getContext('2d');
        histChart = new Chart(ctx, {
            type: 'line',
            data: {
                labels,
                datasets: [{
                    label: 'Temperatura (°C)',
                    data: values,
                    borderColor: '#00cfff',
                    backgroundColor: 'rgba(0,207,255,0.06)',
                    borderWidth: 2,
                    tension: 0.3,
                    pointRadius: 2,
                    pointBackgroundColor: '#00cfff',
                    spanGaps: false,
                }],
            },
            options: {
                animation: false,
                responsive: true,
                maintainAspectRatio: false,
                scales: {
                    x: {
                        ticks: { color: '#4d7fa0', maxTicksLimit: 12, font: { family: 'Consolas, monospace', size: 11 } },
                        grid:  { color: '#103354' },
                    },
                    y: {
                        ticks: { color: '#4d7fa0', font: { family: 'Consolas, monospace', size: 11 } },
                        grid:  { color: '#103354' },
                        title: { display: true, text: '°C', color: '#4d7fa0', font: { size: 11 } },
                    },
                },
                plugins: {
                    legend: { labels: { color: '#cce8ff', font: { size: 11, family: 'Consolas, monospace' } } },
                },
            },
        });
    } else {
        histChart.data.labels = labels;
        histChart.data.datasets[0].data = values;
        histChart.update('none');
    }

    // Tabla (últimas 100, más reciente primero, omite puntos de gap)
    const tbody = document.querySelector('#histTable tbody');
    tbody.innerHTML = '';
    [...rawData].reverse().slice(0, 100).forEach(({ timestamp, temperatura }) => {
        const [date, time] = timestamp.split(' ');
        const tr = document.createElement('tr');
        tr.innerHTML = `<td>${date}</td><td>${time}</td><td>${temperatura.toFixed(2)}</td>`;
        tbody.appendChild(tr);
    });
}

// ── Formatters ────────────────────────────────────────────────────────────
function fmtDelta(v) {
    if (v === null || v === undefined) return '--';
    return (v >= 0 ? '+' : '') + v + '°C';
}

function fmtPct(v) {
    if (v === null || v === undefined) return '--';
    return (v >= 0 ? '+' : '') + v + '%';
}

function deltaClass(v) {
    if (v === null || v === undefined) return '';
    return v > 0 ? 'up' : v < 0 ? 'down' : '';
}

function trendArrow(t) {
    return t === 'subiendo' ? '↑' : t === 'bajando' ? '↓' : t === 'estable' ? '→' : '';
}
