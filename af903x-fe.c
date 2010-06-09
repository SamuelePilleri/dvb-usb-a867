#include <linux/sched.h>
#include <linux/smp_lock.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/version.h>

#include "af903x.h"
#include "dvb_frontend.h"
#include "standard.h" //s009

//s024+s
#define A333_FREQ_MIN	44250000
#define A333_FREQ_MAX	867250000
//s024+e

//s026+s
static int alwayslock; // default to 0
module_param(alwayslock, int, 0644);
MODULE_PARM_DESC(alwayslock, "Whether to always report channel as locked (default:no).");
//s026+e



struct af903xm_state {
	struct dvb_frontend demod;
	fe_bandwidth_t current_bandwidth;
	uint32_t current_frequency; //s024

	//s004+s
	struct completion thread_exit;
	int thread_should_stop;
	atomic_t thread_created;

	u32 ucblocks;
	u32 ber;
	u16 strength;
	//s004+e
	
	int locked; //s019
};

//j012+s
int test_map_snr(u32 snr_data, u32 *snr)
{
	Dword error = 0;
	Dword snr_value = 0;
	Byte constellation = 0;
	Byte transmission_mode = 0;

	*snr = 0;

    /** Get constellation type */
    error = Standard_readRegisterBits ((Demodulator *)&PDC->Demodulator, PDC->Map.RF_SW_HOST, 
										Processor_OFDM, g_reg_tpsd_const, reg_tpsd_const_pos, reg_tpsd_const_len, &constellation);
    if (error) goto exit;

    /** Get FFT mode */
    error = Standard_readRegisterBits ((Demodulator *)&PDC->Demodulator, PDC->Map.RF_SW_HOST, 
										Processor_OFDM, g_reg_tpsd_txmod, reg_tpsd_txmod_pos, reg_tpsd_txmod_len, &transmission_mode);
    if (error) goto exit;

	// Adjust snr data by transmission mode
	if(transmission_mode == 0)		snr_value = snr_data * 4;
	else if(transmission_mode ==2)	snr_value = snr_data * 2;
	else							snr_value = snr_data;

   if( constellation == 0) //Constellation_QPSK
    {
           if(snr_value < 0xB4771)    *snr = 0;
           else if(snr_value < 0xC1AED)           *snr = 1;
           else if(snr_value < 0xD0D27)   *snr = 2;
           else if(snr_value < 0xE4D19)   *snr = 3;
           else if(snr_value < 0xE5DA8)   *snr = 4;
           else if(snr_value < 0x107097)   *snr = 5;
           else if(snr_value < 0x116975)   *snr = 6;
           else if(snr_value < 0x1252D9)   *snr = 7;
           else if(snr_value < 0x131FA4)   *snr = 8;
           else if(snr_value < 0x13D5E1)   *snr = 9;
           else if(snr_value < 0x148E53)   *snr = 10;
           else if(snr_value < 0x15358B)   *snr = 11;
           else if(snr_value < 0x15DD29)   *snr = 12;
           else if(snr_value < 0x168112)   *snr = 13;
           else if(snr_value < 0x170B61)   *snr = 14;
           else if(snr_value < 0x17A532)   *snr = 15;
           else if(snr_value < 0x180F94)   *snr = 16;
           else if(snr_value < 0x186ED2)   *snr = 17;
           else if(snr_value < 0x18B271)   *snr = 18;
           else if(snr_value < 0x18E118)   *snr = 19;
           else if(snr_value < 0x18FF4B)   *snr = 20;
           else if(snr_value < 0x190AF1)   *snr = 21;
           else if(snr_value < 0x191451)   *snr = 22;
           else   *snr = 23;
    }
    else if ( constellation == 1) //Constellation_16QAM
    {
        if(snr_value < 0x4F0D5)    *snr = 0;
           else if(snr_value < 0x5387A)   *snr = 1;
           else if(snr_value < 0x573A4)   *snr = 2;
           else if(snr_value < 0x5A99E)   *snr = 3;
           else if(snr_value < 0x5CC80)   *snr = 4;
           else if(snr_value < 0x5EB62)   *snr = 5;
           else if(snr_value < 0x5FECF)   *snr = 6;
           else if(snr_value < 0x60B80)   *snr = 7;
           else if(snr_value < 0x62501)   *snr = 8;
           else if(snr_value < 0x64865)   *snr = 9;
           else if(snr_value < 0x69604)   *snr = 10;
           else if(snr_value < 0x6F356)   *snr = 11;
           else if(snr_value < 0x7706A)   *snr = 12;
           else if(snr_value < 0x804D3)   *snr = 13;
           else if(snr_value < 0x89D1A)   *snr = 14;
           else if(snr_value < 0x93E3D)   *snr = 15;
           else if(snr_value < 0x9E35D)   *snr = 16;
           else if(snr_value < 0xA7C3C)   *snr = 17;
           else if(snr_value < 0xAFAF8)   *snr = 18;
           else if(snr_value < 0xB719D)   *snr = 19;
           else if(snr_value < 0xBDA6A)   *snr = 20;
           else if(snr_value < 0xC0C75)   *snr = 21;
           else if(snr_value < 0xC3F7D)   *snr = 22;
           else if(snr_value < 0xC5E62)   *snr = 23;
           else if(snr_value < 0xC6C31)   *snr = 24;
           else if(snr_value < 0xC7925)   *snr = 25;
           else    *snr = 26;
    }
    else if ( constellation == 2) //Constellation_64QAM
    {
        if(snr_value < 0x256D0)    *snr = 0;
           else if(snr_value < 0x27A65)   *snr = 1;
           else if(snr_value < 0x29873)   *snr = 2;
           else if(snr_value < 0x2B7FE)   *snr = 3;
           else if(snr_value < 0x2CF1E)   *snr = 4;
           else if(snr_value < 0x2E234)   *snr = 5;
           else if(snr_value < 0x2F409)   *snr = 6;
           else if(snr_value < 0x30046)   *snr = 7;
           else if(snr_value < 0x30844)   *snr = 8;
           else if(snr_value < 0x30A02)   *snr = 9;
           else if(snr_value < 0x30CDE)   *snr = 10;
           else if(snr_value < 0x31031)   *snr = 11;
           else if(snr_value < 0x3144C)   *snr = 12;
           else if(snr_value < 0x315DD)   *snr = 13;
           else if(snr_value < 0x31920)   *snr = 14;
           else if(snr_value < 0x322D0)   *snr = 15;
           else if(snr_value < 0x339FC)   *snr = 16;
           else if(snr_value < 0x364A1)   *snr = 17;
           else if(snr_value < 0x38BCC)   *snr = 18;
           else if(snr_value < 0x3C7D3)   *snr = 19;
           else if(snr_value < 0x408CC)   *snr = 20;
           else if(snr_value < 0x43BED)   *snr = 21;
           else if(snr_value < 0x48061)   *snr = 22;
           else if(snr_value < 0x4BE95)   *snr = 23;
           else if(snr_value < 0x4FA7D)   *snr = 24;
           else if(snr_value < 0x52405)   *snr = 25;
           else if(snr_value < 0x5570D)   *snr = 26;
           else if(snr_value < 0x59FEB)   *snr = 27;
           else if(snr_value < 0x5BF38)   *snr = 28;
           else    *snr = 29;
    }
	
	
exit:
	
	deb_data(" %s Function, SNR = %d -\n",__FUNCTION__, *snr);
	return error;
}
//j012+e

//j008+s
int test_read_strength_dbm(struct dvb_frontend *demod, u32 *str)
{
	
	Dword dwError = 0;
	Byte str_dbm = 0;


	// Read StrengthDBM
    dwError = Standard_readRegister ((Demodulator *)&PDC->Demodulator, PDC->Map.RF_SW_HOST, Processor_OFDM, est_rf_level_dbm, &str_dbm);
    if(dwError) {
	    deb_data("%s error, ret=0x%x\n", __FUNCTION__, dwError);
		*str = 0;	
		goto exit;
    }
 
	deb_data(" %s Function, StrengthDbm = %d -\n",__FUNCTION__, str_dbm);

exit:
	return dwError;
}
//j008+e

//j007+s
int test_read_snr(struct dvb_frontend *demod, u32 *snr_data)
{
	Byte check1 = 0x0, check2 = 0x0;
	Byte base1  = 0x0, base2  = 0x0;
	Byte snr1   = 0x0, snr2   = 0x0, snr3=0x0;			
	u16 baseAddr = 0x0;

	*snr_data = 0x000000;

    Standard_readRegister ((Demodulator *)&PDC->Demodulator, PDC->Map.RF_SW_HOST, Processor_OFDM, 0xf21b, &check1);
    Standard_readRegister ((Demodulator *)&PDC->Demodulator, PDC->Map.RF_SW_HOST, Processor_OFDM, 0xf999, &check2);

	deb_data(" %s Function, check1=0x%x -\n",__FUNCTION__, check1);

	if(check1 != 0x0c || check2 != 0x01)	return 1;
	
    Standard_readRegister ((Demodulator *)&PDC->Demodulator, PDC->Map.RF_SW_HOST, Processor_OFDM, 0x418b, &base1);
    Standard_readRegister ((Demodulator *)&PDC->Demodulator, PDC->Map.RF_SW_HOST, Processor_OFDM, 0x418c, &base2);

	baseAddr = (base1<<8) + base2 + 0x2C;

	deb_data(" %s Function, base1 = 0x%x -\n",__FUNCTION__, base1);
	deb_data(" %s Function, base2 = 0x%x -\n",__FUNCTION__, base2);
	deb_data(" %s Function, (base1<<8) + base2 + 0x2c = baseAddr = 0x%x -\n",__FUNCTION__, baseAddr);
	
    Standard_readRegister ((Demodulator *)&PDC->Demodulator, PDC->Map.RF_SW_HOST, Processor_OFDM, baseAddr,   &snr1);
    Standard_readRegister ((Demodulator *)&PDC->Demodulator, PDC->Map.RF_SW_HOST, Processor_OFDM, baseAddr+1, &snr2);
    Standard_readRegister ((Demodulator *)&PDC->Demodulator, PDC->Map.RF_SW_HOST, Processor_OFDM, baseAddr+2, &snr3);

	*snr_data = (snr3<<16)+(snr2<<8)+(snr1);

	deb_data(" %s Function, snr data = 0x%x -\n",__FUNCTION__, *snr_data);

	return 0;

}
//j007+e


static int af903x_set_bandwidth(struct dvb_frontend *demod, u8 bw_idx)
{
	struct af903xm_state *state = demod->demodulator_priv;

	deb_data("- Enter %s Function - bandwidth= %d \n",__FUNCTION__,bw_idx);

//	state->current_bandwidth = bw_idx;

	DL_Tuner_SetBW(bw_idx);

	return 0;
}

static void af903x_set_channel(struct af903x_ofdm_channel *ch)
{
	deb_data("- Enter %s Function - RF=%d, BW=%d\n",__FUNCTION__,ch->RF_kHz,ch->Bw);

	DL_Tuner_SetFreq(ch->RF_kHz,ch->Bw);

}

static int af903x_tune(struct dvb_frontend *demod, struct af903x_ofdm_channel *ch)
{
	if (ch != NULL)
		af903x_set_channel(ch);

	return 0;
}

static int af903x_init(struct dvb_frontend *demod)
{
	int ret = 0;
	struct af903xm_state *state = demod->demodulator_priv;
	deb_data("- Enter %s Function -\n",__FUNCTION__);
 
	//s005
	ret = usb_autopm_get_interface(uintfs);
	if(ret) {
		deb_data("%s calling usb_autopm_get_interface failed with %d\n", __FUNCTION__, ret);
		return ret;
	}

	ret = DL_ApCtrl(1);
	if(ret) {
		deb_data("af903x_init Fail: 0x%04X", ret);
		return -EIO; //s018
	}

	//s019+s
	// reset statistics
	state->ber = 0;
	state->ucblocks = 0;
	state->strength = 0;
	state->locked = 1; 
	//s019+e
	
	// reset values
	state->current_frequency = 0; //s024
	state->current_bandwidth = 0; //s024

	//s004
	af903x_start_monitor_thread(demod);

	deb_data("- Exit %s Function -\n",__FUNCTION__);
	return 0;
}

static int af903x_sleep(struct dvb_frontend *demod)
{
	int error;
        deb_data("Enter %s Function\n",__FUNCTION__);

	//s004
	af903x_stop_monitor_thread(demod);

        error = DL_ApCtrl(0);
        if (error) {
		deb_data("%s calling DL_ApCtrl error : 0x%x\n", __FUNCTION__, error);
		return -EIO; //s018
	}

	//s005
	usb_autopm_put_interface(uintfs);	

	deb_data("- Exit %s Function -\n",__FUNCTION__);
	return 0;
}
static int af903x_identify(struct af903xm_state *state)
{
	return 0;
}

static int af903x_get_frontend(struct dvb_frontend* fe,
				struct dvb_frontend_parameters *fep)
{
	struct af903xm_state *state = fe->demodulator_priv;

	deb_data("- Enter %s Function -\n",__FUNCTION__);
	memset(fep, 0, sizeof(*fep)); //s024
	fep->frequency = state->current_frequency; //s024
	fep->inversion = INVERSION_AUTO;
	fep->u.ofdm.bandwidth = state->current_bandwidth;
	return 0;
}

static int af903x_set_frontend(struct dvb_frontend* fe,
				struct dvb_frontend_parameters *fep)
{
	struct af903xm_state *state = fe->demodulator_priv;
	struct af903x_ofdm_channel ch;
	u16 bw=0;
	int ret=0;

	deb_data("- Enter %s Function -\n",__FUNCTION__);

	//s024+s
#if 1 //s030, re-enable this code //s029+s
	if( fep->frequency < A333_FREQ_MIN || fep->frequency > A333_FREQ_MAX ) {
		deb_data("- %s freq=%d Hz out of range(%d~%d)-\n",__FUNCTION__, fep->frequency, 
				A333_FREQ_MIN, A333_FREQ_MAX);
		// set to zero so we can report unlock to AP, because return -EINVAL apparently 
		// does not stop AP from continuing tuning.
		state->current_frequency = 0;
		return -EINVAL;
	}
#endif //s029+e
	//s024+e

#if 0 //s023+s
	if (fep->u.ofdm.bandwidth == 0) fep->u.ofdm.bandwidth=8;
	if (fep->u.ofdm.bandwidth == 1) fep->u.ofdm.bandwidth=7;
	if (fep->u.ofdm.bandwidth == 2) fep->u.ofdm.bandwidth=6;
#else
	switch(fep->u.ofdm.bandwidth) {
	case BANDWIDTH_8_MHZ: bw=8; break;
	case BANDWIDTH_7_MHZ: bw=7; break;
	case BANDWIDTH_6_MHZ: bw=6; break;

	case 6: 
	case 7:
	case 8:
		bw = fep->u.ofdm.bandwidth;
		deb_data("- %s wrong bw value: %d -\n",__FUNCTION__, fep->u.ofdm.bandwidth);
		break;
	default:

		deb_data("- %s unknown bw value: %d -\n",__FUNCTION__, fep->u.ofdm.bandwidth);
		return -EINVAL;
	}
#endif //s023+e

	ch.RF_kHz           = fep->frequency / 1000;
//s023	ch.Bw               = fep->u.ofdm.bandwidth;
	ch.Bw               = bw; //s023
	deb_data("- %s freq=%d KHz, bw=%d MHz -\n",__FUNCTION__, ch.RF_kHz,  ch.Bw);

	state->current_bandwidth = fep->u.ofdm.bandwidth;
	state->current_frequency = fep->frequency; //s024

//s023	af903x_set_bandwidth(fe, fep->u.ofdm.bandwidth);
//stylontest	af903x_set_bandwidth(fe, bw); //s023

//s023	return af903x_tune(fe, &ch);
	ret = af903x_tune(fe, &ch); //s023

	//s031, start monitor thread if not yet.
	af903x_start_monitor_thread(fe);
	
	return ret;
}

static int af903x_read_status(struct dvb_frontend *fe, fe_status_t *stat)
{
	DWORD dwError;
	Bool bLock;
	struct af903xm_state *state = fe->demodulator_priv;
	u32	snr_data; //j007
	u32 str_dbm_data; //j008
	u32 snr; //j012	

	deb_data("- Enter %s Function -\n",__FUNCTION__);
	*stat = 0;

//s019+s
#if !USE_MONITOR_TH	
	{	
	unsigned long j = (HZ*500)/1000;
	set_current_state(TASK_UNINTERRUPTIBLE);
	schedule_timeout(j);
	}
	dwError = DL_GetLocked(&bLock);
	if( dwError ) {
		deb_data("- Function %s error=0x%x\n",__FUNCTION__, dwError);	
		return -EIO;
	}
#else
	//s026+s
	if( alwayslock ) {
		bLock = True;
	}
	else {
		bLock = state->locked;
	}
	//s026+e
#endif
	if( bLock ) {
		*stat |= FE_HAS_SIGNAL;
		*stat |= FE_HAS_CARRIER;
		*stat |= FE_HAS_LOCK;
		*stat |= FE_HAS_VITERBI;
		*stat |= FE_HAS_SYNC;
	}
//s019+e

	//s030, report unlock if frequency is out of bound
	if( 0==state->current_frequency ) {
		*stat = 0;
	}
	
#if ENABLE_TEST_FUNCTION //j016
	test_read_snr(fe, &snr_data); 				//j007
	test_map_snr(snr_data, &snr);				//j012
	test_read_strength_dbm(fe, &str_dbm_data);	//j008
#endif

	deb_data("- Exit %s Function, status=0x%x -\n",__FUNCTION__, *stat);
	return 0;
}

//s003+s
static int af903x_read_ubc(struct dvb_frontend *fe, u32* ucblocks)
{
	struct af903xm_state *state = fe->demodulator_priv;
	DWORD dwError;
	Bool bLock;

	deb_data("- Enter %s Function -\n",__FUNCTION__);	

#if !USE_MONITOR_TH //s004
	DL_GetChannelStat(NULL, NULL, ucblocks);

	dwError = DL_GetLocked(&bLock);
	// if signal is not locked, fill with 65535 to indicate loss of signal
	if( dwError || !bLock ) {
		deb_data("- Function %s lost lock , error=0x%x, bLock=%d -\n",__FUNCTION__, dwError, bLock);	
		if( ucblocks ) *ucblocks = 65535;
		return 0;
	}
#else
	if( ucblocks ) *ucblocks = state->ucblocks;
#endif //0

	deb_data("- Exit %s Function, ubc=%d -\n",__FUNCTION__, ucblocks? *ucblocks:-1);
	return 0;
}
//s003+e

static int af903x_read_ber(struct dvb_frontend *fe, u32 *ber)
{
	struct af903xm_state *state = fe->demodulator_priv;
	//s003+s
	DWORD dwError;
	u32 berbits;
	Bool bLock;
	deb_data("- Enter %s Function -\n",__FUNCTION__);	

#if !USE_MONITOR_TH //s004
	DL_GetChannelStat(ber, &berbits, NULL);

	// if signal is not locked, fill BER with BER total bits
	dwError = DL_GetLocked(&bLock);
	if( dwError || !bLock ) {
		deb_data("- Function %s lost lock , error=0x%x, bLock=%d -\n",__FUNCTION__, dwError, bLock);	
		if( ber ) *ber = berbits;
		return 0;
	}
#else
	if( ber ) *ber = state->ber;
#endif //0
	//s003+e
	//
	deb_data("- Exit %s Function , ber=%d-\n",__FUNCTION__, ber? *ber:(-1));
	return 0;
}

static int af903x_read_signal_strength(struct dvb_frontend *fe, u16 *strength)
{
	struct af903xm_state *state = fe->demodulator_priv;
	//s003+s
	DWORD dwError;
	Bool bLock;
	deb_data("- Enter %s Function -\n",__FUNCTION__);	

#if !USE_MONITOR_TH  //s004
	DL_GetSignalStrength(strength);

	dwError = DL_GetLocked(&bLock);
	if( dwError || !bLock ) {
		deb_data("- Function %s lost lock , error=0x%x, bLock=%d -\n",__FUNCTION__, dwError, bLock);	
		if( strength ) *strength = 0;
		return 0;
	}
#else
	if( strength ) *strength = state->strength;
#endif //0

	//s003+e
	
	deb_data("- Exit %s Function, strength=%d -\n",__FUNCTION__, strength? *strength:-1);	
	return 0;
}

static int af903x_read_snr(struct dvb_frontend* fe, u16 *snr)
{
#if !ENABLE_READ_REG //jamietest+s
	*snr = 0x0000;
//s018	return 0;

	return -ENOSYS; //s018
#else
	
	Byte value = 0;
	Dword addr = 0;
	Dword dwError = 0;

	printk("Regiter Addr: 0x%x", *snr);

	addr = *snr;

    dwError = Standard_readRegister ((Demodulator *)&PDC->Demodulator, PDC->Map.RF_SW_HOST, Processor_OFDM, addr, &value);
	if(dwError)	{
		printk("Read Register fail!! \n");
		*snr = 0xffff;
		return 0;
	}

	*snr = value;

	return 0;
#endif
}
//
static void af903x_release(struct dvb_frontend *demod)
{
	struct af903xm_state *st = demod->demodulator_priv;
	deb_data("- Enter %s Function -\n",__FUNCTION__);
	af903x_stop_monitor_thread(demod); //s004
	kfree(st);
}

static struct dvb_frontend_ops af903x_ops = {
	.info = {
		.name = "A867 USB DVB-T",
		.type = FE_OFDM,
		.frequency_min      = A333_FREQ_MIN, //s024, 44250000,
		.frequency_max      = A333_FREQ_MAX, //s024, 867250000,
		.frequency_stepsize = 62500,
		.caps = FE_CAN_INVERSION_AUTO |
			FE_CAN_FEC_1_2 | FE_CAN_FEC_2_3 | FE_CAN_FEC_3_4 |
			FE_CAN_FEC_5_6 | FE_CAN_FEC_7_8 | FE_CAN_FEC_AUTO |
			FE_CAN_QPSK | FE_CAN_QAM_16 | FE_CAN_QAM_64 | FE_CAN_QAM_AUTO |
			FE_CAN_TRANSMISSION_MODE_AUTO |
			FE_CAN_GUARD_INTERVAL_AUTO |
			FE_CAN_RECOVER |
			FE_CAN_HIERARCHY_AUTO,
	},

	.release              = af903x_release,

	.init                 = af903x_init,
	.sleep                = af903x_sleep,

	.set_frontend         = af903x_set_frontend,
	.get_frontend         = af903x_get_frontend,

	.read_status          = af903x_read_status,
	.read_ber             = af903x_read_ber,
	.read_signal_strength = af903x_read_signal_strength,
	.read_snr             = af903x_read_snr,
	.read_ucblocks	      = af903x_read_ubc, //s003
};

static struct dvb_frontend_ops af903x_ops;
struct dvb_frontend * af903x_attach(u8 tmp)
{
	struct dvb_frontend *demod;
	struct af903xm_state *st;

	deb_data("- Enter %s Function -\n",__FUNCTION__);	
	st = kzalloc(sizeof(struct af903xm_state), GFP_KERNEL);
	if (st == NULL)
		return NULL;

	demod                   = &st->demod;
	demod->demodulator_priv = st;
	memcpy(&st->demod.ops, &af903x_ops, sizeof(struct dvb_frontend_ops));
	atomic_set(&st->thread_created, 0); //s004

	af903x_identify(st); 

	return demod;
}
//s012 EXPORT_SYMBOL(af903x_attach);

//s009+s
static Dword Monitor_GPIO8(void)
{
    Dword error = Error_NO_ERROR;
    Byte PinValue;

    if ( PDC->idProduct!=0xa333 /* only for a333 */ ) {
	goto exit;
    }

    if( PDC->fc[0].AVerFlags&0x04 ) {
	deb_data("%s, skip read GPIO8\n", __FUNCTION__);
	PDC->fc[0].AVerFlags &= ~(0x04);
	goto exit;
    }

    // read GPIO8
    error = Standard_readRegister ((Demodulator *)&PDC->Demodulator, 
			PDC->Map.RF_SW_HOST, 
			Processor_LINK, 
			PDC->Map.GPIO_STR_i, 
			&PinValue);
    if( error ) goto exit;

    if( PDC->Demodulator.GPIO8Value[0] != PinValue ) {

        if( PinValue == 1 ) {
	    error = Standard_writeRegister ( (Demodulator *)&PDC->Demodulator, 
			PDC->Map.RF_SW_HOST, 
			Processor_LINK, 
			PDC->Map.GPIO_UHF_o, 
			1);
	    error = Standard_writeRegister ( (Demodulator *)&PDC->Demodulator, 
			PDC->Map.RF_SW_HOST, 
			Processor_LINK, 
			PDC->Map.GPIO_VHF_o, 
			1);
        }
        else if (PDC->fc[0].ulDesiredFrequency > 300000)
        {
	     //UHF
	    error = Standard_writeRegister ( (Demodulator *)&PDC->Demodulator, PDC->Map.RF_SW_HOST, 
					Processor_LINK, PDC->Map.GPIO_UHF_o, 1);
	    error = Standard_writeRegister ( (Demodulator *)&PDC->Demodulator, PDC->Map.RF_SW_HOST, 
					Processor_LINK, PDC->Map.GPIO_VHF_o, 0);
        }
        else
        {
	    //VHF
	    error = Standard_writeRegister ( (Demodulator *)&PDC->Demodulator, PDC->Map.RF_SW_HOST, 
					Processor_LINK, PDC->Map.GPIO_UHF_o, 0);
	    error = Standard_writeRegister ( (Demodulator *)&PDC->Demodulator, PDC->Map.RF_SW_HOST, 
					Processor_LINK, PDC->Map.GPIO_VHF_o, 1);
        }
	 PDC->Demodulator.GPIO8Value[0] = PinValue;
    }
    else if( PinValue == 0 ) { // pin value remains 0
	if( PDC->Demodulator.channelStatistic[0].abortCount > 2500  &&
		(PDC->fc[0].AVerFlags&0x08)==0 ) {
		// pull high GPIO8
		PDC->fc[0].AVerFlags |= 0x08; // pull H flag bit 3
		Demodulator_writeRegister( (Demodulator *)&PDC->Demodulator, 0, Processor_LINK, PDC->Map.GPIO_STR_en, 1);
		Demodulator_writeRegister( (Demodulator *)&PDC->Demodulator, 0, Processor_LINK, PDC->Map.GPIO_STR_on, 1);
		Demodulator_writeRegister( (Demodulator *)&PDC->Demodulator, 0, Processor_LINK, PDC->Map.GPIO_STR_o, 1);
	}
    }
    else if( PinValue == 1 ) { // pin value remains 1
	if( PDC->Demodulator.channelStatistic[0].abortCount < 2500 &&
		(PDC->fc[0].AVerFlags&0x08)!=0 ) {
		// after manual tune and result is good
		PDC->fc[0].AVerFlags &= ~(0x08);
	}
    }

exit:
    return error;
}
//s009+e

//s004+s
static int af903x_monitor_thread_func(void *data)
{
	struct dvb_frontend *demod = data;
	struct af903xm_state *state = demod? demod->demodulator_priv:NULL;
	const char *thread_name = "a333_monitor_thread";
	unsigned long loopcount = 0;
	Bool bLock = True;

	deb_data("- Enter %s Function -\n",__FUNCTION__);
	if( !state ) return -1;

	lock_kernel();
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,5,61)
	daemonize();
	sigfillset(&current->blocked);
	sprintf(current->comm, "%s", thread_name);
#else
	daemonize("%s", thread_name);
	allow_signal(SIGTERM);
#endif
	siginitsetinv(&current->blocked, sigmask(SIGKILL)|sigmask(SIGINT)|\
			sigmask(SIGTERM));
	unlock_kernel();

	while(!state->thread_should_stop && !signal_pending(current)) {
	
		DWORD dwError;
		u32 ber, berbits, ucblocks;
		u16 strength;

		loopcount++;

		// sleep for 500 mili seconds
		set_current_state(TASK_UNINTERRUPTIBLE);
		schedule_timeout((HZ*500)/1000);

		// monitor lock and return lock status
		// reacquire channel if lock is lost for quiet a while
		#if 1 //s015, 0 //s008
//s019		if( loopcount%4 == 1 ) {
			DL_MonitorReception(&bLock);
			state->locked = bLock? 1:0; //s019
//s019		}
		#endif //0
		// obtain statistics
		// do this every 2 loops, which is 1 second
		if( loopcount%2 == 1  ) {

//s015			dwError= Demodulator_isLocked((Demodulator*) &PDC->Demodulator, 0, &bLock);
//s015			if( dwError ) bLock = False;
			// do not do this if bLock is false, because 
			//    1. this is not necessary
			//    2. frequency is not yet set
			if( bLock ) {
				DL_GetChannelStat(&ber, &berbits, &ucblocks);
				DL_GetSignalStrength(&strength);

				// sometimes strength can drop to zero while demod
				// is still pumping out stream. In which case we
				// maintain strength to a locked level because 
				// some AP(kaffeine) seem to rely on strength
				// for signs of signal lock.
				//if( strength==0 ) strength = 10;
				
				strength = (strength*65535/100); //s020

			}
			else {
				DL_GetChannelStat(NULL, &berbits, NULL);
				ber = berbits;
				ucblocks = 65535;
				strength = 0;
			}

			state->ber = ber;
			state->ucblocks = ucblocks;
			state->strength = strength;
		}

		//s009
		// monitor GPIO8
	        Monitor_GPIO8();
	}

#if 0 //s019+s, moved to af903x_init, where frontend is first opened
	// reset statistics
	state->ber = 0;
	state->ucblocks = 0;
	state->strength = 0;
	state->locked = 1; 
#endif //s019+e

	//s031+s
	//Firstly, clear thread_created flag so monitor thead can be restored later.
	atomic_set(&state->thread_created, 0);
	//Secondly, clear current frequency so we can depent on later set_frequency to restore
	//this thread.
	state->current_frequency = 0;
	//s031+e

	deb_data("- Exit %s Function -\n",__FUNCTION__);
	complete_and_exit(&state->thread_exit, 0);
}
//s004+e

//s004+s
void af903x_start_monitor_thread(struct dvb_frontend *demod)
{
	struct af903xm_state *st = demod? demod->demodulator_priv:NULL;
	if( !st ) return;

#if USE_MONITOR_TH
	// if st->thread_created is already 1, then skip thread creation
	if( atomic_add_unless(&st->thread_created, 1, 1) ) {
		st->thread_should_stop = 0;
		init_completion(&st->thread_exit); 
		kernel_thread(af903x_monitor_thread_func, st, 0);
	}
#endif //USE_MONITOR_TH

}
//s004+e

//s004+s
void af903x_stop_monitor_thread(struct dvb_frontend *demod)
{
	struct af903xm_state *st = demod? demod->demodulator_priv:NULL;
	deb_data("- Enter %s Function -\n",__FUNCTION__);
	if( !st ) return;

#if USE_MONITOR_TH
	// if st->thread_created is alread 0, then skip thread destruction
	if( atomic_add_unless(&st->thread_created, -1, 0) ) {
		st->thread_should_stop = 1;
		wait_for_completion(&st->thread_exit);
	}
#endif //USE_MONITOR_TH
	deb_data("- Exit %s Function -\n",__FUNCTION__);
}
//s004+e

//MODULE_AUTHOR("Jimmy Chen<jimmy.chen@afatech.com>");
//MODULE_DESCRIPTION("Driver for the AF903X demodulator");
//MODULE_LICENSE("GPL");
