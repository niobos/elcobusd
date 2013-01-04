#!/bin/bash

: ${TOOLPATH:=$(cd $(dirname "$0"); echo $PWD)}

cd ${RRDDIR:-/var/lib/rrd/} || exit 1

for f in *-BT.rrd; do
	ADDR="${f%%-BT.rrd}"
	echo "Updating BT $ADDR"
	"$TOOLPATH/update-BT.sh" "$ADDR"
done

for f in *-TWT.rrd; do
	ADDR="${f%%-TWT.rrd}"
	echo "Updating TWT $ADDR"
	"$TOOLPATH/update-TWT.sh" "$ADDR"
done

for f in *-OT.rrd; do
	ADDR="${f%%-OT.rrd}"
	echo "Updating OT $ADDR"
	"$TOOLPATH/update-OT.sh" "$ADDR"
done

for f in *-burner.rrd; do
	ADDR="${f%%-burner.rrd}"
	echo "Updating burner $ADDR"
	"$TOOLPATH/update-burner.sh" "$ADDR"
done

for f in *-status.rrd; do
	ADDR="${f%%-status.rrd}"
	echo "Updating status $ADDR"
	"$TOOLPATH/update-status.sh" "$ADDR"
done
