<?php
/*
Author: Daniel Benamy <dbenamy1@binghamton.edu>
License: To be determined
*/

require_once('config.inc');
require('../web/logging.inc');

// Processes the submission and fills in some variables
// Should not write anything out
function process_submission() {
	// Variable that will be used to display the page
	global $message;
	global $user_id;
	
	$users_filename = $contest_root . 'manager/conf/users.txt';
	
	// TODO check to make sure _GET has name
	
	// Make sure requested name is sane
	$allowable_characters = array_merge(range('a', 'z'), range('A', 'Z'));
	$allowable_characters = array_merge($allowable_characters, range(0, 9));
	$allowable_characters = array_merge($allowable_characters, array(' '));
/*	echo "<pre>"; print_r(str_split($_GET['name'])); echo "</pre>";
	echo "<pre>"; print_r($allowable_characters); echo "</pre>";*/
	foreach(str_split($_GET['name']) as $character) {
		if (!in_array($character, $allowable_characters, true)) {
			$message = "You requested a name that contains invalid characters. Please choose a different name.";
			return;
		}
	}
	
	// Read in existing user file
	$users = file($users_filename);
	if ($users === false) {
		$message = "ERROR: Opening users file failed.";
		return;
	}
	$ids = array();
	$names = array();
	foreach ($users as $index => $user_entry) {
		$ids[] = strtok($user_entry, ':');
		$names[] = strtok("\n");
	}
	
	// Make sure name is unique
	if (in_array($_GET['name'], $names)) {
		$message = "An account already exists for that name.";
		return;
	}
	
	// Find unique ID
//	$possible_id = 1000;
//	while (1) {
//		if (!in_array($possible_id, $ids)) {
//			$user_id = $possible_id;
//			break;
//		}
//		++$possible_id;
//	}

        $output = array();
        $genid = $contest_root . "manager/genid $users_filename";
        $ret = exec($genid, $output);

        $user_id = $output[0];
		
	// Append new entry to user file
	$users_file = fopen($users_filename, 'ab');
	if ($users_file === false) {
		$message = "ERROR: Opening users file for append failed.";
		return;
	}
	fputs($users_file, $user_id . ':' . $_GET['name'] . "\n");
	fclose($users_file);
	
	$message = "You have been successfully registered. Your user id is $user_id.";
        $success = 1;
}

process_submission();

app_log(get_ip() . ' attemped to register with name "' . $_GET['name'] . '". Result: ' . $message);
?>


<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
   "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html>
<head>
  <title>ACM Coding Contest Submission</title>
  <link rel="stylesheet" type="text/css" href="style.css" />
</head>

<body>
  <p>
    <?php 

    if (isset($user_id)) {
      echo "<h3> You have been successfully registered. </h3>";
      echo "<h1> Your User ID is: $user_id </h1>";
      echo "<h2> Write this number down now! You may need it later to login again. </h2>";
    } else {
      echo $message; 
    }

    ?>
  </p>
  <p>
    <?php
  	if (isset($user_id)) {
                echo "After you have written down your User ID, ";
		echo '<a href="scoreboard.php?id=' . $user_id . '">';
                echo "click here to continue</a>";
	}
	?>
  </p>
  <hr />
  
  <p>
    <a href="http://validator.w3.org/check?uri=referer"><img
        src="http://www.w3.org/Icons/valid-xhtml10"
        alt="Valid XHTML 1.0 Transitional" height="31" width="88" /></a>
  </p>

</body>
</html>
