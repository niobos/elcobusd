<?php
require("../functions.php");

$time=validate_time($_GET['start'], $_GET['end']);
$server=validate_server($_GET['server']);

$timestamp = rrd_timestamp($time);

$command = <<<EOT
--imgformat PNG --start {$time['start']} --end {$time['end']} \
--width 600 --height 200 \
--lower-limit 0 --upper-limit 100 --rigid \
DEF:b=/mnt/data/rrdtool/elco/00-burner.rrd:modulation:AVERAGE \
DEF:s=/mnt/data/rrdtool/elco/00-status.rrd:status:LAST \
CDEF:b_rad=s,10,EQ,b,UNKN,IF \
CDEF:b_tap=s,11,EQ,b,UNKN,IF \
CDEF:b_other=s,10,NE,s,11,NE,+,2,NE,UNKN,b,IF \
\
"AREA:b_rad#ff0000:Active on radiators" \
CDEF:b_rad_active=b_rad,UN,0,b_rad,IF,0,GT,100,* \
VDEF:b_rad_duty=b_rad_active,AVERAGE \
VDEF:b_rad_avg=b_rad,AVERAGE \
VDEF:b_rad_min=b_rad,MINIMUM \
VDEF:b_rad_max=b_rad,MAXIMUM \
VDEF:b_rad_last=b_rad,LAST \
"GPRINT:b_rad_duty:duty=%4.1lf%%" \
"GPRINT:b_rad_avg:avg=%4.1lf%%" \
"GPRINT:b_rad_min:min=%4.1lf%%" \
"GPRINT:b_rad_max:max=%4.1lf%%" \
"GPRINT:b_rad_last:last=%4.1lf%%" \
"COMMENT:\\n" \
\
"AREA:b_tap#0000ff:Active on tap water" \
CDEF:b_tap_active=b_tap,UN,0,b_tap,IF,0,GT,100,* \
VDEF:b_tap_duty=b_tap_active,AVERAGE \
VDEF:b_tap_avg=b_tap,AVERAGE \
VDEF:b_tap_min=b_tap,MINIMUM \
VDEF:b_tap_max=b_tap,MAXIMUM \
VDEF:b_tap_last=b_tap,LAST \
"GPRINT:b_tap_duty:duty=%4.1lf%%" \
"GPRINT:b_tap_avg:avg=%4.1lf%%" \
"GPRINT:b_tap_min:min=%4.1lf%%" \
"GPRINT:b_tap_max:max=%4.1lf%%" \
"GPRINT:b_tap_last:last=%4.1lf%%" \
"COMMENT:\\n" \
\
"AREA:b_other#ff00ff:Active on other    " \
CDEF:b_other_active=b_other,UN,0,b_other,IF,0,GT,100,* \
VDEF:b_other_duty=b_other_active,AVERAGE \
VDEF:b_other_avg=b_other,AVERAGE \
VDEF:b_other_min=b_other,MINIMUM \
VDEF:b_other_max=b_other,MAXIMUM \
VDEF:b_other_last=b_other,LAST \
"GPRINT:b_other_duty:duty=%4.1lf%%" \
"GPRINT:b_other_avg:avg=%4.1lf%%" \
"GPRINT:b_other_min:min=%4.1lf%%" \
"GPRINT:b_other_max:max=%4.1lf%%" \
"GPRINT:b_other_last:last=%4.1lf%%" \
"COMMENT:\\n" \
\
{$timestamp} \
--vertical-label "%"  \
--title "Burner"
EOT;

serve_png($command);
?>
