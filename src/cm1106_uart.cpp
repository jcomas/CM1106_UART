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

#if (CM1106_LOG_LEVEL > CM1106_LOG_LEVEL_NONE)
    #ifdef CM1106_DEBUG_SOFTWARE_SERIAL
        SoftwareSerial CM1106_DEBUG_SERIAL(CM1106_DEBUG_SERIAL_RX, CM1106_DEBUG_SERIAL_TX);
    #endif        
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
    if (valid_response_len(CM1106_CMD_GET_SERIAL_NUMBER, nb, 14)) {

        uint16_t sn_int;
        char sn_string[5];

        for (int i = 0; i < 5; i++)
        {
            sn_int = ((buf_msg[3 + 2 * i] & 0x00FF) << 8) | (buf_msg[4 + 2 * i] & 0x00FF);
            snprintf(sn_string, sizeof(sn_string)-1, "%04d", sn_int);
            strcat(sn, sn_string);
        }
        CM1106_LOG("DEBUG: Serial number: %s\n", sn);

    } else {
        CM1106_LOG("DEBUG: Serial number not available!\n");
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
    if (valid_response_len(CM1106_CMD_GET_SOFTWARE_VERSION, nb, 15)) {
        strncpy(softver, (const char *)&buf_msg[3], CM1106_LEN_SOFTVER);
        CM1106_LOG("DEBUG: Software version: %s\n", softver);
    } else {
        CM1106_LOG("DEBUG: Software version not available!\n");
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
    if (valid_response_len(CM1106_CMD_GET_CO2, nb, 8)) {
        co2 = (buf_msg[3] * 256) + buf_msg[4];
        CM1106_LOG("DEBUG: CO2 value = %d ppm\n", co2);
    } else {
        CM1106_LOG("DEBUG: Error getting CO2 value!\n");
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
        uint8_t nb = serial_read_bytes(4, CM1106_TIMEOUT);

        // Check response and get data
        if (valid_response_len(CM1106_CMD_START_CALIBRATION, nb, 4)) {
            result = true;
            CM1106_LOG("DEBUG: Successful start of calibration\n");
        } else {
            CM1106_LOG("DEBUG: Error in start of calibration!\n");
        }

    } else {     
       CM1106_LOG("DEBUG: Invalid CO2 value! Valid range is 400-1500 ppm\n");
    }

    return result;
}


/* Setting ABC */
bool CM1106_UART::set_ABC(uint8_t open_close, uint8_t cycle, int16_t base) {
    bool result = false;

    if ((open_close == CM1106_ABC_OPEN || open_close == CM1106_ABC_CLOSE) && cycle >= 1 && cycle <= 7 && base >= 400 && base <= 1499) {

        // Put ABC parameters in buffer
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
        uint8_t nb = serial_read_bytes(4, CM1106_TIMEOUT);

        // Check response and get data
        if (valid_response_len(CM1106_CMD_SET_ABC, nb, 4)) {
            result = true;
            CM1106_LOG("DEBUG: Successful setting of ABC\n");
        } else {
            CM1106_LOG("DEBUG: Error in setting of ABC!\n");
        }

    } else {
        CM1106_LOG("DEBUG: Invalid ABC parameters!\n");
    }

    return result;
}


/* Getting ABC */
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
    if (valid_response_len(CM1106_CMD_GET_ABC, nb, 10)) {
        abc->open_close = buf_msg[4];
        abc->cycle = buf_msg[5];
        //abc->base = (buf_msg[6] * 256) + buf_msg[7];
        abc->base = (buf_msg[6] << 8) | buf_msg[7];
        result = true;
        CM1106_LOG("DEBUG: Successful getting ABC parameters\n");
    } else {
        CM1106_LOG("DEBUG: Error getting ABC parameters\n");
    }

    return result;
}


/* Storing ABC data */
bool CM1106_UART::store_ABC_data() {
    bool result = false;

    // Ask store ABC data
    send_cmd(CM1106_CMD_STORE_ABC_DATA);

    // Wait response
    uint8_t nb = serial_read_bytes(4, CM1106_TIMEOUT);

    // Check response and get data
    if (valid_response_len(CM1106_CMD_STORE_ABC_DATA, nb, 4)) {
        result = true; 
        CM1106_LOG("DEBUG: Successful storing ABC data!\n");
    } else {
        CM1106_LOG("DEBUG: Error storing ABC data!\n");
    }

    return result;
}


/* Setting measurement period and smoothed data */
bool CM1106_UART::set_measurement_period(int16_t period, uint8_t smoothed) {
    bool result = false;

    if (period >= 1 && period <= 600) {

        // Put data in buffer
        buf_msg[3] = (period & 0xFF00 ) >> 8;
        buf_msg[4] = (period & 0xFF);            
        buf_msg[5] = smoothed;

        // Ask set measurement period and number of smoothed data
        send_cmd_data(CM1106_CMD_MEASUREMENT_PERIOD, 7);

        // Wait response
        memset(buf_msg, 0, CM1106_LEN_BUF_MSG);
        uint8_t nb = serial_read_bytes(4, CM1106_TIMEOUT);

        // Check response and get data
        if (valid_response_len(CM1106_CMD_MEASUREMENT_PERIOD, nb, 4)) {
            result = true;
            CM1106_LOG("DEBUG: Successful setting of measurement period\n");
        } else {
            CM1106_LOG("DEBUG: Error in setting of measurement period!\n");
        }

    } else {
        CM1106_LOG("DEBUG: Invalid measurement period!\n");
    }

    return result;
}


/* Getting measurement period and smoothed data */
bool CM1106_UART::get_measurement_period(int16_t *period, uint8_t *smoothed) {
    bool result = false;

    if (period == NULL || smoothed == NULL)
        return result;

    // Ask set measurement period and number of smoothed data
    send_cmd_data(CM1106_CMD_MEASUREMENT_PERIOD, 4);

    // Wait response
    memset(buf_msg, 0, CM1106_LEN_BUF_MSG);
    uint8_t nb = serial_read_bytes(7, CM1106_TIMEOUT);

    // Check response and get data
    if (valid_response_len(CM1106_CMD_MEASUREMENT_PERIOD, nb, 7)) {
        *period = (buf_msg[3] << 8) | buf_msg[4];
        *smoothed = buf_msg[5];
        result = true;
        CM1106_LOG("DEBUG: Successful setting of measurement period\n");
    } else {
        CM1106_LOG("DEBUG: Error in setting of measurement period!\n");
    }

    return result;
}


/* Setting working status */
bool CM1106_UART::set_working_status(uint8_t mode) {
    bool result = false;

    if ((mode == CM1106_SINGLE_MEASUREMENT || mode == CM1106_CONTINUOUS_MEASUREMENT)) {

        // Put measurement mode in buffer
        buf_msg[3] = mode;

        // Ask set measurement mode
        send_cmd_data(CM1106_CMD_WORKING_STATUS, 5);

        // Wait response
        memset(buf_msg, 0, CM1106_LEN_BUF_MSG);
        uint8_t nb = serial_read_bytes(4, CM1106_TIMEOUT);

        // Check response and get data
        if (valid_response_len(CM1106_CMD_WORKING_STATUS, nb, 4)) {
            result = true;
            CM1106_LOG("DEBUG: Successful setting of measurement mode\n");
        } else {
            CM1106_LOG("DEBUG: Error in setting of measurement mode!\n");
        }

    } else {
        CM1106_LOG("DEBUG: Invalid measurement mode!\n");
    }

    return result;
}


/* Getting working status */
bool CM1106_UART::get_working_status(uint8_t *mode) {
    bool result = false;

    if (mode == NULL)
        return result;

    // Ask set measurement mode
    send_cmd_data(CM1106_CMD_WORKING_STATUS, 4);

    // Wait response
    memset(buf_msg, 0, CM1106_LEN_BUF_MSG);
    uint8_t nb = serial_read_bytes(5, CM1106_TIMEOUT);

    // Check response and get data
    if (valid_response_len(CM1106_CMD_WORKING_STATUS, nb, 5)) {
        *mode = buf_msg[3];
        result = true;
        CM1106_LOG("DEBUG: Successful getting working status\n");
    } else {
        CM1106_LOG("DEBUG: Error in getting working status!\n");
    }

    return result;
}


/* Send bytes to sensor */
void CM1106_UART::serial_write_bytes(uint8_t size) {

#if (CM1106_LOG_LEVEL > CM1106_LOG_LEVEL_NONE)     
    CM1106_LOG("DEBUG: Bytes to send => ");
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

        CM1106_LOG("DEBUG: Bytes received => ");

        while ((difftime(end_t, start_t) <= timeout_seconds) && !readed) {
            if(mySerial->available()) {
                nb = mySerial->readBytes(buf_msg, max_bytes);
                readed = true;
            }            
            time(&end_t);
        }

#if (CM1106_LOG_LEVEL > CM1106_LOG_LEVEL_NONE)
        print_buffer(nb);
#endif

    } else {
        CM1106_LOG("DEBUG: Invalid parameters!\n");
    }

    return nb;
}


/* Check valid response and length of received message */
bool CM1106_UART::valid_response_len(uint8_t cmd, uint8_t nb, uint8_t len) {
    bool result = false;

    if (nb == len) {
        result = valid_response(cmd, nb);
    } else {
        CM1106_LOG("DEBUG: Unexpected length\n");
    }

    return result;
}


/* Check if it is a valid message response of the sensor */
bool CM1106_UART::valid_response(uint8_t cmd, uint8_t nb) {
    bool result = false;

    if (nb >= 4) {
        if (buf_msg[nb-1] == calculate_cs(nb) && buf_msg[1] == nb-3) {

            if (buf_msg[0] == CM1106_MSG_ACK && buf_msg[2] == cmd) {
                CM1106_LOG("DEBUG: Valid response\n");
                result = true;

            } else if (buf_msg[0] == CM1106_MSG_NAK && nb == 4) {
                CM1106_LOG("DEBUG: Response with error 0x%02x\n", buf_msg[2]);
                // error 0x02 = cmd not recognised, invalid checksum...
                // If invalid length then no response.
            }

        } else {
            CM1106_LOG("DEBUG: Checksum/length is invalid\n");
        }

    } else {
        CM1106_LOG("DEBUG: Invalid length\n");
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
        CM1106_LOG("DEBUG: Checksum => 0x%02x\n", cs);

    } else {
        CM1106_LOG("DEBUG: Invalid packet!\n");
    }

    return cs;
}


/* Show buffer in hex bytes */
void CM1106_UART::print_buffer(uint8_t size) {

    for (int i = 0; i < size; i++) {
        CM1106_LOG("0x%02x ", buf_msg[i]);
    }
    CM1106_LOG("(%u bytes)\n", size);
}


#ifdef CM1106_ADVANCED_FUNC

/* Detect implemented Cubic UART commands */
void CM1106_UART::detect_commands() {

    uint8_t nb = 0;

    for (uint16_t i = 0x01; i <= 0x5f; i++) {
        send_cmd(i);
        nb = serial_read_bytes(CM1106_LEN_BUF_MSG, CM1106_TIMEOUT);
        if (valid_response(i, nb)) {
            CM1106_LOG("Command 0x%02x implemented\n", buf_msg[2]);
        } else {
            CM1106_LOG("Command 0x%02x not available\n", buf_msg[2]);
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
            CM1106_LOG("Command 0x%02x implemented\n", buf_msg[2]);
        } else {
            CM1106_LOG("Command 0x%02x not available\n", buf_msg[2]);
        }
    }
}

#endif


/*
// test cmd
void CM1106_UART::test_cmd() {

    // Put data in buffer
    buf_msg[3] = 0x00;

    // Ask
    send_cmd_data(0x02, 5);

    // Wait response
    memset(buf_msg, 0, CM1106_LEN_BUF_MSG);
    serial_read_bytes(20, CM1106_TIMEOUT);
}
*/
