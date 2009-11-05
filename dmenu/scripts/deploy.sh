#!/bin/bash
MNT="$1"
DMN="$MNT/local/dmenu"
./scripts/build.sh && mount $MNT && cp dmenu $DMN && cp dmenu.bin $DMN && sleep 3 && umount $MNT
