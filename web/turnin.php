<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
   "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html>
<head>
  <title>ACM Coding Contest Submission</title>
  <link rel="stylesheet" type="text/css" href="style.css" />
</head>

<body>
<?php

// phpinfo();

$dir = "../submissions/";
$submission_time = date("H:i:s");

$users = file($dir . 'handles');
foreach ($users as $index => $user) {
    $users[$index] = strtok($user, ':');
}
$user_id = $_POST["id"];

if (!in_array($user_id, $users)) {
    echo "You tried to submit a file using an invalid user id.";
} else {
    $upload = $_FILES["program"];
    $allowable_types = array("text/x-c++src", "text/x-csrc", "text/x-java", "text/plain", "application/octet-stream");
    if ($upload["error"] != 0) { // TODO this doesn't seem to be working
        printf("An error occured while uploading your file. Error: %d", $upload["error"]);
    } else if (!in_array($upload["type"], $allowable_types)) {
        printf("Error uploading file. We don't accept %s files.", $upload["type"]);
    } else if ($upload["size"] == 0) {
        echo "You tried to upload a blank file.";
    } else if ($upload["size"] > 100 * 1024) {
        echo "You tried to upload a file that is too big.";
    } else { // Ok to upload file
        if ($_POST["lang"] == "auto") {
            if (strlen($upload['name']) < 5) {
                echo "Your file name is not long enough for auto language detection.";
                exit;
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
            echo "Invalid language specified.";
            exit;
        }
        
        $tempfile = $upload["tmp_name"];        
        $filename = $user_id . "-" . $submission_time . $extension;
        $filename_with_path = $dir . $_POST["progno"] . "/" . $filename;
        copy($tempfile, $filename_with_path);
        
        if ($build_file = fopen($filename_with_path . ".make", "w")) {
            if ($extension == '.java') {
                fputs($build_file, sprintf("javac %s.java\n", $_POST["progno"]));
            }
            if ($extension == '.c') {
                fputs($build_file, sprintf("gcc-3.0 -o a.out %s\n", $filename));
            }
            if ($extension == '.cpp') {
                fputs($build_file, sprintf("g++ -o a.out %s\n", $filename));
            }
        }
        
        if (!$info_file = fopen($filename_with_path . '-info.txt', 'wb')) {
        	echo "Error opening info file.";
        	exit;
        }
        fputs($info_file, sprintf("Submitted from %s\n", $REMOTE_ADDR));
        fclose($info_file);
        
        if ($queue_file = fopen($dir . "/queue", "a")) {
            fputs($queue_file, sprintf("%s\n", $_POST["progno"] . "/" . $filename));
        }
        
        echo "The program was successfully uploaded. Thanks!<br />";
        if (array_key_exists("id", $_POST)) {
            echo '<a href="scores.php?id=' . $_POST["id"] . '">Scoreboard</a>';
        } else {
            echo '<a href="scores.php">Scoreboard</a>';
        }
    }
}
?>

  <hr />
  <p>
    <a href="http://validator.w3.org/check?uri=referer"><img
        src="http://www.w3.org/Icons/valid-xhtml10"
        alt="Valid XHTML 1.0 Transitional" height="31" width="88" /></a>
  </p>

</body>
</html>
