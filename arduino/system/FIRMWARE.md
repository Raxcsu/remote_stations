# Firmware — Nodos del Sistema

Documentación técnica de los dos sketches de producción que conforman el sistema de telemetría acuática.

---

## Nodo Acuático (`nodo_sender`)

Mide temperatura y la transmite vía LoRa cada 5 segundos.

### Flujo de ejecución

```
setup()
  │
  ├── Serial.begin(9600)
  ├── e220ttl.begin()          ← inicializa módulo LoRa
  ├── sensors.begin()          ← inicializa sensor DS18B20
  └── sendFixedMessage × 2     ← mensajes de confirmación al receptor

loop()  [cada 5 segundos]
  │
  ├── sensors.requestTemperatures()
  ├── tempC = getTempCByIndex(0)
  ├── [si DEVICE_DISCONNECTED → return]
  ├── mensaje = "TempC:XX.XXC"
  └── sendFixedMessage(0, DEST=2, CANAL=65, mensaje)
```

### Formato del mensaje transmitido

```
TempC:22.50C
```

El receptor y el script Python esperan exactamente este formato: prefijo `TempC:`, valor decimal con 2 decimales, sufijo `C`.

### Configuración LoRa

| Parámetro | Valor | Nota |
|---|---|---|
| Dirección destino (`DESTINATION_ADDL`) | `2` | Dirección del nodo receptor |
| Canal (`OPERATIONS_CHANN`) | `65` | 850 + 65 = **915 MHz** |
| Módulo | `E220_30` | E220-900T30D |
| Banda | `FREQUENCY_900` | Base 850 MHz |

### Conexión de pines

| Módulo E220 | Arduino Nano | Función |
|---|---|---|
| M0 | 7 | Selección de modo |
| M1 | 6 | Selección de modo |
| TX | 2 (RX Arduino) | Recepción de datos |
| RX | 3 (TX Arduino) | Transmisión de datos |
| AUX | 5 | Estado del módulo |
| VCC | 3.3V – 5V | Alimentación |
| GND | GND | Tierra |

| Sensor DS18B20 | Arduino Nano |
|---|---|
| DATA | Pin **8** (OneWire) |

### Librerías requeridas

- `LoRa_E220` — driver del módulo E220
- `OneWire` — protocolo 1-Wire
- `DallasTemperature` — comunicación con DS18B20

---

## Nodo Terrestre (`nodo_receiver`)

Recibe los mensajes LoRa, los muestra en el LCD y los imprime por Serial para que el script Python los capture.

### Flujo de ejecución

```
setup()
  │
  ├── Serial.begin(9600)
  ├── lcd.init() + lcd.backlight()
  ├── lcd.print("Receptor activo")
  └── e220ttl.begin()

loop()  [polling cada 200 ms]
  │
  ├── [si e220ttl.available() > 1]
  │     │
  │     ├── receiveMessage()
  │     ├── [si error → Serial + LCD "Error RX"]
  │     └── [si ok]
  │           ├── Serial.println(rc.data)   ← capturado por lora_reader.py
  │           └── lcd.print(rc.data[0:16])  ← muestra en pantalla
  │
  └── delay(200)
```

### Salida Serial

El nodo imprime el mensaje recibido completo en cada recepción:

```
Mensaje recibido:
TempC:22.50C
```

El script `lora_reader.py` filtra líneas que comiencen con `TempC:` y terminen en `C` para extraer el valor numérico.

### Configuración LoRa

| Parámetro | Valor | Nota |
|---|---|---|
| Dirección propia (`DESTINATION_ADDL`) | `1` | Debe coincidir con el `DESTINATION_ADDL` del sender |
| Módulo | `E220_30` | E220-900T30D |
| Banda | `FREQUENCY_900` | Base 850 MHz |

> El canal no se define en el receiver porque queda configurado en la EEPROM del módulo. Usar los sketches de `arduino/lora_e220/set_config` para configurarlo a canal 65 (915 MHz) antes del despliegue.

### Conexión de pines

| Módulo E220 | Arduino Nano | Función |
|---|---|---|
| M0 | 7 | Selección de modo |
| M1 | 6 | Selección de modo |
| TX | 2 (RX Arduino) | Recepción de datos |
| RX | 3 (TX Arduino) | Transmisión de datos |
| AUX | 5 | Estado del módulo |
| VCC | 3.3V – 5V | Alimentación |
| GND | GND | Tierra |

| LCD I2C | Arduino Nano |
|---|---|
| SDA | A4 |
| SCL | A5 |
| Dirección I2C | `0x27` |
| Tamaño | 16 columnas × 2 filas |

### Librerías requeridas

- `LoRa_E220` — driver del módulo E220
- `Wire` — protocolo I2C
- `LiquidCrystal_I2C` — comunicación con el LCD

---

## Relación entre nodos

```
nodo_sender                          nodo_receiver
────────────                         ─────────────
DESTINATION_ADDL = 2  ──── RF ────►  DESTINATION_ADDL = 1
OPERATIONS_CHANN  = 65               (canal configurado en EEPROM)
```

Ambos nodos usan la misma asignación de pines para el módulo LoRa E220. La diferencia es que el sender incluye el sensor DS18B20 y el receiver incluye el LCD I2C.

---

## Oportunidades de mejora

### Críticas para producción

| # | Nodo | Mejora | Motivo |
|---|---|---|---|
| 1 | Sender | **Gestión de energía** — reemplazar `delay(5000)` por sleep profundo (`LowPower.powerDown`) | El nodo acuático irá a batería; el delay activo consume igual que en operación, reducción potencial de ~95% entre lecturas |
| 2 | Ambos | **Watchdog timer** — habilitar `wdt_enable(WDTO_8S)` con reset periódico en el loop | Sin watchdog, un cuelgue por buffer LoRa lleno o bus I2C bloqueado requiere intervención manual |
| 3 | Receiver | **Salida Serial limpia** — eliminar o condicionar el encabezado `"Mensaje recibido:\n"` | Si se activan las líneas de debug comentadas, el parser Python recibe líneas inesperadas y puede fallar |

### Importantes para confiabilidad

| # | Nodo | Mejora | Motivo |
|---|---|---|---|
| 4 | Sender | **Número de secuencia** — incluir contador en el mensaje (`TempC:22.50C,SEQ:142`) | Sin él no hay forma de medir la tasa real de paquetes perdidos en tránsito |
| 5 | Sender | **Reintentos en fallo de TX** — hasta 3 intentos con backoff si `rs.code != 1` | Actualmente un fallo transitorio descarta el dato sin aviso ni recuperación |
| 6 | Receiver | **LCD con formato legible** — parsear el valor y mostrar `Temp: 22.50 C` en lugar del string crudo `TempC:22.50C` | Mejora la legibilidad en campo sin cambiar la lógica de transmisión |
| 7 | Sender | **Usar `char[]` + `sprintf`** en lugar de `String` para construir el mensaje | La clase `String` genera fragmentación de heap en Arduino con el tiempo; `sprintf` sobre buffer fijo no tiene allocations dinámicas |

### Mejoras de visibilidad

| # | Nodo | Mejora | Motivo |
|---|---|---|---|
| 8 | Ambos | **Activar RSSI** — descomentar `#define ENABLE_RSSI true` e incluirlo en la salida serial | Permite monitorear la calidad de señal por nodo en el dashboard y diagnosticar problemas de distancia u obstáculos |
| 9 | Sender | **Timestamp desde RTC** — incluir hora del DS3231 en el mensaje (`TempC:22.50C,TS:14:32:01`) cuando se incorpore el módulo | Elimina la dependencia del reloj de la PC para el timestamp y permite correlación precisa de datos offline |

---

## Preparación antes del despliegue

1. **Configurar EEPROM del módulo LoRa** — cargar `arduino/lora_e220/set_config` en cada Arduino para escribir canal 65, dirección y baudrate en la memoria del módulo.
2. **Verificar dirección** — sender con `DESTINATION_ADDL = 2`, receiver con `DESTINATION_ADDL = 1`.
3. **Verificar sensor** — usar `arduino/onewire/onewire.ino` para confirmar que el DS18B20 responde antes de instalar el nodo acuático.
4. **Verificar RTC** — usar `arduino/rtc_setup/rtc_setup.ino` para sincronizar el DS3231 si se incorpora en el diseño futuro.
5. **Puerto Serial** — al conectar el receiver a la PC, verificar el COM asignado y actualizar `PORT` en `data/scripts/lora_reader.py`.
