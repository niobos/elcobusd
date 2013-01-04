#!/bin/bash

ID="$1"
if [ -z "$ID" ]; then
	echo "Usage: $0 ID"
	exit 1
fi

# We have room enough; store full-res for 2 year; 10' average for 10y
rrdtool create "$ID-status.rrd" --no-overwrite --step 60 --start "-8 years" \
	"DS:status:GAUGE:120:0:100" \
	"RRA:LAST:0.5:1:$(( 60*24*365*2 ))" \
	"RRA:LAST:0.5:10:$(( 6*24*365*10 ))"
