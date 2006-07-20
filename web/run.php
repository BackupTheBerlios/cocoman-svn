<?php
// Copyright 2006 Daniel Benamy <dbenamy1@binghamton.edu>
// License to be determined
$COMMAND = "../manager/poll > ../submissions/poll.log &";
$output = array();
passthru($COMMAND);
foreach ($output as $line) {
    echo "$line\n";
}
    
?>
