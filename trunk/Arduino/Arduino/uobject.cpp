//
// Arduino UObject implementation for URBI
// based on Firmata protocol (http://firmata.org)
//
// Реализация UObject-а URBI для взаимодействия с Arduino
// по протоколу Firmata (http://firmata.org)
//
//
// Author: noonv
// http://robocraft.ru
//
// License: BSD
//
// http://code.google.com/p/urbiduino/
//

#include <urbi/uobject.hh>

// for work with Arduino via Firmata
#include "ArduinoFirmata.h"

class Arduino: public urbi::UObject, ArduinoFirmata
{
public:
	Arduino(const std::string& str);
	~Arduino();
	int init(int port, int rate);


	//
	// variables
	//
	//urbi::UVar port;
	//urbi::UVar rate;

	//
	// functions
	//

};

Arduino::Arduino(const std::string& s)
: urbi::UObject(s)
{
	//port = 0;
	//rate = 0;

	//
	// bind variables
	//
	//UBindVar (Arduino, port);
	//UBindVar (Arduino, rate);

	//
	// bind functions
	//

	UBindFunction(Arduino, init);

	//
	// bind functions from ArduinoFirmata
	//
	UBindFunction(ArduinoFirmata, open);
	UBindFunction(ArduinoFirmata, close);
	UBindFunction(ArduinoFirmata, digitalRead);
	UBindFunction(ArduinoFirmata, analogRead);
	UBindFunction(ArduinoFirmata, pinMode);
	UBindFunction(ArduinoFirmata, digitalWrite);
	UBindFunction(ArduinoFirmata, analogWrite);

//	UBindFunction(ArduinoFirmata, sendSysEx);
	UBindFunction(ArduinoFirmata, sendSysExBegin);
	UBindFunction(ArduinoFirmata, sendSysExEnd);
//	UBindFunction(ArduinoFirmata, sendString);

	UBindFunction(ArduinoFirmata, sendVersionRequest);
	UBindFunction(ArduinoFirmata, sendReset);

	UBindFunction(ArduinoFirmata, available);
	UBindFunction(ArduinoFirmata, update);

	UBindFunction(ArduinoFirmata, setVersion);
};

Arduino::~Arduino()
{
	close();
	//port = 0;
	//rate = 0;
}

int Arduino::init(int _port, int _rate)
{
	open(_port, _rate);

	//port = _port;
	//rate = _rate;

	return 0;
};

UStart(Arduino);
