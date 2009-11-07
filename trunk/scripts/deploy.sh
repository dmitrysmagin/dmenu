#!/bin/bash
if [ -z "$1" ]; then
   echo "You must specify the mount point for the script to run.  ex: deploy.sh /media/dingoo"
   exit 1
fi
MNT="$1"
DMN="$MNT/local/dmenu"
FILES="dmenu dmenu.bin dmenu.ini"
mount $MNT && ./scripts/build.sh && for x in $FILES; do cp $x $DMN; done
sleep 3
umount $MNT
