#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc; // Create an RTC object

// Uncomment the following line and set the desired date and time for initial setup.
// This should be commented out after the first successful upload to avoid resetting the time again.
// DateTime now(2025, 07, 10, 16, 15, 15); // YYYY, MM, DD, HH, MM, SS

void setup() {
  Serial.begin(9600);

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, setting the time!");
    // Set the RTC to the date & time this sketch was compiled
    // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // Set the RTC to the manual date & time defined above
    // rtc.adjust(now);
  } else {
    Serial.println("RTC is running with correct time.");
  }
}

void loop() {
  DateTime now = rtc.now();

  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();

  delay(1000); // Print time every second
}