<?php

require_once('functions.php');

// this page is used to POST changes to an existing GET.
// the GET is serialize()d in the $_GET['get'], the url is in $_GET['url']
// ALL vars found in $_POST will be (re)placed in GET and passed to the URL

// undo automatic slashes
$get = get_magic_quotes_gpc() == 0 ? $_GET['get'] : stripslashes($_GET['get']);

header("Location: http://" . $_SERVER['HTTP_HOST'] .
	$_GET['url'] .
	"?" . change_get(unserialize($get), $_POST));

?>
