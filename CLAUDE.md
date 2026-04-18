# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

IoT remote temperature monitoring system for aquatic environments — a BLUME x IMARPE collaboration. Collects temperature data via DS18B20 sensors on Arduino nodes, transmits over LoRa 915 MHz, persists in SQLite, and visualizes via a live web dashboard with map.

## Architecture

Four components in the data pipeline:

1. **Aquatic Node** (`arduino/system/nodo_sender/`) — Arduino Nano + DS18B20 + LoRa E220. Reads temperature every ~5 seconds, transmits `"TempC:XX.XC"` over LoRa at 915 MHz (channel 65) to address 2.

2. **Terrestrial Node** (`arduino/system/nodo_receiver/`) — Arduino Nano + LoRa E220 + LCD. Receives LoRa packets, shows on 16x2 I2C LCD, outputs to Serial at 9600 baud.

3. **Data Acquisition** (`data/scripts/`) — Python scripts read serial or simulate data and write to SQLite.
   - `lora_reader.py` — production: reads serial port → writes to DB
   - `simulador.py` — development: generates synthetic data for all nodes → writes to DB

4. **Web Platform** (`dashboard/` + `server.py`) — Flask serves the dashboard and a JSON API. The frontend polls the API every 5 seconds and renders a live map + charts.

## Running the System

Two terminals required:

**Terminal 1 — Flask server:**
```bash
.venv/Scripts/python server.py
```

**Terminal 2 — simulator (development) or reader (production):**
```bash
.venv/Scripts/python data/scripts/simulador.py     # development
.venv/Scripts/python data/scripts/lora_reader.py   # production (Arduino connected)
```

Open `http://localhost:5000` in the browser.

## Folder Structure

```
2_sistema_remoto/
├── server.py              # Flask API + static file server
├── dashboard/             # Frontend
│   ├── index.html
│   ├── app.js             # Leaflet map + Chart.js + polling logic
│   └── style.css          # Dark navy theme
├── data/
│   ├── temperatura.db     # SQLite database (auto-created)
│   ├── logs/              # Legacy .txt logs
│   └── scripts/
│       ├── simulador.py   # Multi-node synthetic data generator
│       └── lora_reader.py # Production serial reader
├── arduino/
│   ├── system/            # Production firmware (sender + receiver)
│   └── lora_e220/         # LoRa module configuration sketches
└── docs/
```

## API Endpoints

| Endpoint | Description |
|---|---|
| `GET /api/dispositivos` | All nodes with latest reading + computed stats |
| `GET /api/lecturas?dispositivo_id=X&horas=24` | Historical readings for a node |

Stats returned per node: `temperatura`, `status`, `delta_1h`, `delta_1h_pct`, `delta_24h`, `delta_24h_pct`, `tendencia`, `prediccion_1h`, `anomalia`.

Node `status`: `activo` (<10 min since last reading), `alerta` (10–30 min), `offline` (>30 min).

## Database Schema

```sql
CREATE TABLE dispositivos (
    id     INTEGER PRIMARY KEY AUTOINCREMENT,
    nombre TEXT NOT NULL,
    lat    REAL NOT NULL,
    lng    REAL NOT NULL
);
CREATE TABLE lecturas (
    id             INTEGER PRIMARY KEY AUTOINCREMENT,
    dispositivo_id INTEGER NOT NULL REFERENCES dispositivos(id),
    timestamp      TEXT NOT NULL,      -- "YYYY-MM-DD HH:MM:SS"
    temperatura    REAL NOT NULL
);
```

To reset the database: `rm data/temperatura.db` — it will be recreated on next run.

## Dashboard Features

- **Map** (Esri Ocean tiles, 2/3 width): circle markers colored green (active) or grey (offline). Click a marker → popup with name, temperature, status, timestamp. Tooltip with node name on hover (left side). Auto-zooms to fit all nodes on first load.
- **Side panel** (1/3 width): node list with live temperatures + trend arrows. Detail panel with metrics when a node is selected.
- **History panel** (bottom, shown on node select): Chart.js line chart with signal-loss gap detection (null points inserted where gap > 3× median interval). Filterable by 6h / 12h / 24h. Table with last 100 readings.

## Configuration

**Serial port y nodo asignado** — configurar en `data/scripts/lora_reader.py`:
```python
PORT               = "COM14"          # puerto COM del Arduino receptor
BAUDRATE           = 9600
DISPOSITIVO_NOMBRE = "IMARPE-Callao"  # debe coincidir exactamente con la tabla dispositivos
```

`lora_reader.py` resuelve el `dispositivo_id` por nombre al arrancar. Si el nombre no existe en la DB lanza un error antes de abrir el puerto serial. Requisito: correr el simulador al menos una vez (o insertar el dispositivo manualmente) para que el nodo esté registrado.

**Simulated nodes** — defined in `data/scripts/simulador.py` as `DISPOSITIVOS` list with `nombre`, `lat`, `lng`. Current nodes: IMARPE-Callao, Nodo-Huacho, Nodo-Chimbote, Nodo-Paracas, Nodo-Pisco.

**LoRa addressing** — sender uses `DESTINATION_ADDL = 2`, receiver uses address 1. Both on channel 65 (915 MHz). Configuration sketches in `arduino/lora_e220/`.

**Arduino hardware pins:**
- DS18B20 OneWire data: pin 8
- LoRa E220 AUX: pin 2, M0: pin 4, M1: pin 5 (SoftwareSerial TX: 6, RX: 7)
- LCD I2C address: `0x27`, 16×2

## Python Environment

Python 3.13. Virtual environment at `.venv/`. Packages: `flask`, `pyserial`, `matplotlib`, `PyYAML`, `future`, `iso8601`.

Rebuild venv:
```bash
rm -rf .venv && python -m venv .venv
.venv/Scripts/pip install flask pyserial matplotlib PyYAML future iso8601
```

## Arduino Sketches

Upload `.ino` files using Arduino IDE targeting **Arduino Nano 33 IoT**. Required libraries: `LoRa_E220`, `OneWire`, `DallasTemperature`, `RTClib`, `LiquidCrystal_I2C`.

Utility sketches: `arduino/onewire/` (DS18B20 sensor test), `arduino/rtc_setup/` (DS3231 RTC initialization).
