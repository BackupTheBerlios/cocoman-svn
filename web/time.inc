<?php

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

?>
