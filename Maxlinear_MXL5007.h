/** * Maxlinear_MXL5007.h * * Interface btw Afa9035 and Mxl5007T */
#ifndef __Maxlinear_MXL5007_H__#define __Maxlinear_MXL5007_H__#include "type.h"#include "error.h"#include "user.h"#include "register.h"#include "standard.h"#include "MxL5007.h"#include "Common.h"	//for Tuner_struct
extern TunerDescription tuner_MXL5007;
/** * */
UINT32 MxL_I2C_Write(UINT8 DeviceAddr, void* pArray, UINT32 ByteCount, MxL5007_TunerConfigS* myTuner);UINT32 MxL_I2C_Read(UINT8 DeviceAddr, UINT8 Addr, UINT8* mData, MxL5007_TunerConfigS* myTuner);void MxL_Delay(UINT32 mSec);
/** * */Dword MXL5007_open (	IN  Demodulator*	demodulator,	IN  Byte			chip);
/** * */Dword MXL5007_close (	IN  Demodulator*	demodulator,	IN  Byte			chip);
/** * */Dword MXL5007_set (	IN  Demodulator*	demodulator,	IN  Byte			chip,    IN  Word			bandwidth,    IN  Dword			frequency);#endif //__Maxlinear_MXL5007_H__
