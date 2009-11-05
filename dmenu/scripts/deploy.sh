#!/bin/bash
if [ -z "$1" ]; then
   echo "You must specify the mount point for the script to run.  ex: deploy.sh /media/dingoo"
   exit 1
fi
MNT="$1"
DMN="$MNT/local/dmenu"
./scripts/build.sh && mount $MNT && cp dmenu $DMN && cp dmenu.bin $DMN && sleep 3 && umount $MNT
