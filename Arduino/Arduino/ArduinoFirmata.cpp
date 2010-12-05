#include "ArduinoFirmata.h"

ArduinoFirmata::ArduinoFirmata(void)
{
	this->init();
}

ArduinoFirmata::ArduinoFirmata(int _port, int _rate)
{
	this->init();
	if( this->open(_port, _rate) ){
		printf("[!][ArduinoFirmata] Error: cant open port: %d \n", _port);
	}
}

ArduinoFirmata::~ArduinoFirmata(void)
{
	this->close();
}

void ArduinoFirmata::init()
{
	int i=0;

	opened = 0;
	port = 0;
	rate = 0;
	waitForData = 0;

	for(i=0; i<INPUT_SIZE; i++){
		digitalOutputData[i] = 0;
		digitalInputData[i] = 0;
		analogInputData[i] = 0;
	}

	for(i=0; i<MAX_DATA_BYTES; i++){
		inputData[i] = 0;
	}

	majorVersion = 0;
	minorVersion = 0;
}

// open serial port
int ArduinoFirmata::open(int _port, int _rate)
{
	if( !serial.open(_port, _rate) ){
		port = _port;
		rate = _rate;
		opened = 1;
		return 0;
	}
	return -1;
}

// close serial port
void ArduinoFirmata::close()
{
	opened = 0;
	serial.close();
}

// Returns the last known value read from the digital pin: HIGH or LOW.
//
// pin - the digital pin whose value should be returned (from 2 to 13,
// since pins 0 and 1 are used for serial communication) 
//
int ArduinoFirmata::digitalRead(int pin)
{
	if(pin<INPUT_SIZE){
		return (digitalInputData[pin >> 3] >> (pin & 0x07)) & 0x01;
	}
	return -1;
}

// Returns the last known value read from the analog pin: 0 (0 volts) to 1023 (5 volts).
//
// pin - the analog pin whose value should be returned (from 0 to 5) 
//
int ArduinoFirmata::analogRead(int pin)
{
	if(pin<INPUT_SIZE){
		return analogInputData[pin];
	}
	return -1;
}

// Set a digital pin to input or output mode.
//
// pin - the pin whose mode to set (from 2 to 13)
// mode - INPUT or OUTPUT
//
void ArduinoFirmata::pinMode(int pin, int mode)
{
	serial.write(SET_PIN_MODE);
	serial.write(pin);
	serial.write(mode);
}

// Write to a digital pin (the pin must have been put into output mode with pinMode()).
//
// pin - the pin to write to (from 2 to 13)
// value - the value to write: LOW (0-2 volts) or HIGH (3-5 volts)
//
void ArduinoFirmata::digitalWrite(int pin, int value)
{
	if(pin>INPUT_SIZE){
		printf("[!][ArduinoFirmata] Error: too big pin number: %d \n", pin);
		return;
	}

	int portNumber = (pin >> 3) & 0x0F;

	if (value == 0){
		digitalOutputData[portNumber] &= ~(1 << (pin & 0x07));
	}
	else{
		digitalOutputData[portNumber] |= (1 << (pin & 0x07));
	}

	serial.write(DIGITAL_MESSAGE | portNumber);
	serial.write(digitalOutputData[portNumber] & 0x7F);
	serial.write(digitalOutputData[portNumber] >> 7);  
}

// Write an analog value (PWM-wave) to a digital pin.
//
// pin - the pin to write to (must be 9, 10, or 11, as those are they
// only ones which support hardware pwm)
// value - 0 being the lowest (always off), and 255 the highest (always on)
//
void ArduinoFirmata::analogWrite(int pin, int value)
{
	//pinMode(pin, PWM);
	serial.write(ANALOG_MESSAGE | (pin & 0x0F));
	serial.write(value & 0x7F);
	serial.write(value >> 7);
}

void ArduinoFirmata::sendSysEx(int command, char* data, int length)
{
	if(data && length>0){
		serial.write(START_SYSEX);
		serial.write(command);
		for(int i=0; i<length; i++){
			serial.write( data[i] );
		}
		serial.write(END_SYSEX);
	}
}

void ArduinoFirmata::sendSysExBegin()
{
	serial.write(START_SYSEX);
}

void ArduinoFirmata::sendSysExEnd()
{
	serial.write(END_SYSEX);
}

void ArduinoFirmata::sendString(char* str, int length)
{
	if(str && length>0){
		serial.write(START_SYSEX);
		serial.write(FIRMATA_STRING);
		for(int i=0; i<length; i++){
			int value = (int)str[i];
			serial.write(value & 0x7F);
			serial.write(value >> 7);
		}
		serial.write(END_SYSEX);
	}
}

void ArduinoFirmata::sendVersionRequest()
{
	serial.write(REPORT_VERSION);
}

void ArduinoFirmata::sendReset()
{
	serial.write(SYSTEM_RESET);
} 

int ArduinoFirmata::available()
{
	return serial.available();
}

void ArduinoFirmata::setVersion(int majorVersion, int minorVersion)
{
	printf("[i][ArduinoFirmata] set version: %d . %d \n", majorVersion, minorVersion);
	this->majorVersion = majorVersion;
	this->minorVersion = minorVersion;
}

void ArduinoFirmata::update()
{
	int buf[1024];
	int r;

	r = serial.waitInput(40);
	if (r > 0) {
		r = serial.read(buf, sizeof(buf));
		if (r < 0) {
			// error
			printf("[i][ArduinoFirmata] no data!");
			return;
		}
		if (r > 0) {
			// parse
			//for (int i=0; i < r; i++) {
			//	printf("%02X ", buf[i]);
			//}
			//printf("\n");
			this->parse(buf, r);
		}
	} 
	else if (r < 0) {
		printf("[i] no data!");
		return;
	}  
}

void ArduinoFirmata::parse(int *buf, int len)
{
	int *p, *end;

	p = buf;
	end = p + len;
	for (p = buf; p < end; p++) {
		int msn = *p & 0xF0;
		if (msn == ANALOG_MESSAGE || msn == DIGITAL_MESSAGE || *p == REPORT_VERSION) {
			parse_command_len = 3;
			parse_count = 0;
		} 
		else if (msn == REPORT_ANALOG || msn == REPORT_DIGITAL) {
			parse_command_len = 2;
			parse_count = 0;
		} 
		else if (*p == START_SYSEX) {
			parse_count = 0;
			parse_command_len = sizeof(parse_buf);
		} 
		else if (*p == END_SYSEX) {
			parse_command_len = parse_count + 1;
		} 
		else if (*p & 0x80) {
			parse_command_len = 1;
			parse_count = 0;
		}
		if (parse_count < (int)sizeof(parse_buf)) {
			parse_buf[parse_count++] = *p;
		}
		if (parse_count == parse_command_len) {
			this->doMessage();
			parse_count = parse_command_len = 0;
		}
	} 
}

void ArduinoFirmata::doMessage()
{
	int cmd = (parse_buf[0] & 0xF0);

	printf("[i][ArduinoFirmata] message, %d bytes, %02X\n", parse_count, parse_buf[0]);

	if (cmd == ANALOG_MESSAGE && parse_count == 3) {
		int analog_ch = (parse_buf[0] & 0x0F);
		int analog_val = parse_buf[1] | (parse_buf[2] << 7);
		printf("[i][ArduinoFirmata] A%d = %d\n", analog_ch, analog_val);
		analogInputData[analog_ch] = analog_val;
		return;
	}
	if (cmd == DIGITAL_MESSAGE && parse_count == 3) {
		int port_num = (parse_buf[0] & 0x0F);
		int port_val = parse_buf[1] | (parse_buf[2] << 7);
		int pin = port_num * 8;
		printf("[i][ArduinoFirmata] port_num = %d, port_val = %d, pin = %d\n", port_num, port_val, pin);
		digitalInputData[port_num] = port_val;		
		return;
	}
	if(cmd == REPORT_VERSION && parse_count == 3){
		this->setVersion(parse_buf[1], parse_buf[2]);
	}

	if (parse_buf[0] == START_SYSEX && parse_buf[parse_count-1] == END_SYSEX) {
		// Sysex message
		if (parse_buf[1] == REPORT_FIRMWARE) {
			char name[140];
			int len=0;
			for (int i=4; i < parse_count-2; i+=2) {
				name[len++] = (parse_buf[i] & 0x7F)
				  | ((parse_buf[i+1] & 0x7F) << 7);
			}
			name[len++] = '-';
			name[len++] = parse_buf[2] + '0';
			name[len++] = '.';
			name[len++] = parse_buf[3] + '0';
			name[len++] = 0;
			printf("[i][ArduinoFirmata] firmata name: %s\n", name);
			// query the board's capabilities only after hearing the
			// REPORT_FIRMWARE message.  For boards that reset when
			// the port open (eg, Arduino with reset=DTR), they are
			// not ready to communicate for some time, so the only
			// way to reliably query their capabilities is to wait
			// until the REPORT_FIRMWARE message is heard.
			int buf[80];
			len=0;
			buf[len++] = START_SYSEX;
			buf[len++] = ANALOG_MAPPING_QUERY; // read analog to pin # info
			buf[len++] = END_SYSEX;
			buf[len++] = START_SYSEX;
			buf[len++] = CAPABILITY_QUERY; // read capabilities
			buf[len++] = END_SYSEX;
			for (int i=0; i<16; i++) {
				buf[len++] = 0xC0 | i;  // report analog
				buf[len++] = 1;
				buf[len++] = 0xD0 | i;  // report digital
				buf[len++] = 1;
			}
			serial.write(buf, len);
		}
		else if (parse_buf[1] == CAPABILITY_RESPONSE) {
			printf("[i][ArduinoFirmata] CAPABILITY_RESPONSE\n");
		} 
		else if (parse_buf[1] == ANALOG_MAPPING_RESPONSE) {
			printf("[i][ArduinoFirmata] ANALOG_MAPPING_RESPONSE\n");
			return;
		} 
		else if (parse_buf[1] == PIN_STATE_RESPONSE && parse_count >= 6) {
			printf("[i][ArduinoFirmata] PIN_STATE_RESPONSE\n");
			//int pin = parse_buf[2];
			//pin_info[pin].mode = parse_buf[3];
			//pin_info[pin].value = parse_buf[4];
			//if (parse_count > 6) pin_info[pin].value |= (parse_buf[5] << 7);
			//if (parse_count > 7) pin_info[pin].value |= (parse_buf[6] << 14);
			//add_pin(pin);
		}
		return;
	} 
}