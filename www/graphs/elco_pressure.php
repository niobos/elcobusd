<?php
require("../functions.php");

$time=validate_time($_GET['start'], $_GET['end']);
$server=validate_server($_GET['server']);

$timestamp = rrd_timestamp($time);

$command = <<<EOT
--imgformat PNG --start {$time['start']} --end {$time['end']} \
--width 600 --height 200 \
-l 0 \
DEF:pressure=/mnt/data/rrdtool/elco/00-pressure.rrd:pressure:AVERAGE \
"LINE1:pressure#ff0000:Pressure" \
VDEF:pressure_avg=pressure,AVERAGE \
VDEF:pressure_min=pressure,MINIMUM \
VDEF:pressure_max=pressure,MAXIMUM \
VDEF:pressure_last=pressure,LAST \
"GPRINT:pressure_avg:avg=%4.1lfbar" \
"GPRINT:pressure_min:min=%4.1lfbar" \
"GPRINT:pressure_max:max=%4.1lfbar" \
"GPRINT:pressure_last:last=%4.1lfbar" \
"COMMENT:\\n" \
\
{$timestamp} \
--vertical-label "bar"  \
--title "Water pressure"
EOT;

serve_png($command);
?>
