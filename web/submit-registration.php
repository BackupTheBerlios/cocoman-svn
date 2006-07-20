<?php
//ini_set("include_path", ".:../"); doesn't fix copy so I prepended ../ to $dir

//$user_id_given = false;
//$user_id = 0;
$message = "";

// Processes the submission and fills in some variables
// Should not write anything out
function process_submission() {
	// Variable that will be used to display the page
	global $message;
	//global $user_id_given;
	//global $user_id;
	
	//$dir = "../submissions/";
	$users_filename = '../submissions/handles';
	
	// Read in existing user file
	if (!$users = file($users_filename)) {
		$message = "Manager error: Opening users file failed.";
		return;
	}
	$ids = array();
	$names = array();
	foreach ($users as $index => $user_entry) {
		$ids[] = strtok($user_entry, ':');
		$names[] = strtok("\n");
		//echo "Tokenized entry as \"" . $ids[$index] . "\", \"" . $names[$index] . "\".";
	}
	
	// Make sure name is unique
	if (in_array($_GET['name'], $names)) {
		$message = "An account already exists for that name.";
		return;
	}
	
	// Find unique ID
	$possible_id = 1000;
	while (1) {
		if (!in_array($possible_id, $ids)) {
			$user_id = $possible_id;
			break;
		}
		++$possible_id;
	}
	
	// Write entry to log file
	$submission_time = date("H:i:s");
/*	if (!$info_file = fopen($filename_with_path . '-info.txt', 'wb')) {
		$message = "Error opening info file.";
		return;
	}
	fputs($info_file, sprintf("Submitted from %s\n", $REMOTE_ADDR));
	fclose($info_file);*/
	
	// Append new entry to user file
	if (!$users_file = fopen($users_filename, 'ab')) {
		$message = "Manager error: Opening users file for append failed.";
		return;
	}
	fputs($users_file, $user_id . ':' . $_GET['name'] . "\n");
	fclose($users_file);
	
	$message = "You have been successfully registered. Your user id is $user_id.<br />";
}

process_submission();
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
    <?php echo $message; ?>
  </p>
  <p>
    <?php
/*  	if ($user_id_given) {
		echo '<a href="scoreboard.php?id=' . $user_id . '">Scoreboard</a>';
	} else {
		echo '<a href="scoreboard.php">Scoreboard</a>';
	}*/
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
