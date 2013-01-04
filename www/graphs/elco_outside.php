<?php
require("../functions.php");

$time=validate_time($_GET['start'], $_GET['end']);
$server=validate_server($_GET['server']);

$timestamp = rrd_timestamp($time);

$command = <<<EOT
--imgformat PNG --start {$time['start']} --end {$time['end']} \
--width 600 --height 200 \
DEF:temp=/mnt/data/rrdtool/elco/00-OT.rrd:temperature:AVERAGE \
"LINE1:temp#ff0000:Temperature" \
VDEF:temp_avg=temp,AVERAGE \
VDEF:temp_min=temp,MINIMUM \
VDEF:temp_max=temp,MAXIMUM \
VDEF:temp_last=temp,LAST \
"GPRINT:temp_avg:avg=%4.1lf°C" \
"GPRINT:temp_min:min=%4.1lf°C" \
"GPRINT:temp_max:max=%4.1lf°C" \
"GPRINT:temp_last:last=%4.1lf°C" \
"COMMENT:\\n" \
\
{$timestamp} \
--vertical-label "°C"  \
--title "Outside temperature"
EOT;

serve_png($command);
?>
