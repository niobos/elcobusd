#!/bin/bash

: ${TOOLPATH:=$(cd $(dirname "$0"); echo $PWD)}
BINPATH="/usr/local/bin"

ADDR="$1"
if [ -z "$ADDR" ]; then
	echo "Usage: $0 address"
	exit 1
fi
FILENAME="$ADDR-BT.rrd"

if [ ! -e "$FILENAME" ]; then
	echo "File \`$FILENAME\` does not exist"
	exit 1
fi
if [ ! -w "$FILENAME" ]; then
	echo "File \`$FILENAME\` not writable"
	exit 1
fi

DATA="$(
	perl -e '$|=1;
		sleep 1;
		print "\xdc\x81\x00\x0b\x06\x3d\x0d\x05\x19\x99\x23";
		sleep 1;
		print "\xdc\x81\x00\x0b\x06\x3d\x0d\x09\x23\x4b\x57";
		sleep 1;
		print "\xdc\x81\x00\x0b\x06\x3d\x11\x05\x1a\x9f\x42";
		sleep 2;
		' |
	socat - TCP6:[::1]:8446 | $BINPATH/elcobus-log-bus.pl | $BINPATH/elcobus-parse-log.pl
)"


TEMP="$(   echo "$DATA" | grep " : BoilerTemp:" | tail -n1 | sed 's/.*Temp=\([0-9.]*\)C.*/\1/' )"
TTEMP="$(  echo "$DATA" | grep " : BoilerSetTemp:" | tail -n1 | sed 's/.*Temp=\([0-9.]*\)C.*/\1/' )"
RTEMP="$(  echo "$DATA" | grep " : ReturnTemp:" | tail -n1 | sed 's/.*Temp=\([0-9.]*\)C.*/\1/' )"

rrdtool update "$FILENAME" --template \
	"temperature:set_temperature:return_temperature" \
	"N:${TEMP:-U}:${TTEMP:-U}:${RTEMP:-U}"
