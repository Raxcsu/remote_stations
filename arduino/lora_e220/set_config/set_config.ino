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

#include "Arduino.h"
#include "LoRa_E220.h"

// ------------- Arduino Nano 33 IoT -------------
LoRa_E220 e220ttl(2, 3, 5, 7, 6);  // Arduino RX <-- e220 TX, Arduino TX --> e220 RX AUX M0 M1
// -------------------------------------------------

void printParameters(struct Configuration configuration);
void printModuleInformation(struct ModuleInformation moduleInformation);

void setup() {
  Serial.begin(9600);
  while (!Serial) {};
  delay(500);

  Serial.println();


  // Startup all pins and UART
  e220ttl.begin();

  ResponseStructContainer c;
  c = e220ttl.getConfiguration();
  // It's important get configuration pointer before all other operation
  Configuration configuration = *(Configuration*)c.data;
  Serial.println(c.status.getResponseDescription());
  Serial.println(c.status.code);

  printParameters(configuration);

  //	----------------------- DEFAULT TRANSPARENT -----------------------
  configuration.ADDL = 0x01;  // First part of address
  configuration.ADDH = 0x00;  // Second part

  configuration.CHAN = 65;  // Communication channel

  configuration.SPED.uartBaudRate = UART_BPS_9600;        // Serial baud rate
  configuration.SPED.airDataRate = AIR_DATA_RATE_010_24;  // Air baud rate
  configuration.SPED.uartParity = MODE_00_8N1;            // Parity bit

  configuration.OPTION.subPacketSetting = SPS_200_00;                   // Packet size
  configuration.OPTION.RSSIAmbientNoise = RSSI_AMBIENT_NOISE_DISABLED;  // Need to send special command
  configuration.OPTION.transmissionPower = POWER_30;                    // Device power

  configuration.TRANSMISSION_MODE.enableRSSI = RSSI_DISABLED;                       // Enable RSSI info
  configuration.TRANSMISSION_MODE.fixedTransmission = FT_TRANSPARENT_TRANSMISSION;  // Enable repeater mode
  configuration.TRANSMISSION_MODE.enableLBT = LBT_DISABLED;                         // Check interference
  configuration.TRANSMISSION_MODE.WORPeriod = WOR_2000_011;                         // WOR timing
  //	----------------------- DEFAULT TRANSPARENT WITH RSSI -----------------------
  //	configuration.ADDL = 0x03;
  //	configuration.ADDH = 0x00;
  //
  //	configuration.CHAN = 23;
  //
  //	configuration.SPED.uartBaudRate = UART_BPS_9600;
  //	configuration.SPED.airDataRate = AIR_DATA_RATE_010_24;
  //	configuration.SPED.uartParity = MODE_00_8N1;
  //
  //	configuration.OPTION.subPacketSetting = SPS_200_00;
  //	configuration.OPTION.RSSIAmbientNoise = RSSI_AMBIENT_NOISE_DISABLED;
  //	configuration.OPTION.transmissionPower = POWER_22;
  //
  //	configuration.TRANSMISSION_MODE.enableRSSI = RSSI_ENABLED;
  //	configuration.TRANSMISSION_MODE.fixedTransmission = FT_TRANSPARENT_TRANSMISSION;
  //	configuration.TRANSMISSION_MODE.enableLBT = LBT_DISABLED;
  //	configuration.TRANSMISSION_MODE.WORPeriod = WOR_2000_011;
  //	----------------------- FIXED SENDER -----------------------
  //	configuration.ADDL = 0x02;
  //	configuration.ADDH = 0x00;
  //
  //	configuration.CHAN = 23;
  //
  //	configuration.SPED.uartBaudRate = UART_BPS_9600;
  //	configuration.SPED.airDataRate = AIR_DATA_RATE_010_24;
  //	configuration.SPED.uartParity = MODE_00_8N1;
  //
  //	configuration.OPTION.subPacketSetting = SPS_200_00;
  //	configuration.OPTION.RSSIAmbientNoise = RSSI_AMBIENT_NOISE_DISABLED;
  //	configuration.OPTION.transmissionPower = POWER_22;
  //
  //	configuration.TRANSMISSION_MODE.enableRSSI = RSSI_DISABLED;
  //	configuration.TRANSMISSION_MODE.fixedTransmission = FT_FIXED_TRANSMISSION;
  //	configuration.TRANSMISSION_MODE.enableLBT = LBT_DISABLED;
  //	configuration.TRANSMISSION_MODE.WORPeriod = WOR_2000_011;
  //
  //	----------------------- FIXED RECEIVER -----------------------
  //	configuration.ADDL = 0x03;
  //	configuration.ADDH = 0x00;
  //
  //	configuration.CHAN = 23;
  //
  //	configuration.SPED.uartBaudRate = UART_BPS_9600;
  //	configuration.SPED.airDataRate = AIR_DATA_RATE_010_24;
  //	configuration.SPED.uartParity = MODE_00_8N1;
  //
  //	configuration.OPTION.subPacketSetting = SPS_200_00;
  //	configuration.OPTION.RSSIAmbientNoise = RSSI_AMBIENT_NOISE_DISABLED;
  //	configuration.OPTION.transmissionPower = POWER_22;
  //
  //	configuration.TRANSMISSION_MODE.enableRSSI = RSSI_DISABLED;
  //	configuration.TRANSMISSION_MODE.fixedTransmission = FT_FIXED_TRANSMISSION;
  //	configuration.TRANSMISSION_MODE.enableLBT = LBT_DISABLED;
  //	configuration.TRANSMISSION_MODE.WORPeriod = WOR_2000_011;
  //	----------------------- FIXED SENDER RSSI -----------------------
  //	configuration.ADDL = 0x02;
  //	configuration.ADDH = 0x00;
  //
  //	configuration.CHAN = 23;
  //
  //	configuration.SPED.uartBaudRate = UART_BPS_9600;
  //	configuration.SPED.airDataRate = AIR_DATA_RATE_010_24;
  //	configuration.SPED.uartParity = MODE_00_8N1;
  //
  //	configuration.OPTION.subPacketSetting = SPS_200_00;
  //	configuration.OPTION.RSSIAmbientNoise = RSSI_AMBIENT_NOISE_DISABLED;
  //	configuration.OPTION.transmissionPower = POWER_22;
  //
  //	configuration.TRANSMISSION_MODE.enableRSSI = RSSI_ENABLED;
  //	configuration.TRANSMISSION_MODE.fixedTransmission = FT_FIXED_TRANSMISSION;
  //	configuration.TRANSMISSION_MODE.enableLBT = LBT_DISABLED;
  //	configuration.TRANSMISSION_MODE.WORPeriod = WOR_2000_011;
  //
  //	----------------------- FIXED RECEIVER RSSI -----------------------
  //	configuration.ADDL = 0x03;
  //	configuration.ADDH = 0x00;
  //
  //	configuration.CHAN = 23;
  //
  //	configuration.SPED.uartBaudRate = UART_BPS_9600;
  //	configuration.SPED.airDataRate = AIR_DATA_RATE_010_24;
  //	configuration.SPED.uartParity = MODE_00_8N1;
  //
  //	configuration.OPTION.subPacketSetting = SPS_200_00;
  //	configuration.OPTION.RSSIAmbientNoise = RSSI_AMBIENT_NOISE_DISABLED;
  //	configuration.OPTION.transmissionPower = POWER_22;
  //
  //	configuration.TRANSMISSION_MODE.enableRSSI = RSSI_ENABLED;
  //	configuration.TRANSMISSION_MODE.fixedTransmission = FT_FIXED_TRANSMISSION;
  //	configuration.TRANSMISSION_MODE.enableLBT = LBT_DISABLED;
  //	configuration.TRANSMISSION_MODE.WORPeriod = WOR_2000_011;
  //
  //	----------------------- WOR SENDER -----------------------
  //	configuration.ADDL = 0x02;
  //	configuration.ADDH = 0x00;
  //
  //	configuration.CHAN = 23;
  //
  //	configuration.SPED.uartBaudRate = UART_BPS_9600;
  //	configuration.SPED.airDataRate = AIR_DATA_RATE_010_24;
  //	configuration.SPED.uartParity = MODE_00_8N1;
  //
  //	configuration.OPTION.subPacketSetting = SPS_200_00;
  //	configuration.OPTION.RSSIAmbientNoise = RSSI_AMBIENT_NOISE_DISABLED;
  //	configuration.OPTION.transmissionPower = POWER_22;
  //
  //	configuration.TRANSMISSION_MODE.enableRSSI = RSSI_DISABLED;
  //	configuration.TRANSMISSION_MODE.fixedTransmission = FT_FIXED_TRANSMISSION;
  //	configuration.TRANSMISSION_MODE.enableLBT = LBT_DISABLED;
  //	configuration.TRANSMISSION_MODE.WORPeriod = WOR_2000_011;
  //
  //	----------------------- WOR RECEIVER -----------------------
  //	configuration.ADDL = 0x03;
  //	configuration.ADDH = 0x00;
  //
  //	configuration.CHAN = 23;
  //
  //	configuration.SPED.uartBaudRate = UART_BPS_9600;
  //	configuration.SPED.airDataRate = AIR_DATA_RATE_010_24;
  //	configuration.SPED.uartParity = MODE_00_8N1;
  //
  //	configuration.OPTION.subPacketSetting = SPS_200_00;
  //	configuration.OPTION.RSSIAmbientNoise = RSSI_AMBIENT_NOISE_DISABLED;
  //	configuration.OPTION.transmissionPower = POWER_22;
  //
  //	configuration.TRANSMISSION_MODE.enableRSSI = RSSI_DISABLED;
  //	configuration.TRANSMISSION_MODE.fixedTransmission = FT_FIXED_TRANSMISSION;
  //	configuration.TRANSMISSION_MODE.enableLBT = LBT_DISABLED;
  //	configuration.TRANSMISSION_MODE.WORPeriod = WOR_2000_011;
  //	----------------------- BROADCAST MESSAGE 1 -----------------------
  //	configuration.ADDL = 0x04;
  //	configuration.ADDH = 0x00;
  //
  //	configuration.CHAN = 23;
  //
  //	configuration.SPED.uartBaudRate = UART_BPS_9600;
  //	configuration.SPED.airDataRate = AIR_DATA_RATE_010_24;
  //	configuration.SPED.uartParity = MODE_00_8N1;
  //
  //	configuration.OPTION.subPacketSetting = SPS_200_00;
  //	configuration.OPTION.RSSIAmbientNoise = RSSI_AMBIENT_NOISE_DISABLED;
  //	configuration.OPTION.transmissionPower = POWER_22;
  //
  //	configuration.TRANSMISSION_MODE.enableRSSI = RSSI_DISABLED;
  //	configuration.TRANSMISSION_MODE.fixedTransmission = FT_FIXED_TRANSMISSION;
  //	configuration.TRANSMISSION_MODE.enableLBT = LBT_DISABLED;
  //	configuration.TRANSMISSION_MODE.WORPeriod = WOR_2000_011;
  //	----------------------- BROADCAST MESSAGE 2 -----------------------
  //	configuration.ADDL = 0x05;
  //	configuration.ADDH = 0x00;
  //
  //	configuration.CHAN = 23;
  //
  //	configuration.SPED.uartBaudRate = UART_BPS_9600;
  //	configuration.SPED.airDataRate = AIR_DATA_RATE_010_24;
  //	configuration.SPED.uartParity = MODE_00_8N1;
  //
  //	configuration.OPTION.subPacketSetting = SPS_200_00;
  //	configuration.OPTION.RSSIAmbientNoise = RSSI_AMBIENT_NOISE_DISABLED;
  //	configuration.OPTION.transmissionPower = POWER_22;
  //
  //	configuration.TRANSMISSION_MODE.enableRSSI = RSSI_DISABLED;
  //	configuration.TRANSMISSION_MODE.fixedTransmission = FT_FIXED_TRANSMISSION;
  //	configuration.TRANSMISSION_MODE.enableLBT = LBT_DISABLED;
  //	configuration.TRANSMISSION_MODE.WORPeriod = WOR_2000_011;
  //	----------------------- BROADCAST MESSAGE 3 -----------------------
  //	configuration.ADDL = 0x06;
  //	configuration.ADDH = 0x00;
  //
  //	configuration.CHAN = 23;
  //
  //	configuration.SPED.uartBaudRate = UART_BPS_9600;
  //	configuration.SPED.airDataRate = AIR_DATA_RATE_010_24;
  //	configuration.SPED.uartParity = MODE_00_8N1;
  //
  //	configuration.OPTION.subPacketSetting = SPS_200_00;
  //	configuration.OPTION.RSSIAmbientNoise = RSSI_AMBIENT_NOISE_DISABLED;
  //	configuration.OPTION.transmissionPower = POWER_22;
  //
  //	configuration.TRANSMISSION_MODE.enableRSSI = RSSI_DISABLED;
  //	configuration.TRANSMISSION_MODE.fixedTransmission = FT_FIXED_TRANSMISSION;
  //	configuration.TRANSMISSION_MODE.enableLBT = LBT_DISABLED;
  //	configuration.TRANSMISSION_MODE.WORPeriod = WOR_2000_011;
  //	----------------------- BROADCAST MESSAGE RSSI 1 -----------------------
  //	configuration.ADDL = 0x04;
  //	configuration.ADDH = 0x00;
  //
  //	configuration.CHAN = 23;
  //
  //	configuration.SPED.uartBaudRate = UART_BPS_9600;
  //	configuration.SPED.airDataRate = AIR_DATA_RATE_010_24;
  //	configuration.SPED.uartParity = MODE_00_8N1;
  //
  //	configuration.OPTION.subPacketSetting = SPS_200_00;
  //	configuration.OPTION.RSSIAmbientNoise = RSSI_AMBIENT_NOISE_DISABLED;
  //	configuration.OPTION.transmissionPower = POWER_22;
  //
  //	configuration.TRANSMISSION_MODE.enableRSSI = RSSI_ENABLED;
  //	configuration.TRANSMISSION_MODE.fixedTransmission = FT_FIXED_TRANSMISSION;
  //	configuration.TRANSMISSION_MODE.enableLBT = LBT_DISABLED;
  //	configuration.TRANSMISSION_MODE.WORPeriod = WOR_2000_011;
  //	----------------------- BROADCAST MESSAGE RSSI 2 -----------------------
  //	configuration.ADDL = 0x05;
  //	configuration.ADDH = 0x00;
  //
  //	configuration.CHAN = 23;
  //
  //	configuration.SPED.uartBaudRate = UART_BPS_9600;
  //	configuration.SPED.airDataRate = AIR_DATA_RATE_010_24;
  //	configuration.SPED.uartParity = MODE_00_8N1;
  //
  //	configuration.OPTION.subPacketSetting = SPS_200_00;
  //	configuration.OPTION.RSSIAmbientNoise = RSSI_AMBIENT_NOISE_DISABLED;
  //	configuration.OPTION.transmissionPower = POWER_22;
  //
  //	configuration.TRANSMISSION_MODE.enableRSSI = RSSI_ENABLED;
  //	configuration.TRANSMISSION_MODE.fixedTransmission = FT_FIXED_TRANSMISSION;
  //	configuration.TRANSMISSION_MODE.enableLBT = LBT_DISABLED;
  //	configuration.TRANSMISSION_MODE.WORPeriod = WOR_2000_011;
  //	----------------------- BROADCAST MESSAGE RSSI 3 -----------------------
  //	configuration.ADDL = 0x06;
  //	configuration.ADDH = 0x00;
  //
  //	configuration.CHAN = 23;
  //
  //	configuration.SPED.uartBaudRate = UART_BPS_9600;
  //	configuration.SPED.airDataRate = AIR_DATA_RATE_010_24;
  //	configuration.SPED.uartParity = MODE_00_8N1;
  //
  //	configuration.OPTION.subPacketSetting = SPS_200_00;
  //	configuration.OPTION.RSSIAmbientNoise = RSSI_AMBIENT_NOISE_DISABLED;
  //	configuration.OPTION.transmissionPower = POWER_22;
  //
  //	configuration.TRANSMISSION_MODE.enableRSSI = RSSI_ENABLED;
  //	configuration.TRANSMISSION_MODE.fixedTransmission = FT_FIXED_TRANSMISSION;
  //	configuration.TRANSMISSION_MODE.enableLBT = LBT_DISABLED;
  //	configuration.TRANSMISSION_MODE.WORPeriod = WOR_2000_011;
  //	----------------------- MONITORING -----------------------
  //	configuration.ADDL = BROADCAST_ADDRESS;
  //	configuration.ADDH = BROADCAST_ADDRESS;
  //
  //	configuration.CHAN = 23;
  //
  //	configuration.SPED.uartBaudRate = UART_BPS_9600;
  //	configuration.SPED.airDataRate = AIR_DATA_RATE_010_24;
  //	configuration.SPED.uartParity = MODE_00_8N1;
  //
  //	configuration.OPTION.subPacketSetting = SPS_200_00;
  //	configuration.OPTION.RSSIAmbientNoise = RSSI_AMBIENT_NOISE_DISABLED;
  //	configuration.OPTION.transmissionPower = POWER_22;
  //
  //	configuration.TRANSMISSION_MODE.enableRSSI = RSSI_DISABLED;
  //	configuration.TRANSMISSION_MODE.fixedTransmission = FT_FIXED_TRANSMISSION;
  //	configuration.TRANSMISSION_MODE.enableLBT = LBT_DISABLED;
  //	configuration.TRANSMISSION_MODE.WORPeriod = WOR_2000_011;

  // Set configuration changed and set to not hold the configuration
  ResponseStatus rs = e220ttl.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);
  Serial.println(rs.getResponseDescription());
  Serial.println(rs.code);

  c = e220ttl.getConfiguration();
  // It's important get configuration pointer before all other operation
  configuration = *(Configuration*)c.data;
  Serial.println(c.status.getResponseDescription());
  Serial.println(c.status.code);

  printParameters(configuration);
  c.close();
}

void loop() {
}
void printParameters(struct Configuration configuration) {
  DEBUG_PRINTLN("----------------------------------------");

  DEBUG_PRINT(F("HEAD : "));
  DEBUG_PRINT(configuration.COMMAND, HEX);
  DEBUG_PRINT(" ");
  DEBUG_PRINT(configuration.STARTING_ADDRESS, HEX);
  DEBUG_PRINT(" ");
  DEBUG_PRINTLN(configuration.LENGHT, HEX);
  DEBUG_PRINTLN(F(" "));
  DEBUG_PRINT(F("AddH : "));
  DEBUG_PRINTLN(configuration.ADDH, HEX);
  DEBUG_PRINT(F("AddL : "));
  DEBUG_PRINTLN(configuration.ADDL, HEX);
  DEBUG_PRINTLN(F(" "));
  DEBUG_PRINT(F("Chan : "));
  DEBUG_PRINT(configuration.CHAN, DEC);
  DEBUG_PRINT(" -> ");
  DEBUG_PRINTLN(configuration.getChannelDescription());
  DEBUG_PRINTLN(F(" "));
  DEBUG_PRINT(F("SpeedParityBit     : "));
  DEBUG_PRINT(configuration.SPED.uartParity, BIN);
  DEBUG_PRINT(" -> ");
  DEBUG_PRINTLN(configuration.SPED.getUARTParityDescription());
  DEBUG_PRINT(F("SpeedUARTDatte     : "));
  DEBUG_PRINT(configuration.SPED.uartBaudRate, BIN);
  DEBUG_PRINT(" -> ");
  DEBUG_PRINTLN(configuration.SPED.getUARTBaudRateDescription());
  DEBUG_PRINT(F("SpeedAirDataRate   : "));
  DEBUG_PRINT(configuration.SPED.airDataRate, BIN);
  DEBUG_PRINT(" -> ");
  DEBUG_PRINTLN(configuration.SPED.getAirDataRateDescription());
  DEBUG_PRINTLN(F(" "));
  DEBUG_PRINT(F("OptionSubPacketSett: "));
  DEBUG_PRINT(configuration.OPTION.subPacketSetting, BIN);
  DEBUG_PRINT(" -> ");
  DEBUG_PRINTLN(configuration.OPTION.getSubPacketSetting());
  DEBUG_PRINT(F("OptionTranPower    : "));
  DEBUG_PRINT(configuration.OPTION.transmissionPower, BIN);
  DEBUG_PRINT(" -> ");
  DEBUG_PRINTLN(configuration.OPTION.getTransmissionPowerDescription());
  DEBUG_PRINT(F("OptionRSSIAmbientNo: "));
  DEBUG_PRINT(configuration.OPTION.RSSIAmbientNoise, BIN);
  DEBUG_PRINT(" -> ");
  DEBUG_PRINTLN(configuration.OPTION.getRSSIAmbientNoiseEnable());
  DEBUG_PRINTLN(F(" "));
  DEBUG_PRINT(F("TransModeWORPeriod : "));
  DEBUG_PRINT(configuration.TRANSMISSION_MODE.WORPeriod, BIN);
  DEBUG_PRINT(" -> ");
  DEBUG_PRINTLN(configuration.TRANSMISSION_MODE.getWORPeriodByParamsDescription());
  DEBUG_PRINT(F("TransModeEnableLBT : "));
  DEBUG_PRINT(configuration.TRANSMISSION_MODE.enableLBT, BIN);
  DEBUG_PRINT(" -> ");
  DEBUG_PRINTLN(configuration.TRANSMISSION_MODE.getLBTEnableByteDescription());
  DEBUG_PRINT(F("TransModeEnableRSSI: "));
  DEBUG_PRINT(configuration.TRANSMISSION_MODE.enableRSSI, BIN);
  DEBUG_PRINT(" -> ");
  DEBUG_PRINTLN(configuration.TRANSMISSION_MODE.getRSSIEnableByteDescription());
  DEBUG_PRINT(F("TransModeFixedTrans: "));
  DEBUG_PRINT(configuration.TRANSMISSION_MODE.fixedTransmission, BIN);
  DEBUG_PRINT(" -> ");
  DEBUG_PRINTLN(configuration.TRANSMISSION_MODE.getFixedTransmissionDescription());


  DEBUG_PRINTLN("----------------------------------------");
}
void printModuleInformation(struct ModuleInformation moduleInformation) {
  Serial.println("----------------------------------------");
  DEBUG_PRINT(F("HEAD: "));
  DEBUG_PRINT(moduleInformation.COMMAND, HEX);
  DEBUG_PRINT(" ");
  DEBUG_PRINT(moduleInformation.STARTING_ADDRESS, HEX);
  DEBUG_PRINT(" ");
  DEBUG_PRINTLN(moduleInformation.LENGHT, DEC);

  Serial.print(F("Model no.: "));
  Serial.println(moduleInformation.model, HEX);
  Serial.print(F("Version  : "));
  Serial.println(moduleInformation.version, HEX);
  Serial.print(F("Features : "));
  Serial.println(moduleInformation.features, HEX);
  Serial.println("----------------------------------------");
}
