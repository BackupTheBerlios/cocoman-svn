<?php

//require('logging.inc');

$NUM_PROBLEMS=5;
$ROOT_DIR="logs/";
$users_filename = '../manager/users.txt';

// Admin functions 
// starts the poll on the machine
function start_poll() {
    $COMMAND = "../manager/manager> ../logs/poll.log &";
    $output = array();
    exec($COMMAND, $output);
}

// stops the poller.
function stop_poll() {
    $COMMAND = "../manager/manager stop";
    $output = array();
    exec($COMMAND, $output);
}

// start the contest by setting start time and 
// duration time
// $duration is in seconds.
// function start_contest($duration) {
//     $start_time= date("H:i:s");
//     $times_file = fopen($ROOT_DIR."/times.txt", 'w')
//     fputs($times_file, sprintf("%s\n%s\n", $start_time, seconds_to_time($duration));
// }

if (array_key_exists('poll', $_GET)) {
	if ($_GET['poll'] == "Start poller") {
		start_poll();
	} else if ($_GET['poll'] == "Stop poller") {
		stop_poll();
	}
}
?>

<form method="get" action="adminboard.php">
  <input type="submit" name="poll" value="Start poller" />
  <input type="submit" name="poll" value="Stop poller" />
</form>
