<?php
require("../functions.php");

$time = validate_time($_GET['start'], $_GET['end']);
if( ! preg_match( '/^[0-9a-zA-Z][0-9a-zA-Z]$/', $_GET['id'] ) ) {
	die("Unrecognized ID");
}
$id = $_GET['id'];
if( ! preg_match( '/^[a-zA-Z0-9 _-]*$/', $_GET['name'] ) ) {
	die("name has invalid chars");
}
$name = $_GET['name'];

$timestamp = rrd_timestamp($time);

$command = <<<EOT
--imgformat PNG --start {$time['start']} --end {$time['end']} \
--width 600 --height 200 \
--title "Temperature for $name" \
--alt-autoscale --vertical-label "°C" \
'DEF:temp=/mnt/data/rrdtool/$id-TS.rrd:temperature:AVERAGE' \
'DEF:tt=/mnt/data/rrdtool/$id-TS.rrd:set_temperature:AVERAGE' \
'DEF:heater=/mnt/data/rrdtool/$id-TS.rrd:heater:AVERAGE' \
TICK:heater#ff000030:1 \
LINE1:temp#ff0000:'Actual temperature' \
VDEF:temp_avg=temp,AVERAGE \
VDEF:temp_min=temp,MINIMUM \
VDEF:temp_max=temp,MAXIMUM \
'GPRINT:temp_avg:avg=%4.1lf°C' \
'GPRINT:temp_min:min=%4.1lf°C' \
'GPRINT:temp_max:max=%4.1lf°C' \
"COMMENT:\\n" \
LINE1:tt#0000ff:'Target temperature' \
VDEF:tt_avg=tt,AVERAGE \
'GPRINT:tt_avg:avg=%4.1lf°C' \
"COMMENT:\\n" \
CDEF:empty=UNKN,temp,POP \
LINE1:empty#ffa00060:'Heat requested here ' \
CDEF:heater_100=heater,100,* \
VDEF:duty=heater_100,AVERAGE \
'GPRINT:duty:duty=%4.2lf%%' \
"COMMENT:\\n" \
{$timestamp}
EOT;

serve_png($command);
?>

