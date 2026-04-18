/*
 * === Conexión del módulo LoRa E220-900T30D al Arduino Nano ===
 *
 * Módulo E220    ->    Arduino Nano         | Notas
 * ------------------------------------------|-----------------------------
 * M0             ->    Pin 7                | Selección de modo (junto con M1)
 * M1             ->    Pin 6                | Selección de modo (junto con M0)
 * TX (del LoRa)  ->    Pin 2 (RX Arduino)   | Entrada al Arduino (usa divisor de voltaje si necesario)
 * RX (del LoRa)  ->    Pin 3 (TX Arduino)   | Salida del Arduino (puede requerir nivel 3.3V)
 * AUX            ->    Pin 5                | Pin de estado (opcional, pero recomendado)
 * VCC            ->    3.3V o 5V            | Según el modelo, E220-900T30D soporta 3.3V–5.2V
 * GND            ->    GND                  | Tierra común
 */

#define E220_30
#define FREQUENCY_900

// With FIXED SENDER configuration
// #define DESTINATION_ADDL 1

// With FIXED RECEIVER configuration
#define DESTINATION_ADDL 2

#define OPERATIONS_CHANN 65

// If you want use RSSI uncomment //#define ENABLE_RSSI true
// and use relative configuration with RSSI enabled
//#define ENABLE_RSSI true

#include "Arduino.h"
#include "LoRa_E220.h"

// ------------- Arduino Nano 33 IoT -------------
LoRa_E220 e220ttl(2, 3, 5, 7, 6);  // Arduino RX <-- e220 TX, Arduino TX --> e220 RX AUX M0 M1
// -------------------------------------------------

int counter = 0;

void printParameters(struct Configuration configuration);

void setup() {
  Serial.begin(9600);
  delay(500);

  // Startup all pins and UART
  e220ttl.begin();

  Serial.println("Hi, I'm going to send message!");

  // Send message
  ResponseStatus rs = e220ttl.sendFixedMessage(0, DESTINATION_ADDL, OPERATIONS_CHANN, "Hello, world?");
  // Check If there is some problem of succesfully send
  Serial.println(rs.getResponseDescription());
}

void loop() {
  // // If something available
  // if (e220ttl.available() > 1) {
  //   // read the String message
  // #ifdef ENABLE_RSSI
  //   ResponseContainer rc = e220ttl.receiveMessageRSSI();
  // #else
  //   ResponseContainer rc = e220ttl.receiveMessage();
  // #endif
  //   // Is something goes wrong print error
  //   if (rc.status.code != 1) {
  //     Serial.println(rc.status.getResponseDescription());
  //   } else {
  //     // Print the data received
  //     Serial.println(rc.status.getResponseDescription());
  //     Serial.println(rc.data);
  // #ifdef ENABLE_RSSI
  //     Serial.print("RSSI: ");
  //     Serial.println(rc.rssi, DEC);
  // #endif
  //   }
  // }
  // if (Serial.available()) {
  //   String input = Serial.readString();
  //   ResponseStatus rs = e220ttl.sendFixedMessage(0, DESTINATION_ADDL, OPERATIONS_CHANN, input);
  //   // Check If there is some problem of succesfully send
  //   Serial.println(rs.getResponseDescription());
  // }

  String input = String(counter);
  ResponseStatus rs = e220ttl.sendFixedMessage(0, DESTINATION_ADDL, OPERATIONS_CHANN, input);
  // Check If there is some problem of succesfully send
  Serial.println(rs.getResponseDescription());

  counter++;
  Serial.println(counter);

  delay(1000);

}