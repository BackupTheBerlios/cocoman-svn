<?php
$NUM_PROBLEMS=5;
$ROOT_DIR="../logs/";
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
        $this->user_id = $user_id;
        $this->problems = array();
        for ($i = 0; $i < $NUM_PROBLEMS; $i++) {
            $this->problems[] = new Problem($i);
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


$start_time = file($ROOT_DIR.'/start');
$start_time = strtotime($start_time[0]);

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
    $hours = floor($minutes / 60);
    $minutes = $minutes % 60;
    $time = "$hours:$minutes:$seconds";
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

$people = array();

$log = file($ROOT_DIR.'/log');
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
        $people[$user_id] = new Person($user_id);
        $people[$user_id]->name = $name;
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

<html>
<head>
  <meta http-equiv="refresh" content="10" />
  <title>ACM Coding Contest Scoreboard</title>
  <link rel="stylesheet" type="text/css" href="style.css" />
  <style type="text/css">
    th {text-align: center}
    td {text-align: center}
  </style>
</head>

<body>
  <h1>ACM Coding Contest Scoreboard</h1>
  <table border="1">
    <tr>
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
            echo "<td>Submissions</td>";
        }
    ?>
    <td>Time to Solve</td>
    <td>Solved</td>
    </tr>
<?php
//foreach ($people as $user_id => $person) {
foreach ($ranked_user_ids as $ranked_user_id) {
    $person = $people[$ranked_user_id];
    echo "    <tr>";
    printf("      <td>%s</td>", $person->name);
    for ($i = 1; $i <= 4; ++$i) {
        printf("      <td>%s</td><td>%s</td>", 
            $person->problems[$i]->time, $person->problems[$i]->submissions);
    }
    printf("      <td>%s</td>", seconds_to_time($person->total_seconds()));
    printf("      <td>%s</td>", $person->total_problems_solved());
    echo "    </tr>";
}
?>
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
            $compile_log = file($person->last_compile_log_filename);
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
  <hr />
  
  <div>
    <form method="post" enctype="multipart/form-data" action="turnin.php">
    <u>Submit new file</u><br />
<?php
if (array_key_exists("id", $_GET) && array_key_exists($_GET["id"], $people)) {
    $user_id = $_GET["id"];
    printf("User ID: <input type=\"text\" name=\"id\" value=\"%s\" /><br />", $user_id);
} else {
    echo "User ID: <input type=\"text\" name=\"id\" /><br />";
}
?>
    <label>Language:</label>
    <select name="lang">
      <option value="auto">Auto</option>
      <option value="c">C</option>
      <option value="cpp">C++</option>
      <option value="java">Java</option>
    </select><br />
    <label>Problem Number:</label>
    <select name="progno">
    <?php
        for ($i = 1; $i <= $NUM_PROBLEMS; $i++) {
            printf("<option value=\"Problem%s\">Problem %s</option>\n", $i, $i);
        }
    ?>
    </select><br />
    <label>Source file:</label><input type="file" name="program" /><br />
    <input type="hidden" name="MAX_FILE_SIZE" value="100 * 1024" />
    <input type="submit" value="Submit" />
    </form>
  </div>

<?php
//echo "<hr /><pre>"; print_r($people); echo "</pre>";
?>

  <hr />
  <p>
    <a href="http://validator.w3.org/check?uri=referer"><img
        src="http://www.w3.org/Icons/valid-xhtml10"
        alt="Valid XHTML 1.0 Transitional" height="31" width="88" /></a>
  </p>
</body>
</html>
