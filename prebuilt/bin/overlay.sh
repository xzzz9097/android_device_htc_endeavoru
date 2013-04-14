#!/system/xbin/sh
set -x
#
# overlay script - by AlienMind
# 
# Intended to be run at flash time, this script
# will overlay some folders from sdcard to system and/or data
# partitions
#
# This way ROM users can customize their flashes to add
# custom apps, animations or whatever from the very beginning
#
# This is extracted from icetool overlay option,
# available on https://github.com/alienmind-org/BlackICE/blob/master/system/bin/icetool
#

# Device dependent paths
# OVERLAY_DIR is the directory (when in RECOVERY) where the files will be copied from

#SYSTEM_DEV=`mount | awk '($2 == "/system") { print $1 }'`
#DATA_DEV=`mount | awk '($2 == "/data") { print $1 }'`
#OVERLAY_DIR=/sdcard/
. /system/etc/overlay_device.conf

###
#
# Overlay - Copy custom content from sdcard
Overlay() {
  local DST="$1"
  local SRC="$2"

  if [ "$DST" = "/system" ]; then
    DEV=$SYSTEM_DEV
  elif [ "$DST" = "/data" ]; then
    DEV=$DATA_DEV
  else
    die "overlay: Invalid destination $DST"
  fi

  if [ ! -d "$SRC" ]; then
    die "overlay: Invalid source $SRC"
  fi

  # We try both methods for mounting
  mount $DEV $DST &>/dev/null
  mount -o remount,rw $DEV $DST

  # Copy
  cp -av $SRC/* $DST/

  #
  #umount $DST
}


# Fix possible permission issues with overlayed files
# These are only common fixes, some other custom fix may be added
FixPerms() {
  chmod 644 /system/app/* /data/app/*
  chmod 755 /system/etc/init.d/*
}

########

# Test if some folders are ready to be copied over
[ -d "$OVERLAY_DIR/system" ] && \
  Overlay /system "$OVERLAY_DIR/system"
  
[ -d "$OVERLAY_DIR/data" ] && \
  Overlay /data "$OVERLAY_DIR/data"

# Add possible .tar.gz overlays
# It is done now that we have /system and /data mounted
for i in $OVERLAY_DIR/*.tar.gz $OVERLAY_DIR/*.tgz ; do
   tar -C / -xzvf $i
done

# Custom permission fix
if [ -f "$OVERLAY_DIR/fixperms.sh" ]; then
  sh "$OVERLAY_DIR"/fixperms.sh 
else
  # Fix common known permission problems
  FixPerms
fi

exit 0
