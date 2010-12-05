//
// working with serial
//

#include "serial2.h"

#if defined(WINDOWS)
# include <windows.h>
# define win32_err(s) FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, \
			GetLastError(), 0, (s), sizeof(s), NULL)
# define QUERYDOSDEVICE_BUFFER_SIZE 262144
#else
# error "This platform is unsupported, sorry"
#endif


Serial::Serial()
{
	port_is_open = 0;
	baud_rate = 38400;	// default baud rate
}

Serial::~Serial()
{
	close();
}

// Open a port, by number
int Serial::open(int port, int rate)
{
	close();

	port_number = port;
	baud_rate = rate;

#if defined(WINDOWS)
	COMMCONFIG cfg;
	COMMTIMEOUTS timeouts;
	int got_default_cfg=0;
	char buf[1024], name_createfile[64], name_commconfig[64], *p;
	DWORD len;

	sprintf(name_createfile, "\\\\.\\COM%d", port_number);
	sprintf(name_commconfig, "COM%d", port_number);

	len = sizeof(COMMCONFIG);
	if (GetDefaultCommConfig(name_commconfig, &cfg, &len)) {
		// this prevents unintentionally raising DTR when opening
		// might only work on COM1 to COM9
		got_default_cfg = 1;
		memcpy(&port_cfg_orig, &cfg, sizeof(COMMCONFIG));
		cfg.dcb.fDtrControl = DTR_CONTROL_DISABLE;
		cfg.dcb.fRtsControl = RTS_CONTROL_DISABLE;
		SetDefaultCommConfig(name_commconfig, &cfg, sizeof(COMMCONFIG));
	}
	else {
		printf("error with GetDefaultCommConfig\n");
	}
	port_handle = CreateFile(name_createfile, GENERIC_READ | GENERIC_WRITE,
	   0, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	if (port_handle == INVALID_HANDLE_VALUE) {
		printf("[!] Error: unable to open %s \n", name_commconfig);
		return -1;
	}
	len = sizeof(COMMCONFIG);
	if (!GetCommConfig(port_handle, &port_cfg, &len)) {
		CloseHandle(port_handle);
		printf("[!] Error: unable to read communication config on %s \n", name_commconfig);
		return -1;
	}
	if (!got_default_cfg) {
		memcpy(&port_cfg_orig, &port_cfg, sizeof(COMMCONFIG));
	}
	// http://msdn2.microsoft.com/en-us/library/aa363188(VS.85).aspx
	port_cfg.dcb.BaudRate = baud_rate;
	port_cfg.dcb.fBinary = TRUE;
	port_cfg.dcb.fParity = FALSE;
	port_cfg.dcb.fOutxCtsFlow = FALSE;
	port_cfg.dcb.fOutxDsrFlow = FALSE;
	port_cfg.dcb.fDtrControl = DTR_CONTROL_DISABLE;
	port_cfg.dcb.fDsrSensitivity = FALSE;
	port_cfg.dcb.fTXContinueOnXoff = TRUE;	// ???
	port_cfg.dcb.fOutX = FALSE;
	port_cfg.dcb.fInX = FALSE;
	port_cfg.dcb.fErrorChar = FALSE;
	port_cfg.dcb.fNull = FALSE;
	port_cfg.dcb.fRtsControl = RTS_CONTROL_DISABLE;
	port_cfg.dcb.fAbortOnError = FALSE;
	port_cfg.dcb.ByteSize = 8;
	port_cfg.dcb.Parity = NOPARITY;
	port_cfg.dcb.StopBits = ONESTOPBIT;
	if (!SetCommConfig(port_handle, &port_cfg, sizeof(COMMCONFIG))) {
		CloseHandle(port_handle);
		printf("[!] Error: unable to write communication config to %s \n", name_commconfig);
		return -1;
	}
	if (!EscapeCommFunction(port_handle, CLRDTR | CLRRTS)) {
		CloseHandle(port_handle);
		printf("[!] Error: unable to control serial port signals on %s \n", name_commconfig);
		return -1;
	}
	// http://msdn2.microsoft.com/en-us/library/aa363190(VS.85).aspx
	// setting to all zeros means timeouts are not used
	//timeouts.ReadIntervalTimeout		= 0;
	timeouts.ReadIntervalTimeout		= MAXDWORD;
	timeouts.ReadTotalTimeoutMultiplier	= 0;
	timeouts.ReadTotalTimeoutConstant	= 0;
	timeouts.WriteTotalTimeoutMultiplier	= 0;
	timeouts.WriteTotalTimeoutConstant	= 0;
	if (!SetCommTimeouts(port_handle, &timeouts)) {
		CloseHandle(port_handle);
		printf("[!] Error: unable to write timeout settings to %s \n", name_commconfig);
		return -1;
	}
#endif
	port_is_open = 1;
	return 0;
}

// Close the port
void Serial::close()
{
	if (!port_is_open){
		return;
	}
	outputFlush();
	discardInput();
	port_is_open = 0;
	port_number = 0;
#if defined(WINDOWS)
	//SetCommConfig(port_handle, &port_cfg_orig, sizeof(COMMCONFIG));
	CloseHandle(port_handle);
#endif
}

int Serial::connected()
{
	return port_is_open;
}

int Serial::available()
{
	if (!port_is_open){
		return 0;
	}
#if defined(WINDOWS)
	// first, we'll find out how many bytes have been received
	// and are currently waiting for us in the receive buffer.
	//   http://msdn.microsoft.com/en-us/library/ms885167.aspx
	//   http://msdn.microsoft.com/en-us/library/ms885173.aspx
	//   http://source.winehq.org/WineAPI/ClearCommError.html
	COMSTAT st;
	DWORD errmask=0, num_read, num_request;
	OVERLAPPED ov;
	int r;
	if (!ClearCommError(port_handle, &errmask, &st)){
		return -1;
	}
	//printf("Read, %d requested, %lu buffered\n", count, st.cbInQue);
	if (st.cbInQue <= 0){
		return 0;
	}
	return st.cbInQue;
#endif
	return 0;
}

// set the baud rate
int Serial::setBaud(int baud)
{
	if (baud <= 0){
		return -1;
	}
	printf("[i] set baud: %d\n", baud);
	baud_rate = baud;
	if (!port_is_open){
		return -1;
	}
#if defined(WINDOWS)
	DWORD len = sizeof(COMMCONFIG);
	port_cfg.dcb.BaudRate = baud;
	SetCommConfig(port_handle, &port_cfg, len);
#endif
	return 0;
}

// Read from the serial port.  Returns only the bytes that are
// already received, up to count.  This always returns without delay,
// returning 0 if nothing has been received
int Serial::read(void *ptr, int count)
{
	if (!port_is_open){
		return -1;
	}
	if (count <= 0){ 
		return 0;
	}
#if defined(WINDOWS)
	// first, we'll find out how many bytes have been received
	// and are currently waiting for us in the receive buffer.
	//   http://msdn.microsoft.com/en-us/library/ms885167.aspx
	//   http://msdn.microsoft.com/en-us/library/ms885173.aspx
	//   http://source.winehq.org/WineAPI/ClearCommError.html
	COMSTAT st;
	DWORD errmask=0, num_read, num_request;
	OVERLAPPED ov;
	int r;
	if (!ClearCommError(port_handle, &errmask, &st)){
		return -1;
	}
	printf("[i] Read, %d requested, %lu buffered\n", count, st.cbInQue);
	if (st.cbInQue <= 0){
		return 0;
	}
	// now do a ReadFile, now that we know how much we can read
	// a blocking (non-overlapped) read would be simple, but win32
	// is all-or-nothing on async I/O and we must have it enabled
	// because it's the only way to get a timeout for WaitCommEvent
	num_request = ((DWORD)count < st.cbInQue) ? (DWORD)count : st.cbInQue;
	ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (ov.hEvent == NULL){
		return -1;
	}
	ov.Internal = ov.InternalHigh = 0;
	ov.Offset = ov.OffsetHigh = 0;
	if (ReadFile(port_handle, ptr, num_request, &num_read, &ov)){
		// this should usually be the result, since we asked for
		// data we knew was already buffered
		//printf("Read, immediate complete, num_read=%lu\n", num_read);
		r = num_read;
	}
	else {
		if (GetLastError() == ERROR_IO_PENDING) {
			if (GetOverlappedResult(port_handle, &ov, &num_read, TRUE)) {
				//printf("Read, delayed, num_read=%lu\n", num_read);
				r = num_read;
			}
			else {
				//printf("Read, delayed error\n");
				r = -1;
			}
		}
		else {
			printf("[!] Error: read, error\n");
			r = -1;
		}
	}
	CloseHandle(ov.hEvent);
	// TODO: how much overhead does creating new event objects on
	// every I/O request really cost?  Would it be better to create
	// just 3 when we open the port, and reset/reuse them every time?
	// Would mutexes or critical sections be needed to protect them?
	return r;
#endif
	return 0;
}

// Write to the serial port.  This blocks until the data is
// sent (or in a buffer to be sent).  All bytes are written,
// unless there is some sort of error.
int Serial::write(const void *ptr, int len)
{
	//printf("Write %d\n", len);
	if (!port_is_open){
		return -1;
	}
#if defined(WINDOWS)
	DWORD num_written;
	OVERLAPPED ov;
	int r;
	ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (ov.hEvent == NULL) return -1;
	ov.Internal = ov.InternalHigh = 0;
	ov.Offset = ov.OffsetHigh = 0;
	if (WriteFile(port_handle, ptr, len, &num_written, &ov)) {
		//printf("Write, immediate complete, num_written=%lu\n", num_written);
		r = num_written;
	}
	else {
		if (GetLastError() == ERROR_IO_PENDING) {
			if (GetOverlappedResult(port_handle, &ov, &num_written, TRUE)) {
				//printf("Write, delayed, num_written=%lu\n", num_written);
				r = num_written;
			}
			else {
				//printf("Write, delayed error\n");
				r = -1;
			}
		}
		else {
			printf("[!] Error: write, error\n");
			r = -1;
		}
	};
	CloseHandle(ov.hEvent);
	return r;
#endif
	return 0;
}

// Wait up to msec for data to become available for reading.
// return 0 if timeout, or non-zero if one or more bytes are
// received and can be read.  -1 if an error occurs
int Serial::waitInput(int msec)
{
	if (!port_is_open){
		return -1;
	}
#if defined(WINDOWS)
	// http://msdn2.microsoft.com/en-us/library/aa363479(VS.85).aspx
	// http://msdn2.microsoft.com/en-us/library/aa363424(VS.85).aspx
	// http://source.winehq.org/WineAPI/WaitCommEvent.html
	COMSTAT st;
	DWORD errmask=0, eventmask=EV_RXCHAR, ret;
	OVERLAPPED ov;
	int r;
	// first, request comm event when characters arrive
	if (!SetCommMask(port_handle, EV_RXCHAR)){
		return -1;
	}
	// look if there are characters in the buffer already
	if (!ClearCommError(port_handle, &errmask, &st)){
		return -1;
	}
	//printf("Input_wait, %lu buffered, timeout = %d ms\n", st.cbInQue, msec);
	if (st.cbInQue > 0){
		return 1;
	}

	ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (ov.hEvent == NULL) return -1;
	ov.Internal = ov.InternalHigh = 0;
	ov.Offset = ov.OffsetHigh = 0;
	if (WaitCommEvent(port_handle, &eventmask, &ov)) {
		//printf("Input_wait, WaitCommEvent, immediate success\n");
		r = 1;
	} 
	else {
		if (GetLastError() == ERROR_IO_PENDING) {
			ret = WaitForSingleObject(ov.hEvent, msec);
			if (ret == WAIT_OBJECT_0) {
				//printf("Input_wait, WaitCommEvent, delayed success\n");
				r = 1;
			}
			else if (ret == WAIT_TIMEOUT) {
				//printf("Input_wait, WaitCommEvent, timeout\n");
				GetCommMask(port_handle, &eventmask);
				r = 0;
			}
			else {  // WAIT_FAILED or WAIT_ABANDONED
				printf("[!] Error: Input_wait, WaitCommEvent, delayed error\n");
				r = -1;
			}
		}
		else {
			printf("[!] Error: Input_wait, WaitCommEvent, immediate error\n");
			r = -1;
		}
	}
	SetCommMask(port_handle, 0);
	CloseHandle(ov.hEvent);
	return r;
#endif
	return 0;
}

// Discard all received data that hasn't been read
void Serial::discardInput(void)
{
	if (!port_is_open){
		return;
	}
#if defined(WINDOWS)
	PurgeComm(port_handle, PURGE_RXCLEAR);
#endif
}

// Wait for all transmitted data with Write to actually leave the serial port
void Serial::outputFlush(void)
{
	if (!port_is_open){
		return;
	}
#if defined(WINDOWS)
	FlushFileBuffers(port_handle);
#endif
}


// set DTR and RTS,  0 = low, 1=high, -1=unchanged
int Serial::setControl(int dtr, int rts)
{
	if (!port_is_open){
		return -1;
	}
#if defined(WINDOWS)
	// http://msdn.microsoft.com/en-us/library/aa363254(VS.85).aspx
	if (dtr == 1) {
		if (!EscapeCommFunction(port_handle, SETDTR)){
			return -1;
		}
	}
	else if (dtr == 0) {
		if (!EscapeCommFunction(port_handle, CLRDTR)){
			return -1;
		}
	}
	if (rts == 1) {
		if (!EscapeCommFunction(port_handle, SETRTS)){
			return -1;
		}
	}
	else if (rts == 0) {
		if (!EscapeCommFunction(port_handle, CLRRTS)){
			return -1;
		}
	}
#endif
	return 0;
}

