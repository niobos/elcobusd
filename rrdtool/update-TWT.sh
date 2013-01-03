#!/bin/bash

: ${TOOLPATH:=$(cd $(dirname "$0"); echo $PWD)}
BINPATH="/usr/local/bin"

ADDR="$1"
if [ -z "$ADDR" ]; then
	echo "Usage: $0 address"
	exit 1
fi
FILENAME="$ADDR-TWT.rrd"

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
		print "\xdc\x81\x00\x0b\x06\x3d\x31\x05\x2f\x7f\x72";
		sleep 1;
		print "\xdc\x81\x00\x0b\x06\x3d\x31\x07\x4b\x35\x32";
		sleep 2;
		' |
	socat - TCP6:[::1]:8446 | $BINPATH/elcobus-log-bus.pl | $BINPATH/elcobus-parse-log.pl
)"


TEMP="$(   echo "$DATA" | grep " : TapTemp:" | tail -n1 | sed 's/.*Temp=\([0-9.]*\)C.*/\1/' )"
TTEMP="$(  echo "$DATA" | grep " : TapSetTemp:" | tail -n1 | sed 's/.*Temp=\([0-9.]*\)C.*/\1/' )"

rrdtool update "$FILENAME" --template \
	"temperature:set_temperature" \
	"N:${TEMP:-U}:${TTEMP:-U}"
