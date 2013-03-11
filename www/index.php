<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN"
	"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd">
<?php
require_once('functions.php');

if ($_GET['refresh'] != "")
	$refresh=(int)$_GET['refresh'];
?>
<html>
<head>
 <title>Heating statistics</title>
<?php
if ($refresh != 0)
	echo "<meta http-equiv=\"Refresh\" content=\"${refresh}\"/>"
?>
</head>

<body>
<h1>Heating statistics</h1>
<p><?php
	echo '<form name="time" method="POST" action="' .
		"post2get.php?" . change_get(array(), array( 'url' => $_SERVER['PHP_SELF'], 'get' => serialize($_GET) )) .
		'">' . "\n";
	echo '<input type="submit" value="Show" />' . "\n";
	echo presets("time", array(	'Last hour'	=> array('start' => '-1 hour',	'end' => 'now'),
					'Last day'	=> array('start' => '-1 day',	'end' => 'now'),
					'Last 2 days'	=> array('start' => '-2 days',	'end' => 'now'),
					'Last week'	=> array('start' => '-1 week',	'end' => 'now'),
					'Last month'	=> array('start' => '-1 month',	'end' => 'now'),
					'Last year'	=> array('start' => '-1 year',	'end' => 'now'),
					'today'		=> array('start' => 'today 00:00', 'end' => 'tomorrow 00:00'),
					'yesterday'	=> array('start' => 'yesterday 00:00', 'end' => 'today 00:00'),
					'this week'	=> array('start' => '-6 days monday', 'end' => '+1 day monday'),
					'previous week'	=> array('start' => '-13 days monday', 'end' => '-6 days monday'),
					'this month'	=> array('start' => '00:00 '.date('Y-m').'-01', 'end' => '00:00 '.date('Y-m').'-01 +1 month'),
					'previous month'=> array('start' => '00:00 '.date('Y-m').'-01 -1 month', 'end' => '00:00 '.date('Y-m').'-01'),
					'this year'	=> array('start' => '00:00 '.date('Y').'-01-01', 'end' => '00:00 '.date('Y').'-01-01 +1 year'),
					'previous year' => array('start' => '00:00 '.date('Y').'-01-01 -1 year', 'end' => '00:00 '.date('Y').'-01-01'),
				  ),
			array( 'start' => $_GET['start'], 'end' => $_GET['end'] ),
			array( 'input_size' => 20,
				'label_input' => TRUE,
				'auto_submit' => TRUE,
			     )
			);
	echo '<font size="-1"><a href="http://www.gnu.org/software/tar/manual/html_node/tar_109.html">? format ?</a></font>';
	echo '</form>' . "\n";
?></p>
<p><?php
	echo '<form name="refresh" method="POST" action="' .
		"post2get.php?" . change_get(array(), array( 'url' => $_SERVER['PHP_SELF'], 'get' => serialize($_GET) )) .
		'">' . "\n";
	echo '<input type="submit" value="Refresh" />' . "\n";
	echo presets("refresh", array(	'Off'		=> array('refresh' => null),
					'1 min'		=> array('refresh' => '60'),
					'5 mins'	=> array('refresh' => '300'),
					'30 mins'	=> array('refresh' => '1800'),
					'1 hour'	=> array('refresh' => '3600'),
					'2 hours'	=> array('refresh' => '7200'),
					'1 day'		=> array('refresh' => '86400'),
				  ),
			array( 'refresh' => $_GET['refresh'] ),
			array( 'input_size' => 5,
				'auto_submit' => TRUE,
			     )
			);
	echo "secs";
	echo '</form>' . "\n";
?></p>

 <p>
  <img alt="graph" src="graphs/elco_burner.php?<?php echo change_get($_GET, array()); ?>" /><br/>
  <img alt="graph" src="graphs/elco_pump.php?<?php echo change_get($_GET, array()); ?>" /><br/>
  <img alt="graph" src="graphs/elco_boiler.php?<?php echo change_get($_GET, array()); ?>" /><br/>
  <img alt="graph" src="graphs/velbus.temp.php?id=04&name=living&<?php echo change_get($_GET, array()); ?>" /><br/>
  <img alt="graph" src="graphs/velbus.temp.php?id=0f&name=bathroom&<?php echo change_get($_GET, array()); ?>" /><br/>
  <img alt="graph" src="graphs/velbus.temp.php?id=10&name=office&<?php echo change_get($_GET, array()); ?>" /><br/>
  <img alt="graph" src="graphs/velbus.temp.php?id=08&name=attic&<?php echo change_get($_GET, array()); ?>" /><br/>
  <img alt="graph" src="graphs/elco_tap.php?<?php echo change_get($_GET, array()); ?>" /><br/>
  <img alt="graph" src="graphs/elco_outside.php?<?php echo change_get($_GET, array()); ?>" /><br/>
  <img alt="graph" src="graphs/elco_pressure.php?<?php echo change_get($_GET, array()); ?>" /><br/>
 </p>
</body>
</html>										     
