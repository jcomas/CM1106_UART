/*
    CM1106 Library for serial communication (UART)

Copyright (c) 2021 Josep Comas

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

*/    


#ifndef _CM1106_UART
    #define _CM1106_UART

    #include "Arduino.h"    
    #include <SoftwareSerial.h>


    #define CM1106_DEBUG  1             // Uncomment for debug messages

    #if (CM1106_DEBUG)
        #define _CM1106_DEBUG_SERIAL 1   // Serial communication for debug: 0 = Softserial, 1 = Hardware Serial, 2 = Hardware Serial Port 2
    #endif


    /* Serial port for debug */
    #if (_CM1106_DEBUG_SERIAL == 0)
        #define CM1106_DEBUG_SERIAL_RX 13
        #define CM1106_DEBUG_SERIAL_TX 15
    #elif (_CM1106_DEBUG_SERIAL == 1)
        #define CM1106_DEBUG_SERIAL Serial
    #elif (_CM1106_DEBUG_SERIAL == 2)
        #define CM1106_DEBUG_SERIAL Serial2
    #endif


    #define CM1106_TIMEOUT  5     // Timeout for communication

    #define CM1106_ABC_OPEN   0   // Open ABC
    #define CM1106_ABC_CLOSE  2   // Close ABC

    #define CM1106_LEN_SN       20   // Length of serial number
    #define CM1106_LEN_SOFTVER  10   // Length of software version

    #define CM1106_LEN_BUF_MSG  20   // Max length of buffer for communication with the sensor


    struct CM1106_ABC {
        uint8_t open_close;
        uint8_t cycle; 
        uint16_t base;
    };

    struct CM1106_sensor {
        char sn[CM1106_LEN_SN + 1];
        char softver[CM1106_LEN_SOFTVER + 1];
        uint16_t co2;
    };


    class CM1106_UART
    {
        public:
            CM1106_UART(Stream &serial);                                        // Initialize
            void get_serial_number(char sn[]);                                  // Get serial number
            void get_software_version(char softver[]);                          // Get software version
            uint16_t get_co2();                                                 // Get CO2 value in ppm
            bool start_calibration(uint16_t concentration);                     // Start calibration
            bool set_ABC(uint8_t open_close, uint8_t cycle, uint16_t base);     // Set ABC parameters
            bool get_ABC(CM1106_ABC *abc);                                      // Get ABC parameters

        private:
            Stream* mySerial;                                                   // Communication serial with the sensor
            uint8_t buf_msg[CM1106_LEN_BUF_MSG];                                // Buffer for communication messages with the sensor

            /* messages to send to the sensor*/
            uint8_t cmd_get_serial_number[4] = {0x11, 0x01, 0x1F, 0xCF};        // Ask serial number of sensor
            uint8_t cmd_get_software_version[4] = {0x11, 0x01, 0x1E, 0xD0};     // Ask software version of sensor
            uint8_t cmd_get_co2[4] = {0x11, 0x01, 0x01, 0xED};                  // Ask CO2 measure of sensor
            uint8_t cmd_start_calibration[3] = {0x11, 0x03, 0x03};              // Ask to start calibration
            uint8_t cmd_set_ABC[3] = {0x11, 0x07, 0x10};                        // Set ABC parameters
            uint8_t cmd_get_ABC[4] = {0x11, 0x01, 0x0F, 0xDF};                  // Ask ABC parameters

            void serial_write_bytes(uint8_t size);                              // Send bytes to sensor
            uint8_t serial_read_bytes(uint8_t max_bytes, int timeout_seconds);  // Read received bytes from sensor
            uint8_t calculate_cs(uint8_t nb);                                   // Calculate checksum of packet
            void print_buffer(uint8_t size);                                    // Show buffer in hex bytes
    };

#endif
