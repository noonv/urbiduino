//
// OpenRoboVision
//
// for working with Arduino/CraftDuino
// via Firmata protocol (http://firmata.org)
//
// для работы с контроллером Arduino/CraftDuino
// по протоколу Firmata (http://firmata.org)
//
//
// robocraft.ru
//


#ifndef _ARDUINO_FIRMATA_H_
#define _ARDUINO_FIRMATA_H_

#include "serial2.h"

// Constant to set a pin to output mode (in a call to pinMode()).
#define OUTPUT	0
// Constant to set a pin to input mode (in a call to pinMode()).
#define INPUT	1
// Constant to set a pin to analog mode (in a call to pinMode()).
#define ANALOG	2
// Constant to set a pin to PWM mode (in a call to pinMode()).
#define PWM		3
// Constant to set a pin to servo mode (in a call to pinMode()).
#define SERVO	4
// Constant to set a pin to shiftIn/shiftOut mode (in a call to pinMode())
#define SHIFT	5
// Constant to set a pin to I2C mode (in a call to pinMode()). 
#define I2C		6

// Constant to write a high value (+5 volts) to a pin (in a call to digitalWrite()).
#define LOW		0
// Constant to write a low value (0 volts) to a pin (in a call to digitalWrite()).
#define HIGH	1

// send data for a digital port 
#define DIGITAL_MESSAGE		0x90
// send data for an analog pin (or PWM) 
#define ANALOG_MESSAGE		0xE0
// enable analog input by pin
#define REPORT_ANALOG		0xC0
// enable digital input by port 
#define REPORT_DIGITAL		0xD0
// set a pin to INPUT/OUTPUT/PWM/etc 
#define SET_PIN_MODE		0xF4
// report firmware version 
#define REPORT_VERSION		0xF9
// reset from MIDI 
#define SYSTEM_RESET 		0xFF
// start a MIDI SysEx message 
#define START_SYSEX 		0xF0
// end a MIDI SysEx message 
#define END_SYSEX 			0xF7

#define MODE_INPUT    0x00
#define MODE_OUTPUT   0x01
#define MODE_ANALOG   0x02
#define MODE_PWM      0x03
#define MODE_SERVO    0x04
#define MODE_SHIFT    0x05
#define MODE_I2C      0x06

#define FIRMATA_STRING          0x71
#define REPORT_FIRMWARE         0x79 // report name and version of the firmware

#define PIN_MODE_QUERY          0x72 // ask for current and supported pin modes
#define PIN_MODE_RESPONSE       0x73 // reply with current and supported pin modes
#define PIN_STATE_QUERY         0x6D
#define PIN_STATE_RESPONSE      0x6E
#define CAPABILITY_QUERY        0x6B
#define CAPABILITY_RESPONSE     0x6C
#define ANALOG_MAPPING_QUERY    0x69
#define ANALOG_MAPPING_RESPONSE 0x6A

#define INPUT_SIZE 16
#define MAX_DATA_BYTES 32

class ArduinoFirmata
{
	Serial serial;
public:
	ArduinoFirmata(void);
	ArduinoFirmata(int port, int rate);
	~ArduinoFirmata(void);

	void init();

	// open serial port
	int open(int port, int rate);
	// close serial port
	void close();

	//
	// Arduino functions 
	//

	// Returns the last known value read from the digital pin: HIGH or LOW.
	int digitalRead(int pin);
	// Returns the last known value read from the analog pin: 0 (0 volts) to 1023 (5 volts).
	int analogRead(int pin);
	// Set a digital pin to input or output mode.
	void pinMode(int pin, int mode);
	// Write to a digital pin (the pin must have been put into output mode with pinMode()).
	void digitalWrite(int pin, int value);
	// Write an analog value (PWM-wave) to a digital pin.
	void analogWrite(int pin, int value);

	// SysEx
	void sendSysEx(int command, char* data, int length);
	void sendSysExBegin();
	void sendSysExEnd();
	void sendString(char* str, int length);

	// ask Firmata version
	void sendVersionRequest();
	// send reset
	void sendReset();

	// available data in serial
	int available();

	void update();
	void parse(int *buf, int len);
	void doMessage();


	// set version
	void setVersion(int majorVersion, int minorVersion);

	int opened;
	int port;
	int rate;	

	int waitForData;

	// Firmata version
	int majorVersion;
	int minorVersion;

	int digitalOutputData[INPUT_SIZE];
	int digitalInputData[INPUT_SIZE];
	int analogInputData[INPUT_SIZE];

	int inputData[MAX_DATA_BYTES];

	int parse_count;
	int parse_command_len;
	int parse_buf[4096];
};

#endif //#ifndef _ARDUINO_FIRMATA_H_