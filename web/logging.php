<?php
/*
Author: Daniel Benamy <dbenamy1@binghamton.edu>
License: To be determined
*/

// Get client's ip address
function get_ip() {
	// TODO make this more robust. see user comments on http://php3.de/getenv
	return($_SERVER['REMOTE_ADDR']);
}

// Write entry to log file
// If log can't be opened, echos an error and exits
function app_log($message) {
	$log_filename = '../logs/manager.log'; // this isn't what is called the log
	// in a few other place. The other log just has the results of users' 
	// submissions. This refers to a general log where various events that are 
	// recorded.
	if (!$log = fopen($log_filename, 'ab')) {
		echo "Manager error: Error opening info file.";
		exit;
	}
	$time = date("H:i:s");
	fputs($log, sprintf("%s: %s\n", $time, $message));
	fclose($log);
}

?>
