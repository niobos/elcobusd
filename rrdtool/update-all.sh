#!/bin/bash

: ${TOOLPATH:=$(cd $(dirname "$0"); echo $PWD)}

cd ${RRDDIR:-/var/lib/rrd/} || exit 1

for f in *-BT.rrd; do
	ADDR="${f%%-BT.rrd}"
	echo "Updating BT $ADDR"
	"$TOOLPATH/update-BT.sh" "$ADDR"
done
