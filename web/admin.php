<?php
// Copyright 2006 Daniel Benamy <dbenamy1@binghamton.edu>
// License to be determined

require('logging.php');

$NUM_PROBLEMS=5;
$ROOT_DIR="logs/";
$users_filename = '../submissions/handles';

// Admin functions 
// starts the poll on the machine
function start_poll() {
    $COMMAND = "../manager/poll > ../submissions/poll.log &";
    $output = array();
    exec($COMMAND, $output);
}

// stops the poller.
function stop_poll() {
    $COMMAND = "../manager/poll stop";
    $output = array();
    exec($COMMAND, $output);
}

// start the contest by setting start time and 
// duration time
// $duration is in seconds.
function start_contest($duration) {
    $start_time= date("H:i:s");
    $times_file = fopen($ROOT_DIR."/times.txt", 'w')
    fputs($times_file, sprintf("%s\n%s\n", $start_time, seconds_to_time($duration));
    
    
}
// More like a struct. Member variables get updated by Person
class Problem {
    var $number; // set by contructor. not used yet.
    var $submissions;
    var $time; // time taken to complete since start of competition. 
               // 24 hour format hhmmss. "-" if not completed.
    
    function Problem($number) {
        $this->number = $number;
        $this->time = "-";
        $this->submissions = 0;
    }
}

// hardcoded to have 4 problems. could be made more generic later if needed.
class Person {
    var $user_id;
    var $name;
    var $problems;
    var $last_status_code; // -1 if no submissions
    var $last_compile_log_filename;
    
    function Person($user_id) {
        global $NUM_PROBLEMS; 
        $this->user_id = $user_id;
        $this->problems = array();
        // HOW DO WE FIX THIS?
        for ($i = 1; $i <= $NUM_PROBLEMS; ++$i) {
            $this->problems["$i"] = new Problem($i);
        }
//	$this->problems = array("1" => new Problem(1), new Problem(2), 
//	                        new Problem(3), new Problem(4));
        $this->last_status_code = -1;
        $this->last_compile_log_filename = "";
    }
    
    // Called each time a line is read from the log file about this person
    // problem is an int from 1 to 4
    // time is a string in the log file format of hhmmss (24 hour time)
    // succeeded is a bool
    function submitted_problem($problem, $time, $status_code) {
        //echo "Called submitted_problem($problem, $time, $status_code)<br />";
        if ($this->problems[$problem]->time == "-") { // no working submission yet for this problem
            ++$this->problems[$problem]->submissions;
            if ($status_code == "0") {
                $this->problems[$problem]->time = $time;
            }
            $this->last_status_code = $status_code;
        }
        // if there was already a working submission we ignore this submission
    }
    
    function total_problems_solved() {
        $total_problems = 0;
        foreach ($this->problems as $problem) {
            if ($problem->time != '-') {
                ++$total_problems;
            }
        }
        return $total_problems;
    }
    
    function total_seconds() {
        $total_time = 0; // in seconds
        foreach ($this->problems as $problem) {
            if ($problem->time != "-") {
                $total_time = $total_time + time_to_seconds($problem->time);
            }
        }
        return $total_time;
    }
}


$times = file($ROOT_DIR.'/times.txt', 1); // TODO error checking
$start_time = strtotime($times[0]);
$contest_length_in_seconds = time_to_seconds($times[1]);

// takes an elapsed time in the format hh:mm:ss (date format His) and converts 
// it to seconds
function time_to_seconds($time) {
    $hours = strtok($time, ':');
    $minutes = strtok(':');
    $seconds = strtok(':');
    return $hours * 3600 + $minutes * 60 + $seconds;
}

// takes an elapsed time in seconds and returns it in the format hh:mm:ss (date 
// format His)
function seconds_to_time($seconds) {
    $minutes = floor($seconds / 60);
    $seconds = $seconds % 60;
    if ($seconds < 10) {
        $seconds = "0".$seconds;
    }
    $hours = floor($minutes / 60);
    if ($hours < 10) {
        $hours = "0".$hours;
    }
    $minutes = $minutes % 60;
    if ($minutes < 10) {
        $minutes = "0".$minutes;
    }
    $time = "$hours:$minutes:$seconds";
//    $time = $hours."h".$minutes."m".$seconds."s";
    return $time;
}

// Returns the time between the start time read from the file and the time passed 
// in. It will be in the format of 'H:i:s'. Gets passed to strtotime elsewhere so
// don't change the return format without making sure users are cool.
function time_from_start($time) {
    global $start_time;
    $time = strtotime($time);
    $time = $time - $start_time; // submission time in seconds since start of contest
    return seconds_to_time($time);
}

// ***Program flow starts here (sorta)***

// Read all registered users in from the users file
if (!$users = file($users_filename)) {
        echo "Manager error: Opening users file failed.";
        exit;
}
$people = array();
foreach ($users as $user_entry) {
        $user_id = strtok($user_entry, ':');
        $name = strtok("\n");
    if (array_key_exists($user_id, $people)) {
        app_log(sprintf("ERROR: Users file contains 2 entries with user id %d", $user_id));
        continue;
    }
        $people[$user_id] = new Person($user_id);
        $people[$user_id]->name = $name;
}

// Fill in their submission data from the submission results log file
// Syntax of log file:
// <hhmmss>,<user id>,<first name last name>,Problem<problem number>,<status code>,[text]
// status codes:
// "0" success
// "1" compile failure
// "2" test failure
// "3" timeout while running
// "4" crash
// "15" unknown error while executing program
// if status code is 1, there is another token before text which has the 
// filename of a log of the compile

$log = file($ROOT_DIR.'/log', 1);
foreach ($log as $line) {        
//echo "tokenized log entry as:<br />"; echo "<pre>"; print_r($entry); echo "</pre>";

    $time = strtok($line, ",");
    $time = time_from_start($time);
    $user_id = strtok(",");
    $name = strtok(",");
    $problem_number = substr(strtok(","), 7);
    $status_code = strtok(",");
    
    //echo "tokenized line: time=$time, name=$name, problem number=$problem_number, code=$status_code<br />";
    
        if (!array_key_exists($user_id, $people)) {
        app_log(sprintf("ERROR: Submission results log refers to user id %d which is not in the users file.", $user_id));
        continue;
    }
    
    $people[$user_id]->submitted_problem($problem_number, $time, $status_code);
    
    if ($status_code == 1) {
        $people[$user_id]->last_compile_log_filename = strtok(",");
    }
}

$problems_solved = array();
$total_times = array(); // in seconds
$ranked_user_ids = array();
foreach ($people as $person) {
    $problems_solved[] = 0 - $person->total_problems_solved();
    $total_times[] = $person->total_seconds();
    $ranked_user_ids[] = $person->user_id;
}

array_multisort($problems_solved, $total_times, $ranked_user_ids);
//echo "<pre>"; print_r($problems_solved); print_r($total_times); print_r($user_ids); echo "</pre><br />";
?>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN"
   "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<!-- <html> -->
<!-- <head> -->
<!--   <meta http-equiv="refresh" content="10" /> -->
<!--   <title>ACM Coding Contest Scoreboard</title> -->
<!--   <link rel="stylesheet" type="text/css" href="style.css" /> -->
<!--   <style type="text/css"> -->
<!--     th {text-align: center} -->
<!--     td {text-align: center} -->
<!--   </style> -->
<!-- </head> -->

<!-- <body> -->
  <h1>ACM Coding Contest Scoreboard</h1>
  
  <p>
    Time left in contest: 
    <?php
    $end_time = $start_time + $contest_length_in_seconds; // unix time format (seconds since epoch)
    $current_time = time(); // unix time format
    $seconds_left = $end_time - $current_time;
    if ($seconds_left > 0) {
        echo seconds_to_time($seconds_left);
    } else {
        echo "The contest is over. Thanks for participating!.";
        // TODO lock submissions?
    }
    ?>
  </p>
  
  <table id="scoreboard" cellspacing="0" border="1">
    <tr align="center">
    <thead>
      <th rowspan="2">Name</th>
      <?php
          for ($i = 1; $i <= $NUM_PROBLEMS; $i++) {
              printf("<th colspan=\"2\">Problem %s</th>\n", $i);
          }
      ?>
      <th colspan="2">Total</th>
    </tr>
    <tr>
    <?php
        for ($i = 1; $i <= $NUM_PROBLEMS; $i++) {
            echo "<td>Time</td>";
            echo "<td>Attempts</td>";
        }
    ?>
    <td>Total Time</td>
    <td>Solved</td>
    </tr>
    </thead>
    <tbody>
<?php
//foreach ($people as $user_id => $person) {
$row= false;
foreach ($ranked_user_ids as $ranked_user_id) {
    $person = $people[$ranked_user_id];
    // change the color of alternating rows.
    if ($row == true) {
        echo "    <tr class=\"even\">";
        $row = false;
    } else {
        echo "    <tr class=\"odd\">";
        $row = true;
    }
    printf("      <td>%s</td>", $person->name);
    for ($i = 1; $i <= $NUM_PROBLEMS; ++$i) {
        printf("      <td align=\"center\" id=\"timecol\">%s</td><td align=\"right\" id=\"numcol\">%s</td>", 
        $person->problems[$i]->time, $person->problems[$i]->submissions );
    }
    printf("      <td>%s</td>", seconds_to_time($person->total_seconds()));
    printf("      <td>%s</td>", $person->total_problems_solved());
    echo "    </tr>";
}
?>
  </tbody>
  </table>

<?php
if (array_key_exists("id", $_GET)) {
    if (!array_key_exists($_GET["id"], $people)) {
        echo "<p>The user you specified hasn't submitted anything.</p>";
    } else {
        $person = $people[$_GET["id"]];
        switch ($person->last_status_code) {
            case 0:
                $last_status = "Submission ok!";
                break;
            case 1:
                $last_status = "Compile failure.";
                break;
            case 2:
                $last_status = "Program failed test.";
                break;
            case 3:
                $last_status = "Program timed out while running.";
                break;
            case 4:
                $last_status = "Program crashed while running.";
                break;
            case 5:
                $last_status = "Unknown error while running program.";
                break;
            default:
                $last_status = "Unknown error. This should never happen. Please inform the proctor.";
        }
        echo "<hr/>";
        echo "<p>";
        echo "  Please note that it may take a couple of minutes for your ";
        echo "  submissions to be processed.";
        echo "</p>";
        //echo "<p>";
        printf("  <p>Your most recent submission's status is: %s</p>", $last_status);
        if ($person->last_status_code == "1") {
            echo "  <div>";
            echo "    Compile log:<br />";
            //echo "reading log from" . $person->last_compile_log_filename . "<br />";
            $compile_log = file($person->last_compile_log_filename, 1);
            foreach ($compile_log as $line) {
                printf("    <pre>%s</pre>", htmlentities($line));
            }
            echo "  </div>";
        }
    }
}
?>

  <hr />
  <div>
    <form method="get" action="scores.php">
      <u>View person's submission status</u><br />
      <label>User ID:</label><input name="id" type="text" />
      <input type="submit" value="Submit" />
    </form>
  </div>
<?php
//echo "<hr /><pre>"; print_r($people); echo "</pre>";
?>

<!--   <hr /> -->
<!--   <p> -->
<!--     <a href="http://validator.w3.org/check?uri=referer"><img -->
<!--         src="http://www.w3.org/Icons/valid-xhtml10" -->
<!--         alt="Valid XHTML 1.0 Transitional" height="31" width="88" /></a> -->
<!--   </p> -->
<!-- </body> -->
<!-- </html> -->
