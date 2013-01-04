<?php
require("../functions.php");

$time=validate_time($_GET['start'], $_GET['end']);
$server=validate_server($_GET['server']);

$timestamp = rrd_timestamp($time);

$command = <<<EOT
--imgformat PNG --start {$time['start']} --end {$time['end']} \
--width 600 --height 200 \
DEF:t_temp=/mnt/data/rrdtool/elco/00-TWT.rrd:temperature:AVERAGE \
DEF:t_set_temp=/mnt/data/rrdtool/elco/00-TWT.rrd:set_temperature:AVERAGE \
"LINE1:t_temp#ff0000:Temperature" \
VDEF:t_temp_avg=t_temp,AVERAGE \
VDEF:t_temp_min=t_temp,MINIMUM \
VDEF:t_temp_max=t_temp,MAXIMUM \
VDEF:t_temp_last=t_temp,LAST \
"GPRINT:t_temp_avg:avg=%4.1lf°C" \
"GPRINT:t_temp_min:min=%4.1lf°C" \
"GPRINT:t_temp_max:max=%4.1lf°C" \
"GPRINT:t_temp_last:last=%4.1lf°C" \
"COMMENT:\\n" \
\
"LINE1:t_set_temp#0000ff:Target temperature" \
VDEF:t_set_temp_avg=t_set_temp,AVERAGE \
VDEF:t_set_temp_min=t_set_temp,MINIMUM \
VDEF:t_set_temp_max=t_set_temp,MAXIMUM \
VDEF:t_set_temp_last=t_set_temp,LAST \
"GPRINT:t_set_temp_avg:avg=%4.1lf°C" \
"GPRINT:t_set_temp_min:min=%4.1lf°C" \
"GPRINT:t_set_temp_max:max=%4.1lf°C" \
"GPRINT:t_set_temp_last:last=%4.1lf°C" \
"COMMENT:\\n" \
\
{$timestamp} \
--vertical-label "°C"  \
--title "Tap water temperature"
EOT;

serve_png($command);
?>
