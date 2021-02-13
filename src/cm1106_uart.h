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


    //#define CM1106_DEBUG  1             // Uncomment for debug messages

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


    //#define CM1106_ADVANCED_FUNC  1      // Don't uncomment, can be dangerous, internal use functions


    #define CM1106_TIMEOUT  5     // Timeout for communication

    #define CM1106_ABC_OPEN   0   // Open ABC (enable auto calibration)
    #define CM1106_ABC_CLOSE  2   // Close ABC (disable auto calibration)

    #define CM1106_LEN_SN       20   // Length of serial number
    #define CM1106_LEN_SOFTVER  10   // Length of software version

    #define CM1106_LEN_BUF_MSG  20   // Max length of buffer for communication with the sensor
    #define CM1106_MSG_IP     0x11   // Packet identifier byte of sensor communication response 
    #define CM1106_MSG_ACK    0x16   // ACK byte of sensor communication response 
    #define CM1106_MSG_NAK    0x06   // NAK byte of sensor communication response 

    /* messages to send to the sensor*/
    #define CM1106_CMD_GET_CO2                0x01   // Ask CO2 measure of sensor
    #define CM1106_CMD_START_CALIBRATION      0x03   // Ask to start calibration
    #define CM1106_CMD_GET_ABC                0x0F   // Ask ABC parameters
    #define CM1106_CMD_SET_ABC                0x10   // Set ABC parameters
    #define CM1106_CMD_GET_SOFTWARE_VERSION   0x1E   // Ask software version of sensor
    #define CM1106_CMD_GET_SERIAL_NUMBER      0x1F   // Ask serial number of sensor

    struct CM1106_ABC {
        uint8_t open_close;
        uint8_t cycle; 
        int16_t base;
    };

    struct CM1106_sensor {
        char sn[CM1106_LEN_SN + 1];
        char softver[CM1106_LEN_SOFTVER + 1];
        int16_t co2;
    };


    class CM1106_UART
    {
        public:
            CM1106_UART(Stream &serial);                                        // Initialize
            void get_serial_number(char sn[]);                                  // Get serial number
            void get_software_version(char softver[]);                          // Get software version
            int16_t get_co2();                                                  // Get CO2 value in ppm
            bool start_calibration(int16_t concentration);                      // Start single point calibration
                                                                                   // Before calibration, please make sure CO 2 concentration in current ambient 
                                                                                   // is calibration target value. Keeping this CO2 concentration for two 2 minutes,
                                                                                   // and then began calibration.
            bool set_ABC(uint8_t open_close, uint8_t cycle, int16_t base);      // Set ABC parameters (enable (open)/disable(close) auto calibration, cycle days, baseline co2)
            bool get_ABC(CM1106_ABC *abc);                                      // Get ABC parameters

#ifdef CM1106_ADVANCED_FUNC
            void detect_commands();                                             // Detect implemented commands
            void test_implemented();                                            // Check implemented commands
#endif

        private:
            Stream* mySerial;                                                   // Communication serial with the sensor
            uint8_t buf_msg[CM1106_LEN_BUF_MSG];                                // Buffer for communication messages with the sensor



            void serial_write_bytes(uint8_t size);                              // Send bytes to sensor
            uint8_t serial_read_bytes(uint8_t max_bytes, int timeout_seconds);  // Read received bytes from sensor
            bool valid_response(uint8_t cmd, uint8_t nb);                       // Check if response is valid according to sent command
            void send_cmd(uint8_t cmd);                                         // Send command without additional data
            void send_cmd_data(uint8_t cmd, uint8_t size);                      // Send command with additional data
            uint8_t calculate_cs(uint8_t nb);                                   // Calculate checksum of packet
            void print_buffer(uint8_t size);                                    // Show buffer in hex bytes

    };

#endif
