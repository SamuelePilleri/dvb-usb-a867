#include "af903x.h"

//j005+s
int dvb_usb_af903x_hwpid = 1; //s033,0 // enable hw pid filter 
module_param_named(hwpid,dvb_usb_af903x_hwpid, int, 0644); 
MODULE_PARM_DESC(debug, "set hw pid filter.(disable=0, enable=1)" DVB_USB_DEBUG_STATUS);
//j005+e

static u16 gSWPIDTable[32];
static int gTblUsed = 0;

static int af903x_download_firmware(struct usb_device *udev, const struct firmware *fw)
{
	int ret=0;
	deb_data("- Enter %s Function -\n",__FUNCTION__);
	
	return ret;
}

static int af903x_powerctrl(struct dvb_usb_device *d, int onoff)
{

	int ret;
	deb_data("- Enter %s Function - %s\n",__FUNCTION__,onoff?"ON":"OFF");

	//s005, resume device before DL_ApCtrl(on)
	if( onoff ) {
		ret = usb_autopm_get_interface(uintfs);
		if(ret) {
			deb_data("%s calling usb_autopm_get_interface failed with %d\n", __FUNCTION__, ret);
			gTblUsed = 0; //j006

			return ret;
		}
	}

	ret = DL_ApCtrl(onoff);
	if(ret) deb_data("	af903x_powerctrl Fail: 0x%04X\n", ret);

	//s005, suspend device after DL_ApCtrl(off)
	if( !onoff ) {
		usb_autopm_put_interface(uintfs);	
	}

	deb_data("- Exit %s Function - %s, ret=%d\n",__FUNCTION__,onoff?"ON":"OFF", ret);
	return ret;
}

static int af903x_identify_state(struct usb_device *udev, struct dvb_usb_device_properties *props,
			struct dvb_usb_device_description **desc, int *cold)
{
	deb_data("- Enter %s Function -\n",__FUNCTION__);
	*cold = 0;

	return 0;
}

static int af903x_frontend_attach(struct dvb_usb_adapter *adap)
{
	deb_data("- Enter %s Function -\n",__FUNCTION__);
	adap->fe = af903x_attach(1);	

	return adap->fe == NULL ? -ENODEV : 0;
}

static int af903x_tuner_attach(struct dvb_usb_adapter *adap)
{
	deb_data("- Enter %s Function -\n",__FUNCTION__);
	tuner_attach(adap->fe);
	return  0;
}

static int af903x_streaming_ctrl(struct dvb_usb_adapter *adap, int onoff)
{
	deb_data("- Enter %s Function - (%d) streaming state for %d\n",__FUNCTION__,onoff, adap->id);
	

	return 0;
}

#if 0 //j013+s
//j004+s
static int af903x_pid_filter_ctrl(struct dvb_usb_adapter *adap, int onoff)
{
	deb_data("- Enter %s Function - (%d) HW PID Filter for %d\n",__FUNCTION__,onoff, adap->id);

	return 0;
}


static int af903x_pid_filter(struct dvb_usb_adapter *adap, int index, u16 pid, int onoff)
{
	Pid mypid;
	int i =0; //j006
	


	if(onoff)	
	{
		deb_data("- Enter %s Function - Add HW PID %d on index %d\n",__FUNCTION__, pid, index);
#if 0 //j006+s
		mypid.value = pid;

		if(dvb_usb_af903x_hwpid)	//j005
		{
			DL_AddPID(index,mypid);
		}
#else
		gSWPIDTable[index] = pid;
		gTblUsed++;

#endif //j006+e
	}
	else
	{
		deb_data("- Enter %s Function - Remove HW PID %d on index %d\n",__FUNCTION__, pid,index);
		gSWPIDTable[index] = -1;
		gTblUsed--;
	}



	
	// j006+s, Write SW table to HW table
	i=0;
	while(i<gTblUsed)
	{
		deb_data("- Index %d to PID ========= %d \n" ,i ,gSWPIDTable[i]);
	
		mypid.sectionType = SectionType_TABLE;
		mypid.table = 0;
		mypid.duration = 0xff;
		mypid.frameRow = 0;
		mypid.priority = 0;
		mypid.version = 0;
		mypid.cache = false;
		mypid.value = gSWPIDTable[i];

		DL_AddPID(i, mypid);

		i++;
	}
	
	// Always add the following PID "0,16,17,18,20,21" to PID table
	mypid.value = 0;
	DL_AddPID(31, mypid);
	mypid.value = 16;
	DL_AddPID(30, mypid);
	mypid.value = 17;
	DL_AddPID(29, mypid);
	mypid.value = 18;
	DL_AddPID(28, mypid);
	mypid.value = 20;
	DL_AddPID(27, mypid);
	mypid.value = 21;
	DL_AddPID(26, mypid);

	msleep(100);
	//j006+e
	
	return 0;
}
//j004+e

#else

static int af903x_pid_filter_ctrl(struct dvb_usb_adapter *adap, int onoff)
{
    int ret =0;
    deb_data("%s: onoff:%d\n", __func__, onoff);

    if (!onoff){
        deb_data("  Reset PID Table\n");
        DL_ResetPID();
        PDC->fc[adap->id].ulcPIDs = 0;
        memset(PDC->fc[adap->id].aulPIDs, 0, sizeof(FILTER_INFO));
    }
    PDC->fc[adap->id].bEnPID =  onoff;
    ret = DL_PIDOnOff(PDC->fc[adap->id].bEnPID);

    deb_data("  set pid onoff ok\n");
    return ret;
}


static int af903x_pid_filter(struct dvb_usb_adapter *adap, int index, u16 pidnum,
    int onoff)
{
//    if (down_interruptible (&my_sem))
//            return -ERESTARTSYS;

    int ret = 0;
    Pid pid;
    deb_data("- %s: set pid filter, index %d, pid %d, onoff %d.\n",
    __func__, index, pidnum, onoff);


    if (onoff) { //add filter to table
        PDC->fc[adap->id].aulPIDs[index].filternum = pidnum;
        if (!PDC->fc[adap->id].aulPIDs[index].onoff ){
            PDC->fc[adap->id].aulPIDs[index].onoff = true;
            PDC->fc[adap->id].ulcPIDs ++;
        }
    }
    else{   //del filter from table
        PDC->fc[adap->id].aulPIDs[index].filternum = 0;
        if (PDC->fc[adap->id].aulPIDs[index].onoff ){
            PDC->fc[adap->id].aulPIDs[index].onoff = false;
            PDC->fc[adap->id].ulcPIDs --;
        }
    }

    if (onoff) {
        /* cannot use it as pid_filter_ctrl since it has to be done
 *            before setting the first pid */
        if (adap->feedcount == 1) {
            deb_data("  first pid set, enable pid table\n");

            pid.sectionType = SectionType_TABLE;
            pid.table = 0x00;
            pid.duration = 0xFF;

            ret = af903x_pid_filter_ctrl(adap, onoff);
            if (ret)
                return ret;
        }
        pid.value = (Word)pidnum;
        ret = DL_AddPID(index, pid);
        if (ret)
            return ret;

    }
    else{

        pid.value = (Word)pidnum;
        ret = DL_RemovePID(index, pid);
        if (ret)
            return ret;

        if (adap->feedcount == 0) {
            deb_data("  last pid unset, disable pid table\n");
            ret = af903x_pid_filter_ctrl(adap, onoff);
            if (ret)
                return ret;
        }

    }

    deb_data("  set pid ok, total pid num :%d\n", PDC->fc[adap->id].ulcPIDs);

	msleep(100);
	
//    up(&my_sem);
    return 0;
}


#endif //j013+e

struct usb_device_id af903x_usb_id_table[] = {
//s028+s
		{ USB_DEVICE(0x07ca,0xa333) },
		{ USB_DEVICE(0x07ca,0xb867) },
		{ USB_DEVICE(0x07ca,0x1867) },//j001
		{ USB_DEVICE(0x07ca,0x0337) },//j003
		{ USB_DEVICE(0x07ca,0xa867) },//j003
		{ USB_DEVICE(0x07ca,0x0867) },//s034
#if SUPPORT_AF903X_EVB 
		{ USB_DEVICE(0x15A4,0x1000) },
		{ USB_DEVICE(0x15A4,0x1001) },
		{ USB_DEVICE(0x15A4,0x1002) },
		{ USB_DEVICE(0x15A4,0x1003) },
		{ USB_DEVICE(0x15A4,0x9035) },
#endif //SUPPORT_AF903X_EVB
//s028+e
		{ 0 /*s016*/}		/* Terminating entry */
};
MODULE_DEVICE_TABLE(usb, af903x_usb_id_table);

struct dvb_usb_device_properties af903x_properties[] = {
	{
		.usb_ctrl          = DEVICE_SPECIFIC,
		.download_firmware = af903x_download_firmware,
		.no_reconnect      = 1,
		.power_ctrl		   = af903x_powerctrl,
		.identify_state    = af903x_identify_state,
		
		.num_adapters = 1,
		.adapter = {
			{
#if ENABLE_HW_PID
					.caps = DVB_USB_ADAP_HAS_PID_FILTER | DVB_USB_ADAP_NEED_PID_FILTERING,
#else
					.caps = DVB_USB_ADAP_HAS_PID_FILTER,
#endif
					.pid_filter_count = 32,
				.frontend_attach  = af903x_frontend_attach,
				.tuner_attach     = af903x_tuner_attach,
				.streaming_ctrl   = af903x_streaming_ctrl,
				.pid_filter_ctrl  = af903x_pid_filter_ctrl,	//j004, Called to en/disable HW pid filter
				.pid_filter		  = af903x_pid_filter,		//j004, Called to set/unset a PID for filtering

				.stream = { 
				.type = USB_BULK,
				.count = 10, //s017, 4
				.endpoint = 0x84,
				.u = {
					.bulk = {
						.buffersize = (188 * TS_PACKET_COUNT), //j010
						}
					}
				}
			},
		},
#if 0
		.num_device_descs = 1,
#else
		.num_device_descs = 2,
#endif
		.devices = {
			{   "AVerMedia A333 DVB-T Recevier",  //s007
//s028+s
				{ &af903x_usb_id_table[0]
				 ,&af903x_usb_id_table[1]
#if SUPPORT_AF903X_EVB 
				 ,&af903x_usb_id_table[5]	//jamietest, each time you add new ID, you have to change the value
				 ,&af903x_usb_id_table[6] 	//jamietest
#endif //SUPPORT_AF903X_EVB
//s028+e
				},

				{ NULL },
			},
//j001+s, add an device entry
			{   "AVerMedia A867/A337 DVB-T Recevier",  
				{ &af903x_usb_id_table[2]
				 ,&af903x_usb_id_table[3]	//j003
				 ,&af903x_usb_id_table[4]	//j003
				 ,&af903x_usb_id_table[5]	//s034
				},

				{ NULL },
			}
//j001+e
		}
	}
};

int af903x_device_count = ARRAY_SIZE(af903x_properties);
