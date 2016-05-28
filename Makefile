
EXTRA_CFLAGS = -DEXPORT_SYMTAB


ifneq ($(KOBJ),)
version := $(shell egrep -e '^VERSION ' $(KSRC)/Makefile | awk 'BEGIN{FS="="}{print $$2}' | tr -d ' ')
patchlevel := $(shell egrep -e '^PATCHLEVEL ' $(KSRC)/Makefile | awk 'BEGIN{FS="="}{print $$2}' | tr -d ' ')
sublevel := $(shell egrep -e '^SUBLEVEL ' $(KSRC)/Makefile | awk 'BEGIN{FS="="}{print $$2}' | tr -d ' ')
extra-version := $(shell egrep -e '^EXTRAVERSION ' $(KSRC)/Makefile | awk 'BEGIN{FS="="}{print $$2}' | tr -d ' ')
local-version := $(shell egrep -e '^CONFIG_LOCALVERSION' $(KOBJ)/.config | awk 'BEGIN{FS="="}{print $$2}' | tr -d ' "')
CURRENT = $(version).$(patchlevel).$(sublevel)$(extra-version)$(local-version)
else
CURRENT = $(shell uname -r)
endif

ifeq ($(KOBJ),)
KOBJ = $(DESTDIR)/lib/modules/$(CURRENT)/build
endif
ifeq ($(KSRC),)
KSRC = $(DESTDIR)/lib/modules/$(CURRENT)/source
endif

PWD = $(shell pwd)
#KDIR26	 := $(DESTDIR)/lib/modules/$(CURRENT)/kernel/drivers/media/dvb/AF903x
#DEST = $(DESTDIR)/lib/modules/$(CURRENT)/kernel/$(MDIR)
#KFIRMWARM = $(DESTDIR)/lib/firmware/
INSTALL := $(shell which install)
DEPMOD  := $(shell which depmod || which /sbin/depmod || echo "true You should now run depmod")

# SYSTEM DVB setting
ifeq (,$(wildcard $(KSRC)/drivers/media/dvb/.))
BUILD_DEP = dvb/dvb-usb/dvb-usb-common.h dvb/dvb-usb/dvb-usb.h dvb/dvb-usb/dvb-usb-ids.h dvb/dvb-core/dvb_frontend.h dvb/dvb-core/dvbdev.h dvb/dvb-core/dmxdev.h dvb/dvb-core/dvb_demux.h dvb/dvb-core/dvb_net.h dvb/dvb-core/demux.h dvb/dvb-core/dvb_ringbuffer.h dvb/frontends/dvb-pll.h

MISSINGFILE := $(shell for i in $(BUILD_DEP); do if [ ! -f $(KSRC)/drivers/media/$$i ]; then echo $$i; fi; done)

DVB_FLAGS += -I$(KSRC)/drivers/media/dvb/dvb-core/
DVB_FLAGS += -I$(KSRC)/drivers/media/dvb/dvb-usb/
DVB_FLAGS += -I$(KSRC)/drivers/media/dvb/frontends/
else
BUILD_DEP = usb/dvb-usb/dvb-usb-common.h usb/dvb-usb/dvb-usb.h dvb-core/dvb-usb-ids.h dvb-core/dvb_frontend.h dvb-core/dvbdev.h dvb-core/dmxdev.h dvb-core/dvb_demux.h dvb-core/dvb_net.h dvb-core/demux.h dvb-core/dvb_ringbuffer.h dvb-frontends/dvb-pll.h

MISSINGFILE := $(shell for i in $(BUILD_DEP); do if [ ! -f $(KSRC)/drivers/media/$$i ]; then echo $$i; fi; done)

DVB_FLAGS += -I$(KSRC)/drivers/media/dvb-core/
DVB_FLAGS += -I$(KSRC)/drivers/media/usb/dvb-usb/
DVB_FLAGS += -I$(KSRC)/drivers/media/dvb-frontends/
endif

EXTRA_CFLAGS += $(DVB_FLAGS)

EXTRA_CFLAGS += -Wno-unused-value -Wno-unused-variable -Wno-unused-parameter \
		-Wno-switch

a867-objs := af903x-core.o af903x-devices.o af903x-drv.o af903x-fe.o af903x-tuner.o cmd.o standard.o \
			demodulator.o demodulatorextend.o usb2impl.o user.o mxl5007t.o Maxlinear_MXL5007.o Afa_AF9007.o
obj-m += a867.o


ifeq ($(MISSINGFILE),)
default:
	make -C $(KSRC) O=$(KOBJ) SUBDIRS=`pwd` modules
else
default:
	@echo Missing files that required to build driver: $(KSRC)/drivers/media/$(MISSINGFILE)
	@echo Try to get kernel source closest to $(CURRENT) from www.kernel.org and then copy to $(KSRC)/drivers/media 
endif

install:
	if [ -d "$(DESTDIR)/lib/modules/$(CURRENT)/kernel/drivers/media/dvb" ];then  \
             install -d $(DESTDIR)/lib/modules/$(CURRENT)/kernel/drivers/media/dvb/a867; \
             install -c -D -m 644 -c a867.ko $(DESTDIR)/lib/modules/$(CURRENT)/kernel/drivers/media/dvb/a867/;    \
        else \
               install -c -D -m 644 -c a867.ko $(DESTDIR)/lib/modules/$(CURRENT)/kernel/drivers/media/usb/dvb-usb/;   \
        fi 
	/sbin/depmod -a

remove:

#	echo -e "\nRemoving old $(KDIR26)/ files:"
#	@files='dvb-core.ko dvb-pll.ko dvb-usb.ko dvb-af903x.ko'; for i in $$files;do if [ -e $(KDIR26)/$$i ]; then echo -n "$$i "; rm $(KDIR26)/$$i; fi; done; for i in $$files;do if [ -e $(KDIR26)/$$i.gz ]; then echo -n "$$i.gz "; rm $(KDIR26)/$$i.gz; fi; done; echo;


clean::
	-rm -f *~ *.o *.ko .*.o.cmd .*.ko.cmd .*.o.d *.mod.c *.*~ Module.symvers

-include $(KOBJ)/Rules.make
