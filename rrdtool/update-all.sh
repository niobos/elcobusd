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
