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


    #define CM1106_TIMEOUT  5     // Timeout for communication

    #define CM1106_ABC_OPEN   0   // Open ABC
    #define CM1106_ABC_CLOSE  2   // Close ABC

    #define CM1106_LEN_SN       20   // Length of serial number
    #define CM1106_LEN_SOFTVER  10   // Length of software version

    #define CM1106_LEN_BUF_MSG  20   // Max length of buffer for communication with the sensor


    class CM1106_UART
    {
        public:
            CM1106_UART(Stream &serial);                                        // Initialize
            void get_serial_number(char sn[]);                                  // Get serial number
            void get_software_version(char softver[]);                          // Get software version
            uint16_t get_co2();                                                 // Get CO2 value in ppm
            bool start_calibration(uint16_t concentration);                     // Start calibration
            bool set_ABC(uint8_t open_close, uint8_t cycle, uint16_t base);     // Set ABC parameters

        private:
            Stream* mySerial;                                                   // Communication serial with the sensor
            uint8_t buf_msg[CM1106_LEN_BUF_MSG];                                // Buffer for communication messages with the sensor

            /* messages to send to the sensor*/
            uint8_t cmd_get_serial_number[4] = {0x11, 0x01, 0x1F, 0xCF};        // Ask serial number of sensor
            uint8_t cmd_get_software_version[4] = {0x11, 0x01, 0x1E, 0xD0};     // Ask software version of sensor
            uint8_t cmd_get_co2[4] = {0x11, 0x01, 0x01, 0xED};                  // Ask CO2 measure of sensor
            uint8_t cmd_start_calibration[3] = {0x11, 0x03, 0x03};              // Ask to start calibration
            uint8_t cmd_set_ABC[3] = {0x11, 0x07, 0x10};                        // Set ABC

            void serial_write_bytes(uint8_t size);                              // Send bytes to sensor
            uint8_t serial_read_bytes(uint8_t max_bytes, int timeout_seconds);  // Read received bytes from sensor
            uint8_t calculate_cs(uint8_t nb);                                   // Calculate checksum of packet
            void print_buffer(uint8_t size);                                    // Show buffer in hex bytes
    };

#endif
