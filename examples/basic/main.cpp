#include <Arduino.h>
#include "cm1106_uart.h"


#ifdef USE_SOFTWARE_SERIAL
    // Modify if CM1106 is connected using softwareserial
    #define CM1106_RX_PIN 14                                   // Rx pin which the CM1106 Tx pin is attached to
    #define CM1106_TX_PIN 12                                   // Tx pin which the CM1106 Rx pin is attached to
    SoftwareSerial CM1106_serial(CM1106_RX_PIN, CM1106_TX_PIN);
#else
    // Modify if CM1106 is attached to a hardware port
    #define CM1106_serial Serial2
#endif

#ifdef NODEMCUV2
    #define CONSOLE_BAUDRATE 74880
#else    
    #define CONSOLE_BAUDRATE 115200
#endif    


CM1106_UART *sensor_CM1106;
CM1106_sensor sensor;
CM1106_ABC abc;


void setup() {

    // Initialize console serial communication
    Serial.begin(CONSOLE_BAUDRATE);
    Serial.println("");

    Serial.println("Init");

    // Initialize sensor
    CM1106_serial.begin(CM1106_BAUDRATE);
    sensor_CM1106 = new CM1106_UART(CM1106_serial);

    // Check if CM1106 is available
    sensor_CM1106->get_software_version(sensor.softver);
    int len = strlen(sensor.softver);
    if (len > 0) {
        if (len >= 10 && !strncmp(sensor.softver+len-5, "SL-NS", 5)) {
            Serial.println("CM1106SL-NS detected");
        } else if (!strncmp(sensor.softver, "CM", 2)) {
            Serial.println("CM1106 detected");
        } else {
            Serial.println("CM1106 unknown version");
        }
    } else {
        Serial.println("CM1106 not found!");
        while (1) { delay(1); };
    }     

    // Show sensor info
    Serial.println(">>> Cubic CM1106 NDIR CO2 sensor <<<");  
    sensor_CM1106->get_serial_number(sensor.sn);
    Serial.printf("Serial number: %s\n", sensor.sn);
    Serial.printf("Software version: %s\n", sensor.softver);

    // Setup ABC parameters
    Serial.println("Setting ABC parameters...");
    sensor_CM1106->set_ABC(CM1106_ABC_OPEN, 7, 415);    // 7 days cycle, 415 ppm for base

    // Getting ABC parameters
    if (sensor_CM1106->get_ABC(&abc)) {
        Serial.println("ABC parameters:");
        if (abc.open_close == CM1106_ABC_OPEN) {
            Serial.println("Auto calibration is enabled");
        } else if (abc.open_close == CM1106_ABC_CLOSE) {
            Serial.println("Auto calibration is disabled");
        }
        Serial.printf("Calibration cycle: %d\n", abc.cycle);
        Serial.printf("Calibration baseline: %d\n", abc.base);
    }

    // Start calibration
    Serial.println("Starting calibration...");
    sensor_CM1106->start_calibration(400);

    Serial.println("Setup done!");
}


void loop() {

    sensor.co2 = sensor_CM1106->get_co2();

    Serial.printf("CO2 value: %d ppm\n", sensor.co2);   
    //Serial.printf("/*%u*/\n", sensor.co2);   // Format to use with Serial Studio program

    delay(5000);
}
