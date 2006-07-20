<?php
require('../web/logging.php');
//ini_set("include_path", ".:../"); doesn't fix copy so I prepended ../ to $dir

// Processes the submission and fills in some variables
// Should not write anything out
function process_submission() {
	// Variable that will be used to display the page
	global $message;
	global $user_id;
	
	$dir = "../submissions/";
	$submission_time = date("H:i:s");
	
	if (!$users = file($dir . 'handles')) {
		$message = "Manager error: Opening users file failed.";
		return;
	}
	foreach ($users as $index => $user) {
		$users[$index] = strtok($user, ':');
	}
	
	if (!array_key_exists("id", $_POST)) {
		$message = 'You came here from a bad form.';
		return;
	}
	$user_id = $_POST["id"];
	
	if (!in_array($user_id, $users)) {
		$message = "You tried to submit a file using an invalid user id.";
		unset($GLOBALS['user_id']); // unset $user_id would only destroy it in this function's scope
		return;
	}
	
	$upload = $_FILES["program"];
	$allowable_types = array("text/x-c++src", "text/x-csrc", "text/x-java", "text/plain", "application/octet-stream");
	if ($upload["error"] != 0) { // TODO this doesn't seem to be working
		$message = sprintf("An error occured while uploading your file. Error: %d", $upload["error"]);
		return;
	} else if (!in_array($upload["type"], $allowable_types)) {
		$message = sprintf("Error uploading file. We don't accept %s files.", $upload["type"]);
		return;
	} else if ($upload["size"] == 0) {
		$message = "You tried to upload a blank file.";
		return;
	} else if ($upload["size"] > 100 * 1024) {
		$message = "You tried to upload a file that is too big.";
		return;
	}
	
	// Ok to upload file
	if ($_POST["lang"] == "auto") {
		if (strlen($upload['name']) < 5) {
			$message = "Your file name is not long enough for auto language detection.";
			return;
		} else {
			$extension = stristr($upload['name'], '.');
		}
	} else if ($_POST["lang"] == "java") {
		$extension = '.java';
	} else if ($_POST["lang"] == "c") {
		$extension = '.c';
	} else if ($_POST["lang"] == "cpp") {
		$extension = '.cpp';
	} else {
		$message = "Invalid language specified.";
		return;
	}
	
	$tempfile = $upload["tmp_name"];        
	$filename = $user_id . "-" . $submission_time . $extension;
	$filename_with_path = $dir . $_POST["progno"] . "/" . $filename;
	$result = copy($tempfile, $filename_with_path);
	if ($result === false) {
		$message = "ERROR: Could not copy submitted file to permanent location.";
		return;
	}
	
	$build_file = fopen($filename_with_path . ".make", "w");
	if ($build_file === false) {
		$message = "ERROR: Could not open build file.";
		return;
	}
	if ($extension == '.java') {
		fputs($build_file, sprintf("javac %s.java\n", $_POST["progno"]));
	}
	if ($extension == '.c') {
		fputs($build_file, sprintf("gcc-3.0 -o a.out %s\n", $filename));
	}
	if ($extension == '.cpp') {
		fputs($build_file, sprintf("g++ -o a.out %s\n", $filename));
	}
	
	if ($queue_file = fopen($dir . "/queue", "a")) {
		fputs($queue_file, sprintf("%s\n", $_POST["progno"] . "/" . $filename));
	}
	
	$message = "The program was successfully uploaded. Thanks!";
}

process_submission();

app_log(sprintf("User %d submitted a file. IP address: %s. Language: %s. Problem: %s. Result: %s", $user_id, get_ip(), $_POST['lang'], $_POST['progno'], $message));
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
  	if (isset($user_id)) {
		echo '<a href="scoreboard.php?id=' . $user_id . '">Scoreboard</a>';
	} else {
		echo '<a href="scoreboard.php">Scoreboard</a>';
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
