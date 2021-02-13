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


#include "cm1106_uart.h"


#if (_CM1106_DEBUG_SERIAL == 0)
    SoftwareSerial CM1106_DEBUG_SERIAL(CM1106_DEBUG_SERIAL_RX, CM1106_DEBUG_SERIAL_TX);
#endif        


/* Initialize */
CM1106_UART::CM1106_UART(Stream &serial)
{
    mySerial = &serial;
}


/* Get serial number */
void CM1106_UART::get_serial_number(char sn[] ) {

    if (sn == NULL) {
        return;
    }

    strcpy(sn, "");

    // Ask serial number
    send_cmd(CM1106_CMD_GET_SERIAL_NUMBER);

    // Wait response
    memset(buf_msg, 0, CM1106_LEN_BUF_MSG);
    uint8_t nb = serial_read_bytes(14, CM1106_TIMEOUT);

    // Check response and get data
    if (nb == 14 && valid_response(CM1106_CMD_GET_SERIAL_NUMBER, nb)) {

        uint16_t sn_int;
        char sn_string[5];

        for (int i = 0; i < 5; i++)
        {
            sn_int = ((buf_msg[3 + 2 * i] & 0x00FF) << 8) | (buf_msg[4 + 2 * i] & 0x00FF);
            snprintf(sn_string, sizeof(sn_string)-1, "%04d", sn_int);
            strcat(sn, sn_string);
        }

#ifdef CM1106_DEBUG        
        CM1106_DEBUG_SERIAL.printf("DEBUG: Serial number: %s\n", sn);
#endif

    } else {

#ifdef CM1106_DEBUG            
        CM1106_DEBUG_SERIAL.println("DEBUG: Serial number not available!");
#endif

    }

}


/* Get software version */
void CM1106_UART::get_software_version(char softver[]) {

    if (softver == NULL) {
        return;
    }    

    strcpy(softver, "");

    // Ask software version
    send_cmd(CM1106_CMD_GET_SOFTWARE_VERSION);

    // Wait response
    memset(buf_msg, 0, CM1106_LEN_BUF_MSG);
    uint8_t nb = serial_read_bytes(15, CM1106_TIMEOUT);

    // Check response and get data
    if (nb == 15 && valid_response(CM1106_CMD_GET_SOFTWARE_VERSION, nb)) {
      strncpy(softver, (const char *)&buf_msg[3], CM1106_LEN_SOFTVER);

#ifdef CM1106_DEBUG        
        CM1106_DEBUG_SERIAL.printf("DEBUG: Software version: %s\n", softver);
#endif

    } else {

#ifdef CM1106_DEBUG         
        CM1106_DEBUG_SERIAL.println("DEBUG: Software version not available!");        
#endif

    }
    
}


/* Get CO2 value in ppm */
int16_t CM1106_UART::get_co2() {

    int16_t co2 = 0;

    // Ask CO2 value
    send_cmd(CM1106_CMD_GET_CO2);

    // Wait response
    memset(buf_msg, 0, CM1106_LEN_BUF_MSG);
    uint8_t nb = serial_read_bytes(8, CM1106_TIMEOUT);

    // Check response and get data
    if (nb == 8 && valid_response(CM1106_CMD_GET_CO2, nb)) {
        co2 = (buf_msg[3] * 256) + buf_msg[4];

#ifdef CM1106_DEBUG
        CM1106_DEBUG_SERIAL.printf("DEBUG: CO2 value = %d ppm\n", co2);
#endif

    } else {

#ifdef CM1106_DEBUG        
        CM1106_DEBUG_SERIAL.println("DEBUG: Error getting CO2 value!");
#endif

    }
    return co2;
}


/* Start calibration */
bool CM1106_UART::start_calibration(int16_t concentration) {
    bool result = false;

    if (concentration >= 400 && concentration <= 1500) {

        // Put data
        buf_msg[3] = (concentration & 0xFF00 ) >> 8;
        buf_msg[4] = (concentration & 0xFF);

        // Ask start calibration
        send_cmd_data(CM1106_CMD_START_CALIBRATION, 6);

        // Wait response
        memset(buf_msg, 0, CM1106_LEN_BUF_MSG);
        uint8_t nb = serial_read_bytes(8, CM1106_TIMEOUT);

        // Check response and get data
        if (nb == 4 && valid_response(CM1106_CMD_START_CALIBRATION, nb)) {
            result = true;

#ifdef CM1106_DEBUG
            CM1106_DEBUG_SERIAL.println("DEBUG: Successful start of calibration");
#endif

        } else {

#ifdef CM1106_DEBUG            
            CM1106_DEBUG_SERIAL.println("DEBUG: Inesperated response!");
#endif

        }
    } else {

#ifdef CM1106_DEBUG        
        CM1106_DEBUG_SERIAL.println("DEBUG: Invalid CO2 value! Valid range is 400-1500 ppm");
#endif

    }
    return result;
}


/* Setting ABC */
bool CM1106_UART::set_ABC(uint8_t open_close, uint8_t cycle, int16_t base) {
    bool result = false;

    if ((open_close == CM1106_ABC_OPEN || open_close == CM1106_ABC_CLOSE) && cycle >= 1 && cycle <= 7 && base >= 400 && base <= 1499) {

        buf_msg[3] = 0x64;
        buf_msg[4] = open_close;
        buf_msg[5] = cycle;
        buf_msg[6] = (base & 0xFF00 ) >> 8;
        buf_msg[7] = (base & 0xFF);        
        buf_msg[8] = 0x64;

        // Ask set ABC
        send_cmd_data(CM1106_CMD_SET_ABC, 10);

        // Wait response
        memset(buf_msg, 0, CM1106_LEN_BUF_MSG);
        uint8_t nb = serial_read_bytes(10, CM1106_TIMEOUT);

        // Check response and get data
        if (nb == 4 && valid_response(CM1106_CMD_SET_ABC, nb)) {
            result = true;

#ifdef CM1106_DEBUG
            CM1106_DEBUG_SERIAL.println("DEBUG: Successful setting of ABC");
#endif

        } else {

#ifdef CM1106_DEBUG            
            CM1106_DEBUG_SERIAL.println("DEBUG: Inesperated response!");
#endif

        }
    } else {

#ifdef CM1106_DEBUG        
        CM1106_DEBUG_SERIAL.println("DEBUG: Invalid parameters!");
#endif

    }
    return result;
}


/* Setting ABC */
bool CM1106_UART::get_ABC(CM1106_ABC *abc) {
    bool result = false;

    if (abc == NULL)
        return result;

    abc->open_close = 0; abc->cycle = 0; abc->base = 0;

    // Ask get ABC parameters
    send_cmd(CM1106_CMD_GET_ABC);

    // Wait response
    uint8_t nb = serial_read_bytes(10, CM1106_TIMEOUT);

    // Check response and get data
    if (nb == 10 && valid_response(CM1106_CMD_GET_ABC, nb)) {
        abc->open_close = buf_msg[4];
        abc->cycle = buf_msg[5];
        //abc->base = (buf_msg[6] * 256) + buf_msg[7];
        abc->base = (buf_msg[6] << 8) | buf_msg[7];
        result = true;
    } else {
        
#ifdef CM1106_DEBUG            
            CM1106_DEBUG_SERIAL.println("DEBUG: Inesperated response!");
#endif

    }
    return result;

}


/* Send bytes to sensor */
void CM1106_UART::serial_write_bytes(uint8_t size) {

#ifdef CM1106_DEBUG        
    CM1106_DEBUG_SERIAL.printf("DEBUG: Bytes to send => ");
    print_buffer(size);
#endif

    mySerial->write(buf_msg, size);
    mySerial->flush();
}


/* Read answer of sensor */
uint8_t CM1106_UART::serial_read_bytes(uint8_t max_bytes, int timeout_seconds) {
    time_t start_t, end_t;
    //double diff_t;
    time(&start_t); end_t = start_t;
    bool readed = false;

    uint8_t nb = 0;
    if (max_bytes > 0 && timeout_seconds > 0) {

#ifdef CM1106_DEBUG
        CM1106_DEBUG_SERIAL.printf("DEBUG: Bytes received => ");
#endif

        while ((difftime(end_t, start_t) <= timeout_seconds) && !readed) {
            if(mySerial->available()) {
                nb = mySerial->readBytes(buf_msg, max_bytes);
                readed = true;
            }            
            time(&end_t);
        }

#ifdef CM1106_DEBUG
        print_buffer(nb);
#endif

    } else {

#ifdef CM1106_DEBUG        
        CM1106_DEBUG_SERIAL.println("DEBUG: Invalid parameters!");
#endif

    }

    return nb;
}


/* Check if it is a valid message response of the sensor */
bool CM1106_UART::valid_response(uint8_t cmd, uint8_t nb) {
    bool result = false;

    if (nb >= 4) {
        if (buf_msg[nb-1] == calculate_cs(nb) && buf_msg[1] == nb-3) {
            if (buf_msg[0] == CM1106_MSG_ACK && buf_msg[2] == cmd) {

#ifdef CM1106_DEBUG      
                CM1106_DEBUG_SERIAL.printf("DEBUG: Valid response\n");
#endif
                result = true;

            } else if (buf_msg[0] == CM1106_MSG_NAK && nb == 4) {

#ifdef CM1106_DEBUG      
                CM1106_DEBUG_SERIAL.printf("DEBUG: Response with error 0x%02x\n", buf_msg[2]);
                // error 0x02 = cmd not recognised, invalid checksum...
                // If invalid length then no response.
#endif

            }
        } else {

#ifdef CM1106_DEBUG      
                CM1106_DEBUG_SERIAL.printf("DEBUG: Checksum/length is invalid\n");
#endif

        }

    }
    else {

#ifdef CM1106_DEBUG      
                CM1106_DEBUG_SERIAL.printf("DEBUG: Invalid length\n");
#endif

    }
    
    return result;
}


/* Send command without addtional data */
void CM1106_UART::send_cmd(uint8_t cmd) {
    send_cmd_data(cmd, 4);    
}


/* Send command with addtional data */
void CM1106_UART::send_cmd_data(uint8_t cmd, uint8_t size) {
    if (size >= 4 && size <= CM1106_LEN_BUF_MSG) {
        buf_msg[0] = CM1106_MSG_IP;   // Packet identifier
        buf_msg[1] = size-3;            // Length
        buf_msg[2] = cmd;             // Command to send
        buf_msg[size-1] = calculate_cs(size);
        serial_write_bytes(size);    
    }
}


/* Calculate checksum */
uint8_t CM1106_UART::calculate_cs(uint8_t nb) {
    uint8_t cs = 0;

    if (nb >= 4) {
        cs = buf_msg[0] + buf_msg[1] + buf_msg[2];
        for (int i = 3; i < (nb-1); i++) {
            cs = cs + buf_msg[i];
        }
        cs = 256 - (cs % 256);

#ifdef CM1106_DEBUG
        CM1106_DEBUG_SERIAL.printf("DEBUG: Checksum => 0x%02x\n", cs);
#endif

    } else {

#ifdef CM1106_DEBUG      
        CM1106_DEBUG_SERIAL.printf("DEBUG: Invalid packet!\n");
#endif

    }
    return cs;
}


/* Show buffer in hex bytes */
void CM1106_UART::print_buffer(uint8_t size) {

    for (int i = 0; i < size; i++) {
        CM1106_DEBUG_SERIAL.printf("0x%02x ", buf_msg[i]);
    }
    CM1106_DEBUG_SERIAL.printf("(%u bytes)\n", size);
}


#ifdef CM1106_ADVANCED_FUNC

/* Detect implemented Cubic UART commands */
void CM1106_UART::detect_commands() {

    uint8_t nb = 0;

    for (uint16_t i = 0x01; i <= 0x5f; i++) {
        send_cmd(i);
        nb = serial_read_bytes(CM1106_LEN_BUF_MSG, CM1106_TIMEOUT);
        if (valid_response(i, nb)) {
            Serial.printf("Command 0x%02x implemented\n", buf_msg[2]);
        } else {
            Serial.printf("Command 0x%02x not available\n", buf_msg[2]);
        }        
    }
}


/* Check implemented commands */
void CM1106_UART::test_implemented() {
    char cmds[26] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x0d, 0x0e, 0x0f, 0x10, 0x1e, 0x1f, 0x23, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c};
    //char cmds[1] = {0x02};

    uint8_t nb = 0;

    for (uint8_t i = 0; i < sizeof(cmds); i++) {
        send_cmd(cmds[i]);
        nb = serial_read_bytes(CM1106_LEN_BUF_MSG, CM1106_TIMEOUT);
        if (valid_response(cmds[i], nb)) {
            Serial.printf("Command 0x%02x implemented\n", buf_msg[2]);
        } else {
            Serial.printf("Command 0x%02x not available\n", buf_msg[2]);
        }
    }
}

#endif
