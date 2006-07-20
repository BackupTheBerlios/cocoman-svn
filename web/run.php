<?php
// Copyright 2006 Daniel Benamy <dbenamy1@binghamton.edu>
// License to be determined
$COMMAND = "../manager/poll";
$output = array();
exec($COMMAND, $output);
foreach ($output as $line) {
    echo "$line\n";
}
    
?>
