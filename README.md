# Compilation on Ubuntu
In order to build the module correctly (tested on Ubuntu 16.04, kernel 4.4.0), you should have kernel source installed.
But since this is really a pain in the *ss and it's required only for building we will work around this.

You'll have to download the _linux-source_ package specific from your kernel from [security.ubuntu.com/ubuntu/pool/main/l/linux/](http://security.ubuntu.com/ubuntu/pool/main/l/linux/), extract it, navigate to its `usr/src/` folder, extract the archive and have it ready.
```
#!/bin/bash

# Get the filename
FILE=$(curl http://security.ubuntu.com/ubuntu/pool/main/l/linux/ 2> /dev/null | grep -Eo "linux\-source\-.*`uname -r | grep -Eo "[0-9\.\-]+" | sed "s/-$//"`" | grep -Eo '^[^"]+')

# Create a temp folder
FLD="/tmp/linux_source_`cat /dev/urandom | tr -dc 'a-zA-Z0-9' | fold -w 32 | head -n 1`"

# Download archive
wget -P $FLD "http://security.ubuntu.com/ubuntu/pool/main/l/linux/$FILE"

# Extract archive (to the Desktop for instance)
tar xjf $FLD/usr/src/linux-source-4.4.0.tar.bz2 -C ~/Desktop/

# Clean
rm -rf $FLD
```

At this point, building is easy. From your project git directory (see above for correct path)
```
export KSRC=~/Desktop/linux-source-4.4.0
make
```


Credits: https://askubuntu.com/a/510609
