#include <Arduino.h>
#include <SoftwareSerial.h>
#include "cm1106_uart.h"


CM1106_UART *sensor_CM1106;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("");
  Serial.println("Getting info of Cubic CM1106 NDIR CO2 sensor...");
  sensor_CM1106 = new CM1106_UART();
  Serial.printf("Serial number: %s\n", sensor_CM1106->get_serial_number());  
  Serial.printf("Software version: %s\n", sensor_CM1106->get_software_version());
  Serial.println("Setting ABC parameters...");
  sensor_CM1106->set_ABC(CM1106_ABC_OPEN, 15, 400);    // 15 days cycle, 400 ppm for base
  Serial.println("Starting calibration...");
  sensor_CM1106->start_calibration(400);
  Serial.println("Setup done!");
}

void loop() {
  // put your main code here, to run repeatedly:
  //Serial.printf("CO2 value: %u ppm\n", sensor_CM1106->get_co2());
  Serial.printf("/*%u*/\n", sensor_CM1106->get_co2());   // Format to use with Serial Studio program
  delay(5000);
}
