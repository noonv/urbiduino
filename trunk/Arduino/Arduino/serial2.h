//
// working with serial
//

#ifndef _SERIAL2_H_
#define _SERIAL2_H_

#ifndef WINDOWS
# define WINDOWS 1
#endif

#include <stdio.h>
#include <stdlib.h>

#if defined(WINDOWS)
# include <windows.h>
#endif

class Serial
{
public:
	Serial();
	~Serial();

	// open the port
	int open(int port, int rate);
	// Close the port
	void close();

	int connected();
	int available();

	// Read from the serial port.  Returns only the bytes that are
	// already received, up to count.
	int read(void *ptr, int count);
	// Write to the serial port.
	int write(const void *ptr, int len);
	int write(int val){ char buf[1]; buf[0] = val&0xff; return write(buf, 1); };

	void outputFlush();

	// set the baud rate
	int setBaud(int baud);

	void discardInput(void);

	int waitInput(int msec);
	
	// set DTR and RTS,  0 = low, 1=high, -1=unchanged
	int setControl(int dtr, int rts);
	
private:
	int port_is_open;

	int port_number;
	int baud_rate;

#if defined(WINDOWS)
	HANDLE port_handle;
	COMMCONFIG port_cfg_orig;
	COMMCONFIG port_cfg;
#endif
};

#endif //#ifndef _SERIAL2_H_