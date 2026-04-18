/*
 * === Conexión del módulo LoRa E220-900T30D al Arduino Nano ===
 *
 * Módulo E220    ->    Arduino Nano         | Notas
 * ------------------------------------------|-----------------------------
 * M0             ->    Pin 7                | Selección de modo (junto con M1)
 * M1             ->    Pin 6                | Selección de modo (junto con M0)
 * TX (del LoRa)  ->    Pin 2 (RX Arduino)   | Entrada al Arduino (usar divisor de voltaje si es necesario)
 * RX (del LoRa)  ->    Pin 3 (TX Arduino)   | Salida desde el Arduino (puede requerir conversión a 3.3V)
 * AUX            ->    Pin 5                | Pin de estado (recomendado para confirmar transmisiones)
 * VCC            ->    3.3V o 5V            | El E220-900T30D soporta 3.3V–5.2V
 * GND            ->    GND                  | Tierra común
 */

// ------------------------ Definiciones del sistema ------------------------
#define E220_30              // Identificador del módulo usado
#define FREQUENCY_900        // Para que interprete canal como MHz (base 850MHz)
#define DESTINATION_ADDL 2   // Dirección destino (baja), configurada en el receptor
#define OPERATIONS_CHANN 65  // Canal de operación: 850 + 65 = 915 MHz
#define ONE_WIRE_BUS 8       // Pin de datos del sensor DS18B20


// #define ENABLE_RSSI true     // Descomentar si usas RSSI y lo habilitaste en configuración

#include "Arduino.h"
#include "LoRa_E220.h"
#include <OneWire.h>
#include <DallasTemperature.h>

// ------------------- Inicialización del sensor -------------------
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

// ------------- Arduino Nano 33 IoT -------------
LoRa_E220 e220ttl(2, 3, 5, 7, 6);  // Arduino RX <-- e220 TX, Arduino TX --> e220 RX AUX M0 M1
// -------------------------------------------------

void setup() {
  Serial.begin(9600);
  delay(500);  // Espera para estabilizar el monitor serial

  // Inicialización del módulo LoRa
  e220ttl.begin();

  // Inicializar sensor de temperatura
  sensors.begin();

  // Mensajes de estado para diagnóstico por puerto serial
  Serial.println("=====================================");
  Serial.println("     Sistema de Monitoreo LoRa");
  Serial.println("        Nodo Acuático Activo");
  Serial.println("=====================================");
  Serial.println("Inicializando comunicación LoRa...");
  delay(200);

  // Envío de mensajes de confirmación al nodo receptor
  ResponseStatus rs;
  rs = e220ttl.sendFixedMessage(0, DESTINATION_ADDL, OPERATIONS_CHANN, "Mensaje de confirmacion enviado");
  Serial.println("TX 1: " + rs.getResponseDescription());

  rs = e220ttl.sendFixedMessage(0, DESTINATION_ADDL, OPERATIONS_CHANN, "Sistema de monitoreo operativo");
  Serial.println("TX 2: " + rs.getResponseDescription());

  Serial.println("Inicialización completa.");
  Serial.println("-------------------------------------");
}

void loop() {
  // Pedir medición al sensor
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);

  // Validar si la lectura es válida
  if (tempC == DEVICE_DISCONNECTED_C) {
    Serial.println("Sensor desconectado");
    return;
  }

  // Construir mensaje
  String mensaje = "TempC:" + String(tempC, 2) + "C";

  // Enviar mensaje vía LoRa
  ResponseStatus rs = e220ttl.sendFixedMessage(0, DESTINATION_ADDL, OPERATIONS_CHANN, mensaje);

  // Mostrar resultado en consola
  // Serial.println("Enviado: " + mensaje + " | Estado: " + rs.getResponseDescription());
  Serial.println(mensaje);

  delay(5000);  // Esperar 2 segundos antes de la próxima lectura
}
