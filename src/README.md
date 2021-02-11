# Library

Please, you change the settings for your board in file cm1106_uart.h

#define _CM1106_SERIAL 0   // Serial communication for communication with sensor: 0 = Softserial, 1 = Hardware Serial, 2 = Hardware Serial Port 2

if you use Softserial also you must change:
#define CM1106_SERIAL_RX 13
#define CM1106_SERIAL_TX 15
