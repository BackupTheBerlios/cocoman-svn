<?php
// Copyright 2006 Daniel Benamy <dbenamy1@binghamton.edu>
// License to be determined

require_once('config.inc');

$COMMAND = $contest_root . "manager/poll > " . $contest_root .  "submissions/poll.log &";
$output = array();
passthru($COMMAND);
foreach ($output as $line) {
    echo "$line\n";
}
    
?>
