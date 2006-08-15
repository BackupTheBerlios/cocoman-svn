<?php
// For now it is sensitive to trailing slash so put it in.
$conf = parse_ini_file("../manager/conf/contest.ini");
$contest_root = $conf['root'];
$num_problems = $conf['num_problems'];
//$contest_root = '/home/acmweb/construction/contest';
//$num_problems = 3;
?>
