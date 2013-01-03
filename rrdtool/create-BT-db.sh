#!/bin/bash

ID="$1"
if [ -z "$ID" ]; then
	echo "Usage: $0 ID"
	exit 1
fi

# We have room enough; store full-res for 2 year; 10' average for 10y
rrdtool create "$ID-BT.rrd" --no-overwrite --step 60 --start "-8 years" \
	"DS:temperature:GAUGE:120:0:100" \
	"DS:set_temperature:GAUGE:120:0:100" \
	"DS:return_temperature:GAUGE:120:0:100" \
	"RRA:AVERAGE:0.5:1:$(( 60*24*365*2 ))" \
	"RRA:AVERAGE:0.5:10:$(( 6*24*365*10 ))"
