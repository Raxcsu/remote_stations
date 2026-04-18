# Sistema de Monitoreo Acuático Remoto

**BLUME × IMARPE** — Plataforma de monitoreo de temperatura en tiempo real para entornos acuáticos mediante sensores IoT, transmisión LoRa y visualización web con mapa interactivo.

---

## Descripción

Sistema de telemetría diseñado para el monitoreo continuo de temperatura en cuerpos de agua. Los nodos sensores transmiten datos vía radio LoRa (915 MHz) hacia una estación receptora terrestre, que los persiste en una base de datos local y los expone en un dashboard web con mapa georreferenciado y análisis estadístico en tiempo real.

## Características

- **Transmisión inalámbrica** — LoRa 915 MHz con alcance extendido mediante módulos E220-900T30D
- **Persistencia local** — Base de datos SQLite, sin dependencias externas de nube
- **Dashboard en tiempo real** — Mapa interactivo (Esri Ocean) con actualización automática cada 5 segundos
- **Múltiples nodos** — Soporte para +20 dispositivos simultáneos
- **Análisis por nodo** — Δ temperatura (1h / 24h), tendencia, predicción +1h, detección de anomalías (2σ)
- **Histórico visual** — Gráfica con detección automática de pérdidas de señal
- **Estado del nodo** — Activo / Alerta / Offline según tiempo de última lectura

## Arquitectura

```
[Nodo Acuático]          [Nodo Terrestre]        [Servidor Local]
Arduino Nano             Arduino Nano             Python / Flask
DS18B20 sensor    ──►    LoRa E220        ──►     SQLite DB
LoRa E220                LCD 16x2                 API REST
                         Serial USB              
                                                         │
                                                         ▼
                                                  [Dashboard Web]
                                                  Mapa + Gráficas
                                                  http://localhost:5000
```

## Requisitos

### Hardware
- 2× Arduino Nano 33 IoT
- 2× Módulo LoRa E220-900T30D
- Sensor de temperatura DS18B20 (OneWire)
- RTC DS3231
- Display LCD 16×2 I2C

### Software
- Python 3.13+
- Arduino IDE
- Librerías Arduino: `LoRa_E220`, `OneWire`, `DallasTemperature`, `RTClib`, `LiquidCrystal_I2C`

## Instalación

```bash
git clone https://github.com/<usuario>/sistema-remoto.git
cd sistema-remoto
```

Luego ejecuta el setup (crea el entorno virtual, instala dependencias y registra los nodos en la DB):

```bash
setup.bat
```

Eso es todo. El script reporta cada paso y muestra cómo iniciar el sistema al terminar.

## Uso

### Modo desarrollo (sin hardware)

Abre dos terminales:

```bash
# Terminal 1 — servidor web
.venv/Scripts/python server.py

# Terminal 2 — simulador de nodos
.venv/Scripts/python data/scripts/simulador.py
```

Accede a `http://localhost:5000`.

### Modo producción (con Arduino)

Configura el puerto y el nodo asignado en `data/scripts/lora_reader.py`:

```python
PORT               = "COM14"          # puerto COM del Arduino receptor
DISPOSITIVO_NOMBRE = "IMARPE-Callao"  # nombre del nodo en la base de datos
```

> El nodo debe existir en la DB antes de correr el reader. Corre el simulador al menos una vez para registrarlo, luego puedes detenerlo.

```bash
# Terminal 1 — servidor web
.venv/Scripts/python server.py

# Terminal 2 — lectura serial del Arduino
.venv/Scripts/python data/scripts/lora_reader.py
```

### Cargar firmware Arduino

1. Abrir `arduino/system/nodo_sender/nodo_sender.ino` → cargar al nodo acuático
2. Abrir `arduino/system/nodo_receiver/nodo_receiver.ino` → cargar al nodo terrestre

## Estructura del proyecto

```
sistema-remoto/
├── server.py                   # API Flask + servidor de archivos estáticos
├── dashboard/
│   ├── index.html              # Interfaz principal
│   ├── app.js                  # Lógica: mapa Leaflet, Chart.js, polling
│   └── style.css               # Tema dark navy
├── data/
│   ├── temperatura.db          # Base de datos SQLite (auto-generada)
│   └── scripts/
│       ├── simulador.py        # Generador de datos sintéticos (desarrollo)
│       └── lora_reader.py      # Lector serial (producción)
├── arduino/
│   ├── system/
│   │   ├── nodo_sender/        # Firmware nodo acuático
│   │   └── nodo_receiver/      # Firmware nodo terrestre
│   └── lora_e220/              # Utilidades de configuración LoRa
└── docs/
    └── Blume - Imarpe Proyecto.pdf
```

## API

| Endpoint | Parámetros | Descripción |
|---|---|---|
| `GET /api/dispositivos` | — | Todos los nodos con métricas actuales |
| `GET /api/lecturas` | `dispositivo_id`, `horas` | Histórico de lecturas |

### Respuesta `/api/dispositivos`

```json
[
  {
    "id": 1,
    "nombre": "IMARPE-Callao",
    "lat": -12.05,
    "lng": -77.16,
    "temperatura": 22.4,
    "status": "activo",
    "delta_1h": 0.8,
    "delta_1h_pct": 3.6,
    "tendencia": "subiendo",
    "prediccion_1h": 23.1,
    "anomalia": "normal",
    "timestamp": "2026-04-18 14:32:01"
  }
]
```

## Configuración de hardware

### Pines Arduino (ambos nodos)

| Componente | Pin |
|---|---|
| DS18B20 OneWire | 8 |
| LoRa E220 AUX | 2 |
| LoRa E220 M0 | 4 |
| LoRa E220 M1 | 5 |
| SoftwareSerial TX | 6 |
| SoftwareSerial RX | 7 |
| LCD SDA | A4 |
| LCD SCL | A5 |

### Parámetros LoRa

| Parámetro | Valor |
|---|---|
| Frecuencia | 915 MHz (canal 65) |
| Baudrate | 9600 |
| Dirección nodo acuático | 1 |
| Dirección nodo terrestre | 2 |

## Colaboradores

| Organización | Rol |
|---|---|
| [BLUME](https://blume.pe) | Desarrollo del sistema |
| [IMARPE](https://www.gob.pe/imarpe) | Investigación y despliegue |

## Licencia

Uso restringido — proyecto en desarrollo. Contactar a [oscar@blume.pe](mailto:oscar@blume.pe) para más información.
