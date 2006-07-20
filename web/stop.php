<?php
// Copyright 2006 Daniel Benamy <dbenamy1@binghamton.edu>
// License to be determined

require_once('config.inc');

$COMMAND = $contest_root . "manager/poll stop";
$output = array();
exec($COMMAND, $output);
?>
