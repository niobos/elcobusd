<?php

// validate functions
function validate_time($start, $end) {
	if( $start === null ) $time['start'] = strtotime("-1 day");
	else $time['start'] = strtotime($start);
	
	if( $end === null ) $time['end'] = time(); // now
	else $time['end'] = strtotime($end);
	
	return $time; 
}

function validate_refresh($time) {
	if( $time == null ) return "";
	return "<meta http-equiv=\"refresh\" content=\"" . (int)$time . "\">\n";
}

function validate_server($server) {
	switch ($server) {
	case "alcatraz":
		return "alcatraz";
	case "genie":
	default:
		return "genie";
	}
}

function rrd_timestamp($time) {
	return "'COMMENT: \\n' " .
		"'COMMENT:Showing from " . date("D Y-m-d H\\\:i\\\:s O", $time['start']) . "\\n' " .
		"'COMMENT:          to " . date("D Y-m-d H\\\:i\\\:s O", $time['end']) . "\\n' " .
		"'COMMENT:  Created at " . date("D Y-m-d H\\\:i\\\:s O") . "' ";
}

function serve_png($command) {
	header("Content-Type: image/png"); // tell the browser that PNG data is comming up

	passthru("rrdtool graph - " . $command);
}

// tools
function change_get($orig, $new) {
	# change the original get-request
	# with the name-value-pairs supplied in $new
	if($new !== null ) foreach($new as $name => $value) {
		if( $value === null ) unset($orig[$name]);
		else $orig[$name] = $value;
	}
	if($orig !== null) foreach($orig as $name => $value) {
		$orig[$name] = urlencode($name) . '=' . urlencode($value);
	}
	return implode('&', $orig);
}

function select($name, $values, $current_value, $onChange = "") {
	$output = "";
	
	$output .= "<select name=\"{$name}\"";
	if( $onChange != "" ) $output .= " onChange=\"{$onChange}\"";
	$output .= ">\n";

	foreach($values as $value => $text) {
		$output .= "<option value=\"{$value}\"" .
				( $value == $current_value ? " selected=\"selected\"" : "" ) .
				">{$text}</option>\n";
	}
	$output .= "</select>\n";

	return $output;
}

function presets($form, $presets, $current, $extra_opts = array()) {
	# extra_opts:
	#   label_input		show labels before the input boxes
	#   input_size		sets the size for the input fields
	#   between_select_and_input
	#			printed between the select box and the first input-field
	#   between_inputs	printed between 2 input-fields
	#   auto_submit		true to auto-submit
	$output = "";
	
	$inputbox = array();
	$select_option = array();
	
	# prepend "custom" to be the first item
	# we can't use array_merge because of it's side-effect:
	# it will "renumber" the keys, thus reducing bandwidths to 0,1,2 bps...
	$preset = array( "Custom" => array() );
	foreach($presets as $key => $values) {
		$preset[$key] = $values;
		foreach( $values as $name => $value) {
			$inputbox[$name] = "";		# remember we've seen this input
		}
	}

	# Custom should clear all input boxes
	$preset['Custom'] = $inputbox;

	# fabricate a name for the combo_box (use $form BEFORE transform!!)
	$preset_name = "preset_" . $form . "_" . implode("_", array_keys($inputbox));

	# translate form
	if( is_int($form) ) $form = "forms[$form]";
	
	# read all presets, remember inputs to make
	# and make the JavaScript to fill in the input-boxes when the drop-down changes
	$output .= "<script language=\"JavaScript1.2\"><!--\n";
	$output .= "  var {$preset_name} = new Array();\n";
	foreach($preset as $text => $values) {
		$select_option[$text] = $text;		# make list for select box
		$output .= "    {$preset_name}['{$text}'] = new Array();\n";	# make new array-element in java-script
		$this_is_current = TRUE;		# find out if this element is the current one
		foreach($values as $name => $value) {
			if( $this_is_current && $current[$name] != $value ) $this_is_current = FALSE;	# there is a difference
			$output .= "    {$preset_name}['{$text}']['{$name}'] = '{$value}';\n";	# fill data into javascript
		}
		if( $this_is_current ) $current_value = $text;	# all name-value pairs match on this preset, mark it as current
	}
	
	# the javascript function to do the fill-in
	$output .=	"  function fill_in_{$preset_name}_inputboxes() {\n" .
			"    for( name in {$preset_name}[ document.{$form}.{$preset_name}.value ] ) {\n" .
			"      document.{$form}.elements[name].value = {$preset_name}[ document.{$form}.{$preset_name}.value][name];\n" .
			"    }\n" .
			"  }\n" .
			"--></script>\n";

	# print the select box
	$output .= select("{$preset_name}", $select_option, $current_value,
			"fill_in_{$preset_name}_inputboxes();" .
			( $extra_opts['auto_submit'] ? "document.{$form}.submit();" : ""));

	$output .= $extra_opts['between_select_and_input'];
	foreach($inputbox as $name => $dummy) {
		$output .=	( $extra_opts['label_input'] ? "{$name}:" : "" ) .
				"<input type=\"text\" name=\"{$name}\" value=\"{$current[$name]}\"" .
				( $extra_opts['input_size'] != 0 ? " size=\"{$extra_opts['input_size']}\"" : "" ) .
				" onChange=\"javascript:document.{$form}.{$preset_name}.selectedIndex=0;\" " .
				"/>\n" .
				$extra_opts['between_inputs'];
	}

	return $output;
}

?>
