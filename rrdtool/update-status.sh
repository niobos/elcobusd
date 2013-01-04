#!/bin/bash

: ${TOOLPATH:=$(cd $(dirname "$0"); echo $PWD)}
BINPATH="/usr/local/bin"

ADDR="$1"
if [ -z "$ADDR" ]; then
	echo "Usage: $0 address"
	exit 1
fi
FILENAME="$ADDR-status.rrd"

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
		print "\xdc\x81\x00\x0b\x06\x3d\x09\x30\x34\x4a\x4c";
		sleep 2;
		' |
	socat - TCP6:[::1]:8446 | $BINPATH/elcobus-log-bus.pl | $BINPATH/elcobus-parse-log.pl
)"


STATUS="$(   echo "$DATA" | grep " : Status:" | tail -n1 | sed 's/.*Status=\([0-9]*\).*/\1/' )"

# cap status measurment to the beginning of each minute in order to avoid averaging by RRD
TS="$( date +%s )"
TS=$(( $TS - ($TS % 60) ))

rrdtool update "$FILENAME" --template \
	"status" \
	"$TS:${STATUS:-U}"
