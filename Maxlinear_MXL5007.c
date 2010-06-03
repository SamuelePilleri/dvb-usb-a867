/**
 * Maxlinear_MXL5007.cpp
 *
 * Interface btw Afa9035 and Mxl5007T
 */
/*
#include <stdio.h>
#include "type.h"
#include "error.h"
#include "user.h"
#include "register.h"
#include "standard.h"
#include "MXL5007.h"
#include "MXL5007_API.h"
#include "Maxlinear_MXL5007_Script.h"
#include "Common.h"	//for Tuner_struct
*/


#include "Maxlinear_MXL5007.h"
#include "Maxlinear_MXL5007_Script.h"
//#include <Wdm.h>
#include "MxL5007_API.h"
#include "af903x.h"

//s
MxL5007_TunerConfigS MxL5007_TunerConfig;
//e


Dword MXL5007_WriteI2C(
	IN  Demodulator*	demodulator,
	IN  Byte			chip,
	IN  Byte*			pAddress,
	IN  Byte*			pData,
	IN  Dword			count
) {
    Dword error = Error_NO_ERROR;                /* Status to be returned        */
    Byte buffer[25];
    //Byte numberOfRegisters = 9;
    Byte i;
    
/** single write */
    for (i = 0; i < count; i++) {
        buffer[0] = *(pAddress + i);
        buffer[1] = *(pData + i);
        //buffer[2] = 0xFE;  /** Latch CMD request by MXL5005 Confidential Datasheet */
		error = Standard_writeTunerRegisters (demodulator, chip, buffer[0], 1, &buffer[1]);
		if (error){
			deb_data("MXL5007_WriteI2C failed");
			goto exit;
		}
    }

exit:
    return (error);
}

#if 0 // n001
UINT32 MxL_I2C_Write(UINT8 DeviceAddr, void* pArray, UINT32 count, MxL5007_TunerConfigS* myTuner){

    Dword error = Error_NO_ERROR;                /* Status to be returned */
    Byte buffer[25];
    Byte i;
	Byte* pData;
	pData = (Byte*)pArray;
    
/** single write */
    for (i = 0; i < count; i++) {
        buffer[0] = *(pData + 2*i);
        buffer[1] = *(pData + 2*i + 1);
        //buffer[2] = 0xFE;  /** Latch CMD request by MXL5005 Confidential Datasheet */
		error = Standard_writeTunerRegisters ((myTuner->demodulator), (myTuner->chip), buffer[0], 1, &buffer[1]);
		if (error){
			KdPrint(("MxL_I2C_Write failed"));
			goto exit;
		}
    }

exit:
    return (error);
}
#else // n001
UINT32 MxL_I2C_Write(UINT8 DeviceAddr, void* pArray, UINT32 ByteCount, MxL5007_TunerConfigS* myTuner){

    Dword error = Error_NO_ERROR;                /* Status to be returned */
    Byte buffer[25];
    Byte i;
	Byte* pData;
	pData = (Byte*)pArray;
    
/** single write */
    //buffer[2] = 0xFE;  /** Latch CMD request by MXL5005 Confidential Datasheet */
	if (ByteCount == 1)
		error = Standard_writeTunerRegisters ((myTuner->demodulator), (myTuner->chip), pData[0], 0, NULL);
	else if (ByteCount > 1)
		error = Standard_writeTunerRegisters ((myTuner->demodulator), (myTuner->chip), pData[0], ByteCount - 1, &pData[1]);
	else
	{	
		error = Error_INVALID_DATA_LENGTH;
		deb_data("MxL_I2C_Write:: ByteCount = 0"); 
	}

	if (error){
		deb_data("MxL_I2C_Write failed");
		goto exit;
	}

exit:
    return (error);
}
#endif // n001

UINT32 MxL_I2C_Read(UINT8 DeviceAddr, UINT8 Addr, UINT8* mData, MxL5007_TunerConfigS* myTuner){

	Dword error = Error_NO_ERROR;

#if 0 // n001, for matching MXL5007 I2C format
	error = Standard_readTunerRegisters (myTuner->demodulator, myTuner->chip, Addr, 1, mData);
#else // n001
	error = Standard_writeTunerRegisters(myTuner->demodulator, myTuner->chip, 0xFB, 1, &Addr);
	error = Standard_readTunerRegisters (myTuner->demodulator, myTuner->chip, 0xffff, 1, mData);
#endif // n001

	return (error);
}

void MxL_Delay(UINT32 mSec){

	User_delay(NULL, mSec);
}

Dword MXL5007_open (
	IN  Demodulator*	demodulator,
	IN  Byte			chip
) {
	Ganymede* ganymede;
	ganymede = (Ganymede*) demodulator;

	MxL5007_TunerConfig.demodulator = demodulator;
	MxL5007_TunerConfig.chip = chip;

	//MxL5007_TunerConfig.I2C_Addr = MxL_I2C_ADDR_96;	// 7-bit address
	MxL5007_TunerConfig.I2C_Addr = (MxL5007_I2CAddr)(ganymede->tunerDescription->tunerAddress); //Get from tunerDescription
	//MxL5007_TunerConfig.IF_Freq = MxL_IF_4_57_MHZ;
	MxL5007_TunerConfig.IF_Freq = (MxL5007_IF_Freq)(ganymede->tunerDescription->ifFrequency); //Get from tunerDescription
	//MxL5007_TunerConfig.IF_Spectrum = MxL_NORMAL_IF;

#if 0 // n001, IF spectrum of tuner is inverse IF spectrum of demodulator 
	MxL5007_TunerConfig.IF_Spectrum = (MxL5007_IF_Spectrum)(ganymede->tunerDescription->inversion); //Get from tunerDescription
#else
	if (ganymede->tunerDescription->inversion == True)
		MxL5007_TunerConfig.IF_Spectrum = MxL_NORMAL_IF; //Get from tunerDescription
	else
		MxL5007_TunerConfig.IF_Spectrum = MxL_INVERT_IF;
#endif
#if 0 // n001, Maxlinear changes definition
	MxL5007_TunerConfig.Mode = MxL_MODE_OTA_DVBT_ATSC;
#else
	MxL5007_TunerConfig.Mode = MxL_MODE_DVBT;
#endif
	MxL5007_TunerConfig.IF_Diff_Out_Level = -8; //Setting for Cable mode only
	MxL5007_TunerConfig.Xtal_Freq = MxL_XTAL_24_MHZ;	//24Mhz
	MxL5007_TunerConfig.ClkOut_Setting = MxL_CLKOUT_DISABLE;
	MxL5007_TunerConfig.ClkOut_Amp = MxL_CLKOUT_AMP_0;
#if 0 // n001
	MxL5007_TunerConfig.LoopThru_Setting = MxL_LT_DISABLE;
#endif


	deb_data("MxL5007 Open");
#if 0 // n002
	MxL_Tuner_Init(&MxL5007_TunerConfig);
#endif
//e
    return (Error_NO_ERROR);
}

Dword MXL5007_close (
	IN  Demodulator*	demodulator,
	IN  Byte			chip
) {
    return (Error_NO_ERROR);
}


Dword MXL5007_set (
	IN  Demodulator*	demodulator,
	IN  Byte			chip,
	IN  Word			bandwidth,
	IN  Dword			frequency
) {

	Dword status = 0;       
	deb_data("MxL5007 Set");
	if(	MxL_Tuner_RFTune( &MxL5007_TunerConfig, 
				  frequency * 1000 /* Hz */, 
				  (MxL5007_BW_MHz)(bandwidth / 1000) /* NHz */)
	){
		deb_data("MxL5007 Set fail");
		return (Error_WRITE_TUNER_FAIL);
	}

	return (Error_NO_ERROR);
}


Dword SwPowerCtrlMXL5007(
	IN  Demodulator*	demodulator,
	IN  Byte			chip,
	IN  Byte			control			/** 0 for power down mode, 1 for normal operation mode */
)
{
    Dword 	status = 0;
    Dword      	error = Error_NO_ERROR;   
    Byte    		AGC_MASTER_BYTE = 0x00;  /** For Single AGC =0 */
	Byte    		address[104] ;
    Byte    		data[104] ;
#if 0   
	if (control) {
    	/** SetTuner will wake up tuner once it is called and so users don't have to call this function */
    	/** in order to wake-up MXL5005/5003S */
	} else {
	    status 	    += MXL_GetMasterControl(data, MC_POWER_DOWN) ;
	    address[0]  = MASTER_CONTROL_ADDR;
	    data[0] |= AGC_MASTER_BYTE;
	    data[1]  = 0xFE; /** Latch CMD */
		error = Standard_writeTunerRegisters (demodulator, chip, address[0], 2, &data[0]);
	    if (error) goto exit;
    }
#endif
//exit:
    return (error);
}

TunerDescription tuner_MXL5007 = {
    MXL5007_open,
    MXL5007_close,
    MXL5007_set,
    MXL5007_scripts,
    MXL5007_scriptSets,
    0xC0,                           /** tuner i2c address */
    1,                              /** length of tuner register address */
    4570000,                        /** tuner if */
    True                           /** spectrum inverse */
};

