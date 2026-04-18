/*
 * === Nodo Receptor - Sistema de Monitoreo con LoRa E220-900T30D ===
 * Este código recibe mensajes enviados por un nodo emisor (como datos de sensores)
 * y los imprime por el monitor serial.
 *
 * Conexión física con Arduino Nano:
 *
 * Módulo E220    ->    Arduino Nano         | Notas
 * ------------------------------------------|-----------------------------
 * M0             ->    Pin 7                | Selección de modo (junto con M1)
 * M1             ->    Pin 6                | Selección de modo (junto con M0)
 * TX (del LoRa)  ->    Pin 2 (RX Arduino)   | Entrada al Arduino (usa divisor de voltaje si necesario)
 * RX (del LoRa)  ->    Pin 3 (TX Arduino)   | Salida del Arduino (puede requerir nivel 3.3V)
 * AUX            ->    Pin 5                | Pin de estado (recomendado para confiabilidad)
 * VCC            ->    3.3V o 5V            | E220-900T30D soporta 3.3V–5.2V
 * GND            ->    GND                  | Tierra común
 */

// --------------------- Configuración del sistema ---------------------
#define E220_30             // Identificador del módulo
#define FREQUENCY_900       // Banda de operación 900MHz (canal base: 850MHz)
#define DESTINATION_ADDL 1  // Dirección propia (baja), debe coincidir con la configurada en el emisor

// #define ENABLE_RSSI true     // Descomentar si se desea medir la intensidad de señal

#include "Arduino.h"
#include "LoRa_E220.h"
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ------------- Arduino Nano 33 IoT -------------
LoRa_E220 e220ttl(2, 3, 5, 7, 6);  // Arduino RX <-- e220 TX, Arduino TX --> e220 RX AUX M0 M1
// -----------------------------------------------

// ------------ Configuración I2C LCD ------------
#define LCD_ADDR 0x27
#define LCD_COLS 16
#define LCD_ROWS 2

LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);
// -----------------------------------------------

void setup() {
  Serial.begin(9600);
  delay(500);  // Esperar a que inicie correctamente el monitor serial

  // Inicializar pantalla LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Receptor activo");

  // Inicialización del módulo LoRa
  e220ttl.begin();

  // Mensajes de estado
  Serial.println("=====================================");
  Serial.println("     Sistema de Monitoreo LoRa");
  Serial.println("       Nodo Terrestre Activo");
  Serial.println("=====================================");
  Serial.println("Esperando mensajes desde nodo emisor...");
  Serial.println("-------------------------------------");

  lcd.setCursor(0, 1);
  lcd.print("Esperando datos");
}

void loop() {
  if (e220ttl.available() > 1) {
    Serial.println("\nMensaje recibido:");

    ResponseContainer rc = e220ttl.receiveMessage();

    if (rc.status.code != 1) {
      Serial.print("Error: ");
      Serial.println(rc.status.getResponseDescription());
      lcd.setCursor(0, 1);
      lcd.print("Error RX        ");
    } else {
      // Serial.print("Estado: ");
      // Serial.println(rc.status.getResponseDescription());
      // Serial.print("Datos: ");
      Serial.println(rc.data);

      // Mostrar en LCD
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Temp Actual:");

      lcd.setCursor(0, 1);
      String msg = rc.data;
      msg.trim();
      msg = msg.substring(0, 16);  // Asegura que no se desborde
      lcd.print(msg);
    }
  }

  delay(200);  // Espera corta antes de volver a revisar
}
