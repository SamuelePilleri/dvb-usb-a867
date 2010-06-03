/**
 * Maxlinear_MXL5007.h
 *
 * Interface btw Afa9035 and Mxl5007T
 */
#ifndef __Maxlinear_MXL5007_H__
#define __Maxlinear_MXL5007_H__

//#include <wdm.h>	//for LARGE_INTEGER
//#include <stdio.h>
#include "type.h"
#include "error.h"
#include "user.h"
#include "register.h"
#include "standard.h"
#include "MxL5007.h"
//#include "MxL5007_API.h"
//#include "Maxlinear_MXL5007_Script.h"
#include "Common.h"	//for Tuner_struct


extern TunerDescription tuner_MXL5007;

//ValueSet Maxlinear_MXL5005_EXT_script[];

/**
 *
 */
#if 0 // n001
UINT32 MxL_I2C_Write(UINT8 DeviceAddr, void* pArray, UINT32 count, MxL5007_TunerConfigS* myTuner);
#else // n001
UINT32 MxL_I2C_Write(UINT8 DeviceAddr, void* pArray, UINT32 ByteCount, MxL5007_TunerConfigS* myTuner);
#endif // n001
UINT32 MxL_I2C_Read(UINT8 DeviceAddr, UINT8 Addr, UINT8* mData, MxL5007_TunerConfigS* myTuner);
void MxL_Delay(UINT32 mSec);

/**
 *
 */
Dword MXL5007_open (
	IN  Demodulator*	demodulator,
	IN  Byte			chip
);


/**
 *
 */
Dword MXL5007_close (
	IN  Demodulator*	demodulator,
	IN  Byte			chip
);


/**
 *
 */
Dword MXL5007_set (
	IN  Demodulator*	demodulator,
	IN  Byte			chip,
    IN  Word			bandwidth,
    IN  Dword			frequency
);
#endif //__Maxlinear_MXL5007_H__
