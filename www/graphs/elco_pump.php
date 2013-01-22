<?php
require("../functions.php");

$time=validate_time($_GET['start'], $_GET['end']);
$server=validate_server($_GET['server']);

$timestamp = rrd_timestamp($time);

$command = <<<EOT
--imgformat PNG --start {$time['start']} --end {$time['end']} \
--width 600 --height 200 \
--lower-limit 0 --upper-limit 100 --rigid \
DEF:p=/mnt/data/rrdtool/elco/00-pump.rrd:modulation:AVERAGE \
"AREA:p#ff0000:Pump" \
CDEF:p_active=p,UN,0,p,IF,0,GT,100,* \
VDEF:p_duty=p_active,AVERAGE \
VDEF:p_avg=p,AVERAGE \
VDEF:p_min=p,MINIMUM \
VDEF:p_max=p,MAXIMUM \
VDEF:p_last=p,LAST \
"GPRINT:p_duty:duty=%4.1lf%%" \
"GPRINT:p_avg:avg=%4.1lf%%" \
"GPRINT:p_min:min=%4.1lf%%" \
"GPRINT:p_max:max=%4.1lf%%" \
"GPRINT:p_last:last=%4.1lf%%" \
"COMMENT:\\n" \
\
{$timestamp} \
--vertical-label "%"  \
--title "Pump"
EOT;

serve_png($command);
?>
