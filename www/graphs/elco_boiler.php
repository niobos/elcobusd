<?php
require("../functions.php");

$time=validate_time($_GET['start'], $_GET['end']);
$server=validate_server($_GET['server']);

$timestamp = rrd_timestamp($time);

$command = <<<EOT
--imgformat PNG --start {$time['start']} --end {$time['end']} \
--width 600 --height 200 \
DEF:b_temp=/mnt/data/rrdtool/elco/00-BT.rrd:temperature:AVERAGE \
DEF:b_set_temp=/mnt/data/rrdtool/elco/00-BT.rrd:set_temperature:AVERAGE \
DEF:b_return_temp=/mnt/data/rrdtool/elco/00-BT.rrd:return_temperature:AVERAGE \
CDEF:empty=UNKN,b_temp,POP \
CDEF:b_delta_t=b_temp,b_return_temp,- \
\
"LINE1:b_return_temp#ffaa00" \
"AREA:b_delta_t#ffaa00::STACK" \
"LINE1:b_temp#ff0000" \
"LINE1:b_set_temp#0000ff" \
\
"LINE1:empty#ff0000:Temperature" \
"VDEF:b_temp_avg=b_temp,AVERAGE" \
"VDEF:b_temp_min=b_temp,MINIMUM" \
"VDEF:b_temp_max=b_temp,MAXIMUM" \
"VDEF:b_temp_last=b_temp,LAST" \
'GPRINT:b_temp_avg:avg=%4.1lf°C' \
'GPRINT:b_temp_min:min=%4.1lf°C' \
'GPRINT:b_temp_max:max=%4.1lf°C' \
'GPRINT:b_temp_last:last=%4.1lf°C' \
"COMMENT:\\n" \
\
"LINE1:empty#0000ff:Target temperature" \
"VDEF:b_set_temp_avg=b_set_temp,AVERAGE" \
"VDEF:b_set_temp_min=b_set_temp,MINIMUM" \
"VDEF:b_set_temp_max=b_set_temp,MAXIMUM" \
"VDEF:b_set_temp_last=b_set_temp,LAST" \
'GPRINT:b_set_temp_avg:avg=%4.1lf°C' \
'GPRINT:b_set_temp_min:min=%4.1lf°C' \
'GPRINT:b_set_temp_max:max=%4.1lf°C' \
'GPRINT:b_set_temp_last:last=%4.1lf°C' \
"COMMENT:\\n" \
\
"LINE1:empty#ffaa00:Return temperature" \
"VDEF:b_return_temp_avg=b_return_temp,AVERAGE" \
"VDEF:b_return_temp_min=b_return_temp,MINIMUM" \
"VDEF:b_return_temp_max=b_return_temp,MAXIMUM" \
"VDEF:b_return_temp_last=b_return_temp,LAST" \
'GPRINT:b_return_temp_avg:avg=%4.1lf°C' \
'GPRINT:b_return_temp_min:min=%4.1lf°C' \
'GPRINT:b_return_temp_max:max=%4.1lf°C' \
'GPRINT:b_return_temp_last:last=%4.1lf°C' \
"COMMENT:\\n" \
\
"LINE1:empty#ffaa00:delta T" \
"VDEF:b_delta_t_avg=b_delta_t,AVERAGE" \
"VDEF:b_delta_t_min=b_delta_t,MINIMUM" \
"VDEF:b_delta_t_max=b_delta_t,MAXIMUM" \
"VDEF:b_delta_t_last=b_delta_t,LAST" \
'GPRINT:b_delta_t_avg:avg=%4.1lf°C' \
'GPRINT:b_delta_t_min:min=%4.1lf°C' \
'GPRINT:b_delta_t_max:max=%4.1lf°C' \
'GPRINT:b_delta_t_last:last=%4.1lf°C' \
"COMMENT:\\n" \
\
{$timestamp} \
--vertical-label "°C"  \
--title "Boiler temperature"
EOT;

serve_png($command);
?>
