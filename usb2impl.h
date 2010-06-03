#ifndef __USB2IMPL_H__
#define __USB2IMPL_H__


#include "type.h"
#include "error.h"
#include "user.h"
#include "cmd.h"


Dword Usb2_getDriver (
	IN  Demodulator*	demodulator,
	OUT Handle*			handle
);


Dword Usb2_writeControlBus (
	IN  Demodulator*	demodulator,
	IN  Dword			bufferLength,
	IN  Byte*			buffer
);


Dword Usb2_readControlBus (
	IN  Demodulator*	demodulator,
	IN  Dword			bufferLength,
	OUT Byte*			buffer
);


Dword Usb2_readDataBus (
	IN  Demodulator*	demodulator,
	IN  Dword			bufferLength,
	OUT Byte*			buffer
);

#endif

