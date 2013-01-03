#!/bin/bash

: ${TOOLPATH:=$(cd $(dirname "$0"); echo $PWD)}
BINPATH="/usr/local/bin"

ADDR="$1"
if [ -z "$ADDR" ]; then
	echo "Usage: $0 address"
	exit 1
fi
FILENAME="$ADDR-burner.rrd"

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
		print "\xdc\x81\x00\x0b\x06\x3d\x11\x30\x5f\x7d\x43";
		sleep 2;
		' |
	socat - TCP6:[::1]:8446 | $BINPATH/elcobus-log-bus.pl | $BINPATH/elcobus-parse-log.pl
)"


MOD="$(   echo "$DATA" | grep " : Modulation:" | tail -n1 | sed 's/.*Modulation=\([0-9.]*\)%.*/\1/' )"

rrdtool update "$FILENAME" --template \
	"modulation" \
	"N:${MOD:-U}"
