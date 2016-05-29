# Compilation on Ubuntu
In order to build the module correctly (tested on Ubuntu 16.04, kernel 4.4.0), you should have kernel source installed.
But since this is really a pain in the *ss and it's required only for building we will work around this.

You'll have to download the _linux-source_ package specific for your kernel from [security.ubuntu.com/ubuntu/pool/main/l/linux/](http://security.ubuntu.com/ubuntu/pool/main/l/linux/), extract it, navigate to its `usr/src/` folder, extract the archive and have it ready.
```bash
#!/bin/bash

# Get the filename
FILE=$(curl http://security.ubuntu.com/ubuntu/pool/main/l/linux/ 2> /dev/null | grep -Eo "linux\-source\-.*`uname -r | grep -Eo "[0-9\.\-]+" | sed "s/-$//"`" | grep -Eo '^[^"]+')

# Create a temp folder
FLD="/tmp/linux_source_`cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 32 | head -n 1`"

# Download archive
wget -P $FLD "http://security.ubuntu.com/ubuntu/pool/main/l/linux/$FILE"

# Unpack Debian archive
dpkg -x "$FLD/$FILE" $FLD

# Extract archive (to the Desktop for instance)
tar xjf $FLD/usr/src/linux-source-4.4.0.tar.bz2 -C ~/Desktop/

# Clean
rm -rf $FLD
```

At this point, building is easy. From your project git directory (see above for correct path)
```bash
export KSRC=~/Desktop/linux-source-4.4.0
make
```

To install it and make it work correctly one should first disable `dvb-usb-af9035` kernel module (possible conflicts, didn't tested) by editing the `/etc/modprobe.d/blacklist.conf` adding, at the end, `blacklist dvb-usb-af9035`, then run `sudo make install`.
Note that, at this point, there's still no uninstall support and you _might_ need to recompile the driver every time the kernel is updated (again, didn't tested). If you want to uninstall the module, just look at the Makefile target _install_. Theorically, you'll have to remove the `a867.ko` kernel object from `/lib/modules/$(uname -r)/kernel/drivers/media/usb/dvb-usb` and, maybe, run a `/sbin/depmod -a`.


Credits: https://askubuntu.com/a/510609
