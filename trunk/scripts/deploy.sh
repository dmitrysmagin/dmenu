#!/bin/bash
if [ -z "$1" ]; then
   echo "You must specify the mount point for the script to run.  ex: deploy.sh /media/dingoo"
   exit 1
fi

MNT=$( echo $1 | sed -e 's/\/$//g')
DMN="$MNT/local/dmenu"
FILES="dmenu dmenu.bin dmenu.ini"
NEED_MOUNT=1

if mount | grep "on $MNT type" > /dev/null; then 
    NEED_MOUNT=0
fi

if [ $NEED_MOUNT -gt 0 ]; then 
   if ! mount $MNT; then
     exit 1 
   fi
fi

if ./scripts/build.sh; then
   for x in $FILES; do 
       cp $x $DMN
   done
fi

if [ $NEED_MOUNT -gt 0 ]; then 
    sleep 3
    umount $MNT
fi
