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
    memcpy(buf_msg, cmd_get_serial_number, sizeof(cmd_get_serial_number));
    serial_write_bytes(sizeof(cmd_get_serial_number));

    // Wait response
    memset(buf_msg, 0, sizeof(buf_msg));
    uint8_t nb = serial_read_bytes(14, CM1106_TIMEOUT);

    if (nb == 14 && buf_msg[0] == 0x16 && buf_msg[1] == 0x0b && buf_msg[2] == 0x1f && buf_msg[13] == calculate_cs(nb)) {
    
        int sn_int;
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
    memcpy(buf_msg, cmd_get_software_version, sizeof(cmd_get_software_version));
    serial_write_bytes(sizeof(cmd_get_software_version));

    // Wait response
    memset(buf_msg, 0, sizeof(buf_msg));
    uint8_t nb = serial_read_bytes(15, CM1106_TIMEOUT);

    if (nb == 15 && buf_msg[0] == 0x16 && buf_msg[1] == 0x0c && buf_msg[2] == 0x1e && buf_msg[14] == calculate_cs(nb)) {
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
uint16_t CM1106_UART::get_co2() {

    uint16_t co2 = 0;

    // Ask CO2 value
    memcpy(buf_msg, cmd_get_co2, sizeof(cmd_get_co2));
    serial_write_bytes(sizeof(cmd_get_co2));

    // Wait response
    memset(buf_msg, 0, sizeof(buf_msg));
    uint8_t nb = serial_read_bytes(8, CM1106_TIMEOUT);
    if (nb == 8 && buf_msg[0] == 0x16 && buf_msg[1] == 0x05 && buf_msg[2] == 0x01 && buf_msg[7] == calculate_cs(nb)) {
        co2 = (buf_msg[3] * 256) + buf_msg[4];

#ifdef CM1106_DEBUG
        CM1106_DEBUG_SERIAL.printf("DEBUG: CO2 value = %u ppm\n", co2);
#endif

    } else {

#ifdef CM1106_DEBUG        
        CM1106_DEBUG_SERIAL.println("DEBUG: Error getting CO2 value!");
#endif

    }
    return co2;
}


/* Start calibration */
bool CM1106_UART::start_calibration(uint16_t concentration) {
    bool result = false;

    if (concentration >= 400 && concentration <= 1500) {
        memcpy(buf_msg, cmd_start_calibration, sizeof(cmd_start_calibration));
        buf_msg[3] = (concentration & 0xFF00 ) >> 8;
        buf_msg[4] = (concentration & 0xFF);
        buf_msg[5] = calculate_cs(6);
        serial_write_bytes(6);
        uint8_t nb = serial_read_bytes(8, CM1106_TIMEOUT);

        if (nb == 4 && buf_msg[0] == 0x16 && buf_msg[1] == 0x01 && buf_msg[2] == 0x03 && buf_msg[3] == calculate_cs(nb)) {
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
bool CM1106_UART::set_ABC(uint8_t open_close, uint8_t cycle, uint16_t base) {
    bool result = false;

    if ((open_close == CM1106_ABC_OPEN || open_close == CM1106_ABC_CLOSE) && cycle >= 1 && cycle <= 7 && base >= 400 && base <= 1499) {
        memcpy(buf_msg, cmd_set_ABC, sizeof(cmd_set_ABC));
        buf_msg[3] = 0x64;
        buf_msg[4] = open_close;
        buf_msg[5] = cycle;
        buf_msg[6] = (base & 0xFF00 ) >> 8;
        buf_msg[7] = (base & 0xFF);        
        buf_msg[8] = 0x64;
        buf_msg[9] = calculate_cs(10);
        serial_write_bytes(10);
        uint8_t nb = serial_read_bytes(10, CM1106_TIMEOUT);

        if (nb == 4 && buf_msg[0] == 0x16 && buf_msg[1] == 0x01 && buf_msg[2] == 0x10 && buf_msg[3] == calculate_cs(nb)) {
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

    // Get ABC parameters
    memcpy(buf_msg, cmd_get_ABC, sizeof(cmd_get_ABC));
    serial_write_bytes(sizeof(cmd_get_ABC));

    uint8_t nb = serial_read_bytes(10, CM1106_TIMEOUT);

    if (nb == 10 && buf_msg[0] == 0x16 && buf_msg[1] == 0x07 && buf_msg[2] == 0x0f && buf_msg[9] == calculate_cs(nb)) {
        abc->open_close = buf_msg[4];
        abc->cycle = buf_msg[5];
        abc->base = (buf_msg[6] * 256) + buf_msg[7];        
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
        CM1106_DEBUG_SERIAL.printf("DEBUG: Invalid packet\n");
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
